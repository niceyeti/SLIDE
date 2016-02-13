#include "Controller.hpp"

LayoutManager::LayoutManager()
{
  layoutWidth = 0;
  layoutHeight = 0;

  cout << "ERROR default ctor building LayoutManager. This will fail!" << endl;
}

LayoutManager::LayoutManager(const string& keyFileName)
{
  cout << "Building ui-key map from file " << keyFileName << "..." << endl;
  BuildKeyMap(keyFileName);
  PrintKeyMap();
}

LayoutManager::~LayoutManager()
{
  keyMap.clear();
}

//Finds the avg direction for a sequence of vectors. This is given by the average of the normalized components
// StackOverflow: Compute unit vectors from the angles and take the angle of their average.
// Avg is over the velocity vectors <dx,dy> for that point-set. So if there are n points, then there are n-1 direction/velocity vectors in the average.
double LayoutManager::AvgTheta(vector<Point>& inData, int start, int npts)
{
  //double xSum, ySum, len, muTheta;
  double muTheta, vX, vY; //velocity vector components

  muTheta = 0.0; 
  //get the normalized vectors of this sequence of vectors
  for(int i = start; i < inData.size() && i < (start+npts); i++){
    vX = inData[i+1].X - inData[i].X;
    vY = inData[i+1].Y - inData[i].Y;

    if(vX != 0.0){
      muTheta += atan2(vY,vX);
    }/*
    if(vY == 0.0){
      muTheta += 1.571; //pi/2
    }*/

    //len = VecLength(inData[i]);
    //xSum += (len)inData[i].X / len;  //TODO: div Zero checks!!
    //ySum += (len)inData[i].Y / len;
  }
  muTheta /= (double)npts;  //TODO div zero check

/*
  //get the average x,y vals  
  xSum /= (double)i;
  ySum /= (double)i;

  if(ySum > 0.0){
    ret = atan2(xSum,ySum);
  }
  else{
    cout << "ERROR ySum zero in AvgTheta" << endl;
    ret = 0.0;
  }
*/
  return muTheta;
}

//averages dydx over some segment of a point sequence
double LayoutManager::AvgDyDx(vector<Point>& pts, int start, int npts)
{
  double avgDyDx = 0.0;

  for(int i = start; i < (start+npts) && i < pts.size(); i++){
    avgDyDx += DyDx(pts[i],pts[i+1]);
  }
  avgDyDx /= (double)npts;

  return avgDyDx;
}

double LayoutManager::DyDx(const Point& p1, const Point& p2)
{
  double dy = p2.Y - p1.Y;
  double dx = p2.X - p1.X;

  if(dx != 0.0){
    return dy/dx;
  }
  else{
    return 0.0;
  }
}



//  |V|
double LayoutManager::VecLength(double x, double y)
{
  if(x == 0 && y == 0){
    return 0.0;
  }

  return pow((pow(x,2.0) + pow(y,2.0)),0.5);
}

//  V1 dot V2
double LayoutManager::DotProduct(const Point& v1, const Point& v2)
{
  return (v1.X * v2.X + v1.Y * v2.Y);
}


/*
  This is like a delta-Theta value for two sequences of coordinates. Each sequence
  is a vector of (x,y) pairs, and thus a vector of vectors. To compare two such vectors,
  here I used cosine similarity (look it up) to gain an attribute resembling delta-Theta.

  Thus, given two segments of length l from the sensor stream, compare their relative angle to one another.
*/
double LayoutManager::CosineSimilarity(const Point& v1, const Point& v2)
{
  int i;

  double dotProd = DotProduct(v1,v2);
  double lenProd = VecLength(v1.X,v1.Y) * VecLength(v2.X,v2.Y);

  if(lenProd > 0.0){
    return dotProd / lenProd;
  }
  else{
    cout << "ERROR div-zero in cosSim: " << lenProd << endl;
    return 0.0;
  }

  /*cosine sim is ( v1 dot v2 ) div ( |v1||v2| )
  for(i = begin; (i < end) && (i+length) < pts.size(); i++){
    pts[i].
  }
  */
  //
}

