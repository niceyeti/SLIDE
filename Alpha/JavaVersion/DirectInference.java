import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Collections; //sorting
import java.util.ArrayList;



/*
  This is the primary "inference" class. The signal processor class just does its best to create a stream of likely key presses,
  while this class takes that stream (as a string) and tries to infer to which words in the vocab model the stream is nearest.
  Thus it can receive input from either the SignalProcessor output, or from any other input source that formats a stream
  of key presses as a list of type ArrayList<PointMu>.

  Given some sequence of key-presses, the class searches across an entire lexicon (a word set) looking for the words whose geometry
  on the ui is *closest* to that of the input sequence. So if the input is "DOG", then "DOG", "FOG" and "FIG" might be very
  nearby words, in geometric terms. Note the exception cases for input sequences and words of differing lengths. All of 
  that is handled by this class. The inference method is really a cross between the pure-geometric approach of comparing 
  word/input geometries and classic edit distance metrics. But at a high level, the geometry-based approach is very effective
  in that it creates a strong linear separator between terms, since the geometry of all words is highly unique.

*/
public class DirectInference
{
  KeyMap m_keyMapRef;
  Set<String> m_wordSet;

  //DO NOT USE DEFAULT CTOR
  public DirectInference(){
    m_wordSet = new HashSet<String>();
    String filePath = "../resources/vocabModel.txt";
    System.out.println("WARNING default DirectInference ctor called with no input word-file param. Only use the constructor with a file param.");
    System.out.println("Default ctor guessing file path is >"+filePath+"< which will fail if path is invalid...");
    BuildWordSet(filePath);
  }
  //ONLY USE THIS CTOR
  public DirectInference(String vocabFileName, KeyMap kmap){
    if(kmap == null){
      System.out.println("ERROR null keyMap reference passed to DirectInference constructor, expect crash...");
    }
    m_keyMapRef = kmap;
    m_wordSet = new HashSet<String>();
    System.out.println("Building word model from >"+vocabFileName+"< for DirectInference class ctor...");
    BuildWordSet(vocabFileName);
  }

  /*  Expect word model as flat file word database, one (unique) word per line.
      Requirements for the word model (at least for fastmode) are no punctuation,
      and all upper case. This is because the fastmode is more conversaitonal than
      grammatical, selecting complete words rather than contractions or other. Its
      really more of a phonetic word model than a written word model. Forward all complaints
      to the American Spell Check society.
  */
  private void BuildWordSet(String vocabFile)
  {
    try{
      //m_wordSet = new Set<String>();
      BufferedReader br = new BufferedReader(new FileReader(vocabFile));
      String line;

      if(m_wordSet.size() > 0){
        m_wordSet.clear(); //clear the word model if it contains existing items
      }

      while((line = br.readLine()) != null){
        // process the line
        String upper = line.toUpperCase();
        m_wordSet.add(upper);
      }

      br.close();
      System.out.println("Building word model completed. wordSet.size()="+Integer.toString(m_wordSet.size()));
    }
    catch(java.io.FileNotFoundException ex){
      System.out.println("ERROR FileNotFoundException thrown for file: "+vocabFile);
    }
    catch(java.io.IOException ex){
      System.out.println("ERROR IOException caught for file "+vocabFile+" due to cause: "+ex.getCause());
    }
  }

  /*
    Primary method/driver. This takes in a vector of X/Y points (triggers or simulated key-presses)
    and tries to correlate these with words to find the nearest word.
  */
  public ArrayList<SearchResult> Process(ArrayList<PointMu> pointMeans)
  {
    ArrayList<SearchResult> results = new ArrayList<SearchResult>();

    if(pointMeans.isEmpty()){
      System.out.println("WARN no data passed to DirectInference.Process(), returning zero results");
      return results;
    }

    //clear any existing results
    if(results.size() > 0){
      results.clear();
    }

    //The inference method. Other methods were tested (string distance methods, combined methods, etc), but no worries,
    //The vector distance method outperformed all of them, by enormous margins. The other versions can be found in the C++ version.
    VectorDistInference(pointMeans,results);

    /*
    //print the results, for debugging
    int i;
    for(i = 0; i < 75 && i < results.size(); i++){
      System.out.println(Integer.toString(i)+": "+results.get(i).toString());
    }
    */

    return results;
  }

