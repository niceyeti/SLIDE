#ifndef HEADER_HPP
#define HEADER_HPP


#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <ctime>

//these are memory consumption parameters. 
#define MAX_COLS 26 //estimated maximmum-length word one might input. Recall for very long words, it becomes very for a language model to estimate the input from prefixes
#define MAX_CLUSTER_ALPHAS 7 //maximum number of alphas in a cluster, aka, the max number of adjacent keys for a point on the keyboard
#define BUFSIZE 256
#define STATE_PROB_PRUNE_THRESHOLD 0.10 // an optimization for the Viterbi algorithm runtime: if some state's probability is less than (or greater than, for log-based probs) this threshold, such that its overwhelmingly unlikely to lead to a most probable path, ignore it.
#define STATE_LOG_PRUNE_THRESHOLD 3  //for -log-based constraints.   -log-base2(0.125) = 3    -log-base2(0.0675) = 4
#define ZERO_LOG_PROB 99999.0
#define INIT_STATE_LOG_PROB -1.0 // A flag value for the initial states. Using a negative value is easier to detect as flag than 0.0
#define REFLEXIVE_TICK_THRESHOLD 200  //TODO: this is a random number
#define DBG 1
#define USE_NGRAM_DATA 1  //this enables n-gram models, but note separate locations. Trigram model breaks the dynamic programming lattice model, and is only used in Viterbi class.
#define CHAR_NGRAM_MODEL_WEIGHT 1.0  //weight to use for charater ngram data
#define CHAR_UNIGRAM_LAMBDA  1.0
#define CHAR_BIGRAM_LAMBDA 2.0           //these were optimized with python, in  Viterbi/charGram/optimizeLambdas.py. 
#define CHAR_TRIGRAM_LAMBDA 42.666666    //as shown, the analysis showed that its more less best to let the most confident model dominate 
#define CHAR_QUADGRAM_LAMBDA 3413.333333 //except for the penta/quad gram models.
#define CHAR_PENTAGRAM_LAMBDA 1820.444444
#define DEFAULT_LOG_PROB 15.0  //a default, punitive log-probability for sequences not found in a model (which therefore have the least likelihood)
#define SMALL_BUFSIZE 256


using std::list;
using std::vector;
using std::fstream;
using std::string;
using std::ios;
using std::cout;
using std::flush;
using std::endl;
using std::cin;
using std::getline;
using std::map;
using std::unordered_map;
using std::pair;
using std::sort;
using std::pow;
using std::sqrt;


typedef unsigned char U8;
typedef unsigned short int U16;
typedef unsigned int U32;

/*
  TODO: 
    -Work out more math, figure out how to handle the reflexive probabilities:
    View the lattice as a graph inside a graph. The metagraph has the columns as vertices,
    from which we either go to the next column, or transition reflexively. The inside graph has all
    the substates broken out, with their arcs. Therefore, the summation of outgoing arcs from all states
    in a column is the total probability of leaving that column, and one minus that quantity is the probability
    of a reflexive transition. Leverage these sort of properties when defining the reflexive behavior, but try to capture
    it at the outermost preprocessing stage; ideally, the topology of the graph will be static and will include few reflexive
    transitions by the time it gets to Viterbi. The earlier stages should do their best to bifurcate bimodal clusters.
    
    -Get rid of parameters like Lattice& lattice from private functions, since these will be class variables.

  Another module for this project may include a forward-backward based autocompletion mechanism. The motivation for looking
  forward and backward is bidirectional search, the the inverted graph is still valid, and bidirectional search will help overcome
  errors in edits.

  
  ngram data from http://practicalcryptography.com/cryptanalysis/text-characterisation/quadgrams/
  We likely need to generate our own n-gram data from a huge data set, for IP considerations.

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Expansive during design, reductive during testing/optimization: don't optimize during design. We need the flexibility
  of creating lots of inefficient solutions, then trimming the fat and lifting the efficient/precise solutions at the end.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


*/


/*

*/


//primitive class
typedef struct point{
  short int X;
  short int Y;
} Point;

short int IntDistance(const point& p1, const point& p2);
double DoubleDistance(const point& p1, const point& p2);
char FindNearestKey(const Point& p);
vector<char>* GetNeighborPtr(char index);
Point GetPoint(char symbol);

typedef map<char,pair<Point,vector<char> > > KeyMap; //lookup data structure of manually defined key/neighbor relationships
typedef KeyMap::iterator KeyMapIt;


//a vector of a point and a raw measure of time (ticks) spent in this state, which can be used to estimate reflexive likelihood
// signal class outputs these, per detected cluster
typedef struct pointMu{
  Point p;
  U16 ticks;  //some value representing the time spent in a cluster, variance, etc, used to determine likelihood of state
} PointMu; //interpret as "point mean"

typedef struct state state;

typedef pair<double,state*> BackLinks;

