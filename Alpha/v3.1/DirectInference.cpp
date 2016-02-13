#include "Controller.hpp"

/*
	This is a prototype of the direct inference method. The inference method is:
		 Given a set of clusters (mean points), correlate these with words, and return a list of most likely words.

  The approach of this class could eliminate the lattice all-together, going directly from word cluster to words!!!

	The cluster points are mean points, possible key "events", which forms a set of (x,y) pairs. Thus,
	a complete set of clusters represents some event-path through the keys for a given word, as a vector of
	vectors: path_vector = <(x,y), (x,y), ... (x,y)>

	Every possible word in the lexicon also maps to a unique such vector as well.
	So given this vector of vectors, we try to find which word (itself as a vector of points) is nearest to this one,
	using some distance function. So go to some data structure of word vectors, and find
	the nearest vector(s). Next of course, re-rank these according to their linguistic validity using some language model, but that's
	not important here.

	The data model of vectors is just a black box here, since this is just a proof of the algorithmic concept of mapping
	cluster vectors directly to word vectors. The data model would likely be factored to facilitate fast, partitioned
	searches and so on, but none of that matters here (in the scope of this prototype). Thus, the word-vector lookups
	will just be brute-force searched across the entire word set.

	The method places strong emphasis on the distance metric:
		dist = distance(clusterVector<>, wordVector<>)



	The vocabulary is given by COCA, which averages about 60% recall in most cases. This is just a test model. A more complete
	vocabulary model will be needed for real use.


  Distance function is cosine based.

  Questions: should we instead feed multiple edits to the distance function? Or, instead
  evaluate edit-distances in the later language-modeling stage?

  Ideas for partitioning the data structure will likely come from thinking about the vectors and 
  error tolerance: for instance, given word of length k, only search vectors of length k, k-1, and k+1.
  Think about query access instead of implementation of word storage, all of which is dependent on the needs
  of the method, once its developed. Hence, don't overdevelop the datamodel before the method...
  Could also partition by key regions, somehow.
*/

DirectInference::DirectInference()
{
  cout << "ERROR building DirectInference model using default constructor, not built yet. Expect crash..." << endl;
  //TODO dont hardcode filename

  string s = "../vocabModel.txt";
  BuildWordModel(s);
}

void DirectInference::SetLayoutManager(LayoutManager* layoutManagerPtr)
{
  layoutManager = layoutManagerPtr;
}

DirectInference::DirectInference(const string& vocabFile, LayoutManager* layoutManagerPtr)
{
  cout << "Building DirectInference model..." << endl;
  layoutManager = layoutManagerPtr;
  BuildWordModel(vocabFile);
}

DirectInference::~DirectInference()
{
  wordModel.clear();
}

//converts non-printable chars to toChar
void DirectInference::Strip(char buf[], char toChar)
{
  for(int i = 0; buf[i] != '\0'; i++){
    if(buf[i] < ' ' || buf[i] > 126){
      buf[i] = toChar;
    }
  }
}

