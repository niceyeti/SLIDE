#include "Header.hpp"

/*
  TODO: rewrite all Controller component classes (lm, lb, se, etc) as pointers to these classes,
  so their various contructors may be used. And dont forget to delete them in the dtor. 
*/

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

//manages the key-map, dist functions, etc. Anything that needs to be aggregated (called by) other classes
class LayoutManager{
  public:
    //a global data structure for correlating points with ui-keys
    KeyMap keyMap;
    double minKeyRadius; //minimum radius between the two nearest keys (eg, this distance/2)
    double minKeyDiameter;
    int layoutWidth;
    int layoutHeight;

    LayoutManager();
    LayoutManager(const string& keyFileName);
    ~LayoutManager();

    //testing
    void PrintKeyMap(void);
    int GetWidth(void);
    int GetHeight(void);
    double GetMinKeyRadius(void);
    double GetMinKeyDiameter(void);
    void InitLayoutDimensions(void);
    void BuildKeyMap(const string& keyFileName);
    void BuildKeyMapClusters(void);
    void BuildKeyMapCoordinates(const string& keyFileName);
    void SetMinKeyDists(void);
    char FindNearestKey(const Point& p);
    void SearchForNeighborKeys(const Point& p, vector<State>& neighbors);  //might be obsolete
    vector<char>* GetNeighborPtr(char index);    
    Point GetPoint(char symbol);

    //TODO: static?
    double DyDx(const Point& p1, const Point& p2);
    double AvgDyDx(vector<Point>& pts, int start, int npts);
    int AbsDiff(int i, int j);
    double DoubleDistance(const Point& p1, const Point& p2);
    int IntDistance(const Point& p1, const Point& p2);
    double AvgDistance(vector<Point>& pts, int begin, int nPts);
    double CoVariance(vector<Point>& pts, int begin, int nPts);
    double CoStdDeviation(vector<Point>& pts, int begin, int nPts);
		double StDev_X(vector<Point>& pts, int begin, int nPts);
		double StDev_Y(vector<Point>& pts, int begin, int nPts);
		double VecLength(double x, double y);
		double AvgTheta(vector<Point>& inData, int start, int npts);
		double DotProduct(const Point& v1, const Point& v2);
    double CosineSimilarity(const Point& v1, const Point& v2);
    //double AvgDyDx(vector<Point>& inData, int start, int npts);
};


class SearchEngine{
  public:
		SearchEngine();
		~SearchEngine();

		void Process(Lattice& lattice, LatticePaths& wordList);
		void SimpleViterbi(Lattice& lattice, LatticePaths& results);
		void RunViterbi(Lattice& lattice, LatticePaths& results);
		void RunExhaustiveSearch(Lattice& lattice, LatticePaths& results, int depthBound);
		void DFS(State* curState, char prefix[256], double cumulativeProb, int curDepth, const int depthBound, LatticePaths& resultListRef);
		void RunPrunedSearch(Lattice& lattice, LatticePaths& results, int depthBound);
		void RunPrunedDFS(State* curState, char prefix[256], double cumulativeProb, const double pruneThresholdProb, int curDepth, const int depthBound, LatticePaths& resultList);
		void PrunedDFS(State* curState, char prefix[256], double cumulativeProb, const double pruneThresholdProb, int curDepth, const int depthBound, LatticePaths& resultList);
		double ThresholdHeuristic(LatticePaths& subList);
		double WorstBestHeuristic(Lattice& lattice);
		void SortArcs(Lattice& lattice);
		void AbsorbStateProbabilitiesInArcs(Lattice& lattice);
		void PrintResultList(LatticePaths& results);
};

class LanguageModel{
  public:
    LanguageModel();
    ~LanguageModel();
    U8 charMap[SMALL_BUFSIZE];    //data structure for majority voting filter method
    CharGramModel unigramModel;
    CharGramModel bigramModel;
    CharGramModel trigramModel;
    CharGramModel quadgramModel;
    CharGramModel pentagramModel;

