//#include "SingularityBuilder.hpp"
#include "Controller.hpp"

SingularityBuilder::SingularityBuilder()
{
  //streaming clustering. still reliant on a tick input, but should be near realtime
  SetEventParameters(14,16,4);

  //inData.reserve(1000);   //32k
  //outData.reserve(64);

  cout << "ERROR SingularityBuilder default ctor called, with no layout params. Expect failure" << endl;

  //these must be determined by some outer class with UI-access
  activeRegion_Left = 0;
  activeRegion_Right = 1670;
  activeRegion_Top = 0;
  activeRegion_Bottom = 460;
}

SingularityBuilder::SingularityBuilder(int left, int right, int top, int bottom, LayoutManager* layoutManagerPtr)
{
  //streaming clustering. still reliant on a tick input, but should be near realtime
  SetEventParameters(14,16,4);
  
  //inData.reserve(1000);   //32k
  //outData.reserve(64);

  layoutManager = layoutManagerPtr;

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

void SingularityBuilder::SetLayoutManager(LayoutManager* layoutManagerPtr)
{
  layoutManager = layoutManagerPtr;
}

//Set the event parameters.
void SingularityBuilder::SetEventParameters(int dxThresh, int innerDxThresh, int triggerThresh)
{
  dxThreshold = dxThresh;      // higher threshold means more precision, higher density clusters, but with fewer members, lower likelihood of "elbow" effect
  innerDxThreshold = innerDxThresh;  // a softer theshold once we're in the event state
  triggerThreshold = triggerThresh;   // receive this many triggers before throwing. may also need to correlate these as consecutive triggers.
}

void SingularityBuilder::SetUiBoundaries(int left, int right, int top, int bottom)
{
  activeRegion_Left = left;
  activeRegion_Right = right;
  activeRegion_Top = top;
  activeRegion_Bottom = bottom;
}

/*
  Calls test repeatedly for a hard-coded source directory.
  The test files are sequences of x/y mouse coordinates, generated
  elsewhere (I used c-sharp) at 30 or 60 Hz (the sensor frequency).

  These are validation tests, not, uh, code quality tests...
*/
void SingularityBuilder::UnitTests(const string& testDir)
{
  string suffix = ".txt";
  int i;
  string fname;
  string delim = "\t";
  string prefix;

  if(testDir.length() == 0){
    cout << "ERROR testDir null in SingularityBuilder::UnitTests, tests aborted" << endl;
    return;
  }

  if(testDir[testDir.length()-1] != PATH_ESCAPE){
    prefix = testDir;
    prefix += PATH_ESCAPE;
    prefix += "word";
    //testDir += prefix;
    //prefix = testDir;
    //prefix = PATH_ESCAPE + "word";
    //prefix = testDir + prefix;
  }
  else{
    prefix = testDir;
    prefix += "word";
  }

  for(i = 1; i <= 13; i++){
    fname = prefix;
    fname += std::to_string(i);
    fname += suffix;
    cout << "testing " << fname << endl;
    TestSingularityBuilder(fname,delim);
  }
}

/*
  Tests the clustering methods of the Singulariy Builder based on a stream of inputs from a file.
*/
void SingularityBuilder::TestSingularityBuilder(const string& testFile, string& fileDelimiter)
{
  vector<Point> inputData;
  vector<PointMu> outputData;

  cout << "Testing inputs from file: " << testFile << endl;
  BuildTestData(testFile,inputData,fileDelimiter);
  Process(inputData,outputData);
  PrintOutData(outputData);
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
  if(dt <= 0){
    cout << "ERROR zero-denominator passed to CalculateDeltaTheta: dt=" << dt << endl;
  }

  if(theta1 > theta2){
    return (theta1 - theta2) / (double)dt;
  }
  else{ //(theta1 <= theta2){
    return (theta2 - theta1) / (double)dt;
  }
  //return 0.0;
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
  short int ptX, ptY;
  fstream testFile;
  string line;
  char* x = NULL;
  char* y = NULL;
  char buf[256];

  testFile.open(fname, ios::in);
  if(!testFile.is_open()){
    cout << "ERROR could not open test file: " << fname << endl;
    return;
  }

  cout << "Building test input from file: " << fname << " using delimiter ascii# " << (int)fileDelimiter[0] << endl;
  while(getline(testFile,line)){
    if(line.length() > 3 && line.length() < 254){
      strncpy(buf,line.c_str(),line.length());
      buf[line.length()] = '\0';
      //cout << "line=" << line << " line.length=" << line.length() << endl;
      //cout << "buf=" << buf << endl;
      i = line.find_first_of(fileDelimiter,0);
      //i = myFind(line.c_str(),fileDelimiter);
      //cout << "i=" << i << " for find() of delim >" << (int)fileDelimiter[0] << "<" << endl;
      if(i != std::string::npos && i >= 0){
        buf[i] = '\0';
        x = buf;
        y = &buf[i+1];

        //randomizes the input to simulate sensor error
        //ptX = RandomizeVal((short int)atoi(x),10);
        //ptY = RandomizeVal((short int)atoi(y),10);
        ptX = (short int)atoi(x);
        ptY = (short int)atoi(y);

        //cout << "pushing x/y: " << x << "/" << y << endl;
        Point p(ptX,ptY);
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

//simulates +/- input error of sensor shakiness, by adding/subtracting random noise from input
short int SingularityBuilder::RandomizeVal(short int n, short int error)
{
  short int r = (short int)rand() % error;

  if((r % 2) == 0){
    r *= -1;
  }

  return n + r;
}

/*
  Ui coordinates are defined such that the origin (0,0) is the upper left corner of the form,
  at least on the windows box that generated the test inputs.
*/
bool SingularityBuilder::InBounds(const Point& p)
{
  if(p.X >= activeRegion_Left && p.X <= activeRegion_Right){
    if(p.Y <= activeRegion_Bottom && p.Y >= activeRegion_Top){
      return true;
    }
  }
  return false;
}

/*
  Verifies clusters differ by sufficient value (ticks X dist) ("X" is CROSS, not multiply)

  VERY IMPORTANT:
  The motivation here is to have the singularity builder do its best at geometric separability.
  The consumer of this class' output will then evaluate time-based factors (ticks) to determine
  repeat chars. Its very important to divide the geometric vs. time based separators in this fashion,
  at least for a discrete implementation.

  This is a hard threshold filter. Other filters could handle confidence measures for the pointMu's generated.
  Note that some of the event threshold, sampling, and especially the trigger threshold overlap significantly with
  the input/output of this function.
*/
bool SingularityBuilder::MinSeparation(const PointMu& mu1, const PointMu& mu2)
{
  if(mu1.alpha != mu2.alpha){  //TODO: this is redundant with a check in Process(). Oh well.
    if(layoutManager->DoubleDistance(mu1.pt,mu2.pt) < (layoutManager->GetMinKeyDiameter() * 1.5)){
	    if(mu1.ticks <= 4 || mu2.ticks <= 4){  //time separation is INF for now
				cout << "minkeyrad: " << layoutManager->GetMinKeyRadius() << " dist: " << layoutManager->DoubleDistance(mu1.pt,mu2.pt) <<  endl;
				cout << "mindist failed, ticks are (" << mu1.alpha << "," <<  mu1.ticks << ")  (" << mu2.alpha << "," << mu2.ticks << ")" << endl;
				return false;
			}
		}
  }

  return true;
}

/*
  This is slightly hackish. Sometimes the SingularityBuilder identifies two clusters for the same key,
  for example, the user looks at 'A' once, then again slightly off-center.  This behavior should
  also be captured by the clustering parameters. However, it is still little extra cost to do a linear
  scan of the output, merging such possible clusters anyway.

  TODO: Minimize the importance of this function by optimizing the clustering parameters of the sensor.
*/
void SingularityBuilder::MergeClusters(vector<PointMu>& rawClusters, vector<PointMu>& mergedData)
{
  bool lastMerge = false;

  for(int i = 1; i < rawClusters.size(); i++){
    // Only append output clusters which exceeds some inter-key distance
    // Note that this discrimination continues until sufficient inter-cluster distance is achieved.
    if(MinSeparation(rawClusters[i-1],rawClusters[i])){
      mergedData.push_back(rawClusters[i-1]);
    }
    //else, forward-accumulate the reflexive likelihood to preserve repeat char info
    else{
      cout << "merged clusters " << (i-1) << "/" << (i) << endl;
      //identical alphas, so just merge the dupes, for instance, merge "AA" to "A"
      if(rawClusters[i-1].alpha == rawClusters[i].alpha){
        rawClusters[i-1].ticks += rawClusters[i].ticks;
        mergedData.push_back(rawClusters[i-1]);        
      }
      //else, point with more ticks (higher confidence) wins (outcome is same as prior 'if': these two blocks could be merged, but with less clarity
      else if(rawClusters[i-1].ticks > rawClusters[i].ticks){
        rawClusters[i-1].ticks += rawClusters[i].ticks;
        mergedData.push_back(rawClusters[i-1]);
      }
      else{
        rawClusters[i].ticks += rawClusters[i-1].ticks;
        mergedData.push_back(rawClusters[i]);
      }
      i++; //TODO: this advances the index to skip the next comparison. As a result, this method only merges two
           // consecutive 'merge-able' means, when in reality, there may be multiple ones. For that reason,
           // this should really be iteration (when minSep fails). That way successive errors are absorbed.

      //rawClusters[i].ticks += rawClusters[i-1].ticks;
      if(i >= rawClusters.size()-1){
        lastMerge = true;
      }
    }
  }

  //variable detects if last two clusters were merged or not
  if(!lastMerge && !rawClusters.empty()){
    mergedData.push_back(rawClusters[rawClusters.size()-1]);
  }
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

  TODO: recode this using int32, not short. Cast to short where needed.
*/
void SingularityBuilder::Process(vector<Point>& inData, vector<PointMu>& outData)
{
  //char c;
  vector<PointMu> midData;
  int i, trigger, dx, eventStart, eventEnd;

  cout << "processing " << inData.size() << " data points in sb.process()" << endl;

  //TODO: sleep if no data
  trigger = 0;
  for(i = 0; i < inData.size() - 4; i++){
    //ignore points outside of the active region, including the <start/stop> region
    if(InBounds(inData[i]) && InBounds(inData[i+3])){
      dx = layoutManager->IntDistance(inData[i],inData[i+3]);

      /*
      //debug output, to view how data vals change
      cout << "dx: " << dx;
      if(dx > 0){
        cout << "  1/dx: " << (1.0f / (float)dx);
      }
      cout << endl;
      */

      //TODO: advancing the index i below is done without InBounds() checks
      if(dx < dxThreshold){  //determine velocity: distance of points three ticks apart
        trigger++;           //receive n-triggers before fully triggering, to buffer noise; like using a timer, but event-based
        if(trigger >= triggerThreshold){  //trigger event and start collecting event data

          //capture the event
          eventStart = i;
          while(i < (inData.size() - 4) && dx < innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
            dx = layoutManager->IntDistance(inData[i],inData[i+3]);
            i++;
          }
          eventEnd = i;   // exited Event state, so store right bound of the event cluster
          //TODO: below is a small optimization to skip some data points following an event, eg, perhaps by dx difference of 3 data points.
          //i += 2;

          //get the mean point w/in the event cluster and store it
          PointMu outPoint;
          //outPoint.ticks = eventEnd - eventStart;
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          cout << "hit mean" << endl;
          outPoint.alpha = layoutManager->FindNearestKey(outPoint.pt);

          //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
          if(midData.empty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
            midData.push_back(outPoint);
          }
          //verify incoming alpha cluster is unique from previous alpha
          else if(outPoint.alpha != midData[midData.size()-1].alpha){
            midData.push_back(outPoint);
          }

          //reset trigger for next event detection
          trigger = 0;
        } //exit the event-capture state, and continue streaming (reading inData and detecting the next event)
      }
    }
  }

  cout << "clusters before merging..." << endl;
  PrintOutData(midData);
  MergeClusters(midData,outData);
  //dbg
  //PrintOutData(outData);
}

/*

  More advanced event detection, using any attributes that are meaningful (dTheta, coVar(X,Y), etc.).
  -dTheta(eventBegin,eventEnd)
  -coVar(X,Y) (for some last k-inputs: this is like assessing when a set of point converges to a likely mean
  -nKeyHits: num discrete key hits in a single key region; this may be no different than coVar()
  -dist(pt[i],pt[i+]) Distance between points some k-ticks apart

  This starts with analysis, to figure out what parameters look meaningful, by the data
*/
void SingularityBuilder::Process2(vector<Point>& inData, vector<PointMu>& outData)
{
  bool trig = false;
  //char c;
  vector<PointMu> midData;
  int i, sampleRate, segmentWidth, trigger, eventStart, eventEnd, stDevTrigger;
  double dx, dist, lastDist, coVar, stDev, nKeyHits, avgDist, avgTheta, lastTheta, dTheta; 

  //segment width is the sample radius for assessing a trigger
  sampleRate = 1;  //sample every two ticks. Thus, there will be n/2 analyses
  segmentWidth = 3; //every two ticks, grab next four point for analysis
  lastTheta = dTheta = avgTheta = 0.0;

  double stDevThreshold = 30;

  cout << "processing " << inData.size() << " data points in sb.process(), dxThreshold=" << dxThreshold << endl;

  //TODO: sleep if no data
  trigger = 0;
  for(i = 0; i < (inData.size() - segmentWidth - 1); i += sampleRate){
    //ignore points outside of the active region, including the <start/stop> region
    if(InBounds(inData[i]) && InBounds(inData[i+segmentWidth-1])){
      //measures absolute change in distance, a dumb attribute
      lastDist = dist;
      dist = layoutManager->DoubleDistance(inData[i],inData[i+segmentWidth-1]);
      dx = dist - lastDist;
      avgDist = layoutManager->AvgDistance(inData,i,segmentWidth);
      //measures clustering of a group of points, but only the points themselves
      //coVar = layoutManager->CoVariance(inData,i,segmentWidth);
      stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth); //(sigmaX*sigmaY)^2

      //only update theta when on the move (bounds should be the same as event capture)
      if(stDev > 100){ //stop capturing a little ahead of event capture. capture only at med/hi velocity
		    lastTheta = avgTheta;
		    avgTheta = layoutManager->AvgTheta(inData,i,segmentWidth);  //get the avg direction for a sequence of points
		    dTheta = (avgTheta - lastTheta) / 2.0;
      }
      else{
        //avgTheta = 0.0;
        dTheta = 0.0;
      }
      //dTheta = layoutManager->CosineSimilarity(inData,i,segmentWidth); // record change in angle or two sequences of points
      //correlates points with nearest key. Something like this is important to map kays to points, instead of points to keys... get creative.
      //cout << "(x,y) (" << inData[i].X << "," << inData[i].Y << ")" << endl;
      //cout << "stDev, dx, 1/dx, coVar, 1/coVar:  " << (int)stDev << "  " << (int)dx << "  " << (dx == 0 ? 0 : (1/dx)) << "  " << (int)coVar << "  " << (coVar == 0 ? 0 : (1/coVar)) << endl;
      //cout << "pt. stDev, avgDist, dx, avgTheta:  " << (int)stDev << "  " << avgDist << " " << (int)dx << " " << avgTheta << endl;
      printf("pt. stDev, avgDist, dist, dx, avgTheta, dTheta: %9.3f  %9.3f  %9.3f  %9.3f  %9.3f  %9.3f\n",stDev,avgDist,dist,dx,avgTheta,dTheta);
      //cout << "stDev:  " << (int)stDev << endl;

      if(stDev < stDevThreshold){
        stDevTrigger++;
        if(stDevTrigger > 2){
          eventStart = i;
          //event triggered, so gather event data
          i++;
          //stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
          while(i < inData.size() && stDev < stDevThreshold){
            stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
            printf("pt. stDev, avgDist, dist, dx, avgTheta, dTheta: %9.3f\n",stDev);
            i++;
          }
          eventEnd = i;
          i--;

          PointMu outPoint;
          outPoint.ticks = eventEnd - eventStart;
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          outPoint.alpha = layoutManager->FindNearestKey(outPoint.pt);
          cout << "hit mean for " << outPoint.alpha << endl;

          //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
          if(midData.empty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
            midData.push_back(outPoint);
          }
          //verify incoming alpha cluster is unique from previous alpha
          else if(outPoint.alpha != midData[midData.size()-1].alpha){
            midData.push_back(outPoint);
          }
          stDevTrigger = 0;
        }
      }


/*
      if(!trig && dx < dxThreshold){
        cout << "Trigger!" << endl;
        trig = true;
      }
      else{
        trig = false;
      }
*/

      /*
      //debug output, to view how data vals change
      cout << "dx: " << dx;
      if(dx > 0){
        cout << "  1/dx: " << (1.0f / (float)dx);

      }
      cout << endl;
      */
      /*
      //TODO: advancing the index i below is done without InBounds() checks
      if(dx < dxThreshold){  //determine velocity: distance of points three ticks apart
        trigger++;           //receive n-triggers before fully triggering, to buffer noise; like using a timer, but event-based
        if(trigger >= triggerThreshold){  //trigger event and start collecting event data

          //capture the event
          eventStart = i;
          while(i < (inData.size() - 4) && dx < innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
            dx = layoutManager->IntDistance(inData[i],inData[i+3]);
            i++;
          }
          eventEnd = i;   // exited Event state, so store right bound of the event cluster
          //TODO: below is a small optimization to skip some data points following an event, eg, perhaps by dx difference of 3 data points.
          //i += 2;

          //get the mean point w/in the event cluster and store it
          PointMu outPoint;
          outPoint.ticks = eventEnd - eventStart;
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          cout << "hit mean" << endl;
          outPoint.alpha = layoutManager->FindNearestKey(outPoint.pt);

          //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
          if(midData.empty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
            midData.push_back(outPoint);
          }
          //verify incoming alpha cluster is unique from previous alpha
          else if(outPoint.alpha != midData[midData.size()-1].alpha){
            midData.push_back(outPoint);
          }

          //reset trigger for next event detection
          trigger = 0;
        } //exit the event-capture state, and continue streaming (reading inData and detecting the next event)
      }
      */
    }
  }

  cout << "clusters before merging..." << endl;
  PrintOutData(midData);
  MergeClusters(midData,outData);
  //dbg
  //PrintOutData(outData);
}

/*
  See "change detection" wikis and http://people.irisa.fr/Michele.Basseville/

  The letter-detection needs to be done online, but offline may be doable as well.

  Implements an event oriented state machine in which it is easier to exit states than enter them.
  This helps detect the confounded edge between neighbor-key transitions, a difficult case.
  The only event parameter is stDev; other attributes tend to lead stDev, so a better trigger function
  could probably be devised.
*/
void SingularityBuilder::Process3(vector<Point>& inData, vector<PointMu>& outData)
{
  bool trig = false;
  //char c;
  vector<PointMu> midData;
  int i, sampleRate, segmentWidth, trigger, eventStart, eventEnd, stDevTrigger;
  double dDist, dist, dydx, lastDist, coVar, stDev, nKeyHits, avgDist, avgTheta, lastTheta, dTheta; 
  char currentAlpha = '!', prevAlpha = '!';

  //segment width is the sample radius for assessing a trigger
  sampleRate = 1;  //sample every two ticks. Thus, there will be n/2 analyses
  segmentWidth = 3; //every k ticks, grab next k point for analysis
  lastTheta = dTheta = avgTheta = 0.0;

  int triggerThreshold = 4;
  int highFocus = 45;
  double stDev_HardTrigger = 225;
  double stDev_SoftTrigger = 14;

  cout << "processing " << inData.size() << " data points in sb.process(), dxThreshold=" << dxThreshold << endl;

  //TODO: sleep if no data
  trigger = 0;
  for(i = 0; i < (inData.size() - segmentWidth - 1); i += sampleRate){
    //ignore points outside of the active region, including the <start/stop> region
    if(InBounds(inData[i]) && InBounds(inData[i+segmentWidth-1])){
      //measures absolute change in distance, a dumb attribute
      //lastDist = dist;
      //dist = layoutManager->DoubleDistance(inData[i],inData[i+segmentWidth-1]);
      //dDist = dist - lastDist;
      //dydx = layoutManager->DyDx(inData[i],inData[i+segmentWidth-1]); //slope
      //dydx = layoutManager->AvgDyDx(inData,i,segmentWidth);
      //avgDist = layoutManager->AvgDistance(inData,i,segmentWidth);
      //measures clustering of a group of points, but only the points themselves
      //coVar = layoutManager->CoVariance(inData,i,segmentWidth);
      stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth); //(sigmaX*sigmaY)^2
	    //lastTheta = avgTheta;
	    //avgTheta = layoutManager->AvgTheta(inData,i,segmentWidth);  //get the avg direction for a sequence of points
	    //dTheta = (avgTheta - lastTheta) / 2.0;
      //printf("stDev,avgDist,dDist,dydx,avgTheta,dTheta: %9.3f  %9.3f  %9.3f  %9.3f  %9.3f  %9.3f\n",stDev,avgDist,dDist,dydx,avgTheta,dTheta);
      currentAlpha = layoutManager->FindNearestKey(inData[i]);

      if(stDev < stDev_HardTrigger){
        printf("curAlpha, prevAlpha, pt-stDev: %c  %c  %9.3f  %d  %d  pretrig\n",currentAlpha,prevAlpha,stDev,inData[i].X,inData[i].Y);
      }
      else{
        printf("curAlpha, prevAlpha, pt-stDev: %c  %c  %9.3f  %d  %d\n",currentAlpha,prevAlpha,stDev,inData[i].X,inData[i].Y);
      }
      /*
        Implements an event oriented state machine in which it is easier to exit states than enter them.
        This helps detect the confounded edge between neighbor-key transitions, a difficult case.
      */
      if(stDev < stDev_HardTrigger){

        //gives the trigger faster progress when confidence is higher; there ought to be a more continuous way to do this
        if(stDev < highFocus){
          trigger += 2;
        }
        else{
          trigger++;
        }
        
        //trigger and collect event
        if(trigger > triggerThreshold){
          cout << "trig" << endl;
          //this optimistically assumes current position has (intentional) focus on some key
          prevAlpha = currentAlpha = layoutManager->FindNearestKey(inData[i]);
          eventStart = i;
          i++;
          stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
          //hold state, unless there is a stDev change and an alpha change. Only if both alpha changes and stDev throws, will we exit.
          // A new event will immediately be thrown to catch the neighbor key event.
          while(i < inData.size() && (stDev < stDev_SoftTrigger || prevAlpha == currentAlpha) && (stDev < stDev_HardTrigger)){
          //while(i < inData.size() && (stDev < stDev_SoftTrigger && prevAlpha == currentAlpha) && (stDev < stDev_HardTrigger)){
          //while(i < inData.size() && (stDev < stDev_SoftTrigger || prevAlpha == currentAlpha) && (stDev < stDev_HardTrigger)){
            stDev = layoutManager->CoStdDeviation(inData,i,segmentWidth);
            currentAlpha = layoutManager->FindNearestKey(inData[i]);
            printf("curAlpha, prevAlpha, pt-stDev: %c  %c  %9.3f  trig\n",currentAlpha,prevAlpha,stDev);
            i++;
          }
          //exit state either by hard/fast exit, or soft-exit to an adjacent key
          eventEnd = i;
          i--;

          //capture event data and push it, then continue monitoring for new ones. Event is pushed only if unique from previous one..
          PointMu outPoint;
          outPoint.ticks = eventEnd - eventStart + trigger; //ticks can be used as a confidence measure of the event
          CalculateMean(eventStart,eventEnd,inData,outPoint);
          outPoint.alpha = layoutManager->FindNearestKey(outPoint.pt);
          cout << "hit mean for " << outPoint.alpha << " ticks: " << outPoint.ticks << endl;
          trigger = 0;
          //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
          if(midData.empty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
            midData.push_back(outPoint);
          }
          //verify incoming alpha cluster is unique from previous alpha; accumulate ticks if not
          else if(outPoint.alpha == midData[midData.size()-1].alpha){
            midData[midData.size()-1].ticks += outPoint.ticks;
          }
          else{
            midData.push_back(outPoint);
          }
        }
      }
      else{
        trigger = 0;
      }
    }
  }

  cout << "clusters before merging..." << endl;
  PrintOutData(midData);
  MergeClusters(midData,outData);
  //dbg
  //PrintOutData(outData);
}




void SingularityBuilder::PrintInData(vector<Point>& inData)
{
  cout << "Input data to SingularityBuilder:" << endl;
  for(int i = 0; i < inData.size(); i++){
    cout << i << ": " << inData[i].X << " " << inData[i].Y << endl;
  }
}

void SingularityBuilder::PrintOutData(vector<PointMu>& outData)
{
  cout << "There are " << outData.size() << " (x,y,ticks,alpha) Singularities/Means:" << endl;
  for(int i = 0; i < outData.size(); i++){
    cout << i << ": " << outData[i].pt.X << " " << outData[i].pt.Y << " " << outData[i].ticks << " " << outData[i].alpha << endl;
  }
}

/*
  Clusters are linear/time-based, so essentially every Point also has a time value, giving 3-d vectors.
  We could use a 3-d distance function to help separate these, using the tick input to separate clusters
  in similar 2-d regions which are separated by time-step(s) t. Especially since time ticks are monotonic--
  does this lend to a separable input, or take away?

  Here I'll cluster in 2-d, then subdivide the clusters by time; if some sub-cluster has a linear/time width
  of less than some threshold, then eliminate it as a cluster. Then sort the clusters by time to get the ordered
  sequence (of characters).

  The cluster assignment is quadratic, since every point searches the entire mean space. But presumably its
  actually more logarithmic, since the size of the mean set decreases with each iteration. Thus the first clustering
  iteration is O(n^2), but each successive iteration searches a much smaller mean set. So its more like O(log(n)*n^2).
  This doesn't look good, but since there are generally at most a few hundred points, and since the log(n) terms is probably
  bounded much lower, s'not so bad, like log*(n)*n^2.

  This version of clustering fits points to clusters, rather than clusters to points, as k-means does.
  Results: In its raw form, this clustering method settles unsatisfactorily; typically an input of 200 points
  yields 70-90 clusters. This is because this method really just find the "leafmost" clusters for which
  all members of the cluster are closer to one another than any other points. Thus it finds the lowest point of 
  the system at this point. Workarounds start to just approximate the event-based, streaming clustering method,
  of triggering cluster-detection once some score for a segment of last k-input points crosses some binary threshold.

  You could run this recursively on its own output (once the means converge), the regroup means, etc. This would
  essentially result in an MST of the points. Similarly, you could still process the linear sequence of points
  assigning graph-edges to nearest points for each point, within some search radius. The, re-scan the sequence
  and find the most strongly-connected points to yield the means. This strategy arises by observing that a point
  with maney neighbors will always be the most connected point (the most edges), signaling the presence of a cluster,
  or even the approximation of the mean itself.

  The problem with using k-means is that it still requires approximating the best k value, or even running
  k-means for 1-n and analyzing when the output clusters have the strongest density (or other intra/inter-cluster
  metrics).

*/
void SingularityBuilder::SimpleClustering(vector<Point>& inData, vector<PointMu>& outData)
{
  int i, j, ct;
  U32 key, nearest;
  //Point nearest;
  double muX, muY, dist, minDist;
  vector<U32> means;  //vector's almost always faster than list for small list sizes and small objects, even for sort
  map<U32,vector<Point> > clusters;
  map<U32,vector<Point> >::iterator it;

  means.reserve(inData.size());

  //init all points as means
  for(i = 0; i < inData.size(); i++){
    key = ((U32)inData[i].X << 16) | (U32)inData[i].Y;
    clusters[key].push_back(inData[i]);
    //clusters[key].first = key;
  }

  for(ct = 0; ct < 20; ct++){
    cout << "iteration " << ct << ", means: " << clusters.size() << endl;
    //assign all points to their nearest neighbor, w/in some tick radius of eachother
    //the purpose of the tick radius is not to divide clusters, but just to limit the search distance; no
    //point can be in the same cluster as a point that is k-ticks away
    for(i = 0; i < inData.size(); i++){
      //find this point's nearest mean
      minDist = 9999;
      key = (U32)inData[i].X << 16 | (U32)inData[i].Y;
      for(it = clusters.begin(); it != clusters.end(); ++it){
        if(it->first != key){  //verifies we dont compare point with itself
          dist = layoutManager->DoubleDistance(inData[i],Point((short int)(it->first & 0xFFFF0000),(short int)(it->first & 0x0000FFFF)));
          if(minDist > dist){
            nearest = it->first;
            minDist = dist;
          }
        }
      }
      //key = (U32)nearest.X | (U32)nearest.Y;
      clusters[nearest].push_back(inData[i]);
    }

    //calculate the new means within each cluster
    means.clear(); //clear the old means
    for(it = clusters.begin(); it != clusters.end(); ++it){
      muY = muX = 0.0;
      for(i = 0; i < it->second.size(); i++){
        muX += it->second[i].X;
        muY += it->second[i].Y;
      }
      muX /= (double)i; //TODO: div zero checks
      muY /= (double)i;
      key = ((short int)muX << 16) | ((short int)muY & 0x0000FFFF);
      means.push_back(key);
      cout << it->second.size() << " ";
    }
    cout << endl;
    clusters.clear();

    //copy in the new means
    for(i = 0; i < means.size(); i++){
      //key = ((U32)means[i].X << 16) | (U32)means[i].Y;
      if(clusters.find(means[i]) == clusters.end()){
        clusters[means[i]];
      }
    }
  }

  cout << "cluster sizes: " << endl;
  for(it = clusters.begin(); it != clusters.end(); ++it){
      cout << it->second.size() << " ";
  }
  cout << endl;

  cout << "clusters (unordered): " << endl;
  for(it = clusters.begin(); it != clusters.end(); ++it){
    if(it->second.size() > 4){
      Point p;
      p.X = (short int)((it->first >> 16) & 0x0000FFFF);
      p.Y = (short int)(it->first & 0x0000FFFF);
      cout << layoutManager->FindNearestKey(p);
    }
  }
  cout << endl;
}

/*
  A clustering based method of event detection. This assumes all the data is known in advance, although
  streaming clustering algorithms do exist.
*/
void SingularityBuilder::Process4(vector<Point>& inData, vector<PointMu>& outData)
{
  SimpleClustering(inData,outData);
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
void SingularityBuilder::CalculateMean(int begin, int end, const vector<Point>& coorList, PointMu& pointMean)
{
  U32 sumX = 0; //ints are fine, since pixel math is all integer based
  U32 sumY = 0;
  U32 ct = 0;

  for(int i = begin; i < end && i < coorList.size(); i++){
    //TODO: bounds checking (a state machine, really) needs to be defined in ui testing. This check is redundant, poorly factored, w.r.t. other checks
    if(InBounds(coorList[i])){
		  sumX += (U32)coorList[i].X;
		  sumY += (U32)coorList[i].Y;
		  ct++;
    }
  }

  if(ct > 0){
    pointMean.pt.X = sumX / ct;
    pointMean.pt.Y = sumY / ct;
  }
  else{
    cout << "ERROR ct==0 in CalculateMean ??" << endl;
    pointMean.pt.X = 0;
    pointMean.pt.Y = 0;
  }

  if(end > begin){
    pointMean.ticks = end - begin;
  }
  else{
    cout << "ERROR end < begin in CalculateMean, check bounds and counter rollover  end=" << end << "  begin=" << begin << endl;
  }
}