//expect word model as flat file word database, one (unique) word per line
void DirectInference::BuildWordModel(const string& vocabFile)
{
  int ntoks;
  char buf[BUFSIZE];
  char* tokens[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
  fstream infile(vocabFile.c_str(), ios::in);
  string delims = "\t";
  string s;

  if(!infile){
    cout << "ERROR could not open file: " << vocabFile << endl;
    return;
  }

  cout << "Building word model..." << endl;
  while(infile.getline(buf,BUFSIZE)){  // same as: while (getline( myfile, line ).good())
    StrToUpper(buf);
    s = buf;
    wordModel.insert(s);
    /*  //was used for building word model from bigram model; better to keep them separate, since word model is more constricted/filtered
    Strip(buf,'\t');
    ntoks = Tokenize(tokens,buf,delims);
    if(ntoks >= 3){
      //cout << "adding " << tokens[1] << "  " << tokens[2] << endl;
      s = tokens[1];
      wordModel.insert(s);
      s = tokens[2];
      wordModel.insert(s);
    }
    */
  }
  cout << "Building word model completed. WordModel.size()=" << wordModel.size() << endl;
  infile.close();
}

/*
  Given a set of mean points, generates a raw string from them.
  This gives the DIRECT parse of the clusters, w/out accounting for possible insertions.
*/
void DirectInference::MeansToString(vector<PointMu>& pointMeans, string& output)
{
  if(output.size()){
    output.clear();
  }

  for(int i = 0; i < pointMeans.size(); i++){
    output += layoutManager->FindNearestKey(pointMeans[i].pt);
  }
  cout << "returning " << output << " from Controller::MeansToString" << endl;
}

/*
  Given a set of cluster point-means, return a ranked list of the most likely words
  in that set of clusters, based on reflexive likelihoods. 

  Currently returns string set containing at most two-reflexive events (two sets of repeated letters).

  This is just a soft filter/transformer for improving the initial representation of the data,
  before it reaches some string-distance function. I think this should only branch likely
  repeated chars when it is very confident that one occurred, and further, it will only
  repeat a char once (no consecutive char sequences of longer than 2, as a result). This means
  this will return a max of 8 edits.

  TODO: Number of branches may be relative to length of string. Hardcoded 3 branches for now.

  NOTE: This worked, but it could be, and was, eliminated! This was great news. The context
  for this function was to emulate consecutive repeat-key sequences, as in difficult words like
  "MISSIISSIPPI". However, observe that you can get around this by simply skipping the repeated
  chars in any candidate word, when trying to map the cluster-sequence to such candidate words!
  This is the secret sauce of this program... mwahahaaha
*/

void DirectInference::MeansToEditList(vector<PointMu>& pointMeans, vector<string>& stringList)
{
  int i, j, k, insertCt;
  char a;
  //vector<string> prefixList;
  string s;

  cout << "in meanstoeditlist" << endl;
  s += layoutManager->FindNearestKey(pointMeans[0].pt);  
  stringList.push_back(s);
  insertCt = 0;
  //every reflexive event is a branch, so this is recursive. Be sure to understand it. The grammar is R -> R | RR (for some reflexive state R)
  for(i = 1; i < pointMeans.size(); i++){

    //grow all current sequences by this alpha
    a = layoutManager->FindNearestKey(pointMeans[i].pt);
    for(j = 0; j < stringList.size(); j++){
      stringList[j] += a;
    }

    //insert repeat chars, if any. But only branch a max of three times
    if(insertCt <= 3 && pointMeans[i].ticks > REFLEXIVE_TICK_THRESHOLD){
      k = stringList.size();  //need a static size val, since size() of list will grow as elems are appended
		  for(j = 0; j < k; j++){
        s = stringList[j];
        s += a;
		    stringList.push_back(s);
		  }
      insertCt++;
    }
  }
  cout << "after means to string list, there are " << stringList.size() << " edits: (insertCt=" << insertCt << ")" << endl;
  for(i = 0; i < stringList.size(); i++){
    cout << " > " << stringList[i] << endl;
  }
}


/*
  Uses an initial string distance, either as the prime metric, or as a filter for 
  more fine-grained coordinate/feature comparisons.
*/
void DirectInference::StringDistInference(vector<PointMu>& pointMeans, SearchResults& results)
{
  int i, diff;
  double dist, minDist;
  WordModelIt minIt, it;
  string edit;
  //vector<string> edits;

  //get the character representation of the cluster. note how this flattens the possible coordinate distances.
  MeansToString(pointMeans,edit);
  //MeansToEditList(pointMeans,edits);  //Obsolete, if non-unique filter method is used (secret sauce)
  cout << "done with means to edit" << endl;
  minDist = 99999;
  //iterates ENTIRE wordModel
  for(WordModelIt it = wordModel.begin(); it != wordModel.end(); ++it){
    diff = it->size() - edit.size();
		//optimization: only compare strings of roughly equal length (+-1 char)
    //check verifies diff is in range [-1,5], meaning edit can be longer by 1 char, or shorter by 5 (due to compression of rpt chars)
    //TODO: the error in this diff check cries out for evaluation of whether or not a compressed vocab model should
    //      be built (containing all words with repeate sequences removed: MISSISSIPPI -> MISISIPI).
    //      This generous search radius (of word lengths) indeed slows query times, which is an argument in favor.
		if((diff >= -1) && (diff <= 5)){  //this tolerance must account for the number of chars potentially squeezed in edit string
      //choose a string-distance function to test
			//dist = StringDist_Levenshtein(edit,*it);
			//dist = StringDist_Hamming(edits[i],*it);
      //dist = StringDist_HammingSkipChar(edit,*it); //A hamming distance, but one that compares only the unique character sequences of each string
      dist = StringDist_HammingFwdBkwd(edit,*it);

			//punish the difference in string length
			dist += layoutManager->AbsDiff(edit.size(),it->size());

			if(dist < minDist){
				minDist = dist;
				minIt = it;
			}
			results.push_back({*it,dist});
	  }
    
    /* Use for checking list of possible edits; look for ways to avoid having to do so, due to added complexity
	  //iterates list of possible edits. So this complexity is edits.size() * wordModel.size(), where edits.size() is 1-8
	  for(i = 0; i < edits.size(); i++){
			//optimization: only compare strings of roughly equal length (+-1 char)
			if(layoutManager->AbsDiff(it->size(),edits[i].size()) < 2){
				//dist = StringDist_Levenshtein(edit,*it);
				//dist = StringDist_Hamming(edits[i],*it);
        dist = StringDist_HammingSkipChar(edits[i],*it); //A hamming distance, but one that compares only the unique character sequences of each string
				if(dist < minDist){
					minDist = dist;
					minIt = it;
				}
				results.push_back({*it,dist});
			}
		}
    */
  }
  results.sort(ByDistance);
  cout << "StringDistInference complete. Min-dist string is: " << *minIt << endl;
  SearchResultIt mit;
  for(i = 0, mit = results.begin(); mit != results.end() && i < 50; i++, ++mit ){
    cout << i << ": " << mit->first << "|" << mit->second << endl;
  }
}

/*
  Hamming distance is a brute, substitution string comparison metric. It
  only compares strings with equal length, but here I add ||s1|-|s2||
  to the difference value.
*/
double DirectInference::StringDist_HammingFwd(const string& s1, const string& s2)
{
  int i, j;
  double diff = 0.0;

  i = 0, j = 0;
  while(i < s1.size() && j < s2.size()){
    if(s1[i] != s2[j]){
      diff++;
    }

    //Only skips one repeat, for a given bigram sequence. Skipping more than two is undesirable for words like "AAAAAAAAH"
    if(SKIPCHAR && (j < s2.size()-1) && (s2[j] == s2[j+1])){
      j++;
    }
/*  //skips 3 or more repeated chars, which causes problems for some junk in the vocab set: LAZY matches "LAAAAAAAAAAAA"
    if(SKIPCHAR){
      while(j < (s2.size()-1) && s2[j] == s2[j+1]){
        j++;
      }
    }
*/

    i++, j++;
  }

  return diff;
}

//String distance, backward only. s1 is the input, s2 is some candidate.
double DirectInference::StringDist_HammingBkwd(const string& s1, const string& s2)
{
  int i, j;
  double diff = 0.0;

  i = s1.size()-1;
  j = s2.size()-1;
  while(i >= 0 && j >= 0){
    if(s1[i] != s2[j]){
      diff++;
    }

    //only skip one repeated char. see previous fwd function notes.
    if(SKIPCHAR && (j > 0) && (s2[j] == s2[j-1])){
      j--;
    }

/*  //skips chars that are repeated more than twice, like "AAAAAAA", but this yields higher scores for sequences with many repeats
    //TODO straighten out skipping characters. Build the models without repeats, or formally capture the code-logic differences elsewhere.
    if(SKIPCHAR){
      while(j >= 1 && s2[j] == s2[j-1]){
        j--;
      }
    }
*/
    i--, j--;    
  }

  return diff;
}

//Hamming dist, forward only
double DirectInference::StringDist_Hamming(const string& s1, const string& s2)
{
  double dist = StringDist_HammingFwd(s1,s2);
  
  //punish the difference in string length
  //dist += AbsDiff(s1.size(),s2.size());  Removed this to a higher level

  return dist;
}

//Hamming dist over a backward pass over strings s1 and s2
double DirectInference::StringDist_HammingFwdBkwd(const string& s1, const string& s2)
{
  double dist = StringDist_HammingFwd(s1,s2);
  dist += StringDist_HammingBkwd(s1,s2);

  return dist;
}

/*
  This takes the Hamming distance between the cluster-sequence string and candidate words,
  BUT skips the repeated chars

  Precondition: To use this metric, one assumes that the cluster-sequence string contains NO
  repeated chars. Otherwise this function breaks.

  Note s1 is the edit (the cluster-sequence string) and s2 is some candidate word from a vocabulary set.
  TODO: This is awesome, but it needs some rigorous unit-testing of various bounds. Cases involving length,
  position, and nature of rpt sequences: "AAA" "MESS" "WALL" "RRRAR" etc.

  This is degenerate; the assumption is that error will very rarely be consecutive, such that realignment occurs
  within one character.
*/
double DirectInference::StringDist_HammingSkipChar(const string& s1, const string& s2)
{
  int i, j;
  double diff = 0.0;

/*
  if(s1.size() < s2.size()){
    //i increments normally. j increments similarly, but skips repeated chars.
    for(i = 0, j = 0; i < s1.size() && j < s2.size(); i++, j++){
      if(s1[i] != s2[j]){
        diff++;
      }
      //NOTE advances s2's index j, while character is not unique
      while(j < (s2.size()+1) && s2[j] == s2[j+1]){
        j++;
      }  //post: j points at last instance of repeated sequence, as if repeats didn't happen
    }
  }
  else{
    for(i = 0; i < s2.size(); i++){
      if(s1[i] != s2[i]){
        diff++;
      }
    }
  }
*/

  //i increments normally. j increments similarly, but skips repeated chars.
  i = 0, j = 0;
  while(i < s1.size() && j < s2.size()){
    if(s1[i] != s2[j]){
      diff++;
    }

    // advances s2's index j while character is not unique w.r.t. next char
    while(j < (s2.size()+1) && s2[j] == s2[j+1]){
      j++;
    }  //post: j points at last instance of repeated sequence, as if repeats didn't happen

    j++, i++;
  } //post: either i or j 

  //this is not technically Hamming: add the difference in str.length to the difference in substitutions. This could be weighted somehow.
  //diff += layoutManager->AbsDiff(s1.size(),s2.size());
  diff += layoutManager->AbsDiff(s1.size()-i,s2.size()-j);  //TODO: might add a weight to this diff

  return diff;
}


/*
  Primary method. This takes in a vector of X/Y points and tries to correlate these
  with words to find the nearest words.
*/
void DirectInference::Process(vector<PointMu>& pointMeans, SearchResults& results)
{

  if(results.size() > 0){
    results.clear();
     //results.reserve(wordModel.size() * 4);
  }

  //a string-distance approximation method, as opposed to brute-force geometry-based distance comparisons
  //StringDistInference(pointMeans,results);
  VectorDistInference(pointMeans,results);
  //MergeInference(pointMeans,results);  //merges multiple inference models' results: in this case, fast string-dist and geometric approaches 

  int i;
  SearchResultIt it;
  for(i = 0, it = results.begin(); i < 75 && it != results.end(); ++it, i++){
    cout << i << ": " << it->first << "|" << it->second << endl;
  }
}

/*
  Inference on steroids. Run each error correction model score and sort results. Each time
  you do this, a particular word receives rank i. So in the final output, simply assign each word/result
  a score of its mean i, and re-sort by these mean-ranks. You can also weight each models "i" value, 
  but here I'll just... not. For now.

  Notes: Only merges top 150 or so results, to cut down on process time.
*/
void DirectInference::MergeInference(vector<PointMu>& pointMeans, SearchResults& results)
{
  U32 i;
  //TODO: lots of extraneous data structures and inefficiency here. factor out the data model of the merge-listed representation (where results are <object,score,rank> tuples)
  list<pair<U32,SearchResult> > mergedRankList;
  list<pair<U32,SearchResult> >::iterator mit;
  SearchResults l1;
  SearchResults l2;
  SearchResultIt it1, it2;
  bool found;

  //runs fwd-bkwd hamming dist and forward geometric approach, merging results into a single set
  StringDistInference(pointMeans,l1);
  for(i = 0, it1 = l1.begin(); it1 != l1.end() && i < 200; i++, ++it1){  //results are bounded at 200, to cut down processing
    pair<U32,SearchResult> mergeResult = {i,*it1};
    mergedRankList.push_back(mergeResult);
  }

  //build next list and merge with metalist
  VectorDistInference(pointMeans,l2);
  for(i = 0, it2 = l2.begin(); it2 != l2.end() && i < 200; i++, ++it2){  //results are bounded at 200, to cut down processing
    found = false;
    for(mit = mergedRankList.begin(); !found && mit != mergedRankList.end(); ++mit){
      if(mit->second.first == it2->first){
        mit->first = (mit->first + i) / 2;  //avg the ranks
        found = true;
      }
    }
    if(!found){
      pair<U32,SearchResult> mergeResult = {i*2,*it2};
      mergedRankList.push_back(mergeResult);
    }
  }

  //sort by merged ranks
  mergedRankList.sort(ByRank);

  //TODO: limit the number of results in output and intermediate output
  //copy result of merged list to output parameter list
  for(i = 0, mit = mergedRankList.begin(); i < 200 && mit != mergedRankList.end(); i++, ++mit){
    SearchResult result = {mit->second.first,(double)mit->first};
    results.push_back(result);
  }
}

/*
  An exhaustive coordinate-vector comparison method.
*/
void DirectInference::VectorDistInference(vector<PointMu>& pointMeans, SearchResults& results)
{
  int i;
  double dist, min;
  WordModelIt minIt;
  //vector<PointMu> revPointMeans;

  //TODO: factor this out. For many distance metrics, running forward-backward may be superfluous
  //build reversal of pointMeans, for inference procedures that run backward-forward logic
  //RevPointMeans(pointMeans,revPointMeans);

  cout << "In DirectInference, process..." << endl;

  if(results.size() > 0){
    results.clear();
    //results.reserve(wordModel.size() * 4);
  }

  min = 99999;
  for(WordModelIt it = wordModel.begin(); it != wordModel.end(); ++it){
    dist = VectorDistance(pointMeans,it);
    //dist = VectorDistance(pointMeans,revPointMeans,it);  //overload for fwd-bkwd versions
    if(dist < min){
      min = dist;
      minIt = it;
    }
    if(dist < 1500){  //push threshold: near words
		  //cout << "pushing >" << *it << "," << dist << "<" << endl;
		  results.push_back(SearchResult{*it,dist});
    }
  }
  results.sort(ByDistance);

/*
  SearchResultIt mit;
  for(i = 0, mit = results.begin(); mit != results.end() && i < 120; i++, ++mit){
    cout << i << ": " << mit->first << "|" << mit->second << endl;
  }
*/
}

/*
  Core utlility of the class.  Multiple distance metrics will undoubtedly need to be devised...
  This one should only be geometry-based; later language modeling class can handle unification
  
  Distance metric currently assumes that at least the first cluster aligns with the first letter's coordinates.
*/
double DirectInference::VectorDistance(vector<PointMu>& pointMeans, WordModelIt it)
{
  double dist;

  //small optmization to only compare words within +/- k character length of eachother
  if(layoutManager->AbsDiff(pointMeans.size(),it->size()) > 4){
    return 99999;
  }

  //dist = SumDistMetric_Unaligned(pointMeans, *it);
  //dist = SumDistMetric_Unaligned_FwdBkwd(pointMeans, *it);
  dist = SumDistMetric_Aligned(pointMeans, *it);

  return dist;
}
//overload of previous, but with another parameter revPointMeans, which is just the reverse of pointMeans, to avert
//constantly recreating the reversal for metrics that run in both directions
double DirectInference::VectorDistance(vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans, WordModelIt it)
{
  double dist;

  dist = SumDistMetric_Aligned_FwdBkwd(pointMeans, revPointMeans, *it);

  return dist;
}

/*
  Just returns the sum distance of points in pointMeans, to points in word (letter-key coordinates).
  Remember |word| and |pointMeans| may not be equal, hence the longer one must be the outer loop, such that
  all points are compared. A brute metric.

  Gets sum of nearest points between vectors. This is VERY inefficient, for now.

  Note the alignment problem that occurs between words with this method. We want to compare a set of clusters
  to a word. Both of these are just sequences of points. In the best case, the two sequences align, and the most
  likely word will almost always be given by finding the min value for the sum, pairwise distance of points:

    sumDistance(word.pts,clusters.pts):
      while i < word.pts.length:
        sumDist += distance(word.pts[i],cluster.pts[i])

  Two issues arise: words of different length, and word/clusters which are not aligned. Different length words
  can be handled by ugly error-checking code. But the second problem is more difficult, and will occur often
  with eye sensors, since a user will often look at the wrong character, register the character, before correcting
  and looking at the right char. For example "MOISIZSIPI" might be a two-insertion version of "MISISIPI" (recalling
  that we search of words with repeated chars ignored/removed).

  The hacky solution here is to look up and downstream for the nearest key, but some of this is likely better-handled by 
  a string distance function that corrects insertions well. Another method might be to find common sequence of either string,
  and to evaluate the bad sequences separately. Currently, just take the minimum distance of the right, mid, and left key.
  Exhaustive searching would be quadratic, but note this is not desirable, since it would allow anagrams to be nearest neighbors,
  as respective char-positions are lost.

  The cluster means are mapped onto the candidate word; that is, the length of the distance search is the length of the cluster means.
  This is based on the assumption that the length of the cluster means will nearly always be greater than or equal to the
  length of the candidate word. It is unlikely the user will enter a word that is shorter than the intended word.

  Another fact is that this is essentially pattern matching, for which there ought to be tons and tons of visual
  algorithms to decode from input path to candidate-word path, using various vector features. This problem
  really is equivalent to handwriting recognition, at this point.

  This is a forward-only, one-pass metric: compare distances pairwise through the vector.

*/
double DirectInference::SumDistMetric_Unaligned(vector<PointMu>& pointMeans, const string& candidate)
{
  double sumDist = 0.0;
  int i, j;

  //candidate word is longer, so map its point to points in pointMeans
  //if(it->size() > pointMeans.size()){
    //assume a tight edit-error bound, such that alignment of points occurs within +/- one index 
    for(i = 0, j = 0; i < pointMeans.size() && j < candidate.size(); i++, j++){
      //optimization: only compare distances if letters differ?
      //if()
      sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint( candidate[j]));
      while(j < (candidate.size()-1) && candidate[j] == candidate[j+1]){
        j++;
      }
    }
  //}

  //finally, account for the difference in lengths by comparing every remaining letter in the longer string with the last in the short string
  if(i < pointMeans.size()){ //input word was longer than candidate. compare its remaining chars with last char in candidate
    while(i < pointMeans.size()){
      sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint( candidate[candidate.size()-1]));
      i++;
    }
  }
  if(j < candidate.size()){ //candidate word was shorter. compare its remaining chars with last char in input word
    while(j < candidate.size()){
      sumDist += layoutManager->DoubleDistance(pointMeans[pointMeans.size()-1].pt, layoutManager->GetPoint( candidate[j]) );
      j++;
    }
  }


  return sumDist;
}