    /*
    TODO
    WordGramModel wordMonogramModel;  //could be merged with vocabulary model
    WordGramModel wordBigramModel;
    WordGramModel wordTrigramModel;   

    //SearchForEdits(edits, 2); //edit distance processing. second parameter is max edit-distances to search for.
    //ReconditionByWordGrams(list<string> usersPreviousInputs, edits);  
    */

    //this could be the final output generator, to some edit-distance, vocabulary, and word-n-gram search methods (eg, k-nearest edits)
    void MajorityVoteFilter(LatticePaths& paths, int topN, int k, vector<string>& output);

    double GetUnigramProbability(char a);
    double GetBigramProbability(char a, char b);
    double GetTrigramProbability(char a, char b, char c);
    double GetQuadgramProbability(char a, char b, char c, char d);
    double GetPentagramProbability(char a, char b, char c, char d, char e);
    //compression management methods for the pentagam model, squeezing five chars into a U32 key
    U32 CompressedChar(char c);
    U32 GetCharGramKey(char keyStr[]);
    U32 GetBigramKey(char a, char b);
    U32 GetTrigramKey(char a, char b, char c);
    U32 GetQuadgramKey(char a, char b, char c, char d);
    U32 GetPentagramKey(char a, char b, char c, char d, char e);
    void TruncateResults(LatticePaths& edits, int depth);
    void BuildModels(void);
    void BuildCharacterNgramModel(const string& ngramFile);

    //core functionality
    void ReconditionByCharGrams(LatticePaths& edits);
    void Process(LatticePaths& edits);
};

class SingularityBuilder
{
  public:
    LayoutManager* layoutManager;
    //streaming clustering. still reliant on a tick input, but should be near realtime
    int dxThreshold;        //higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
    int innerDxThreshold;   // a softer theshold once we're in the event state
    int triggerThreshold;      //receive this many trigger before throwing. may also need to correlate these as consecutive triggers

    //UI boundary parameters. The important thing is that we recognize when user is targeting the stop/start state region (space bar).
    int activeRegion_Left;
    int activeRegion_Right;
    int activeRegion_Top;
    int activeRegion_Bottom;

    SingularityBuilder();
    SingularityBuilder(int left, int right, int top, int bottom, LayoutManager* layoutManagerPtr);
    ~SingularityBuilder();

    void SetLayoutManager(LayoutManager* layoutManager);
    void SetEventParameters(int dxThresh, int innerDxThresh, int triggerThresh);
    void SetUiBoundaries(int left, int right, int top, int bottom);

    //testing
    void UnitTests(const string& testDir);
    void TestSingularityBuilder(const string& testFile, string& fileDelimiter);
    void BuildTestData(const string& fname, vector<Point>& inData, string& fileDelimiter);
    short int RandomizeVal(short int n, short int error);

    //some clustering tasks
    void SimpleClustering(vector<Point>& inData, vector<PointMu>& outData);
    void Process4(vector<Point>& inData, vector<PointMu>& outData);

    bool MinSeparation(const PointMu& mu1, const PointMu& mu2);
    void MergeClusters(vector<PointMu>& rawClusters, vector<PointMu>& mergedData);
    bool InBounds(const Point& p);
    void PrintInData(vector<Point>& inData);
    void PrintOutData(vector<PointMu>& outData);
    void Process(vector<Point>& inData, vector<PointMu>& outData);
    void Process2(vector<Point>& inData, vector<PointMu>& outData); //a multi-attribute event detector
    void Process3(vector<Point>& inData, vector<PointMu>& outData);
    void CalculateMean(int begin, int end, const vector<Point>& coorList, PointMu& pointMean);
    double CalculateDeltaTheta(double theta1, double theta2, int dt);  //returns angular velocity as a secondary event trigger


};

//THE DATA MODEL OF THIS CLASS IS PURELY A PROTOTYPE
class DirectInference{  //class which attempts to map cluster input (as a vector) to the nearest word (also as a vector)
  public:
		//set was only used here for prototyping reasons, as a "bag of words". A much better data structure could be devised. 
		WordModel wordModel;
		LayoutManager* layoutManager;

		DirectInference();
		DirectInference(const string& vocabFile, LayoutManager* layoutManagerPtr);
		~DirectInference();

