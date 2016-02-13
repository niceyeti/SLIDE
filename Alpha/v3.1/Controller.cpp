#include "Controller.hpp"
//#include "Header.hpp"

Controller::Controller()
{
  cout << "ERROR Creating Controller object with no parameters. This will break!" << endl;
  string s = "../TestInput/EyeInputs/keyMap.txt";
  lmgr = new LayoutManager(s);
  sb = new SingularityBuilder(0,lmgr->GetWidth(),0,lmgr->GetHeight(),lmgr);
  lb = new LatticeBuilder(lmgr);
  se = new SearchEngine();
  lm = new LanguageModel();
  //lm->BuildModels();
  string vocab = "../vocabModel.txt";
  di = new DirectInference(vocab,lmgr);
}

//TODO: if we keep DirectInference, change its ctor parameters here
Controller::Controller(const string& keyFileName)
{
  lmgr = new LayoutManager(keyFileName);
  sb = new SingularityBuilder(0,lmgr->GetWidth(),0,lmgr->GetHeight(),lmgr);
  lb = new LatticeBuilder(lmgr);
  se = new SearchEngine();
  lm = new LanguageModel();
  string vocab = "../vocabModel.txt";
  di = new DirectInference(vocab,lmgr);
}

Controller::~Controller()
{
  delete sb;
  delete lb;
  delete se;
  delete lm;
  delete di;
  delete lmgr;

  keyMap.clear();
  cout << "Application dtor'ed. Goodbye!" << endl;
}

/*
  Runs through a bunch of files containing example paths. Each file contains the entire
  stream of sensor inputs for a given word. Running over a bunch of these simulates inputs
  from the user, giving us a automated testing framework based on user examples.
  
  Assume all test files are named "wordk", where k is some integer. This is hardcoded for
  now
  
  TODO: Rewrite all of the input file in xml form, so they can contain structured data.
*/
void Controller::PerformanceTest(const string& srcDir)
{
  string path = srcDir;
  string suffix = ".txt";
  int i;
  string fname;
  string delim = "\t";

  if(path[path.length()-1] != PATH_ESCAPE){
    path += PATH_ESCAPE;
  }

  for(i = 1; i <= 12; i++){
    fname = path;
    fname += "word";
    fname += std::to_string(i);
    fname += suffix;
    cout << "Next test file: " << fname << endl;
    TestWordStream(fname,delim);
  }
}

/*
  Driver for testing the application from end-to-end with a file containing a stream of sensor input.
  
  This is to be written to mirror the actual call-sequence of the engine under normal usage.
*/
void Controller::TestWordStream(const string& fname, string& delimiter)
{
	vector<Point> sensorData;
	vector<PointMu> pointMeans;
	Lattice testLattice;
	LatticePaths strings;
  SearchResults diResults;

  cout << "Testing inputs from file: " << fname << endl;
  sb->BuildTestData(fname,sensorData,delimiter);
  cout << "...complete, " << sensorData.size() << " data points" << endl;
  //sb->PrintInData(sensorData);
  
  if(sensorData.size() > 0){
		//call main drivers of each component
		//sb->Process2(sensorData,pointMeans);
		sb->Process3(sensorData,pointMeans);
    //sb->Process4(sensorData,pointMeans);
    //sb->Process(sensorData,pointMeans);
    sb->PrintOutData(pointMeans);

    if(pointMeans.size() > 0){
      //test the direct inference method
      di->Process(pointMeans,diResults);
      //test the search/condition-oriented methods
  		//lb->BuildStaticLattice(pointMeans,testLattice);
			//se->Process(testLattice,strings);
			//lm->Process(strings);
    }
    else{
      cout << "ERROR no point means" << endl;
    }

  }
  
  if(strings.size() == 0 && diResults.size() == 0){
    cout << "no results for wordstream >" << fname << endl;
  }
  if(strings.size() > 0){
    cout << "top result for word stream >" << fname << "< is: " << strings.begin()->first << endl;
  }
  if(diResults.size() > 0){
    cout << "top result for word stream >" << fname << "< is: " << diResults.begin()->first << endl;
  }
}

/*
  Some runs to verify components work, their runtime characteristics.
*/
void Controller::SmokeTest(void)
{
  //long double res, scalar = 1000000000.0;
  struct timespec begin, end;
  vector<string> output;
  //vector<Point> inData;
  //vector<PointMu> outData;
  Lattice testLattice;
  LatticePaths decoded;

  string testDataFile = "./signal.txt";

  //class components
  SingularityBuilder* test_sb = new SingularityBuilder(0,900,0,310,lmgr);
  LatticeBuilder* test_lb = new LatticeBuilder(lmgr);;
  SearchEngine* test_se = new SearchEngine();
  LanguageModel* test_lm = new LanguageModel();

  //lattice and viterbi test
  cout << "Vini Vitti Viterbi..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  test_lb->TestBuildLattice(testLattice);
  test_se->Process(testLattice, decoded);
  //test_lb->ClearLattice(testLattice);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << DiffTimeSpecs(&begin,&end) << " (s)" << endl;

  cout << "decoded: ";
  if(decoded.size() > 0){
    cout << decoded.begin()->first << endl;
  }

  //full path-enumeration testing
  testLattice.clear();
  decoded.clear();
  cout << "Running exhaustive dfs graph test_search, no language modelling..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  test_lb->TestBuildLattice(testLattice);
  test_se->RunExhaustiveSearch(testLattice,decoded,-1);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << DiffTimeSpecs(&begin,&end) << " (s)" << endl;
  test_se->PrintResultList(decoded);

  //try a basic pruned/beam test_search
  testLattice.clear();
  decoded.clear();
  //full path-enumeration testing
  cout << "Running pruned dfs graph test_search, NO language modelling..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  test_lb->TestBuildLattice(testLattice);
  test_se->RunPrunedSearch(testLattice,decoded,-1);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << DiffTimeSpecs(&begin,&end) << " (s)" << endl;
  test_se->PrintResultList(decoded);

  testLattice.clear();
  decoded.clear();
  cout << "Running pruned dfs graph test_search with language models..." << endl;
  clock_gettime(CLOCK_MONOTONIC,&begin);
  test_lb->TestBuildLattice(testLattice);
  test_lb->PrintLattice(testLattice);
  test_se->RunPrunedSearch(testLattice,decoded,-1);
  test_lm->Process(decoded);
  test_lm->MajorityVoteFilter(decoded, 20, 0, output);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "runtime: " << DiffTimeSpecs(&begin,&end) << " (s)" << endl;
  cout << "after language model conditioning: " << endl;
  test_se->PrintResultList(decoded);

  delete test_lm;
  delete test_sb;
  delete test_se;
  delete test_lb;
}