/*
  Run the pariwise method forward and backward. This may handle insert-error well, by attacking in both directions, but needs testing.
*/
double DirectInference::SumDistMetric_Unaligned_FwdBkwd(vector<PointMu>& pointMeans, const string& candidate)
{
  double sumDist = 0.0;
  int i, j;

  //for forward pass, just call forward method
  sumDist = SumDistMetric_Unaligned(pointMeans, candidate);
  cout << "SumDistMetric_Unaligned_FwdBkwd INCOMPLETE just run on reversed strings" << endl;

  //TODO: put this into its own method, once completed.
  /*
  //candidate word is longer, so map its point to points in pointMeans
  //if(it->size() > pointMeans.size()){
    //assume a tight edit-error bound, such that alignment of points occurs within +/- one index 
    for(i = 0, j = 0; i < pointMeans.size() && j < it->size(); i++, j++){
      //optimization: only compare distances if letters differ?
      //if()
      sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint( (*it)[j]));
      while(j < (it->size()-1) && (*it)[j] == (*it)[j+1]){
        j++;
      }
    }
  //}

  //finally, account for the difference in lengths by comparing every remaining letter in the longer string with the last in the short string
  if(i < pointMeans.size()){ //input word was longer than candidate. compare its remaining chars with last char in candidate
    while(i < pointMeans.size()){
      sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint( (*it)[it->size()-1]));
      i++;
    }
  }
  if(j < it->size()){ //candidate word was shorter. compare its remaining chars with last char in input word
    while(j < it->size()){
      sumDist += layoutManager->DoubleDistance(pointMeans[pointMeans.size()-1].pt, layoutManager->GetPoint( (*it)[j]) );
      j++;
    }
  }
  */

  return sumDist;
}

