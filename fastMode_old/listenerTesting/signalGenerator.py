#!/usr/bin/python
from Xlib import display

#This generates sample signals and outputs them to a file. we can simulate real-time this way, but clearly it isn't.
#but the file still contains the geometric encoding to test the basics of the algorithms.
#input was <start>elephant<end>


ofile = open("signal.txt","w+");
coorList = []


#dumb time-based input gathering: gather n inputs, then write to file. Gives us a sample cursor path "signal".
while len(coorList) < 200:
  c = display.Display().screen().root.query_pointer()._data
  #s = str(c)
  s = str(c["root_x"])+" "+str(c["root_y"])+"\n"
  #tup = str((c["root_x"]),str(c["root_y"]))
  #print c["root_x"], c["root_y"]
  coorList.append(s)
  #print tup[0], tup[1]

ofile.writelines(coorList)

#for line in coorList:
#  ofile.write(line)

ofile.close()