		void BuildWordModel(const string& vocabFile);
    void MeansToString(vector<PointMu>& pointMeans, string& output);
    void MeansToEditList(vector<PointMu>& pointMeans, vector<string>& stringList);
    void Strip(char buf[], char toChar);
    void ReverseInPlace(vector<PointMu>& pts);
    void RevPointMeans(const vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans); 
    string ReverseString(const string& str);
    void SetLayoutManager(LayoutManager* layoutManagerPtr);
		double VectorDistance(vector<PointMu>& pointMeans, WordModelIt it);
    double VectorDistance(vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans, WordModelIt it);
    //double SumDistMetric(vector<PointMu>& pointMeans, WordModelIt it);
		void Process(vector<PointMu>& pointMeans, SearchResults& results);
    void MergeInference(vector<PointMu>& pointMeans, SearchResults& results);
    //string distance approximation
    void StringDistInference(vector<PointMu>& pointMeans, SearchResults& results);
    //geometric distance approximation (far more brute force than previous)
		void VectorDistInference(vector<PointMu>& pointMeans, SearchResults& results);
		double SumDistMetric_Aligned(vector<PointMu>& pointMeans, const string& candidate);
    double SumDistMetric_Aligned_FwdBkwd(vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans, const string& candidate);
		double SumDistMetric_Unaligned(vector<PointMu>& pointMeans, const string& candidate);
    double SumDistMetric_Unaligned_FwdBkwd(vector<PointMu>& pointMeans, const string& candidate);
    double InsertionError(const Point& errorPt, const Point& pt1, const Point& pt2);
    double NearestPointCorrection(const Point& errorPt, const Point& pt1, const Point& pt2);
    double MidPointCorrection(const Point& errorPt, const Point& pt1, const Point& pt2);
		//string base distance metrics. these may also belong in edit functions of language class
		double StringDist_Hamming(const string& s1, const string& s2);
    double StringDist_HammingSkipChar(const string& s1, const string& s2);
		double StringDist_HammingFwdBkwd(const string& s1, const string& s2);
		double StringDist_HammingBkwd(const string& s1, const string& s2);
		double StringDist_HammingFwd(const string& s1, const string& s2);
};

class LatticeBuilder{
  public:
    LayoutManager* layoutManager;

    LatticeBuilder();
    LatticeBuilder(LayoutManager* layoutManagerPtr);  //LatticeBuilder needs a reference to the parent container to access keyMap; the pointer is its interface to Controller members
    ~LatticeBuilder();

    void InitCluster(Cluster& newCluster, PointMu& mu);
    void SetLayoutManager(LayoutManager* layoutManagerPtr);
    void PrintLattice(Lattice& lattice);
    void ClearLattice(Lattice& lattice);
    void BuildStaticLattice(vector<PointMu>& inData, Lattice& lattice);
    double CalculateReflexiveLikelihood(U16 ticks);
    void AppendCluster(PointMu& mu, Lattice& lattice);
    void BuildTransitionModel(Lattice& lattice);
    void TestBuildLattice(Lattice& lattice);
    void InitArcSizes(Lattice& lattice);
    //Comparison function for sorting arc's by probability, in ascending order, min item first. We're in -log2-space, so minimization is desired.
    bool MaxArcLlikelihood(const Arc& left, const Arc& right);
};


class Controller{
  private:

    //primary application components. these are defined by their reponsibilities, not their implementation (though there is some overlap of responsibilities)
    LayoutManager* lmgr;
    SingularityBuilder* sb;
    LatticeBuilder* lb;
    SearchEngine* se;
    LanguageModel* lm;
    DirectInference* di;

    //a global data structure for correlating points with ui-keys
    KeyMap keyMap;
    double minKeyRadius; //minimum radius between the two nearest keys (eg, this distance/2)

  public:
    Controller();
    Controller(const string& keyFileName);
    ~Controller();

    //testing
    void SmokeTest(void);
    void PerformanceTest(const string& srcDir);
    void TestWordStream(const string& fname, string& delimiter);
};

#endif




