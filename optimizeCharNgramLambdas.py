"""
  Given a very large corpus, try to optimize the lambdas (the weights) on each char-n-gram model.

  This maximizes the likelihood of the given corpus, given the char-gram models

  
  


"""
import math



#build gram model
def buildNGramModel(fname,n):
  model = {}
  gramFile = open(fname,"r")
  lines = gramFile.readlines()
  for line in lines:  
    tokens = line.strip().split(" ")
    model[tokens[0][n-1]] = tuple(tokens[0][n],float(tokens[1]))
    modelSum += float(tokens[1])
  return model
      


#some filtered data set of only words, fairly large, but not huge. all words, no numbers of punctuation.
ofile = open("./filteredOANC.txt","r")
lines = ofile.readlines()

#build the models

basedir = sys.argv[1]
unifile = open(basedir+"/unigrams.txt","r")
bifile = open(basedir+"/bigrams.txt","r")
trifile = open(basedir+"/trigrams.txt","r")
quadfile = open(basedir+"/quadgrams.txt","r")
pentafile = open(basedir+"/pentagrams.txt","r")














