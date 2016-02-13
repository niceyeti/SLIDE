#ifndef HEADER_HPP
#define HEADER_HPP

/*
  Header file for inference-based word decoding using eye-tracking technology.
  Copyright Jesse Waite, 2014.

*/




#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
//#include <string.h>
#include <algorithm>
#include <ctime>

//OS and machine specific stuff
#ifdef __WINDOWS__
#define PATH_ESCAPE '\\'

#elif _WIN32
#define PATH_ESCAPE '\\'

#elif __linux__
#define PATH_ESCAPE '/'

#elif __unix__
#define PATH_ESCAPE '/'
#endif


//these are memory consumption parameters. 
#define MAX_COLS 26 //estimated maximmum-length word one might input. Recall for very long words, it becomes very for a language model to estimate the input from prefixes
#define MAX_CLUSTER_ALPHAS 7 //maximum number of alphas in a cluster, aka, the max number of adjacent keys for a point on the keyboard
#define BUFSIZE 256
#define STATE_PROB_PRUNE_THRESHOLD 0.10 // an optimization for the Viterbi algorithm runtime: if some state's probability is less than (or greater than, for log-based probs) this threshold, such that its overwhelmingly unlikely to lead to a most probable path, ignore it.
#define STATE_LOG_PRUNE_THRESHOLD 3  //for -log-based constraints.   -log-base2(0.125) = 3    -log-base2(0.0675) = 4
#define ZERO_LOG_PROB 99999.0
#define INIT_STATE_LOG_PROB -1.0 // A flag value for the initial states. Using a negative value is easier to detect as flag than 0.0

#define REFLEXIVE_TICK_THRESHOLD 25  //TODO: this is a magic number

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
#define SKIPCHAR true

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
using std::set;


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

  -Compiler flags for uninit'ed vars, etc?

  Another module for this project may include a forward-backward based autocompletion mechanism. The motivation for looking
  forward and backward is bidirectional search, the the inverted graph is still valid, and bidirectional search will help overcome
  errors in edits.

  If the n-gram search methods are preserved, build character n-gram models by merging a bunch of datasets (COCA-bigram, OANC, etc),
  (and for COCA, multiply each n-gram by the word frequency for the sequence). Build the model using only alpha characters (regular expression
  models can be used for punctuation and numbers), and bound each analysis by word length (don't merge the word sequence into a single
  sequences of chars). In short, map the state machine of this input method to the models generated.

  
  ngram data from http://practicalcryptography.com/cryptanalysis/text-characterisation/quadgrams/
  We likely need to generate our own n-gram data from a huge data set, for IP considerations.

  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Expansive during design, reductive during testing/optimization: don't optimize during design. We need the flexibility
  of creating lots of inefficient solutions, then trimming the fat and lifting the efficient/precise solutions at the end.
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
age 
  TODO: This is IMPORTANT! But also an enhancement. Implement a "So far" inference method. Usage would be, the user is looking
  at the screen creating a path, and we look up the best candidate word in the word set, "so far". This could be done once 
  n characters have been entered, so we don't deploy the search until we have good data (enough chars to make a good separation
  between words). Also, previous input and search could be used to narrow the previous result set, each time a new character
  is detected while the user continues inputting the curret word. This would essentially be a novel form of autocomplete. If you
  think about it, k chars may be sufficient for the lookup of any word of length k+l, for some k and l. That is, for most words
  there will be some number of characters sufficient to distinguish that word (as a path) from any other word. This likelihood
  exactly coincides with the redundancy of english language word prefixes.
*/


/*

*/


//primitive class
/*
typedef struct point{
  short int X;
  short int Y;
} Point;
*/
class Point{
  public:
		short int X;
		short int Y;
    Point();
    Point(short int x, short int y);
    Point(const Point& rhs);
    Point& operator=(const Point& rhs);

/*
    Point(short int x, short int y){
      X = x;
      Y = y;
    };
    Point(){
      X = 0;
      Y = 0;
    };
    //copy constructor
    Point(const Point& rhs){
      X = rhs.X;
      Y = rhs.Y;
    };
    Point& operator=(const Point& rhs){
      if(this != &rhs){
        X = rhs.X;
        Y = rhs.Y;
      }
      return *this;
    };
*/
};


