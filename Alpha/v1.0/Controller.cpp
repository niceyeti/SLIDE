#include "Header.hpp"

//TODO: remove these to higher class, but that they may be used by multiple subclasses.
//It seems to make more sense to condition the generated sequences after they have been, well, generated.
KeyMap CLASS_keyMap;

//TODO some other class needs to claim this function
void PrintLattice(const Lattice& lattice)
{
  int i, j, k, maxht;
  bool hit;

  maxht = 0;
  for(i = 0; i < lattice.size(); i++){
    if(maxht < lattice[i].alphas.size()){
      maxht = lattice[i].alphas.size();
    }
  }

  //prints the vertices in columnar format
  printf("Lattice vertices:\n");
  i = 0; hit = false;
  while(i < maxht){
    for(j = 0; j < lattice.size(); j++){
      if(lattice[j].alphas.size() > i){
        printf("%c %3.1f   ", lattice[j].alphas[i].symbol, lattice[j].alphas[i].pState);
      }
      else{
        printf("        ");
      }
    }
    printf("\n");
    i++;
  }

  //print the edges
  for(i = 0; i < lattice.size() - 1; i++){
    
    printf("Column %d\n",i);
    for(j = 0; j < lattice[i].alphas.size(); j++){
      printf("  State %c: ",lattice[i].alphas[j].symbol);
      for(k = 0; k < lattice[i].alphas[j].arcs.size(); k++){
        if(lattice[i].alphas[j].arcs[k].dest != NULL){
          printf("(%c,%2.1f) ",lattice[i].alphas[j].arcs[k].dest->symbol,lattice[i].alphas[j].arcs[k].pArc);
        }
        else{
          printf("(NULL) ");
        }
      }
      printf("\n");
    }
  }
}

/*
  Iterate over the map looking for key nearest to some point p.
  
  TODO: This could be a lot faster than linear search. It will be called for every mean from
  the SingularityBuilder, so eliminating it might speed things a bit.
*/
char FindNearestKey(const Point& p)
{
  char c = '\0';
  double min = 99999;

  //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
  for(KeyMapIt it = CLASS_keyMap.begin(); it != CLASS_keyMap.end(); ++it){
    if(min > DoubleDistance(it->second.first, p)){
      c = it->first;
    }
  }

  return c;
}

vector<char>* GetNeighborPtr(char index)
{
  return &CLASS_keyMap[index].second;
}
Point GetPoint(char symbol)
{
  return CLASS_keyMap[symbol].first;
}


//returns decimal n-seconds to high-precision
long double diffTimeSpecs(struct timespec* begin, struct timespec* end)
{
  long double scalar = 1000000000.0;
  long double finish = (long double)(end->tv_sec - begin->tv_sec) + (long double)end->tv_nsec / scalar;
  long double start = (long double)begin->tv_nsec / scalar;

  //long finish = ((long double)end->tv_sec + (long double)end->tv_nsec / scalar);
  //long start = ((long double)begin->tv_sec + (long double)begin->tv_nsec / scalar);
  //cout << "end_sec " << end->tv_sec << "  end_nsec " << end->tv_nsec << "  start_sec " << begin->tv_sec << "  start_nsec " << begin->tv_nsec << endl;
  //cout << "finish=" << finish << " start=" << start << endl;

  return finish - start;
}

//for integer pixel distances
short int IntDistance(const point& p1, const point& p2)
{
  return (short int)sqrt(pow((double)(p1.X - p2.Y),2.0) + (short int)pow((double)(p1.Y + p2.Y),2.0));
}

//for real-valued distances, e.g. when mapping distances to a probability in range 0-1
double DoubleDistance(const point& p1, const point& p2)
{
  return sqrt(pow((double)(p1.X - p2.Y),2.0) + pow((double)(p1.Y + p2.Y),2.0));
}

bool ByLogProb(const LatticePath& left, const LatticePath& right)
{
  return left.second < right.second;
}


//Comparison function for sorting arc's by probability, in ascending order, min item first. We're in -log2-space, so minimization is desired.
bool maxArcLlikelihood(const Arc& left, const Arc& right)
{
  return left.pArc < right.pArc;
}

bool isDelimiter(const char c, const string& delims)
{
  int i;

  for(i = 0; i < delims.length(); i++){
    if(c == delims[i]){
      return true;
    }
  }

  return false;
}

/*
  Logically the same as strtok: replace all 'delim' chars with null, storing beginning pointers in ptrs[]
  Input string can have delimiters at any point or multiplicity

  Pre-condition: This function continues tokenizing until it encounters '\0'. So buf must be null terminated,
  so be sure to bound each phrase with null char.

  Testing: This used to take a len parameter, but it was redundant with null checks and made the function 
  too nasty to debug for various boundary cases, causing errors.
*/
int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims)
{
  int i, tokCt;
  //int dummy;

  if((buf == NULL) || (buf[0] == '\0')){
    ptrs[0] = NULL;
    cout << "WARN buf==NULL in tokenize(). delims: " << delims << endl;
    return 0;
  }
  if(delims.length() == 0){
    ptrs[0] = NULL;
    cout << "WARN delim.length()==0 in tokenize()." << endl;
    return 0;
  }

  //consume any starting delimiters then set the first token ptr
  for(i = 0; isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
  //cout << "1. i = " << i << endl;

  if(buf[i] == '\0'){  //occurs if string is all delimiters
    if(DBG)
      cout << "buf included only delimiters in tokenize(): i=" << i << "< buf: >" << buf << "< delims: >" << delims << "<" << endl;
    ptrs[0] = NULL;
    return 0;
  }

  //assign first token
  ptrs[0] = &buf[i];
  tokCt = 1;
  while(buf[i] != '\0'){

    //cout << "tok[" << tokCt-1 << "]: " << ptrs[tokCt-1] << endl;
    //cin >> dummy;
    //advance to next delimiter
    for( ; !isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
    //end loop: buf[i] == delim OR buf[i]=='\0'

    //consume extra delimiters
    for( ; isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++){
      buf[i] = '\0';
    } //end loop: buf[i] != delim OR buf[i]=='\0'

    //at next substring
    if(buf[i] != '\0'){
      ptrs[tokCt] = &buf[i];
      tokCt++;
    }
  } //end loop: buf[i]=='\0'

  //cout << "DEBUG first/last tokens: " << ptrs[0] << "/" << ptrs[tokCt-1] << "<end>" <<  endl; 

  ptrs[tokCt] = NULL;

  return tokCt;
}

