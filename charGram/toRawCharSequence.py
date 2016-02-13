"""
  Converts a corpus to a raw sequence of chars for testing and 
  lambda maximization (maximizing the likelihood of the sequence).

  I keep tokens as words, rather than merging the words into a
  continuous linear sequence. This is because we don't want to maximize
  the char-transitions between words.

"""
import sys
import re


def filterLine(line):
  s = re.sub('[^a-zA-Z]+', '', line)
  #print "s=",s
  return s


ofile = open("./rawSlate.txt","w+")
ifile = open("../../oanc_Slate.txt","r")
lines = ifile.readlines()

for line in lines:
  s = filterLine(line.strip().upper())
  #print "s=",s
  if len(s) > 5:
    ofile.write(s+"\n")


ifile.close()
ofile.close()




