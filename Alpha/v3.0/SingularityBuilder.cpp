#include "SingularityBuilder.hpp"

SingularityBuilder::SingularityBuilder()
{
  //streaming clustering. still reliant on a tick input, but should be near realtime
  dxThreshold = 14;      // higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
  innerDxThreshold = 16;  // a softer theshold once we're in the event state
  triggerThreshold = 3;   // receive this many triggers before throwing. may also need to correlate these as consecutive triggers.
  trigger = 0;
  //inData.reserve(1000);   //32k
  //outData.reserve(64);

  //these must be determined by some outer class with UI-access
  activeRegion_Left = -1;
  activeRegion_Right = 99999;
  activeRegion_Top = -1;
  activeRegion_Bottom = 99999;
}

SingularityBuilder::SingularityBuilder(int left, int right, int top, int bottom)
{
  //streaming clustering. still reliant on a tick input, but should be near realtime
  dxThreshold = 14;      // higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
  innerDxThreshold = 16;  // a softer theshold once we're in the event state
  triggerThreshold = 3;   // receive this many triggers before throwing. may also need to correlate these as consecutive triggers.
  trigger = 0;
  //inData.reserve(1000);   //32k
  //outData.reserve(64);

  //these must be determined by some outer class with UI-access
  activeRegion_Left = left;
  activeRegion_Right = right;
  activeRegion_Top = top;
  activeRegion_Bottom = bottom;
}

SingularityBuilder::~SingularityBuilder()
{
  //nada
}

/*
  Calls test repeatedly for a hard-coded source directory.
  The test files are sequences of x/y mouse coordinates, generated
  elsewhere (I used c-sharp) at 30 or 60 Hz (the sensor frequency).

  These are validation tests, not, uh, code quality tests...
*/
void SingularityBuilder::UnitTests(void)
{
  string prefix = "../FishTrap/nixEncoding/word";
  string suffix = ".txt";
  int i;
  string fname;
  string delim = "\t";

  for(i = 1; i <= 13; i++){
    fname = prefix;
    fname += std::to_string(i);
    fname += suffix;
    TestSingularityBuilder(fname,delim);
  }
}


/*
  Tests the clustering methods of the Singulariy Builder based on a stream of inputs from a file.
*/
void SingularityBuilder::TestSingularityBuilder(const string& testFile, string& fileDelimiter)
{
  vector<Point> testData;
  vector<PointMu> output;

  cout << "Testing inputs from file: " << testFile << endl;
  BuildTestData(testFile,testData,fileDelimiter);
  Process(testData,output);
  PrintOutData(output);
  cout << testFile << " testing complete." << endl;
}


/*
  Technically this returns delta-theta/delta-time, an angular velocity measure.

  Calculate the angular velocity over a given time span, for an event trigger.

  For now, returns the absolute value of deltaTheta, since we're only interested
  in the magnitude of the change to flag state changes (changes in direction).

  Theta is rads, dTheta is rads/tick.
*/

double SingularityBuilder::CalculateDeltaTheta(double theta1, double theta2, int dt)
{
  if(dt == 0){
    cout << "ERROR zero-denominator passed to CalculateDeltaTheta" << endl;
  }

  if(theta1 > theta2){
    return (theta1 - theta2) / (double)dt;
  }

  if(theta1 <= theta2){
    return (theta2 - theta1) / (double)dt;
  }

}

/*
  Calculates the angle between two vectors.

double SingularityBuilder::CalculateTheta(Point p1, Point p2)
{

}
*/


void SingularityBuilder::BuildTestData(const string& fname, vector<Point>& inData, string& fileDelimiter)
{
  int i = 0;
  Point p;
  fstream testFile;
  string line;
  char* x;
  char* y;
  char buf[256];

  testFile.open(fname, ios::in);
  if(!testFile.is_open()){
    cout << "ERROR could not open test file: " << fname << endl;
  }

  while(getline(testFile,line)){
    if(line.length() > 3){
      strncpy(buf,line.c_str(),line.length());
      buf[line.length()] = '\0';
      //cout << "line=" << line << " line.length=" << line.length() << endl;
      //cout << "buf=" << buf << endl;
      i = line.find_first_of(fileDelimiter,0);
      //i = myFind(line.c_str(),fileDelimiter);
      //cout << "i=" << i << " for find() of delim >" << (int)fileDelimiter[0] << "<" << endl;
      if(i != std::string::npos){
        buf[i] = '\0';
        x = buf;
        y = &buf[i+1];
        //cout << "pushing x/y: " << x << "/" << y << endl;
        p.X = atoi(x);
        p.Y = atoi(y);
        inData.push_back(p);
      }
      else{
        cout << "ERROR delimiter >" << fileDelimiter << "< not found in BuildTestData" << endl;
      }
    }
    line.clear();
  }

  testFile.close();
}