  /*
    An exhaustive coordinate-vector comparison method. Given a sequence of XY points (stochastic triggers or
    simulated key presses), this searches over all words in the word model, evaluating each word's distance
    to the input sequence. Nearby words (a distance below some magic threshold) are pushed to the result list,
    list is sorted and returned, nearest word first.
  */
  private void VectorDistInference(ArrayList<PointMu> pointMeans, ArrayList<SearchResult> results)
  {
    int i;
    double dist, min;

    //System.out.println("In DirectInference, process...");

    if(results.size() > 0){
      results.clear();
      //results.reserve(wordSet.size() * 4);
    }

    //min = 99999;
    for(String word : m_wordSet){
    //for(String it = wordSet.begin(); it != wordSet.end(); ++it){
      dist = VectorDistance(pointMeans,word);
      //dist = VectorDistance(ArrayList<PointMu>,revArrayList<PointMu>,it);  //overload for fwd-bkwd versions
      //if(dist < min){
      //  min = dist;
      //}

      //TODO: this is a magic number, change it to parameter elsewhere to make it more obvious as an input param
      if(dist < 1500){  //push threshold: nearby words typically occur w/in a sum distance of 1500
	      //System.out.println("pushing >" << *it << "," << dist << "<");
	      //results.add(SearchResult{*it,dist});
        results.add(new SearchResult(dist,word));
      }
    }

    Collections.sort(results);
  }

  /*
    Core utlility of the class.  Multiple distance metrics will undoubtedly need to be devised...
    This one should only be geometry-based; later language modeling class can handle unification
    
    Distance metric currently assumes that at least the first cluster aligns with the first letter's coordinates.
  */
  private double VectorDistance(ArrayList<PointMu> pointMeans, String candidate)
  {
    double dist = 99999.0; // infinite distance, buries this result if returned by default

    //Small optmization to only compare words within +/- k length of eachother.
    //It is important to think about this value before modifying it. You want to be generous,
    //so as to encompass many words in the scope of the search; but you also want to be constrained (use a small number)
    //so the search space is smaller, and thus faster. The problem is that the input sequence does not represent
    //repeated characters in many words, like 'S' and 'P' in MISSISSIPPI. So making the value too small may omit the target word,
    //if it has k repeated characters, and the chosen omission threshold here is < k, then that word will be effectively omitted.
    if(Utilities.AbsDiff(pointMeans.size(),candidate.length()) <= 4){
      //calculate the distance value for this input sequence and some word in our word model
      dist = SumDistMetric_Aligned(pointMeans,candidate);
    }

    return dist;
  }