/*
  Insertion errors occur when the user accidentally selects a key unintentionally. Observe that most of the time
  these errors will occur nearby the general character sequence. A typical characteristic is probably that such
  error occurs between ("on the way to") two points, but slightly off center. This function therefore smooths
  the distance assigned to this kind of error by defining it as the distance from the error-character to the midpoint 
  between the previous and next points in the candidate string. This makes sense, again, since the insertion errors
  will tend to occur near this midpoint, while very bad errors will tend to be much further away, giving them a
  bad error (distance) value, which is desirable. You kind of have to model this geometrically for it to make sense;
  it seems to obey the triangle inequality, so maybe this implies a metric that could be used elsewhere.
*/
double DirectInference::InsertionError(const Point& errorPt, const Point& pt1, const Point& pt2)
{
  double correctionWeight;

  //midpoint correction likely over-corrects, punishing correct strings.
  //correctionWeight = MidPointCorrection(errorPt, pt1, pt2);
  //nearest point correction returns punishment for error as distance to nearest adjacent correct alpha, since most edit errors occur near a correct neighbor alpha
  correctionWeight = NearestPointCorrection(errorPt, pt1, pt2);

  return correctionWeight;
}
/*Returns error-correction approximation as the dist to the nearest of two neighbor points.
  That is, is E is an insertion-error in input sequence Beta, then return the dist to the nearest of
  its correct neighbors.  Examples are Beta="FOCX" for which E='C' with respect to the candidate string "FOX".
  Clearly for this case we don't want simply the midpoint, since it will punish the candidate score too much.
  The intuition here is that most insertion errors will occur adjacent to one of the neighbors in the candidate.
*/
double DirectInference::NearestPointCorrection(const Point& errorPt, const Point& pt1, const Point& pt2)
{
  double dist1, dist2, ret;

  dist1 = layoutManager->DoubleDistance(errorPt,pt1);
  dist2 = layoutManager->DoubleDistance(errorPt,pt2);

  if(dist1 < dist2){
    ret = dist1;
  }
  else{
    ret = dist2;
  }

  return ret;
}

