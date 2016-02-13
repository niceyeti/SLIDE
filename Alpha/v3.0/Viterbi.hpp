#include "Header.hpp"

#ifndef VITERBI_HPP
#define VITERBI_HPP

/*
  Takes in a fully constructed Lattice and finds the most probable paths through the graph,
  as a list of the top most-probable char sequences, sorted by probability.
  The lattice input is received as a set of states, each with a set of arcs, and all of which have
  an associated probability. Assume the previous Conditioner class component has conditioned
  the both the state probabilities and the arc probabilities; but don't optimize by absorbing one into the other, bearing
  in mind the extrinsic and intrinsic probabilities represented by hidden-Markov models. Although this can be done
  as a final step: once we're completely finished determining the extrinsic arc probabilites (the Conditioner's responsibility),
  we can absorb the state probabilities into the arcs, and then ignore the intrinsic probabilities (State.pState). The probability
  of one path from A to B is given by p(B|A)*p(B), where p(B|A) is usually some n-gram probability and p(B) is State.pState of state B.
  
  
  Step 1: sort outgoing arcs of each state
  
  I named this Viterbi after the core algorithm, but this is likely to change, since Viterbi only returns the most likely output.
  In reality this class will gradually encompass a bunch of graph search behavior, not just Viterbi.


  
  Requirements: 
  Conditioner sends arcs fully conditioned, but does NOT absorb state probabilities into arcs. Viterbi will deploy these strategies,
  however they may be needed (absorbing, sorting, etc).
  
  For now, the strategy for Viterbi will be to accept a Lattice in the above state, absorb state probabilities into arc probabilities,
  sort all of these, and then find the most probable path either recursively or by estimation.


  TODO: make this operate on a streaming Lattice, instead of a full/static one. This may not give
  much of a speed advantage, if any, so its not worth it to optimize until speed is determined.

  TODO: If I stick with first-order lattice (no edge weights), could rewrite things much more simply. Viterbi wouldn't even
  need to generate backpointers, since the most probable path simply entails following the most probable state of each column.
  Particularly, could eliminate AbsorbStateProbabilities, etc., other function calls that mixed edge and state probabilities.
  
  TODO: Could make graph search heuristics smarter, but the existing one probably works alright (heuristic of finding best path,
  backing out two vertices from final node, then taking worst path to get worst-best path, and pruning all paths < worst-best path.
  One method would be to eliminate paths whose n-gram based probability is clearly inconsistent with a language model.


  Output of Viterbi is a set of string-probability pairs defining the most-tightly defined set of possible strings given by the most likely
  paths through the lattice.

*/
class Viterbi{
  public:
  Viterbi(){};
  ~Viterbi(){};

  void Process(Lattice& lattice, LatticePaths& wordList);
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

#endif