//compare the number line distance between two values
int LayoutManager::AbsDiff(int i, int j)
{
  if(i >= j){
    return i - j;
  }
  else{
    return j - i;
  }
}


//for integer pixel distances
int LayoutManager::IntDistance(const Point& p1, const Point& p2)
{
  return (short int)sqrt(pow((double)(p1.X - p2.X),2.0) + pow((double)(p1.Y - p2.Y),2.0));
}

//for real-valued distances, e.g. when mapping distances to a probability in range 0-1
double LayoutManager::DoubleDistance(const Point& p1, const Point& p2)
{
  return sqrt(pow((double)(p1.X - p2.X),2.0) + pow((double)(p1.Y - p2.Y),2.0));
}

void LayoutManager::BuildKeyMap(const string& keyFileName)
{
  if(!this->keyMap.empty()){
    this->keyMap.clear();
  }

  BuildKeyMapCoordinates(keyFileName);
  BuildKeyMapClusters();
  SetMinKeyDists();
  InitLayoutDimensions();
}

//sets the active region for key inputs
void LayoutManager::InitLayoutDimensions(void)
{
  int rightMax, lowerMax;  // origin (0,0) in screen coordinates, so right most and bottom most edges are the extreme of the layout

  //establish the right and left bounds
  rightMax = lowerMax = -1;
  for(KeyMapIt it = keyMap.begin(); it != keyMap.end(); ++it){
    if(it->second.first.X > rightMax){
      rightMax = it->second.first.X;
    }
    /*if(it->second.first.Y > lowerMax){
      lowerMax = it->second.first.Y;
    }*/
    if(it->first == 'b' || it->first == 'B'){  //harshly assume 'b' y coordinate, plus 1.25 radii is the lower bound. This is done to omit the start/end button/region
      lowerMax = it->second.first.Y;
    }
  }

  cout << "Active Layout Dimensions: " << rightMax << "x" << lowerMax << endl;
  layoutWidth = rightMax + (int)(minKeyRadius * 1.0);
  layoutHeight = lowerMax + (int)(minKeyRadius * 1.0);
}
int LayoutManager::GetWidth(void)
{
  return layoutWidth;
}
int LayoutManager::GetHeight(void)
{
  return layoutHeight;
}


/*
  This needs to be manually defined. It would be nice to at least have it read values
  from a file, instead of needing to be recompiled. Nicer still, to define it with ratios
  instead of units... hm... different screens may yield different coordinates as well, screwing
  up the whole mapping...
  Screens, resolutions, and various keyboard layouts are the parameters. I'm basing this off a typical
  DELL qwerty keyboard right now.

  
  TODO: Its important to remember that the set of neighbor's defines the possible letters in the output; so if
  a letter is omitted to eagerly, it may not be possible to correct except with edit-distance logic. Clients
  of the data structure probably want tight-clusters, though, so this error may be tolerable since later
  stages should catch it.

  TODO: This is a rigidly-defined structure, according to the visual key layout. Maybe some better software engineering is needed,
  such as a dynamically-defined key layout generated from a file on the fly, like on-the-fly html generation.

  TODO: Define keyset. Ignoring punctuation and numbers for now.

  TODO: If punct/numbers are included, need to find character n-gram files with these frequencies. The crypto-based files used currently
  only defined alpha frequencies, not numbers or punctuation, etc.

  */