double DirectInference::MidPointCorrection(const Point& errorPt, const Point& pt1, const Point& pt2)
{
  //returns distance from error char to the midpoint between the next and previous chars (pt1, pt2) in a candidate word
  Point temp((pt1.X + pt2.X) / 2, (pt1.Y + pt2.Y) / 2);
  return layoutManager->DoubleDistance(errorPt, temp);
}



//An error heuristic of running the geometry-based methods backward and forward over some input
//TODO: if edit-distance parameters are right, calling this may simply be redundant; that is, calling this will only
//  result in constant 2*fwdDistance values.
double DirectInference::SumDistMetric_Aligned_FwdBkwd(vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans, const string& candidate)
{
  //too lazy to write a new function, so just reverse the input. Just run the forward algorithm twice on the normal and reversed input
  double dist = SumDistMetric_Aligned(pointMeans, candidate);
  string revString = ReverseString(candidate);
  dist += SumDistMetric_Aligned(revPointMeans,revString);

  return dist;
}

//non-inplace reversal, with an output parameter for the reversed point-mean sequence
void DirectInference::RevPointMeans(const vector<PointMu>& pointMeans, vector<PointMu>& revPointMeans)
{
  //stack reversal
  for(int i = pointMeans.size() - 1; i >= 0; i--){
    revPointMeans.push_back(pointMeans[i]);
  }
}

