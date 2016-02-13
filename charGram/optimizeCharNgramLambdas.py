"""
  Given a very large corpus, try to optimize the lambdas (the weights) on each char-n-gram model.

  This maximizes the likelihood of the given corpus, given the char-gram models

  Its down an dirty. I found the following char-n-gram lambdas by manual testing:

    <1.0,2.0,42.666666,3413.33333,1820.44444>

  These need to be really optimized over a million or so predictions, using a neural network,
  but likely these are the best ratios.
  Notice that except for the last two, each model tends to dominate. These lambdas are very near
  to being binary: eg, for each prediction, only need to ask the best expert (the largest avialable
  n-gram model), and its almost always right more than the others:
    varying unigram and bigram weights yield predictions oscillating about 20%
    varying bi and tri weights yielded about 28%
    varying tri and quad varied in the 30's
    varying quad and penta oscillated in the 42% range
  But for all of the above, modifying weights only caused predictions to differ by a very small fraction,
  implying that even an optimal set of lambdas won't improve much more than a few percentage points in accuracy.
  As stated prior, the more confident model is really the winner, up to 4-grams, after which the benefits
  begin to decline. The lambdas are not null, however, since not all sequences are sufficient to form predictions:
  for instance, if a word is only three letters, then we cant get any quad or penta predictions; to the lambdas defer
  to the lower-order models, while keeping their relative weights the same.


"""
import math

#assume models have been built in files, assumes files contain normalized conditional probabilities in -log2-space
#This means the keys are absolute, not nested. To get the conditional probability of D|ABC, just use key ABCD directly.
def buildNGramModelFromFile(fname):
  model = {}

  ifile = open(fname,"r")
  lines = ifile.readlines()
  for line in lines:
    tokens = line.split("\t")
    if len(tokens) == 2:
      model[tokens[0]] = float(tokens[1]) 

  return model

# queries might not be in the large penta/quad models, but will always at least be in uni, bi, and tri
# Note that the return value of NOTFOUND for .get() is 15.0. This is an arbitrary number indicating low conditional -log2 likelihood.
# Cat a n-gram OUT file to understand why; 16.0 is a typical value for very low likelihood sequences.
def getLinearProb(uni,bi,tri,quad,penta,query,lambdas):
  
  '''  
  if bi.get(query[3:],0.0) <= 0.0:
    print "ERROR could not find in bigram model"
  if tri.get(query[2:],0.0) <= 0.0:
    print "ERROR could not find in trigram model"
  if quad.get(query[1:],0.0) <= 0.0:
    print "ERROR could not find in quadgram model"
  if penta.get(query,0.0) <= 0.0:
    print "ERROR could not find in pentagram model"
  '''

  #for this query, query each model
  #print "queries: ",query,query[4],query[3:4],query[2:4],query[1:4]
  pQ = uni.get(query[4],15.0) * lambdas[0]
  #print "pQ="+str(pQ)
  pQ += (bi.get(query[3:],15.0) * lambdas[1])
  pQ += (tri.get(query[2:],15.0) * lambdas[2])
  pQ += (quad.get(query[1:],15.0) * lambdas[3])
  pQ += (penta.get(query,15.0) * lambdas[4])
  res = [pQ,query[4],0.0]
  #print "res=",res, " q=",query

  return res;


"""
  Recursively trims models, to eliminate estimations for sequences that just aren't 
  consistent with english language. If these sequences are encountered, we will just give them
  a default/penalty value.

  Works by iterating over all 26^5 (12 million) sequences in the pentagram model, summing the estimate
  for each sequence across all n-gram models. Sequences whose sum estimate is below some threshold (manually
  determined) will be eliminated.

  Models are not necessarily subsets of one another, so this iterates over the tri, quad and pentagram models
  (the higher order models) looking for sequences to eliminate.

  Models are then output to disk to maintain the trimmed models.

"""
def trimModels(uni,bi,tri,quad,penta):

  lambdas = [1.0,1.0,1.0,1.0,1.0]

  #list of tuples, for every sequence in the 
  pairs = []

  ofile = open("trimmedPenta.txt","w+")
  for key in penta.keys():
    #get sum prob
    pSeq = getLinearProb(uni,bi,tri,quad,penta,key,lambdas)
    pairs.append((key,pSeq))
  pairs = sorted(pairs, key=lambda p: p[1])
  print pairs,"\ndone"







#precondition: prefix is ALWAYS a four-char query, so there are always 26 possible predictions (A-Z)
def predict(uni,bi,tri,quad,penta,prefix,lambdas):
  results = []

  #A-Z == 65-90
  i = 65
  while i <= 90:
    query = prefix + chr(i)
    res = getLinearProb(uni,bi,tri,quad,penta,query,lambdas)
    results.append(res)
    i+=1
    
  results = sorted(results, key=lambda res: res[0])
  #print results
  """
  i = 0
  while i < len(results):
    results[i][2] = i+1
    i+=1
  print "sorted results: ",results
  """
  #raw_input()

  return results[0]


