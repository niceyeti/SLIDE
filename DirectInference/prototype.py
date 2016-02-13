"""
This is a python prototype of the direct inference method. The inference method is:
   Given a set of clusters (mean points), correlate these with words, and return a list of most likely words.

The cluster points are mean points, possible key "events", which forms a set of (x,y) pairs. Thus,
a complete set of clusters represents some event-path through the keys for a given word, as a vector of
vectors: path_vector = <(x,y), (x,y), ... (x,y)>

Every possible word in the lexicon also maps to a unique such vector as well.
So given this vector of vectors, we try to find which word (itself as a vector of points) is nearest to this one,
using some distance function. So go to some data structure of word vectors, and find
the nearest vector(s). Next of course, re-rank these according to their linguistic validity using some language model, but that's
not important here.

The data model of vectors is just a black box here, since this is just a proof of the algorithmic concept of mapping
cluster vectors directly to word vectors. The data model would likely be factored to facilitate fast, partitioned
searches and so on, but none of that matters here (in the scope of this prototype). Thus, the word-vector lookups
will just be brute-force searched across the entire word set.

The method places strong emphasis on the distance metric:
  dist = distance(clusterVector<>, wordVector<>)



The vocabulary is given by COCA, which averages about 60% recall in most cases. This is just a test model. A more complete
vocabulary model will be needed for real use.
"""



def BuildWordModel():
  wordModel = {}

  ifile = open("/Desktop/TeamGleason/corpii/coca_ngrams/w2.txt","r")
  lines = ifile.readlines()
  for line in lines:
    tokens = line.split("\t")
    if len(tokens) > 2:
      if wordModel.get(tokens[1],-1) == -1:
        wordModel = tokens[1].strip().lower()
      if wordModel.get(tokens[2],-1) == -1:
        wordModel = tokens[2].strip().lower()









