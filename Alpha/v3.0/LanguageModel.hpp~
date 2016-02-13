/*
  This class defines the Language model for the application, encompassing all language modelling responsibilities.
  The class assumes that previous stages have done their best to generate and output the likeliest set of strings
  from some lattice of key clusters; each of these strings is a possible near-edit to the intended input word,
  ranked by physical probability with respect to the input.

  This class then re-conditions the probability of each string, based on how well the string matches various language
  modeling characteristics.

  Input: a sorted list of string-probability (string-double) pairs, representing a possible word and its physical likelihood.
  Output: a re-ranked list of the same string-probability pairs.

  The class has three models, in three stages:
    -character-gram models: recondition the probability of each string based on a crypto-based character-gram model
    -edit distance: see if any words fall within an edit-distance of 1 or two rotations/additions/deletions
    -word n-gram model: finally, once a final ranking of words is produced, re-ranked based on previous three or so input words.

  TODO: edit distance, word n-gram model. Word n-gram should be a simple plug-in, with the added user inoput of the user's previous words.

  TODO: optimize char-n-gram models by eliminating all grams below some threshold. Obviously, a gram like ZPXC should have a probabilty
  near zero; so there's little need to store these kind, and to instead return a default probability estimate for such grams.


  Notes: Language models always incur huge memory overhead, when storing various models (n-gram, chargram, edits, etc). Come up with memory allocation
  strategies to overcome this. Stores U32 keys for char sequences, and likewise for words. Stores these in a vocabulary model; only
  store transition encoding in the other models.

  ngram data from http://practicalcryptography.com/cryptanalysis/text-characterisation/quadgrams/
  Originally from worschatz.
  We likely need to generate our own n-gram data from a huge data set, for IP considerations.


  Notes:
  Notice the columns of the final results after char-n-gram conditioning in this output:
    0: sevehth 16.2445
    1: devehth 16.9497
    2: secehth 16.9541
    3: seventh 17.1126
    4: sefehth 17.5389
    5: deventh 17.8178
    6: secenth 18.6614
    7: sevemth 18.9711
    8: sefenth 19.2953
    9: sebenth 19.4955
  It may be possible to flatten these into an edit-distance using a variation of the following algorithm:
    foreach column in columns:
      char = voteMax(column.words)
      char = word[i]
  Just notice that the majority vote of the columns is a very likely char; merging in this way could easily generate 
  the top most likely words in a very fast pseudo-kNN manner. USe some measure of highest std-dev column to figure out
  where likely permuations occur.









*/

#include "Header.hpp"

class LanguageModel{
  public:
    LanguageModel();
    ~LanguageModel();
    U8 charMap[SMALL_BUFSIZE];    //data structure for majority voting filter method
    NgramModel unigramModel;
    NgramModel bigramModel;
    NgramModel trigramModel;
    NgramModel quadgramModel;
    NgramModel pentagramModel;

    /*
    TODO
    WordGramModel wordMonogramModel;
    WordGramModel wordBigramModel;
    WordGramModel wordTrigramModel;   

    //SearchForEdits(edits, 2); //edit distance processing. second parameter is max edit-distances to search for.
    //ReconditionByWordGrams(list<string> usersPreviousInputs, edits);  
    */

    //this could be the final output, to some edit-distance, vocabulary, and word-n-gram search methods (eg, k-nearest edits)
    void MajorityVoteFilter(LatticePaths& paths, int topN, int k, vector<string>& output);

    double GetUnigramProbability(char a);
    double GetBigramProbability(char a, char b);
    double GetTrigramProbability(char a, char b, char c);
    double GetQuadgramProbability(char a, char b, char c, char d);
    double GetPentagramProbability(char a, char b, char c, char d, char e);
    //compression management methods for the pentagam model, squeezing five chars into a U32 key
    U32 CharToCompChar(char c);
    U32 GetPentaKey(char a, char b, char c, char d, char e);
    void ShallowSearch(LatticePaths& edits, int depth);
    void BuildCharacterNgramModel(const string& ngramFile);



    //core functionality
    void ReconditionByCharGrams(LatticePaths& edits);
    void Process(LatticePaths& edits);

};
