import matplotlib.pyplot as plt
from pylab import rcParams


rcParams['figure.figsize'] = 10, 6

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

yys = [tup[yIndex] for tup in biloxiStream["X-sequence"]]
yxs = [i for i in range(len(yys))]

stdevXys = [tup[stdevxIndex] for tup in biloxiStream["X-sequence"]]
xstdevXys = [i for i in range(len(stdevXys))]

stdevYys = [tup[stdevyIndex] for tup in biloxiStream["X-sequence"]]
ystdevYys = [i for i in range(len(stdevYys))]

x, = plt.plot(xs, ys, color="blue", label = 'X-Position')
#plt.savefig("xValues.png")

y, = plt.plot(yxs, yys, color="g", label = 'Y-Position')
#plt.savefig("yValues.png")

dx, = plt.plot(xstdevXys, stdevXys, color="r", label = 'Stdev X-Position')
#plt.savefig("stdev xValues.png")

dy, = plt.plot(ystdevYys, stdevYys, color="y", label = 'Stdev Y-Position')
#plt.savefig("stdev yValues.png")

# plt.xlables("time")
# plt.ylables("position")
plt.legend(handles = [x, y, dx, dy])
plt.xlabel('Time, 50Hz resolution' )
plt.suptitle('All Values',fontweight="bold",fontsize=12)
plt.savefig('xydxdy.png')

plt.figure()
plt.plot(xs, ys, color="blue")
plt.xlabel('Time, 50Hz resolution' )
plt.ylabel("X Position")
plt.suptitle('Cursor X-Displacement',fontweight="bold",fontsize=12)
plt.savefig("xValues.png")

plt.figure()
plt.plot(yxs, yys, color="g")
plt.xlabel('Time, 50Hz resolution' )
plt.ylabel("Y Position")
plt.suptitle('Cursor Y-Displacement',fontweight="bold",fontsize=12)
plt.savefig("yValues.png")

plt.figure()
plt.plot(xstdevXys, stdevXys, color="r")
plt.xlabel('Time, 50Hz resolution' )
plt.ylabel("Stdev Y")
plt.suptitle('Y-Displacement Standard Deviation',fontweight="bold",fontsize=12)
plt.savefig("stdev xValues.png")

plt.figure()
plt.plot(ystdevYys, stdevYys, color="y")
plt.xlabel('Time, 50Hz resolution' )
plt.ylabel("Stdev X")
plt.suptitle('X-Displacement Standard Deviation',fontweight="bold",fontsize=12)
plt.savefig("stdev yValues.png")

plt.show()


###################################################################################

