from math import sqrt, log, atan2




#algorithm for streaming a signal and finding clusters ("events") in a streaming manner, eg, without reading the entire input and then
#performing clustering
# definitions: time is in ticks, although remember the inputs may not really occur in even ticks, due to process swapping of the OS


# get the angular velocity between two sample points. Likely the points should be some selection (average, segment, etc), to cut noise
#def deltaTheta(v1, v2):


#return the theta between two x and y vectors, and map to a euclidean space
def getTheta(p1, p2):
  dx = p1[0] - p2[0]
  dy = p1[1] - p2[1]
  
  theta = atan2(dy,dx) * (360.0 / (2 * 3.141592))
  #print "theta=", theta
  return theta



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



ofile = open("writeablesignal.txt","r")

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
theta = 0.0
while(i < len(coors) - 4): #determine velocity: distance of points three ticks apart
  dx = dist(coors[i],coors[i+1])
  #dTheta = getDeltaTheta(getTheta(coors[i],coors[i+3]), theta)  #change in theta from tick(i-1) to tick(i-1) 
  theta = getTheta(coors[i],coors[i+1])  #current theta
  if dx > 40.0:
    print "theta=",theta,"dx=",dx,"p1=",coors[i]," p2=",coors[i+1]
  """
    if dx > 0:
      print dx, 1/dx
      #print log(1/dx)
    else:
      print dx
  """
  i += 1





#streaming clustering. still reliant on a tick input, but should be near realtime
dxThreshold = 14  #higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
innerDxThreshold = 16  # a softer theshold once we're in the event state
triggerThreshold = 3   # receive this many triggers before throwing. may also need to correlate these as consecutive triggers.

# METHOD 1: velocity/distance clustering
# this method is purely distance/velocity based. It works well and supports streaming, but a purely geometric approach is desirable.
# Some mixture of this model and a geometric model is desirable, since the geometric model will fail for keys  that fall on a line.
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


# METHOD 2: pure geometry
# 
i = 0
trigger = 0
dTheta = 0.0
clusters = []
while(i < len(coors) - 5):
  dx = dist(coors[i],coors[i+3]) / 3.0 # this is a velocity, as long as you model constant-size time slices between dists
  dTheta = getTheta(coors[i],coors[i+1])
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


















