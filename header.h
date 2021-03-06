#include <list>
#include <map>
#include <unordered_set> //used for result duplicate subkey filtering
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>
#include <utility>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>

//using namespace std;
using std::cout;
using std::getline;
using std::endl;
using std::string;
using std::vector;
using std::cin;
using std::map;
using std::multimap;
using std::unordered_set;
using std::list;
using std::sort;
using std::flush;
using std::pair;
using std::pow;
using std::ios;
using std::fstream;
using std::unordered_map;
using std::isfinite; //a f

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long int U64;

/**********************************************************************
Graph data definitions. I hate to build this so early, but this pattern
should allow for random graph transitions. Expect graphs to be directed,
but not completely acyclic, since a vertex could have a reflexive edge.


***********************************************************************/
//input vectors of state/probability pairs of the form <alpha,P(alpha)>
typedef struct edge{
  U16 destId;
  double destProb;
} Edge;

//each vertex has-a set of outgoing edges, which may be reflexive
typedef struct vertex{
  U16 id;
  char alpha;
  vector<Edge> edges;
} Vertex;

//index vertices by their id. a path is encoded in the edge vectors within each vertex
typedef vector<Vertex> Graph;

//a path is encoded as a sequence of vertex id's
typedef struct path{
  vector<U16> path;
  double pathProb;
} Path;
// END graph definition


//the Viterbi algorithm will output a list of paths, ranked by probability
typedef list<Path> ViterbiPaths;

class Prototype{
  private:
    //let 0 be the id of the <start> node. 65535 the id of the <end> node
    Graph graph;  //the transition model
    ViterbiPaths viterbiPaths; //some set of ranked paths. dont confuse with edges.
    U16 idCounter;
    static const U16 startId = 0;   //id of the starting vertex
    U16 endId; //id of the terminal vertex



  public:
    Prototype();
    ~Prototype();
    Vertex getVertex(void); //need some getters/setters so vertices are managed by class, not client
    //bool removeVertex(); shouldn't need this, every graph should be valid
    

}

//clear all vertices, and re-init with only v0
void Prototype::resetGraph(void)
{
  graph.clear();
  Vertex v0 = {0,'$',{}};
  graph.push_back(v0);
}

//
Vertex Prototype::getVertex(char c)
{
  Vertex v;
  v.id = ++idCounter;
  v.alpha = c;

  return Vertex;
}

//complete graph construction by terminating it with the end vertex
void Prototye::finalizeGraph(void)
{
  Vertex v;
  endId = v.id = ++idCounter;
  graph.push_back(v);
}



Prototype::Prototype()
{
  //reserve enough space for 50 character words; each character can assume 26 values, so that's 50*26 size table
  graph.reserve(50*26);  //this only reserves/allocs space, it does not mean there are 50*26 vertices in the graph  
  Vertex v0 = {0,'$',{}};  // c++11 syntax
  //initialize the graph with v0
  graph.push_back(v0);

  idCounter = 0;
  endId = 65535;


}








int main (void)
{




}



















