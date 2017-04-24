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

yys = [tup[yIndex] for tup in biloxiStream["X-sequence"]]
yxs = [i for i in range(len(yys))]

stdevXys = [tup[stdevxIndex] for tup in biloxiStream["X-sequence"]]
xstdevXys = [i for i in range(len(stdevXys))]

stdevYys = [tup[stdevyIndex] for tup in biloxiStream["X-sequence"]]
ystdevYys = [i for i in range(len(stdevYys))]

x, = plt.plot(xs, ys, color="blue", label = 'xValues')
#plt.savefig("xValues.png")

y, = plt.plot(yxs, yys, color="g", label = 'yValues')
#plt.savefig("yValues.png")

dx, = plt.plot(xstdevXys, stdevXys, color="r", label = 'stdev xValues')
#plt.savefig("stdev xValues.png")

dy, = plt.plot(ystdevYys, stdevYys, color="y", label = 'stdev yValues')
#plt.savefig("stdev yValues.png")

# plt.xlables("time")
# plt.ylables("position")
plt.legend(handles = [x, y, dx, dy])
plt.xlabel('time')
plt.ylabel("position")
plt.savefig('xydxdy.png')

plt.figure()
plt.plot(xs, ys, color="blue")
plt.xlabel('time')
plt.ylabel("position")
plt.title('xValues')
plt.savefig("xValues.png")

plt.figure()
plt.plot(yxs, yys, color="g")
plt.xlabel('time')
plt.ylabel("position")
plt.title('yValues')
plt.savefig("yValues.png")

plt.figure()
plt.plot(xstdevXys, stdevXys, color="r")
plt.xlabel('time')
plt.ylabel("position")
plt.title('stdev xValues')
plt.savefig("stdev xValues.png")

plt.figure()
plt.plot(ystdevYys, stdevYys, color="y")
plt.xlabel('time')
plt.ylabel("position")
plt.title('stdev yValues')
plt.savefig("stdev yValues.png")

plt.show()


###################################################################################