void LayoutManager::BuildKeyMapClusters(void)
{
  //q: keyboard neighbors of the letter q
  //keyMap['Q'].second = vector<char>{'W','A'}; //TODO: this syntax is desirable, as opposed to ugly push_back pattern below
  keyMap['Q'].second.push_back('W');
  keyMap['Q'].second.push_back('A');

  /*
  keyMap['Q'].second.push_back('W');
  keyMap['Q'].second.push_back('A');
  for(int i = 0; i < keyMap['Q'].second.size(); i++){
    cout << keyMap['Q'].second[i];
  }
  cout << " end of vec!" << endl;
  */


  //w
  keyMap['W'].second.push_back('Q');
  keyMap['W'].second.push_back('A');
  keyMap['W'].second.push_back('S');
  keyMap['W'].second.push_back('E');

  //e
  keyMap['E'].second.push_back('R');
  keyMap['E'].second.push_back('D');
  keyMap['E'].second.push_back('S');
  keyMap['E'].second.push_back('W');

  //r
  keyMap['R'].second.push_back('T');
  keyMap['R'].second.push_back('F');
  keyMap['R'].second.push_back('D');
  keyMap['R'].second.push_back('E');


  //t
  keyMap['T'].second.push_back('Y');
  keyMap['T'].second.push_back('G');
  keyMap['T'].second.push_back('F');
  keyMap['T'].second.push_back('R');


  //y
  keyMap['Y'].second.push_back('U');
  keyMap['Y'].second.push_back('H');
  keyMap['Y'].second.push_back('G');
  keyMap['Y'].second.push_back('T');


    //u
  keyMap['U'].second.push_back('I');
  keyMap['U'].second.push_back('J');
  keyMap['U'].second.push_back('H');
  keyMap['U'].second.push_back('Y');


    //i
  keyMap['I'].second.push_back('O');
  keyMap['I'].second.push_back('K');
  keyMap['I'].second.push_back('J');
  keyMap['I'].second.push_back('U');


    //o
  keyMap['O'].second.push_back('P');
  keyMap['O'].second.push_back('L');
  keyMap['O'].second.push_back('K');
  keyMap['O'].second.push_back('I');


    //p
  keyMap['P'].second.push_back('L');
  keyMap['P'].second.push_back('O');
  keyMap['P'].second.push_back(';');


    //a
  keyMap['A'].second.push_back('Q');
  keyMap['A'].second.push_back('W');
  keyMap['A'].second.push_back('S');
  keyMap['A'].second.push_back('Z');


    //s
  keyMap['S'].second.push_back('A');
  keyMap['S'].second.push_back('W');
  keyMap['S'].second.push_back('E');
  keyMap['S'].second.push_back('D');
  keyMap['S'].second.push_back('X');
  keyMap['S'].second.push_back('Z');


    //d
  keyMap['D'].second.push_back('S');
  keyMap['D'].second.push_back('E');
  keyMap['D'].second.push_back('R');
  keyMap['D'].second.push_back('F');
  keyMap['D'].second.push_back('C');
  keyMap['D'].second.push_back('X');


    //f
  keyMap['F'].second.push_back('D');
  keyMap['F'].second.push_back('R');
  keyMap['F'].second.push_back('T');
  keyMap['F'].second.push_back('G');
  keyMap['F'].second.push_back('V');
  keyMap['F'].second.push_back('C');


    //g
  keyMap['G'].second.push_back('F');
  keyMap['G'].second.push_back('T');
  keyMap['G'].second.push_back('Y');
  keyMap['G'].second.push_back('H');
  keyMap['G'].second.push_back('B');
  keyMap['G'].second.push_back('V');



    //h
  keyMap['H'].second.push_back('G');
  keyMap['H'].second.push_back('Y');
  keyMap['H'].second.push_back('U');
  keyMap['H'].second.push_back('J');
  keyMap['H'].second.push_back('N');
  keyMap['H'].second.push_back('B');



    //j
  keyMap['J'].second.push_back('H');
  keyMap['J'].second.push_back('U');
  keyMap['J'].second.push_back('I');
  keyMap['J'].second.push_back('K');
  keyMap['J'].second.push_back('M');
  keyMap['J'].second.push_back('N');


    //k
  keyMap['K'].second.push_back('J');
  keyMap['K'].second.push_back('I');
  keyMap['K'].second.push_back('O');
  keyMap['K'].second.push_back('L');
  keyMap['K'].second.push_back(',');
  keyMap['K'].second.push_back('M');


    //l
  keyMap['L'].second.push_back('K');
  keyMap['L'].second.push_back('O');
  keyMap['L'].second.push_back('P');
  keyMap['L'].second.push_back(';');
  keyMap['L'].second.push_back(',');
  keyMap['L'].second.push_back('.');


    //;
  keyMap[';'].second.push_back('P');
  keyMap[';'].second.push_back('L');
  keyMap[';'].second.push_back('.');
  keyMap[';'].second.push_back('?');
  keyMap[';'].second.push_back('"');


    //;
  keyMap['"'].second.push_back(';');
  keyMap['"'].second.push_back('?');



    //z
  keyMap['Z'].second.push_back('A');
  keyMap['Z'].second.push_back('S');
  keyMap['Z'].second.push_back('X');


    //x
  keyMap['X'].second.push_back('Z');
  keyMap['X'].second.push_back('S');
  keyMap['X'].second.push_back('D');
  keyMap['X'].second.push_back('C');


    //c
  keyMap['C'].second.push_back('X');
  keyMap['C'].second.push_back('D');
  keyMap['C'].second.push_back('F');
  keyMap['C'].second.push_back('V');


    //v
  keyMap['V'].second.push_back('C');
  keyMap['V'].second.push_back('F');
  keyMap['V'].second.push_back('G');
  keyMap['V'].second.push_back('B');


    //b
  keyMap['B'].second.push_back('V');
  keyMap['B'].second.push_back('G');
  keyMap['B'].second.push_back('H');
  keyMap['B'].second.push_back('N');


    //n
  keyMap['N'].second.push_back('B');
  keyMap['N'].second.push_back('H');
  keyMap['N'].second.push_back('J');
  keyMap['N'].second.push_back('M');


    //m
  keyMap['M'].second.push_back('N');
  keyMap['M'].second.push_back('J');
  keyMap['M'].second.push_back('K');
  keyMap['M'].second.push_back(',');


    //,
  keyMap[','].second.push_back('M');
  keyMap[','].second.push_back('K');
  keyMap[','].second.push_back('L');
  keyMap[','].second.push_back('.');


    //.
  keyMap['.'].second.push_back(',');
  keyMap['.'].second.push_back(';');
  keyMap['.'].second.push_back('?');
  keyMap['.'].second.push_back('L');


    //?
  keyMap['?'].second.push_back('.');
  keyMap['?'].second.push_back(';');
  keyMap['?'].second.push_back('"');
}

