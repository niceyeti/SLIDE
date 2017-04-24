import matplotlib.pyplot as plt
from pylab import rcParams


rcParams['figure.figsize'] = 20, 12

"""
biloxiStream is just a python dict with keys: 'Word', 'Y-sequence', and 'X-sequence'. 
The 'sequence' keys are lists of tuples. The index of a tuple in X-sequence is a sensor reading,
and the index is effectively its time 't'. The XSequence holds the tuples in the
form (x, y, stdev-X, stdev-Y).
"""
biloxiStream = eval(open("biloxi.py","r").readlines()[0])

xIndex = 0
yIndex = 1
stdevxIndex = 2
stdevyIndex = 3

###################################################################################
#An example of plotting; here the x-position (from Xsequence) is plotted over time
ys = [tup[xIndex] for tup in biloxiStream["X-sequence"]]
xs = [i for i in range(len(ys))]
plt.plot(xs, ys, color="blue")
plt.savefig("xValues.png")
plt.show()
###################################################################################