""""
  Track boolean, real, and kubler-lublach divergence of predictions

  boolean = scores[0]
  real    = scores[1]
  KL      = scores[2]

  prediction and actual are tuples of the form: (score,letter,rank)<double,char,int>

"""
def score(prediction,actual,scores):

  #detect boolean hit
  if prediction[1] == actual[1]:
    scores[0] += 1.0

  #detect real hit, defined as 1 / rank of prediction
  #if prediction[2] > 0.0:
  #  scores[1] += (1.0 / float(prediction[2]))
  #else:
  #  print "WARNING rank < 0 in score() : rank="+actual[2]  

  #update Kullback-Leibler ratio per prediction (subtraction since vals are already logs)
  scores[2] += math.pow(2.0,-1.0*prediction[0]) * (actual[0] - prediction[0])

  return scores

def printScores(scores, i):
  print "boolean: "+str((scores[0] / float(i)) * 100.0)+"%"
  #print "real:    "+str((scores[1] / float(i)) * 100.0)+"%"
  print "KL:      "+str((scores[2] / float(i)) * 100.0)+"  npredictions: "+str(i)
  #print "raw: "+str(scores[0])
  #print "raw: "+str(scores[1])
  #print "raw: "+str(scores[2])
  #print "npreds: "+str(i)







#builds n-gram models of various sizes from the practical-crypto data. This does NOT build the testing data structures.
def buildNGramModel(fname,n):

  model = {}
  modelSum = 0.0
  gramFile = open(fname,"r")
  outFile = open(fname.replace(".txt","_OUT.txt"),"w+")
  lines = gramFile.readlines()

  # build 1-gram model
  if n == 1:
    for line in lines:
      if len(line) > 5:
        tokens = line.strip().split(" ")
        model[tokens[0]] = float(tokens[1])
        modelSum += float(tokens[1])
    #normalize the model
    for key in model.keys():
      model[key] = -1.0 * math.log(model[key] / modelSum, 2.0)
      #print key+"|"+str(model[key])
      outFile.write(key+"\t"+str(model[key]))

  # all ngram models for n > 1
  elif n > 1:
    for line in lines:  
      if len(line) > 5:
        tokens = line.strip().split(" ")
        prefix = tokens[0][0:n-1]  # prefix is first n-1 chars
        suffix = tokens[0][n-1:]      # suffix is the predicted next char, given n-1 prefix
        if model.get(prefix,-1) == -1:
          model[prefix] = {suffix:float(tokens[1])}
        else:
          model[prefix][suffix] = float(tokens[1])

    # model built, now normalize the CONDITIONAL probabilities for each prefix subset
    for key in model.keys():
      normal = 0.0
      for subkey in model[key].keys():
        normal += model[key][subkey]
      for subkey in model[key].keys():
        val = math.log(model[key][subkey] / normal, 2.0)
        if not math.isnan(val) and not math.isinf(val) and val < 0.0:
          model[key][subkey] = -1.0 * val
        else:
          model[key][subkey] = 20.0
        outFile.write(key+subkey+"\t"+str(model[key][subkey])[0:7]+"\n")
        #print key+"|"+subkey+": "+str(model[key][subkey])
  
  return model
      
#given n lines of text, test and return score
def testRun(uni,bi,tri,quad,penta,lines,lambdas):
  scores = [0.0,0.0,0.0,0.0]
  nPredictions = 1
  for line in lines:
    seq = line.strip()
    length = len(seq)
    i = 0
    while i < length - 5:
      prefix = seq[i:i+4]
      #print "prefix=",prefix,"  actual=",seq[i+4]
      result = predict(uni,bi,tri,quad,penta,prefix,lambdas)  # all predictions based on four-char query
      actual = getLinearProb(uni,bi,tri,quad,penta,prefix+seq[i+4],lambdas)
      scores = score(result,actual,scores)
      #printScores(scores,nPredictions)
      i+=1
      nPredictions += 1.0
      #predict(prefix, lambdas, length)
      #score()
  
  scores[3] = nPredictions
  return scores

#returns some weighted update of a lambda, based on error (+/-) and learning rate
def updateLambda(delta,curLambda,learningRate):
  ret = delta * learningRate + curLambda
  return ret

#some filtered data set of only words, fairly large, but not huge. all words, no numbers of punctuation.
#ofile = open("./filteredOANC.txt","r")
#lines = ofile.readlines()

"""
#build the models from original practical crypto files
fileList = ["./unigrams.txt","./bigrams.txt","./trigrams.txt","./quadgrams.txt","./pentagrams.txt"]
unigram  = buildNGramModel(fileList[0],1)
bigram   = buildNGramModel(fileList[1],2)
trigram  = buildNGramModel(fileList[2],3)
quadgram = buildNGramModel(fileList[3],4)
pentagram = buildNGramModel(fileList[4],5)
"""

fileList = ["./unigrams_OUT.txt","./bigrams_OUT.txt","./trigrams_OUT.txt","./quadgrams_OUT.txt","./pentagrams_OUT.txt"]
uni = buildNGramModelFromFile(fileList[0])
bi = buildNGramModelFromFile(fileList[1])
tri = buildNGramModelFromFile(fileList[2])
quad = buildNGramModelFromFile(fileList[3])
penta = buildNGramModelFromFile(fileList[4])

