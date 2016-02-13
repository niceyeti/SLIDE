#include "header.hpp"

/*
  SingularityBuilder takes a raw signal and converts it to an ordered list of
  physical points. Conditioner's primary responsibility is taking that vector
  of points and converting it into a stochastic structure, a directed graph where
  each arc has an associated probability (a letter lattice).

  NOTES: I'm going to simplify the graph and assume a directed lattice, instead of accounting
  for skip-arcs (an arc skipping over sets of nodes, etc) or other complex relationships. It's
  better to define this as "error" to be minimized, for now.

  Both builds and conditions the lattice. The only conditioning data for the arcs, as yet, is bigram
  data. Its not clear what other parameters might be used as input to arc weights. State holds the internal
  probability of a state, given its physically-determined probability.


*/

class LatticeBuilder{
private:
  

  
  

  //Lattice m_lattice;
  void BuildMapCoordinates(void);
  void SearchForNeighborKeys(const Point& p, vector<State>& neighbors);
  void BuildInitialLattice(vector<Point> inData);
  char FindNearestKey(const Point& p);
  void ConditionArcs(Lattice& lattice);
  void BuildKeyMap(void);
  ~LatticeBuilder();

public:
  LatticeBuilder();
  ~LatticeBuilder();

//these don't seem to belong in this class
void BuildKeyMap(void);
void BuildMapCoordinates(void);
char FindNearestKey(const Point& p);
double GetBigramProbability(char a, char b);
void SearchForNeighborKeys(const Point& p, vector<State>& neighbors);
void BuildStaticLattice(vector<PointMu>& inData, Lattice& lattice);
double ReflexiveLikelihood(U16 ticks);
void AppendCluster(PointMu& mu);
void BuildTransitionModel(Lattice& lattice)

};












