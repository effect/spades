#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include "abruijngraph.hpp"
#include "hash.hpp"
#include "graphBuilder.hpp"
#include "parameters.hpp"
#include <ext/hash_map>
#include "logging.hpp"

LOGGER("a.graphBuilder");

using namespace std;
using namespace __gnu_cxx;
using namespace abruijn;

//typedef hash_map< Sequence, int, HashSym<Sequence>, EqSym<Sequence> > SeqCount;
//SeqCount seqCount;
set<hash_t> goodHashes;

abruijn::Graph graph;

LoggerPtr logger(Logger::getLogger("a.graphBuilder"));
HashSym<Sequence> hashSym;

static hash_t H;

void initH() {
	H = 1;
	for (int i = 0; i < K; i++) {
		H = HASH_X(H);
	}
}

/**
 * Hashes of k-mers of s after countHashes(s) is executed
 */
hash_t ha[MPSIZE - K + 1];
/**
 * Auxiliary array, ignore it
 */
hash_t hb[MPSIZE - K + 1];

/**
 * Counts polynomial hashes of all k-mers in s, and puts it to array ha
 */
void countHashes(const Sequence& s) {
	size_t sz = s.size();
	hash_t h = 0;
	for (int i = 0; i < K; i++) {
		h = HASH_X(h) + s[i];
	}
	ha[0] = h;
	for (size_t i = 0; i + K < sz; i++) {
		ha[i + 1] = HASH_X(ha[i]) + s[i + K] - s[i] * H;
	}

	h = 0;
	for (size_t i = sz - 1; i + K >= sz; i--) {
		h = HASH_X(h) + (s[i] ^ 3);
	}
	hb[sz - K] = h;
	for (int i = sz - K; i > 0; i--) {
		hb[i - 1] = HASH_X(hb[i]) + (s[i - 1] ^ 3) - (s[i + K - 1] ^ 3) * H;
		ha[i] ^= hb[i] ^ HASH_XOR;
	}
	ha[0] ^= hb[0] ^ HASH_XOR;
}

/**
 * Calculates two (distinct) minimal hash-values of all k-mers in this read,
 * and puts them into goodHashes
 */
void processReadA(Seq<MPSIZE> r) {
	countHashes(Sequence(r));
	hash_t h1 = maxHash;
//	int pos1 = -1;
	hash_t h2 = maxHash;
//	int pos2 = -1;
	for (int i = 0; i + K <= MPSIZE; i++) {
		if (ha[i] < h1) {
			h2 = h1;
//			pos2 = pos1;
			h1 = ha[i];
//			pos1 = i;
		} else if (ha[i] < h2) {
			h2 = ha[i];
//			pos2 = i;
		}
	}
	assert(h2 != maxHash);
	goodHashes.insert(h1);
	goodHashes.insert(h2);
}

void selectGood() {
//    for (SeqCount::iterator p = seqCount.begin(); p != seqCount.end(); ++p) {
//    	graph.createVertex(&(p->finputStreamt));
//	}
}

void processReadB(Seq<MPSIZE> r) {
	vector<abruijn::Vertex*> vs;
	vector<int> index;
	Sequence s(r);
	countHashes(s);
	for (int i = 0; i + K <= MPSIZE; i++) {
		if (goodHashes.find(ha[i]) != goodHashes.end()) {
			//			Sequence* ss = new Sequence(s.Subseq(i, i + K));
			vs.push_back(graph.getVertex(s.Subseq(i, i + K)));
			index.push_back(i);
		}
	}
	for (size_t i = 0; i + 1 < vs.size(); i++) {
		graph.addEdge(vs[i], vs[i + 1], s.Subseq(index[i], index[i + 1] + K));
	}
}


void condenseA() {
	int condensations = 0;
	for (abruijn::Graph::iterator v = graph.begin(); v != graph.end(); ++v) {
		if (graph.tryCondenseA(*v)) {
			condensations++;
		}
	}
	graph.cleanup();
}

void GraphBuilder::build() {
	initH();
	std::string file_names[2] = {INPUT_FILES};
	ireadstream<MPSIZE, 2> inputStream(file_names);

	INFO("Processing-A...");
	mate_read<MPSIZE>::type mp;
	for (int i = 0; !inputStream.eof() && i < CUT; i++) {
		inputStream >> mp;
		processReadA(mp[0]);
		processReadA(mp[1]);
		VERBOSE(i, " reads read");
	}
	inputStream.reset();
	INFO("processReadA done: " << goodHashes.size() << " vertex-painputStream");

	INFO("Selecting good kmers...");
	selectGood();

	INFO("Processing-B...");
	for (int i = 0; !inputStream.eof() && i < CUT; i++) {
		inputStream >> mp;
		processReadB(mp[0]);
		processReadB(mp[1]);
		VERBOSE(i, " reads processed");
	}
	inputStream.close();

	INFO("Condensing-A graph...");
	condenseA();

	INFO("Outputting graph to " << OUTPUT_FILE);
	ofstream outputStream((OUTPUT_FILES + ".dot").c_str(), ios::out);
	graph.output(outputStream);
	outputStream.close();
}