double LayoutManager::AvgDistance(vector<Point>& pts, int begin, int nPts)
{
  double dist = 0.0;

  for(int i = begin; i < pts.size() && i < (begin+nPts); i++){
    dist += DoubleDistance(pts[i],pts[i+1]);
  }
  dist /= (double)nPts;

  return dist;
}

//return covariance of a set of (x,y) points
double LayoutManager::CoVariance(vector<Point>& pts, int begin, int nPts)
{
  int i, j;
  double muX, muY, cv;

  //get the means  
  muX = muY = 0.0;
  for(i = begin; i < pts.size() && i < (i+nPts); i++){
    muX += pts[i].X;
    muY += pts[i].Y;
  }
  muX /= (double)nPts;
  muY /= (double)nPts;

  cv = 0.0;
  for(i = begin; i < pts.size() && i < (i+nPts); i++){
    cv += ((pts[i].X - muX)*(pts[i].Y - muY));
  }
  cv /= (double)nPts;

  return cv;
}

//returns standard deviation of x values in some sequence of (x,y) coordinates
double LayoutManager::StDev_X(vector<Point>& pts, int begin, int nPts)
{
  int i;
  double muY = 0.0, sum = 0.0;

  //get the mean
  for(i = begin; i < pts.size() && i < (begin+nPts); i++){
    muY += pts[i].Y;
  }
  muY /= nPts;

  for(i = begin; i < pts.size() && i < (begin+nPts); i++){
    sum += pow((pts[i].Y - muY),2.0);
  }

  sum = pow(sum,0.5);
  return sum;
}
//returns standard deviation of y values in some sequence of (x,y) coordinates
double LayoutManager::StDev_Y(vector<Point>& pts, int begin, int nPts)
{
  int i;
  double muX = 0.0, sum = 0.0;

  //get the mean
  for(i = begin; i < pts.size() && i < (begin+nPts); i++){
    muX += pts[i].X;
  }
  muX /= nPts;

  for(i = begin; i < pts.size() && i < (begin+nPts); i++){
    sum += pow((pts[i].X - muX),2.0);
  }
  
  sum = pow(sum,0.5);
  return sum;
}

