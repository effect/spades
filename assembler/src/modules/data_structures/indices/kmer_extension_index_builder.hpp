//***************************************************************************
//* Copyright (c) 2016 Saint Petersburg State University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#pragma once

#include "kmer_extension_index.hpp"
#include "kmer_splitters.hpp"

template<class Builder>
class DeBruijnExtensionIndexBuilder : public Builder {
    typedef Builder base;
public:
    typedef typename Builder::IndexT IndexT;

    template<class ReadStream>
    size_t FillExtensionsFromStream(ReadStream &stream, IndexT &index) const {
        unsigned k = index.k();
        size_t rl = 0;

        while (!stream.eof()) {
            typename ReadStream::read_type r;
            stream >> r;
            rl = std::max(rl, r.size());

            const Sequence &seq = r.sequence();
            if (seq.size() < k + 1)
                continue;

            typename IndexT::KeyWithHash kwh = index.ConstructKWH(seq.start<runtime_k::RtSeq>(k));
            for (size_t j = k; j < seq.size(); ++j) {
                char nnucl = seq[j], pnucl = kwh[0];
                index.AddOutgoing(kwh, nnucl);
                kwh <<= nnucl;
                index.AddIncoming(kwh, pnucl);
            }
        }

        return rl;
    }

    void FillExtensionsFromIndex(const std::string &KPlusOneMersFilename,
                                 IndexT &index) const {
        unsigned KPlusOne = index.k() + 1;

        typename IndexT::kmer_iterator it(
                KPlusOneMersFilename, runtime_k::RtSeq::GetDataSize(KPlusOne));
        for (; it.good(); ++it) {
            runtime_k::RtSeq kpomer(KPlusOne, *it);

            char pnucl = kpomer[0], nnucl = kpomer[KPlusOne - 1];
            TRACE("processing k+1-mer " << kpomer);
            index.AddOutgoing(index.ConstructKWH(runtime_k::RtSeq(KPlusOne - 1, kpomer)),
                              nnucl);
            // FIXME: This is extremely ugly. Needs to add start / end methods to extract first / last N symbols...
            index.AddIncoming(index.ConstructKWH(runtime_k::RtSeq(KPlusOne - 1, kpomer << 0)),
                              pnucl);
        }
    }

public:
    template<class Streams>
    ReadStatistics BuildExtensionIndexFromStream(IndexT &index, Streams &streams, io::SingleStream* contigs_stream = 0,
                                                 size_t read_buffer_size = 0) const {
        unsigned nthreads = (unsigned) streams.size();

        // First, build a k+1-mer index
        DeBruijnReadKMerSplitter<typename Streams::ReadT, StoringTypeFilter<typename IndexT::storing_type>> splitter(
                index.workdir(), index.k() + 1, 0xDEADBEEF, streams,
                contigs_stream, read_buffer_size);
        KMerDiskCounter<runtime_k::RtSeq> counter(index.workdir(), splitter);
        counter.CountAll(nthreads, nthreads, /* merge */false);

        // Now, count unique k-mers from k+1-mers
        DeBruijnKMerKMerSplitter<StoringTypeFilter<typename IndexT::storing_type> > splitter2(index.workdir(), index.k(),
                                           index.k() + 1, IndexT::storing_type::IsInvertable(), read_buffer_size);
        for (unsigned i = 0; i < nthreads; ++i)
            splitter2.AddKMers(counter.GetMergedKMersFname(i));
        KMerDiskCounter<runtime_k::RtSeq> counter2(index.workdir(), splitter2);

        DeBruijnKMerIndexBuilder<IndexT>().BuildIndex(index, counter2, 16, nthreads);

        // Build the kmer extensions
        INFO("Building k-mer extensions from k+1-mers");
#       pragma omp parallel for num_threads(nthreads)
        for (unsigned i = 0; i < nthreads; ++i)
            FillExtensionsFromIndex(counter.GetMergedKMersFname(i), index);
        INFO("Building k-mer extensions from k+1-mers finished.");

        return splitter.stats();
    }

private:
    DECL_LOGGER("DeBruijnExtensionIndexBuilder");
};

template<class Index>
struct ExtensionIndexHelper {
    typedef Index IndexT;
    typedef typename IndexT::traits_t traits_t;
    typedef typename IndexT::KMer Kmer;
    typedef typename IndexT::KMerIdx KMerIdx;
    typedef DeBruijnStreamKMerIndexBuilder<IndexT> DeBruijnStreamKMerIndexBuilderT;
    typedef DeBruijnExtensionIndexBuilder<DeBruijnStreamKMerIndexBuilderT> DeBruijnExtensionIndexBuilderT;
};