bool SingularityBuilder::IsWithinActiveRegion(const Point& p)
{
  if(p.X >= activeRegion_Left && p.X <= activeRegion_Right){
    if(p.Y <= activeRegion_Bottom && p.Y >= activeRegion_Top){
      return true;
    }
  }
  return false;
}


/*
  It will take a lot of test data to figure out what event parameters are even useful, such that we can
  optimize the precision of this function and build a really good state machine for the event triggers.

  For instance, for some test input path zigzag (as a list of xy coordinates in some file), leverage everything:
  build another file of theta, dtheta, velocity, angular acceleration, etc, per that path. Analyze the xy coordinates
  for every possible attribute you might think of, and print these to some other file. Then analyze which of those
  attributes is merely noise, and which corresponds to data that might be useful to event detection.

  You might think of every point in time (each mouse reading, each tick) as a vector, and the current system
  state as a set of k-vectors. Each vector contains a bunch of attributes. A cluster is identified when the
  system (the matrix) suddenly changes in some meaningful way. The matrix analogy is intentional, since it comes from
  control theory, so existing solution undoubtedly exist for this simple problem of geometric key-point detection.

  And use python for these experiments, to avoid the labor overhead of c++.

  Otherwise the highlevel behavior/state machine of this function is simple:
     -detect event
     -capture event
     -process event (clustering)
     -pass event (key-cluster)
     -repeat

*/
void SingularityBuilder::Process(vector<Point>& inData, vector<PointMu>& outData)
{
  short int i, trigger, dx, eventStart, eventEnd;

  //sleep if no data
  for(i = 0; i < inData.size() - 3; i++){
    //ignore points outside of the active region, including the <start/stop> region
    if(IsWithinActiveRegion(inData[i]) && IsWithinActiveRegion(inData[i+3])){
      dx = IntDistance(inData[i],inData[i+3]);

      //debug output, to view how data vals change
      cout << "dx: " << dx;
      if(dx > 0){
        cout << "  1/dx: " << (1 / (float)dx);
      }
      cout << endl;

      if(dx < dxThreshold){  //determine velocity: distance of points three ticks apart
        trigger++;           //receive n-triggers before fully triggering, to buffer noise; this like using a timer, but event-based
        if(trigger >= triggerThreshold){  //trigger event and start collecting event data

          //capture the event
          eventStart = i;
          while(i < (inData.size() - 3) && dx < innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
            dx = IntDistance(inData[i],inData[i+3]);
            i++;
          }
          eventEnd = i;   // exited Event state, so store right bound of the event cluster
          i += 2;

          //get the mean point w/in the event cluster and store it
          PointMu outPoint;
          outPoint.ticks = eventEnd - eventStart;
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          outData.push_back( outPoint );
          trigger = 0;
        } //exit the event-capture state, and continue streaming (reading inData and detecting the next event)
      }
    }
  }

  //dbg
  //PrintOutData(outData);
}

void SingularityBuilder::PrintOutData(vector<PointMu>& outData)
{
  cout << "After SingularityBuilder processing, Singularities:" << endl;
  for(int i = 0; i < outData.size(); i++){
    cout << i << ": " << outData[i].p.X << " " << outData[i].p.Y << " " << outData[i].ticks << endl;
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
void SingularityBuilder::CalculateMean(int begin, int end, const vector<Point>& coorList, PointMu& point)
{
  PointMu mean;
  U16 sumX = 0; //U16 is fine, instead of double, since we're doing pixel math, which is all integer based.
  U16 sumY = 0;
  U16 ct = 0;

  for(int i = begin; i < end && i < coorList.size(); i++){
    sumX += coorList[i].X;
    sumY += coorList[i].Y;
    ct++;
  }

  if(ct > 0){
    point.p.X = sumX / ct;
    point.p.Y = sumY / ct;
  }
  else{
    point.p.X = 0;
    point.p.Y = 0;
  }

  if(end > begin){
    point.ticks = end - begin;
  }
  else{
    cout << "ERROR end < begin in CalculateMean, check bounds and counter rollover  end=" << end << "  begin=" << begin << endl;
  }
}