/*
short int IntDistance(const point& p1, const point& p2);
double DoubleDistance(const point& p1, const point& p2);
char FindNearestKey(const Point& p);
vector<char>* GetNeighborPtr(char index);
Point GetPoint(char symbol);
*/

//a vector of a point and a raw measure of time (ticks) spent in this state, which can be used to estimate reflexive likelihood
// signal class outputs these, per detected cluster
/*typedef struct pointMu{
  Point pt;
  U16 ticks;  //some value representing the time spent in a cluster, variance, etc, used to determine likelihood of state
} PointMu; //interpret as "point mean"
*/
class PointMu{
  public:
		Point pt;
		int ticks;  //some value representing the time spent in a cluster, variance, etc, used to determine likelihood of state
    char alpha;
    PointMu();
    PointMu& operator=(const PointMu& rhs);
    PointMu(const PointMu& rhs);

/*
    PointMu(){
      alpha = 'A';
      pt.X = 0;
      pt.Y = 0;
      ticks = 0;
    };
    PointMu& operator=(const PointMu& rhs){
      if(this != &rhs){
        alpha = rhs.alpha;
        ticks = rhs.ticks;
        pt.X = rhs.pt.X;
        pt.Y = rhs.pt.Y;
      }
      return *this;
    };
    PointMu(const PointMu& rhs){
      alpha = rhs.alpha;
      ticks = rhs.ticks;
      pt.X = rhs.pt.X;
      pt.Y = rhs.pt.Y;
    };
*/
};


typedef struct state State;

//typedef pair<double,state*> BackLinks;

typedef struct arc{
  //pair<U16,char> id; //each arc is uniquely identified by its target column, and the char in that column
  State* dest;
  double pArc;
} Arc;

//state has a symbol, and internal probability, and a set of outgoing arcs
typedef struct state{
  double pState;
  char symbol;
  vector<Arc> arcs;  //each outgoing arc is the index of the next state, and the char-id of that state
  double viterbiMax;  //solely for Viterbi algorithm route-finding
  State* maxPrev;     // ditto. TODO: handle the exception where there is no previous column of states (col=0)
} State;


//each column of the Lattice is a Cluster with alphas (keys/characters) and a vector of transitions. the [0] transition is always reflexive
typedef struct cluster{
  vector<State> alphas;
  double pReflexive;  //reflexive transition probability of this cluster.
} Cluster;

typedef vector<Cluster> Lattice;
//typedef pair<char,char> Transition;
//typedef vector< vector<Transition> > TransitionModel; // index with [nextstate][alpha]
// these are compressible, eg, assign some default low probability to very uncommon sequences like "ZDQ"
typedef unordered_map<U32,double> CharGramModel;
typedef CharGramModel::iterator CharGramIt;
typedef pair<string,double> LatticePath;  // <object,tempScore,cumulativeRank>
typedef list<LatticePath> LatticePaths;  //output of Viterbi stage is a list of strings paired with some probability for that path
typedef LatticePaths::iterator LatticePathsIt;
typedef LatticePath SearchResult;  //all aliases for the previous types...
typedef list<SearchResult> SearchResults;
typedef SearchResults::iterator SearchResultIt;

//ui key map
//TODO: The keymap could be reduced to a static array of objects, which would yield vastly faster queries, though its still a quite small rb-tree (map).
typedef map<char,pair<Point,vector<char> > > KeyMap; //lookup data structure of manually defined key/neighbor relationships
typedef KeyMap::iterator KeyMapIt;

//bag of words
typedef set<string> WordModel;
typedef WordModel::iterator WordModelIt;

//forward declaration
//class Controller ;

//misc global utilities
bool ByLogProb(const LatticePath& left, const LatticePath& right);
bool ByDistance(const SearchResult& left, const SearchResult& right);
bool ByRank(const pair<U32,SearchResult> &left, const pair<U32,SearchResult> &right);
int Tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims);
char ToLower(char c);
char ToUpper(char c);
void StrToUpper(char str[]);
bool IsDelimiter(const char c, const string& delims);
long double DiffTimeSpecs(struct timespec* begin, struct timespec* end);

#endif



