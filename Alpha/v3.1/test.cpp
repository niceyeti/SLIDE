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

//these are memory consumption parameters. 
#define MAX_COLS 26 //estimated maximmum-length word one might input. Recall for very long words, it becomes very for a language model to estimate the input from prefixes
#define MAX_CLUSTER_ALPHAS 7 //maximum number of alphas in a cluster, aka, the max number of adjacent keys for a point on the keyboard
#define BUFSIZE 256
#define STATE_PROB_PRUNE_THRESHOLD 0.10 // an optimization for the Viterbi algorithm runtime: if some state's probability is less than (or greater than, for log-based probs) this threshold, such that its overwhelmingly unlikely to lead to a most probable path, ignore it.
#define STATE_LOG_PRUNE_THRESHOLD 3  //for -log-based constraints.   -log-base2(0.125) = 3    -log-base2(0.0675) = 4
#define ZERO_LOG_PROB 99999
#define INIT_STATE_LOG_PROB -1.0 // A flag value for the initial states. Using a negative value is easier to detect as flag than 0.0
#define REFLEXIVE_TICK_THRESHOLD 200  //TODO: this is a random number
#define DBG 1
#define USE_NGRAM_DATA 1

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


*/

//primitive class
typedef struct point{
  short int X;
  short int Y;
} Point;

//for integer pixel distances
short int IntDistance(const point& p1, const point& p2)
{
  return (short int)sqrt(pow((double)(p1.X - p2.Y),2.0) + (short int)pow((double)(p1.Y + p2.Y),2.0));
}
//for real-valued distances, e.g. when mapping distances to a probability in range 0-1
double DoubleDistance(const point& p1, const point& p2)
{
  return sqrt(pow((double)(p1.X - p2.Y),2.0) + pow((double)(p1.Y + p2.Y),2.0));
}

//a vector of a point and a raw measure of time (ticks) spent in this state, which can be used to estimate reflexive likelihood
// signal class outputs these, per detected cluster
typedef struct pointMu{
  Point p;
  U16 ticks;  //some value representing the time spent in a cluster, variance, etc, used to determine likelihood of state
} PointMu; //interpret as "point mean"

typedef struct state state;

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
  Cluster(const vector<char>& neighbors, double pReflex){
    alphas.resize(neighbors.size());
    for(int i = 0 ; i < neighbors.size(); i++){
      alphas[i].symbol = neighbors[i];
      alphas[i].pState = 0.0;
      alphas[i].viterbiMax = 0.0;
      alphas[i].maxPrev = NULL;
    }
    pReflexive = pReflex;
  }
  Cluster(){
    //default ctor: do nothing
    pReflexive = 0.0;
  }

  vector<State> alphas;
  double pReflexive;  //reflexive transition probability of this cluster.
};

typedef vector<Cluster> Lattice;
//typedef pair<char,char> Transition;
//typedef vector< vector<Transition> > TransitionModel; // index with [nextstate][alpha]
typedef unordered_map<U32,double> BigramModel;  // use only the bigram model for now. keys are U32 just for consistency;
typedef unordered_map<U32,double> TriGramModel; // obviously a two char key of (U8<<8)|U8 could be used (U16) for bigram model,
typedef unordered_map<U32,double> UnigramModel; // U8 keys for the unigram, but consistency is nicer.
typedef pair<string,double> ViterbiWord;
typedef list<ViterbiWord> ViterbiOutput;  //output of Viterbi stage is a list of strings paired with some probability for that path

typedef map<char,pair<Point,vector<char> > > KeyMap; //lookup data structure of manually defined key/neighbor relationships
typedef KeyMap::iterator KeyMapIt;

KeyMap CLASS_keyMap; //put this into the outer class container for all these classes


//Comparison function for sorting arc's by probability, in ascending order, min item first. We're in -log2-space, so minimization is desired.
bool maxArcLlikelihood(const Arc& left, const Arc& right)
{
  return left.pArc < right.pArc;
}



int main(void)
{
  arc a;
  Arc a2;
  State s;
  state s2;

  cout << "success" << endl;


  return 0;
}
