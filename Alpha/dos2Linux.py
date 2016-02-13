import sys

for fname in sys.:
  ifile = open(file,"r")
  lines = ifile.readlines()
  ifile.close()
  outlines = []
  for line in lines: 
    outlines.append(line.strip()+"\n")
  ofile = open("./nixEncoding/"+fname,"w")
  
  ofile.close()