//returns sigmaX*sigmaY across the (X,Y) vals in a sequence
double LayoutManager::CoStdDeviation(vector<Point>& pts, int begin, int nPts)
{
  return StDev_Y(pts,begin,nPts) * StDev_X(pts,begin,nPts);
}



/*
  Given some euclidean point, map it to a collection of neighbor keyboard keys w/in
  some search radius, such that any given point yields up to seven keys.

  TODO: figure out what characteristics the geometry-based probability ought to have. Should keys have gravity? Or should roughly equidistant
  keys be assigned nearly the same probability? What is the probability distribution of error over keys (visually)?
*/
void LayoutManager::SearchForNeighborKeys(const Point& p, vector<State>& neighbors)
{
  double logProbability;
  double minDist = 999999;
  char c = FindNearestKey(p);

  for(int i = 0; i < keyMap[c].second.size(); i++){
    State s;
    s.symbol = keyMap[c].second[i];
    s.pState = DoubleDistance(p,keyMap[c].first);
    if(s.pState < minDist){
      minDist = s.pState;
    }
    neighbors.push_back(s);
  }

  for(int i = 0; i < neighbors.size(); i++){
    neighbors[i].pState = minDist / neighbors[i].pState;     //TODO: define the geometry-based probability. Currently just using some aribtrary inverse-distance, based on the nearest point.
    neighbors[i].pState = -1.0 * log2(neighbors[i].pState);  //convert probability to -log2-space
  }
}


/*
  Consumes some static data (file, etc) containing key coordinate
  info, and fills map with that data. This only builds the coordinates (points)
  in the data structure, not the neighbor vectors, since these may vary with the key layout.
  Initializing the neighbor vectors is done by BuildKeyMapClusters, so normally both these functions
  will be called together.
*/
void LayoutManager::BuildKeyMapCoordinates(const string& keyFileName)
{
  int i = 0, j = 0;
  fstream keyFile;
  string line;
  string delimiter = "\t";
  char alpha;
  string pX;
  string pY;
  char buf[256];

  keyFile.open(keyFileName, ios::in);
  if(!keyFile.is_open()){
    cout << "ERROR could not open test file: " << keyFileName << endl;
    return;
  }

  while(getline(keyFile,line)){
    if(line.length() > 3 && line.length() < 200){
      strncpy(buf,line.c_str(),line.length());
      buf[line.length()] = '\0';
      //cout << "line=" << line << " line.length=" << line.length() << endl;
      //cout << "buf=" << buf << endl;
      i = line.find(delimiter,0);
      j = line.find(delimiter,i+1);
      //i = myFind(line.c_str(),fileDelimiter);
      //cout << "i=" << i << " for find() of delim ascii char >" << (int)delimiter[0] << "<" << endl;
      //cout << "i=" << i << " j=" << j << endl;
      if(i != std::string::npos && j != std::string::npos && j > 0 && i > 0){
        buf[i] = '\0';
        buf[j] = '\0';
        //alpha = ToLower(buf[0]);
        alpha = ToUpper(buf[0]);
        pX = &buf[i+1];
        pY = &buf[j+1];
        
        cout << "entering alpha/x/y: " << alpha << "/" << pX << "/" << pY << endl;
        //pair<Point,vector<char> > myPair;
        Point p((short int)std::stoi(pX), (short int)std::stoi(pY));
        this->keyMap[alpha].first = p;
        //myPair.first.X = std::stoi(pX);
        //myPair.first.Y = std::stoi(pY);
      }
      else{
        cout << "ERROR delimiter >" << delimiter << "< not found in BuildTestData" << endl;
      }
    }
    line.clear();
  }

  //PrintKeyMap();
  keyFile.close();
}

