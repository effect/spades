#include "cute/cute.h"
#include "cute/ide_listener.h"
#include "cute/cute_runner.h"
#include "seq_test.hpp"
#include "sequence_test.hpp"
#include "quality_test.hpp"
#include "nucl_test.hpp"
#include "ireadstream_test.hpp"
#include "online_graph_visualizer_test.hpp"
#include "similar_test.hpp"
#include "cuckoo_test.hpp"
#include "single_read_test.hpp"
#include "paired_read_test.hpp"
#include "parser_test.hpp"
#include "fastqgz_parser_test.hpp"
// TODO(mariyafomkina): Add tests for other parsers here.
#include "reader_singleread_test.hpp"
#include "reader_pairedread_test.hpp"
#include "multifile_reader_test.hpp"
#include "cutting_reader_wrapper_test.hpp"
#include "rc_reader_wrapper_test.hpp"
#include "converting_reader_wrapper_test.hpp"

using namespace std;

void runSuite() {
  cute::suite s;
  s += SeqSuite();
  s += SequenceSuite();
  s += QualitySuite();
  s += NuclSuite();
  s += IReadStreamSuite();
  s += onlineGraphVisualizerSuite();
  s += similarSuite();
  s += CuckooSuite();
  s += SingleReadSuite();
  s += PairedReadSuite();
  s += ParserSuite();
  s += FastqgzParserSuite();
  // TODO(mariyafomkina): Add tests for other parsers here.
  s += ReaderSingleReadSuite();
  s += ReaderPairedReadSuite();
  s += MultifileReaderSuite();
  s += CuttingReaderWrapperSuite();
  s += RCReaderWrapperSuite();
  s += ConvertingReaderWrapperSuite();
  cute::ide_listener lis;
  cute::makeRunner(lis)(s, "The Suite");
}

int main() {
  runSuite();
  return 0;
}