typedef struct arc{
  //pair<U16,char> id; //each arc is uniquely identified by its target column, and the char in that column
  state* dest;
  double pArc;
} Arc;

//state has a symbol, and internal probability, and a set of outgoing arcs
typedef struct state{
  double pState;
  char symbol;
  vector<Arc> arcs;  //each outgoing arc is the index of the next state, and the char-id of that state
  double viterbiMax;  //solely for Viterbi algorithm route-finding
  state* maxPrev;     // ditto. TODO: handle the exception where there is no previous column of states (col=0)
} State;

//each column of the Lattice is a Cluster with alphas (keys/characters) and a vector of transitions. the [0] transition is always reflexive
class Cluster{
  public:
  vector<State> alphas;
  double pReflexive;  //reflexive transition probability of this cluster.

  //given a point-mean, appends a cluster of neighbors around that point, each with a geometry-base probability
  Cluster(const PointMu& mu){
    //get the likelihood of a reflexive arc on this cluster
    pReflexive = ReflexiveLikelihood(mu.ticks);

    double sumDist = 0.0;
    Point tempPt;
    //lookup nearest neighbors
    char index = FindNearestKey(mu.p);    
    vector<char>* neighbors = GetNeighborPtr(index);

    alphas.resize(neighbors->size() + 1);  //add one to account for the nearest key (var "index") itself
    for(int i = 0; i < neighbors->size(); i++){ //from 1, since alphas isn't 

      alphas[i].symbol = (*neighbors)[i];
      alphas[i].viterbiMax = 0.0;
      alphas[i].maxPrev = NULL;

      tempPt = GetPoint(alphas[i].symbol);
      //tempPt = CLASS_keyMap[alphas[i].symbol].first;
      alphas[i].pState = DoubleDistance(mu.p, tempPt); //init every state to its real distance to the mean-point mu
      sumDist += alphas[i].pState;
    }
    //init the nearest key, which by this procedure will always be last in the vector
    alphas[alphas.size()-1].viterbiMax = 0.0;
    alphas[alphas.size()-1].maxPrev = NULL;
    alphas[alphas.size()-1].symbol = index;
    tempPt = GetPoint(index);
    alphas[alphas.size()-1].pState = DoubleDistance(mu.p, tempPt);
    sumDist += alphas[alphas.size()-1].pState;

    //now init the state probabilities by simple normalization
    //TODO: define this geometric probability better. It would probably benefit from being biased toward the mean point, and giving
    //      lower probability to farther keys. The method used is a parameter for our models; the best one needs to be determined through expt.
    //      Curvature of the prob-space should be center-biased; another benefit may be tracking the max distance, and definined this key as
    //      zero-probability.
    for(int i = 0; i < alphas.size(); i++){
      alphas[i].pState /= sumDist;  //TODO: try other functions, such as conical or log-based prob-surfaces
      alphas[i].pState = (-1.0) * log2(alphas[i].pState);
    }
  }
  Cluster(){
    //default ctor: do nothing
    pReflexive = 0.0;
  }
  //TODO: this function is also defined in LatticeBuilder. Also, it needs to be tuned.
  double ReflexiveLikelihood(U16 ticks){
    if(ticks >= REFLEXIVE_TICK_THRESHOLD){
      return 0.5;
    }
    return 0.0;
  }
};

typedef vector<Cluster> Lattice;
//typedef pair<char,char> Transition;
//typedef vector< vector<Transition> > TransitionModel; // index with [nextstate][alpha]
// these might be compressible, eg, assign some default low probability to very uncommon sequences like "ZDQ"
typedef unordered_map<U32,double> NgramModel;
typedef NgramModel::iterator CharGramIt;
typedef pair<string,double> LatticePath;
typedef list<LatticePath> LatticePaths;  //output of Viterbi stage is a list of strings paired with some probability for that path
typedef LatticePaths::iterator LatticePathsIt;


//global utilities. These are not relevant to this close, there's just no other place for them.
bool maxArcLlikelihood(const Arc& left, const Arc& right);
bool isDelimiter(const char c, const string& delims);
int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims);
void PrintLattice(const Lattice& lattice);
bool ByLogProb(const LatticePath& left, const LatticePath& right);
long double diffTimeSpecs(struct timespec* begin, struct timespec* end);
//TODO: remove to some higher class
//void BuildCharacterNgramModel(const string& ngramFile);
//double GetUnigramProbability(char a);
//double GetBigramProbability(char a, char b);
//double GetTrigramProbability(char a, char b, char c);


/*  //outer class
class Controller{
 public: 
    Controller();
    ~Controller();

    //global utilities. These are not relevant to this close, there's just no other place for them.
    bool maxArcLlikelihood(const Arc& left, const Arc& right);
    bool isDelimiter(const char c, const string& delims);
    int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims);
    short int IntDistance(const point& p1, const point& p2);
    double DoubleDistance(const point& p1, const point& p2);
}
*/

#endif