  /*
    //aligned version of previous. this is a much harder problem to program.
    //Currently just looks left/right two chars for a nearer neighbor key

    Intuitively, this is trying to gather the distance of all input points to the nearest, sequential ray
    trace given by the word's coordinates. There is probably a better approach to this than the hacky
    state-based method use here (detect-error, look-ahead, punish, discount, etc).

    This handles insertion error well. But not deletion error, such as if SignalProcessor outputs "QUCK" instead of "QUICK".
    Try to handle "QUCK" "QUICK" case. This may be handled the same as insertion error, by advancing the candidate instead of the
    cluster-point sequence.

    The error values for different types of error are calibrated like a small constraint programming problem. For instance,
    really junky, significant differences in the distance between two geometric sequences should be punished severely. However,
    if one word is a root of another, don't punish the difference in input too much, lest one actually is a root of the other.
    Likewise, give a very small punishment to strings which differ only in the number of repeats, so "DOG" has a higher rank than
    "DOGG" (since repeated chars are eaten). In this way, the intent is to go from coarse to fine "punishment" in terms of string distance,
    such that each successive measure only re-ranks items within some subgrouping.

    An easier way to understand the error arithmetic is in terms of base-ten numbers. Say I have two models that provide scores
    for some objects. The first model gives more significant information than the second, such that I always want to depend more
    on the first model, and rely on the second model only as a tie breaker (or when the first model has no information).
    The easy thing to do is to multiply the 1st model's score for 10, and multiply the scores of the second model by 1, and sum them.
    Thus the scoring is a sum of powers of 10: score1 * 10^1 + score2 * 10^0. Under this scheme, as long as range of each model's scores
    are less than the bucket size (so ranging from 0-9, in this base-ten scheme), then it will always be the case that the first model has
    highest precedence in terms of scoring, followed by the second model. This can be extended to an arbitrary number of models by simply
    appending "buckets" for new models (which are just the columns of some base ten number). So a three model system would be
    score3 * 10^2 + score2 * 10^1 + score1 * 10^0.

    "_Aligned" suffix of this function comes from the fact that this version does a modest edit-distance estimate to compensate for insertion
    and deletion errors in the input stream (extra or missing characters, such as "DOGG" or "DG" instead of "DOG"). True edit-distance
    metrics are dynamic programming methods that use quadratic space/time (size and complexity is string1.length * string2.length). So
    while this method doesn't do that, it approximates the quadratic edit-distance methods for insert/delete accounting in linear time
    by skipping chars and looking for string-realignment using basic assumptions, like checking if the next-two chars match in each
    string, etc. This works only because words are somewhat short (5-15 chars or so). If the input strings were of arbitrarily large
    length, then the approximation wouldn't work and you'd have to use the real dynamic programming methods (which are available in the C++ version).
    "Unaligned" versions were also tested, but this version performed the best, in linear time. Unaligned refers to ignoring insertion
    and deletion errors all together. You just scan the characters in each string pairwise for equality, so if there is an extra char
    in one string, then all the rest of the equality tests fail (Hamming distance). But since insert/deletion errors are infrequent, you can 
    cheat by just running Hamming distance in both directions, and summing the result: pairwise scan each string from the beginning of each string,
    then run it backward by starting from the end of each string. The error will act as a pivot. The assumption is that even with the error(s),
    running in both directions over the correct edit-word and the user input will give a value that is at least less than all (or nearly all)
    other words in the word database. There will still be some error, especially for shorter words (which contain less information), but you
    might decide to tolerate this for the sake of speed, and then just run language estimates (word n-gram) probabilities over the first
    twenty or so results in the output to find the more likely word, per a language model.
  */
  private double SumDistMetric_Aligned(ArrayList<PointMu> pointMeans, String candidate)
  {
    double sumDist, dist;
    int i, j, rpts, edts;

    //System.out.println("In sumdistmetric...");

    //assumes a tight edit-error bound, such that alignment of points occurs within +/- one index 
    rpts = edts = 0;
    sumDist = dist = 0.0;
    for(i = 0, j = 0; i < pointMeans.size() && j < candidate.length(); i++, j++){
      //optimization: only compare distances if letters differ
      if(pointMeans.get(i).GetAlpha() != candidate.charAt(j)){
        //insertion error: essentially, this is a soft check whether *deleting* (skipping) the current letter yields realignment
        if((i+1) < pointMeans.size() && pointMeans.get(i+1).GetAlpha() == candidate.charAt(j)){
          if(j > 0){  //accumulate error, so this character difference receives at least some punishment
            sumDist += InsertionError(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j-1)), m_keyMapRef.GetPoint(candidate.charAt(j)));
          }
          else{ //the first is just an exception case, when we don't have two points to compare. so just compare char i to j
            sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
          }
          i++;  //skip the input error
          edts++;
        }
        //new, 1/14/14: attempts to handle cluster="QUCK" candidate="QUICK" case, where letters are missing from the cluster-points. 
        //In this case, do the opposite of the insertion error case, advancing candidate index
        else if((j+1) < candidate.length() && pointMeans.get(i).GetAlpha() == candidate.charAt(j+1)){
          //deletion error may be remedied the same as insertion error; however, observe that most deletions occur
          //when char i is detected, but i+1 is not, since its just too near to detect. Thus, the distance should be assessed wrt i,
          //instead of the midpoint (as for insertion error).
          if(i > 0 && pointMeans.size() > 1){
            sumDist += Point.DoubleDistance(pointMeans.get(i-1).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
            //sumDist += InsertionError(m_keyMapRef.GetPoint(candidate.charAt(j)), pointMeans.get(i).GetPoint, pointMeans.get(i-1).GetPoint());
          }
          //just an exception case, protecting the pointMeans bounds i and i-1
          else{
            sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
          }
          j++;
          edts++;
        }
        //else, assume we're just off the mark, and let distance accumulate
        //note we hit this case either if the next letters also differ, or if the current (differing) letters includes the last letter of candidate
        else{
          sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
        }
      }

      //advances over repeated chars in the candidate. This is tracked with the rpts var, so we at least know about it
      if(j < (candidate.length()-1) && candidate.charAt(j) == candidate.charAt(j+1)){
        j++;
        rpts++;
      }
    }
    //end loop: either candidate or input-sequence index reached end. At most, one of these includes remaining characters.