void LayoutManager::PrintKeyMap(void)
{
  int i;
  KeyMapIt it;

  cout << "Key map: " << endl;
  for(it = this->keyMap.begin(); it != this->keyMap.end(); it++){
    cout << it->first << " <" << it->second.first.X << "," << it->second.first.Y << "> ";
    cout << "vec.size=" << it->second.second.size() << " ";
    for(i = 0; i < it->second.second.size(); i++){
      cout << it->second.second[i];
    }
    cout << endl;
  }
}

/*
  Once keyMap has been built, find the minimum distance between any two keys in the set (div 2).
  The "radius" is defined as this distance, divided by two. This provides a optimization value such that
  given some point, we can know when no other key could be nearer when searching for nearest keys.
*/
void LayoutManager::SetMinKeyDists(void)
{
  double dist, min = 99999;

  if(keyMap.empty()){
    cout << "ERROR keyMap empty in SetMinKeyDists" << endl;
    this->minKeyRadius = 0;
    return;
  }

  KeyMapIt tortoise, hare;
  for(tortoise = keyMap.begin(); tortoise != keyMap.end(); ++tortoise){
    for(hare = keyMap.begin(); hare != keyMap.end(); ++hare){
      //TODO: get rid of this (0,0) check once ui-model is settled. Shouldn't need a check like this; all keys should have coodinates, else they should not be in the key map
      //added this to skip omitted chars, like punctuation, when these aren't in the model
      if(tortoise->second.first.X != 0 && tortoise->second.first.Y != 0 && hare->second.first.X && hare->second.first.Y != 0){
        if(tortoise != hare && hare->first != tortoise->first){
		      dist = DoubleDistance(tortoise->second.first,hare->second.first);
		      if(dist < min){
            cout << tortoise->second.first.X << "," << tortoise->second.first.Y << " " << hare->second.first.X << "," << hare->second.first.Y << endl;
            //cout << "dist=" << dist << endl;
		        min = dist;
		      }
        }
      }
    }
  }

  cout << "min diameter: " << min << endl;
  this->minKeyDiameter = min;
  this->minKeyRadius = min / 2.0;

  cout << "min diameter: " << this->minKeyDiameter << "  minkey radius: " << this->minKeyRadius << endl;
}

/*
  Iterate over the map looking for key nearest to some point p.
  
  TODO: This could be a lot faster than linear search. It will be called for every mean from
  the SingularityBuilder, so eliminating it might speed things a bit.  Could at least return 
  as soon as we find a min-distance that is less than the key width (or width/2), such that
  no other key could be closer than this value.
*/
char LayoutManager::FindNearestKey(const Point& p)
{
  char c;
  double dist, min = 99999;

  //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
  for(KeyMapIt it = this->keyMap.begin(); it != this->keyMap.end(); ++it){
    dist = DoubleDistance(it->second.first,p);
    //cout << "dist(" << it->first << ",pt)=" << dist << endl;
    if(min > dist){
      min = dist;
      if(min < this->minKeyRadius){  //small, dumb optimization. return when dist is below some threshold, such that no other key could be nearer
        return it->first;
      }
      c = it->first;
    }
  }

  return c;
}

double LayoutManager::GetMinKeyRadius(void)
{
  return minKeyRadius;
}

double LayoutManager::GetMinKeyDiameter(void)
{
  return minKeyDiameter;
}

vector<char>* LayoutManager::GetNeighborPtr(char index)
{
  return &keyMap[index].second;
}
Point LayoutManager::GetPoint(char symbol)
{
  return keyMap[symbol].first;
}


