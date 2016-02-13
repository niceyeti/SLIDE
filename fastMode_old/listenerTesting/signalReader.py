from math import sqrt, log




#algorithm for streaming a signal and finding clusters ("events") in a streaming manner, eg, without reading the entire input and then
#performing clustering
# definitions: time is in ticks, although remember the inputs may not really occur in even ticks, due to process swapping of the OS





#euclidean distance for vectors (points) v1 and v2
def dist(v1,v2):
  return sqrt(pow(v1[0] - v2[0],2) + pow(v1[1] - v2[1],2))

#find the mean of some neighborhood of points, when the event trigger is activated
def cluster(begin,end,coorList):
  sumx = 0
  sumy = 0
  ct = 0
  for reading in coorList[begin:end]:
    sumx += reading[0]
    sumy += reading[1]
    ct += 1

  return (sumx/ct,sumy/ct)



ofile = open("signal.txt","r")

coors = []
lines = ofile.readlines()
for line in lines:
  xy = tuple(line.strip().split(" "))
  coors.append(tuple((int(xy[0]),int(xy[1]))))

#for xy in coors:
#  print xy

print "dist: ",dist(coors[100],coors[103]),coors[100],coors[103]



#inputs in memory, now see if we can find clusters in a streaming manner as we read the input
i = 0
theta = 0
while(i < len(coors) - 4): #determine velocity: distance of points three ticks apart
  dx = dist(coors[i],coors[i+3])
  dTheta = getDeltaTheta(getTheta(coors[i],coors[i+3]), theta)  #change in theta from tick(i-1) to tick(i-1) 
  theta = getTheta(coors[i],coors[i+3])  #current theta
  if dx > 0:
    print dx, 1/dx
    #print log(1/dx)
  else:
    print dx
  i += 1





#streaming clustering. still reliant on a tick input, but should be near realtime
dxThreshold = 14  #higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
innerDxThreshold = 16  # a softer theshold once we're in the event state
triggerThreshold = 3   # receive this many trigger before throwing. may also need to correlate these as consecutive triggers.

i = 0
trigger = 0
clusters = []
while(i < len(coors) - 5): #determine velocity: distance of points three ticks apart
  dx = dist(coors[i],coors[i+3])
  if dx < dxThreshold:
    trigger += 1      #could receive n-triggers before fully triggering, to buffer noise; this like using a timer, but event-based
    if trigger >= triggerThreshold:  #trigger event and start collecting event data
      clusterStart = i
      while((i < len(coors) - 5) and dx < innerDxThreshold):        #event state
        dx = dist(coors[i],coors[i+3])
        i += 1
      clusterEnd = i
      clusters.append(cluster(clusterStart,clusterEnd,coors))
      #print log(1/dx)
      trigger = 0  #reset trig counter
  i += 1


print clusters

















