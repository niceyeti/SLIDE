#include "signal.hpp"

SingularityBuilder::SingularityBuilder()
{
  //streaming clustering. still reliant on a tick input, but should be near realtime
  dxThreshold = 14;      // higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
  innerDxThreshold = 16;  // a softer theshold once we're in the event state
  triggerThreshold = 3;   // receive this many trigger before throwing. may also need to correlate these as consecutive triggers.
  trigger = 0;
  inData.reserve(1000);   //32k
  outData.reserve(64);
}

SingularityBuilder::~SingularityBuilder()
{
  //nada
}

/*

 //ENHANCEMENTS

  Calculate the angular acceleration over a given time span.

  For now, returns the absolute value of deltaTheta, since we're only interested
  in the magnitude of the change to flag state changes (changes in direction).

  Theta is rads, dTheta is rads/tick.

double SingularityBuilder::CalculateDeltaTheta(double t1, double t2, int time)
{
  if(time == 0){
    cout << "ERROR zero-denominator passed to CalculateDeltaTheta" << endl;
  }

  if(t1 > t2){
    return (t1 - t2) / (double)time;
  }

  if(t1 <= t2){
    return (t2 - t1) / (double)time;
  }

}

/*
  Calculates the angle between two vectors.

double SingularityBuilder::CalculateTheta(Point p1, Point p2)
{

}
*/


void SingularityBuilder::BuildTestData(const string& fname, vector<Point>& inData)
{
  int i;
  Point p;
  fstream testFile;
  string line;
  char* x;
  char* y;
  char buf[256];

  testFile.open(fname, ios::in);
  if(!testFile.is_open()){
    cout << "ERROR could not open test file" << endl;
  }

  while(getline(testFile,line)){
    strcpy(buf,line.c_str());
    i = (int)line.find_first_of(' ');
    buf[i] = '\0';
    x = buf;
    y = &buf[i+1];
    cout << "pushing x/y: " << x << "/" << y << endl;
    p.X = atoi(x);
    p.Y = atoi(y);
    inData.push_back(p);
  }
}

void SingularityBuilder::Process(vector<Point>& inData, vector<PointMu>& outData)
{
  int i, trigger, dX, eventStart, eventEnd;

  //sleep if no data
  for(int i = 0; i < inData.size() - 3; i++){
    dX = IntDistance(inData[i],inData[i+3]);

    //debug output, to view how data vals change
    if(dX > 0){
      cout << "dX: " << dX << "  1/Dx: " << (1 / dX) << endl;    
    }

    if(dX < dxThreshold){  //determine velocity: distance of points three ticks apart
      trigger++;           //could receive n-triggers before fully triggering, to buffer noise; this like using a timer, but event-based
      if(trigger >= triggerThreshold){  //trigger event and start collecting event data

        //capture the event
        eventStart = i;
        while(i < inData.size() - 3 && dX < innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
          dX = IntDistance(inData[i],inData[i+3]);
          i++;
        }
        eventEnd = i;   // exited Event state, so store right bound of the event cluster

        //get the mean point w/in the event cluster and store it
        Point p = CalculateMean(eventStart,eventEnd,inData);
        outData.push_back( p );
        trigger = 0;
      }
    }
  }

  //dbg
  PrintOutData(outData);
}

void SingularityBuilder::PrintOutData(vector<PointMu>& outData)
{
  cout << "After SingularityBuilder processing, Singularities:" << endl;
  for(int i = 0; i < outData.size(); i++){
    cout << i << ": " << outData[i].X << " " << outData[i].Y << endl;
  }
}


/*
  Takes a list of points, a begin and end index for some estimate point cluster,
  and returns the mean point of that cluster.

  Notes: This likely suffers what I call the "elbow" problem. Most "clusters" occur at a sharp angle.
  This means that the mean value of that cluster (including both edges of the angle) will be inside
  the angle, instead of very near the point. But we're after the point. We could fix this error later,
  if its even a problem--it mainly depends on the resolution (in Hz) of the sensor. Its also an argument
  for more geometrically-based point analysis, as opposed to this, which is essentially cluster oriented.
*/
Point SingularityBuilder::CalculateMean(int begin, int end, const vector<Point>& coorList)
{
  Point mean;
  U16 sumX = 0; //U16 is fine, instead of double, since we're doing pixel math, which is all integer based.
  U16 sumY = 0;
  U16 ct = 0;

  for(int i = begin; i < end && i < coorList.size(); i++){
    sumX += coorList[i].X;
    sumY += coorList[i].Y;
    ct++;
  }

  if(ct > 0){
    mean.X = sumX / ct;
    mean.Y = sumY / ct;
  }
  else{
    mean.X = 0;
    mean.Y = 0;
  }

  return mean;
}




