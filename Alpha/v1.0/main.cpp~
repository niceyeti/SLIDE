//#include "Header.hpp"
#include "SingularityBuilder.hpp"
#include "LatticeBuilder.hpp"
#include "Viterbi.hpp"
#include "LanguageModel.hpp"

//KeyMap CLASS_keyMap;

int main(void)
{
  long double res, scalar = 1000000000.0;
  struct timespec begin, end;

  vector<Point> inData;
  vector<PointMu> outData;
  Lattice testLattice;
  LatticePaths decoded;

  string testDataFile = "./signal.txt";

  //class components
  SingularityBuilder sb;
  LatticeBuilder lb;
  Viterbi vt;
  LanguageModel lm;

  //lattice and viterbi test
  cout << "Vini Vitti Viterbi..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  lb.TestBuildLattice(testLattice);
  vt.Process(testLattice, decoded);
  //lb.ClearLattice(testLattice);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << diffTimeSpecs(&begin,&end) << " (s)" << endl;

  cout << "decoded: ";
  if(decoded.size() > 0){
    cout << decoded.begin()->first << endl;
  }

  //full path-enumeration testing
  testLattice.clear();
  decoded.clear();
  cout << "Running exhaustive dfs graph search..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  lb.TestBuildLattice(testLattice);
  vt.RunExhaustiveSearch(testLattice,decoded,-1);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << diffTimeSpecs(&begin,&end) << " (s)" << endl;
  vt.PrintResultList(decoded);


  //try a basic pruned/beam search
  testLattice.clear();
  decoded.clear();
  //full path-enumeration testing
  cout << "Running pruned dfs graph search..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  lb.TestBuildLattice(testLattice);
  vt.RunPrunedSearch(testLattice,decoded,-1);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << diffTimeSpecs(&begin,&end) << " (s)" << endl;
  vt.PrintResultList(decoded);

  //try a basic pruned/beam search
  testLattice.clear();
  decoded.clear();
  //full path-enumeration testing
  cout << "Running pruned dfs graph search..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  lb.TestBuildLattice(testLattice);
  vt.RunPrunedSearch(testLattice,decoded,-1);
  //delete candidate words after 100 words
  //for()
  lm.Process(decoded);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << diffTimeSpecs(&begin,&end) << " (s)" << endl;
  cout << "after language model conditioning: " << endl;
  vt.PrintResultList(decoded);



/*
  //read in some test data. note the chained pipe-transform pattern: output param of each class becomes input to next class
  sb.BuildTestData(testDataFile, inData);
  sb.Process(inData, outData);
  lb.BuildStaticLattice(outData, testLattice);
  vt.Process(testLattice, decoded);
*/  


  return 0;
}
