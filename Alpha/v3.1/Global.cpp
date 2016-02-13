#ifndef HEADER_HPP
#include "Header.hpp"
#endif

char ToLower(char c)
{
  if(c >= 'A' && c <= 'Z'){
    return c + 32;
    //cout << buf[i] << " from " << (buf[i] -32) << endl;
  }
  return c;
}

bool ByLogProb(const LatticePath& left, const LatticePath& right)
{
  return left.second < right.second;
}
bool ByDistance(const SearchResult& left, const SearchResult& right)
{
  return left.second < right.second;
}
bool ByRank(const pair<U32,SearchResult> &left, const pair<U32,SearchResult> &right)
{
  return left.first < right.first;
}


//returns decimal n-seconds to high-precision
long double DiffTimeSpecs(struct timespec* begin, struct timespec* end)
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

char ToUpper(char c)
{
  if(c >= 'a' && c <= 'z'){
    return c - 32;
    //cout << buf[i] << " from " << (buf[i] + 32) << endl;
  }
  return c;
}

void StrToUpper(char str[])
{
  for(int i = 0; str[i] != '\0'; i++){
    str[i] = ToUpper(str[i]);
  }
}

/*
  This file just contains a bunch of global functions which don't seem to belong to a particular class or are used by several
  classes.
*/

bool IsDelimiter(const char c, const string& delims)
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
int Tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims)
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
  for(i = 0; IsDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
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
    for( ; !IsDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
    //end loop: buf[i] == delim OR buf[i]=='\0'

    //consume extra delimiters
    for( ; IsDelimiter(buf[i], delims) && (buf[i] != '\0'); i++){
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