#print penta
trimModels(uni,bi,tri,quad,penta)

testFile = open("./rawSlate.txt","r")
lines = testFile.readlines()

#uni bi tri quad penta
lambdas = [8.0, 8.0, 8.0, 8.0, 8.0]  # this is the vector we're trying to maximize/optimize
scores  = [0.0,0.0,0.0]
nPredictions = 1.0

j = 0
'''
while True:
  #maxmimizing over raw char sequence, since that's what the models were constructed from
  for line in lines:
    seq = line.strip()
    length = len(seq)
    i = 0
    while i < length - 5:
      prefix = seq[i:i+4]
      #print "prefix=",prefix,"  actual=",seq[i+4]
      result = predict(uni,bi,tri,quad,penta,prefix,lambdas)  # all predictions based on four-char query
      actual = getLinearProb(uni,bi,tri,quad,penta,prefix+seq[i+4],lambdas)
      scores = score(result,actual,scores)
      printScores(scores,nPredictions)
      i+=1
      nPredictions += 1.0
      #predict(prefix, lambdas, length)
      #score()
    
'''


#perceptron optimizer
#init the weights
lambdas = [64.0,64.0,64.0,64.0,64.0]
learnRate = 30
Convergence = 0.04
posConvergence = Convergence / 2.0 #if we're not doing better than this window value for three iterations or so, then fix, and update Vary and Fixed
negConvergence = Convergence / -2.0

Vary = 3
Fixed = 4  #start lambda optimization at models with highest certainty: 5-grams, and vary 4-gram lambda. Then fix 4-gram lambdas, vary 3-gram, etc.

begin = 0
#lastScore = testRun(uni,bi,tri,quad,penta,lines[begin:(begin+200)],lambdas)
#bestPair = (lastScore[0]/lastScore[3],lambdas[Vary])
#print "beginning score:", lastScore
#begin += 200

#dumb. try all 0*k and 2*k in search space
j = 0
lambdas = [32.0,128.0,0.0,0.0,0.0]     #running best:    (0-3.25) 240.0 128.0
#lambdas = [32.0,32.0,32.0,240.0,128.0]     #running best:    (0-3.25) 240.0 128.0
exhaust = []
Vary = 0
Fixed = 1
lambdas[Vary] = 69.0
lambdas = [1.0,2.0,42.66666,3413.33333,1820.4444]     #running best:    (0-3.25) 240.0 128.0
lastScore = testRun(uni,bi,tri,quad,penta,lines[begin:(begin+4000)],lambdas)
#print "size unigrams: ",uni
print "npredictions: ",lastScore[3]
tup = (lastScore[0]/lastScore[3],lambdas[Vary])
print tup
exhaust.append(tup)
while j < 250:
  lastScore = testRun(uni,bi,tri,quad,penta,lines[begin:(begin+1000)],lambdas)
  tup = (lastScore[0]/lastScore[3],lambdas[Vary])
  exhaust.append(tup)
  j += 4
  lambdas[Vary] += 4
  print "curscore: ",exhaust[-1]
  print "lambdas: ",lambdas

sorted(exhaust, key=lambda score : score[0])
print "final scores: ",exhaust
p = raw_input()
#end of dumb

while True:
  
  #until convergence for this "fixed" and this "vary"
  j = 0
  #20000 test predictions
  curScore = testRun(uni,bi,tri,quad,penta,lines[begin:(begin+200)],lambdas)
  if curScore[0]/curScore[3] > bestScore:
    bestScore = curScore[0]/curScore[3]
  begin += 200

  #evaluate delta
  #delta = getDelta(curScore[0],lastScore[0]) #some function returning a weighted measure of error between these scores and the last
  delta = curScore[0]/curScore[3] - lastScore[0]/lastScore[3] 
  print "delta="+str(delta)+" curScore:"+str(curScore[0]/curScore[3] * 100)+"%  lastScore:"+str(lastScore[0]/lastScore[3] * 100)+"%"
  lastScore = curScore

  if delta > posConvergence: #progress: so see if we can do better in this region
    print "old lambda=",str(lambdas[Vary])
    lambdas[Vary] = updateLambda(delta,lambdas[Vary],learnRate)
    print "new lambda=",str(lambdas[Vary])
    c = raw_input()
  elif delta < negConvergence:
    print "old lambda=",str(lambdas[Vary])
    lambdas[Vary] = updateLambda(delta,lambdas[Vary],learnRate)
    print "new lambda=",str(lambdas[Vary])
    c = raw_input()
  else:
    print "convergence? lambdas:",lambdas
    #maybe randomly restart k times, and see if we converge to similar values, then really optimize before fixing



    #finally, somewhere here update Vary and Fixed, once satisfied
    #Vary  -= 1
    #Fixed -= 1
  if Vary == -1:
    print "done"
    while True:
      Vary += 1
      Vary -= 1

  # if delta == 0
  # increment Vary and Fixed
  # else:
  #update Vary +- some error
  #





print "done"
raw_input()


#power of two rotation
increment = 1