//does an in-place reversal of a point sequence
void DirectInference::ReverseInPlace(vector<PointMu>& pts)
{
  int i, j;
  PointMu temp;

/*
  cout << "input sequence: ";
  for(i = 0; i < pts.size(); i++){
    cout << pts[i].alpha;
  }
  cout << endl;
*/
  for(i = 0, j = pts.size()-1; i < j; i++, j--){
    temp = pts[i];
    pts[i] = pts[j];
    pts[j] = temp;
  }
/*
  cout << "output sequence: ";
  for(i = 0; i < pts.size(); i++){
    cout << pts[i].alpha;
  }
  cout << endl;
*/
}
//returns reversal of a string
string DirectInference::ReverseString(const string& str)
{
  string rev;
  for(int i = str.size()-1; i >= 0; i--){
    rev += str[i];
  }
  return rev;
}



/*
	//aligned version of previous. this is a much harder problem to program.
	//Currently just looks left/right two chars for a nearer neighbor key

  Intuitively, this is trying to gather the distance of all input points to the nearest, sequential ray
  trace given by the word's coordinates. There is probably a better approach to this than the hacky
  state-based method use here (detect-error, look-ahead, punish, discount, etc).

  This handles insertion error well. But not deletion error, such as if SingularityBuilder outputs "QUCK" instead of "QUICK".
  Try to handle "QUCK" "QUICK" case. This may be handled the same as insertion error, by advancing the candidate instead of the
  cluster-point sequence.

  The error values for different types of error are calibrated like a small constraint programming problem. For instance,
  really junky, significant differences in the distance between two geometric sequences should be punished severely. However,
  if one word is a root of another, don't punish the difference in input too much, lest one actually is a root of the other.
  Likewise, give a very small punishment to strings which differ only in the number of repeats, so "DOG" has a higher rank than
  "DOGG" (since repeated chars are eaten). In this way, the intent is to go from coarse to fine "punishment" in terms of string distance,
  such that each successive measure only re-ranks items within some subgrouping.

*/
double DirectInference::SumDistMetric_Aligned(vector<PointMu>& pointMeans, const string& candidate)
{
  double sumDist, dist;
  int i, j, rpts, edts;

  //assumes a tight edit-error bound, such that alignment of points occurs within +/- one index 
  rpts = edts = 0;
  sumDist = dist = 0.0;
  for(i = 0, j = 0; i < pointMeans.size() && j < candidate.size(); i++, j++){
    //optimization: only compare distances if letters differ
    if(pointMeans[i].alpha != candidate[j]){
      //insertion error: essentially, this is a soft check whether *deleting* (skipping) the current letter yields realignment
      //TODO: this essentially uses a unigram rule to declare realignment. Check if a bigram rule occurs better, assuming insert errors typically occur > 2 correct chars apart
      if((i+1) < pointMeans.size() && pointMeans[i+1].alpha == candidate[j]){
        //punishes the insertion error at least a little(?)
        //sumDist += layoutManager->minKeyRadius;
        if(j > 0){  //accumulate error as distance to midpoint between candidate points, or the dist to the nearer of the two point
          sumDist += InsertionError(pointMeans[i].pt, layoutManager->GetPoint(candidate[j-1]), layoutManager->GetPoint(candidate[j]));
        }
        else{ //the first is just an exception case, when we don't have two points to compare. so just compare char i to j
          sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint(candidate[j]));
        }
        i++;  //skip the input error
        edts++;
      }
      //new, 1/14/14: attempts to handle cluster="QUCK" candidate="QUICK" case, where letters are missing from the cluster-points. 
      //In this case, do the opposite of the insertion error case, advancing candidate index
      else if((j+1) < candidate.size() && pointMeans[i].alpha == candidate[j+1]){
        //deletion error may be remedied the same as insertion error; however, observe that most deletions occur
        //when char i is detected, but i+1 is not, since its just too near to detect. Thus, the distance should be assessed wrt i,
        //instead of the midpoint (as for insertion error).
        if(i > 0 && pointMeans.size() > 1){
          sumDist += layoutManager->DoubleDistance(pointMeans[i-1].pt, layoutManager->GetPoint(candidate[j]));
          //sumDist += InsertionError(layoutManager->GetPoint(candidate[j]), pointMeans[i].pt, pointMeans[i-1].pt);
        }
        //just an exception case, protecting the pointMeans bounds i and i-1
        else{
          sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint(candidate[j]));
        }
        j++;
        edts++;
      }
      //else, assume we're just off the mark, and let distance accumulate
      //note we hit this case either if the next letters also differ, or if the current (differing) letters includes the last letter of candidate
      else{
        sumDist += layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint(candidate[j]));
      }
    }

    //dont forget to advance over repeated chars in the candidate
    if(j < (candidate.size()-1) && candidate[j] == candidate[j+1]){
      j++;
      rpts++;
    }
  }
  //end loop: either candidate or input-sequence index reached end. At most, one of these includes remaining characters.

  // ...so account for differences in string length
  //flat metric: diff the adjusted lengths, and multiply by some static value
  //sumDist += ((layoutManager->AbsDiff(candidate.size()-rpts,pointMeans.size()-edts)) * (layoutManager->minKeyRadius * 5.0));
  //natural metric: continue summing distance for remaining characters in either string
  //only one or neither of these loops will execute, since either (or both) i or j is at end of its sequence
  while(i < pointMeans.size()){
    sumDist +=  layoutManager->DoubleDistance(pointMeans[i].pt, layoutManager->GetPoint(candidate[j-1]));
    i++;
  }
  while(j < candidate.size()){
    sumDist +=  layoutManager->DoubleDistance(pointMeans[i-1].pt, layoutManager->GetPoint(candidate[j]));
    while(j < candidate.size()-1 && candidate[j] == candidate[j+1]){  //chew up repeated chars in candidate
      j++;
      rpts++;
    }
    j++;
  }

  //sumDist += (rpts*2.0); //small enough to distinguish DOOG from DOG, but not too large to override the overall distance logic
  //sumDist += rpts * (layoutManager->minKeyDiameter * 999.0));

  //finally, give some small
  sumDist += ((double)rpts * layoutManager->minKeyRadius * 0.15); //token punishment to separate FOXX from FOX

  //}
  return sumDist;
}