    // ...so here we account for differences in string length
    //The natural metric is to continue summing distance for remaining characters in either string.
    //Only one or neither of these loops will execute, since either (or both) i or j is at end of its sequence.
    while(i < pointMeans.size()){
      sumDist +=  Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j-1)));
      i++;
    }
    while(j < candidate.length()){
      //System.out.println("i="+Integer.toString(i)+" j="+Integer.toString(j)+" candidate: "+candidate);
      sumDist +=  Point.DoubleDistance(pointMeans.get(i-1).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
      while(j < (candidate.length()-1) && candidate.charAt(j) == candidate.charAt(j+1)){  //chew up repeated chars in candidate. We assume there aren't any in the input sequence, hence we don't do this in the previous loop
        j++;
        rpts++;
      }
      j++;
    }

    //finally, add some small punishment to repeats
    sumDist += ((double)rpts * m_keyMapRef.GetMinInterKeyRadius() * 0.15); //token punishment to separate FOXX from FOX, DOGG from DOG

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
  private double InsertionError(Point errorPt, Point pt1, Point pt2)
  {
    double correctionWeight;

    //midpoint correction likely over-corrects, punishing correct strings.
    //correctionWeight = MidPointCorrection(errorPt, pt1, pt2);
    //nearest point correction returns punishment for error as distance to nearest adjacent correct alpha, since most edit errors occur near a correct neighbor alpha
    correctionWeight = NearestPointCorrection(errorPt, pt1, pt2);

    return correctionWeight;
  }
  /*
    Returns error-correction approximation as the dist to the nearest of two neighbor points.
    That is, is E is an insertion-error in input sequence Beta, then return the dist to the nearest of
    its correct neighbors.  Examples are Beta="FOCX" for which E='C' with respect to the candidate string "FOX".
    Clearly for this case we don't want simply the midpoint, since it will punish the candidate score too much.
    The intuition here is that most insertion errors will occur adjacent to one of the neighbors in the candidate.
  */
  private double NearestPointCorrection(Point errorPt, Point pt1, Point pt2)
  {
    double dist1, dist2, ret;

    dist1 = Point.DoubleDistance(errorPt,pt1);
    dist2 = Point.DoubleDistance(errorPt,pt2);

    if(dist1 < dist2){
      ret = dist1;
    }
    else{
      ret = dist2;
    }

    return ret;
  }
} // end DirectInference class

