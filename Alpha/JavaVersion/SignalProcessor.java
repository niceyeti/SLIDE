import java.io.*;
import java.util.ArrayList;



/*
  The core responsibility of this class is to consume a stream of raw (x,y) coordinates from the eye tracking hardware,
  and the extract the likely key-events within that stream. For instance, it may be a characteristic that when the user
  hits a key, the standard deviation of the last k x/y tuples falls below some threshold, signalling high density. In that
  very simple pattern recognition scheme, this class would then seek to extract from the stream all the events that occurred
  when the standard deviation falls below this threshold, signalling a likely key-press. That's just one model, and there are
  many kinematic parameters you can throw at the input stream to do pattern recogntion: dx/dy, standard deviation, 
  acceleration/velocity vectors, theta, delta-theta, anything you can think of that will help identify when a key press occurs, 
  but without overestimating them. Note also a side effect of event-based pattern recognition is that two consecutive events may
  be triggered for the same key due to noise, and other unusual events. So it is a subsidiary responsibility of this class
  to do things like consolidating repeated events, and so on, to clean the input and output as much as possible before sending
  it along to the inference class.

  In engineering terms I guess you might call this a transformer class, or a "filter" in a pipe-and-filter pattern: the class
  has no data members, it just takes an input stream, transforms it somehow, and passes along its output to the next class.
  It needs a reference to a KeyMap, and is currently the only FastMode sub-class accessing the key map. But its conceivable other
  classes might need to do so also in the future, so I put the keyMap at the outer class level, and pass it as a ref to this subclass.
*/
public class SignalProcessor
{
  KeyMap _keyMapRef;
  int _dxThreshold;
  int _innerDxThreshold;
  int _triggerThreshold;

  public SignalProcessor(){
    System.out.println("ERROR SignalProcessor default constructor called, with no keyMap parameter. Use ctor with KeyMap parameter only.");
  }
  
  public SignalProcessor(KeyMap kmap, int dxThresh, int innerDxThresh, int triggerThresh){
    if(kmap == null){
      System.out.println("ERROR null keyMap reference passed to SignalProcessor constructor, expect crash...");
    }
    _keyMapRef = kmap;
    SetEventParameters(dxThresh,innerDxThresh,triggerThresh);
  }

  public void SetEventParameters(int dxThresh, int innerDxThresh, int triggerThresh){
    _dxThreshold = dxThresh;
    _innerDxThreshold = innerDxThresh;
    _triggerThreshold = triggerThresh;
  }

  /*
  Just for research: given a list (stream) of x/y points, returns a list of points that are
  the sliding k-width window means of those points. @k is the frame width, such that for each
  k points, the mean is taken as their protoype and appended to the output; then you slide
  k points to the next frame of k-points, and repeat.
  
  This is a pre-filter function, closer to raw signal conditioning, but targeting application specific properties.
  */
  public ArrayList<Point> SlidingMeanFilter(ArrayList<Point> pointStream, int k){
	double muX, muY;
  	ArrayList<Point> output = new ArrayList<Point>();
  	
  	for(int i = 0; i < (pointStream.size() - k); i += k){
		//get the mean point of this frame of points
		muX = muY = 0.0;
  		for(int j = 0; j < k; j++){
  			muX += pointStream.get(i+j).GetX();
  			muY += pointStream.get(i+j).GetY();
  		}
  		muX /= (double)k;
  		muY /= (double)k;
  		//Point mu((int)muX,(int)muY);
  		output.add(new Point((int)muX,(int)muY));
  	}
  
  	return output;
  }

  //Main method needs at least 4 data points to operate (though really more should be required, as a safety).
  private boolean SufficientData(ArrayList<Point> inData)
  {
    int i, validDataPoints;

	i = 0;
    validDataPoints = 0;
    while(i < inData.size()){
      if(_keyMapRef.InBounds(inData.get(i))){
        //System.out.println("In bounds: "+inData.get(i).toString());
        validDataPoints++;
        if(validDataPoints >= (3 * _triggerThreshold)){
          return true;
        }
      }
      i++;
    }

    return false;
  }

  /*
    The primary method. Given an raw input stream/list of (x,y), this attempts to extract the likely key presses
    using pattern recognition schemes. Don't get all hot and impressed with the term "pattern recognition", because
    most of the logic therein just comes from basic kinematics and statistics. In other words, be creative when trying
    to come up with better, faster methods for the event/key-press extraction process. Come up with whatever you can
    in order to anticipate key presses. The challenge is not just detecting events/key presses, but not over-detecting
    them either. A good testing practice to find better event-detection methods and values is to simply define those values
    and print them out, and sketch a graph for how and when they change. For instance, the average theta value (direction vector)
    for the last few points provides a good idea of current direction; thus, delta-theta, some rapid change in direction,
    could be used to help detect events. However, not all events entail a direction change, so it may be a sufficient but not a
    necessary characteristic for triggering an event. There's a lot of data values which you can extract from the raw input
    stream that work this way, the question is how to leverage them, and how perhaps to map them into something like an SVM
    or a neural network that can magically decide for itself when to trigger an event.

    The conservative and reliable measure is to simply use the product of the standard deviation of the last k points, where k is a parameter
    that may depend on the sensor rate (30 or 50 Hx, currently) or other factors. That is, read the input stream and calculate the
    product of the standard deviations of the last say, three (k=3) X and Y points:
      stDevProduct = stdev(point1.X,point2.X,point3.X) * stdev(point1.Y,point2.Y,point3.Y)
    Since this is a product, the value is very high when the eye is moving rapidly or afar to some new key, and the value decreases
    rapidly when the eye is focused on a key (signalling high input density). In my own testing I found this to be a value with very
    reliable characteristics, such that you can easily trigger an event when the stDevProduct falls below some threshold, signalling
    focus on a key. However, per the earlier comments, it is a trailing value. It is very good in the sense that it would be easy for the user
    to "learn" the trigger characteristics of this value, but there are leading values like dy/dx and delta-theta that could help anticipate
    events and trigger them with even greater precision and reliability and of course faster.

    But as far as signal processing goes, realize this is a bit of a goldilocks function. The parameters for triggering events
    need to be "just so", not too hot, not too cold. You just play with the parameters until they feel right, because they 
    will have to change with the sensor hardware. You can factor the parameters out as best you can, but their values still
    just have to be determined by physical testing.
  */
  public ArrayList<PointMu> Process(ArrayList<Point> inData)
  {
    ArrayList<PointMu> intermediateData = new ArrayList<PointMu>();
    ArrayList<PointMu> outputMeans = new ArrayList<PointMu>();
    int i, trigger, eventStart, eventEnd;
    double dx;

    if(!SufficientData(inData)){
      System.out.println("WARN insufficient valid data points for SignalProcessor.Process(). "+Double.toString(3*_triggerThreshold)+" or more (valid) data points required.");
      return outputMeans;
    }

    //TODO: sleep if no data, realtime/streaming trigger model
    trigger = 0;
    for(i = 0; i < inData.size() - 4; i++){
      //ignore points outside of the active region, including the <start/stop> region
      if(_keyMapRef.InBounds(inData.get(i)) && _keyMapRef.InBounds(inData.get(i+3))){
        dx = Point.DoubleDistance(inData.get(i),inData.get(i+3));

        /*
        //debug output, to view how data vals change
        System.out.println("dx="+dx.toString());
        if(dx > 0){
          System.out.println("1/dx="+(1/dx).toString());
        }
        */

        //TODO: advancing the index i below is done without InBounds() checks
        if(dx < _dxThreshold){  //determine velocity: distance of points three ticks apart
          trigger++;           //receive n-triggers before fully triggering, to buffer noise; like using a timer, but event-based
          if(trigger >= _triggerThreshold){  //trigger event and start collecting event data
            //System.out.println("       trigger");
            //capture the event
            eventStart = i;
            while(i < (inData.size() - 4) && dx < _innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
              dx = Point.DoubleDistance(inData.get(i),inData.get(i+3));
              i++;
            }
            eventEnd = i;   // exited Event state, so store right bound of the event cluster
            //TODO: below is a small optimization to skip some data points following an event, eg, perhaps by dx difference of 3 data points.
            //i += 2;

            //get the mean point w/in the event cluster and store it
            PointMu outPoint;
            outPoint = CalculateMean(eventStart,eventEnd,inData);
            outPoint.SetAlpha(_keyMapRef.FindNearestAlphaKey(outPoint.GetPoint()));
            //System.out.println("Hit mean: "+outPoint.toString());

            //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
            if(intermediateData.isEmpty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
              intermediateData.add(outPoint);
            }
            //verify incoming alpha cluster is unique from previous alpha
            else if(outPoint.GetAlpha() != intermediateData.get(intermediateData.size()-1).GetAlpha()){
              intermediateData.add(outPoint);
            }

            //reset trigger for next event detection
            trigger = 0;
          } //exit the event-capture state, and continue streaming (reading inData and detecting the next event)
        }
      }
    }

    if(intermediateData.size() > 0){
      System.out.println("Clusters before merging...");
      PrintMeans(intermediateData);
      MergeMeans(intermediateData,outputMeans);
      //dbg
      //PrintOutData(outData);
    }
    else{
      System.out.println("WARN SignalProcessor.Process() detected no means in this raw input stream. Returning zero means.");
    }

    return outputMeans;
  }

  /*
    Takes a list of points, a begin and end index for some estimate point cluster,
    and returns the mean point of that cluster.

    Notes: This potentially suffers from the "elbow" problem, but I never found it to be a problem.
    Most "clusters" (events) trigger at a sharp angle: the user is gazing from one letter to another,
    creating a sharp corner in the trace of their "gazes". This means that the mean value of that cluster
    (including both edges of the angle) will be inside the angle, instead of very near the point. But we're
    after the point of this corner, when/if it occurs. But notice that with more points in the cluster 
    (a more precise trigger threshold for instance), or fewer extrema in that cluster, then the problem vanishes.
    Again, I never found it to be a problem, ever.
  */
  public PointMu CalculateMean(int begin, int end, ArrayList<Point> coorList)
  {
    PointMu pointMean = new PointMu();
    int sumX, sumY, ct, i; // pixel math is all integer based
    
    //TODO: error checks on begin and end, esp. wrt coorList.size() and so on
    //System.out.println("begin="+Integer.toString(begin)+" end="+Integer.toString(end)+" coorList.size()="+Integer.toString(coorList.size()));
    sumX = sumY = ct = 0;
    for(i = begin; (i < end) && (i < coorList.size()); i++){
      //System.out.println(coorList.get(i).toString());
      if(_keyMapRef.InBounds(coorList.get(i))){  //only accumulate valid points, not extrema outside the bounds of the key region of the ui
	      sumX += coorList.get(i).GetX();
	      sumY += coorList.get(i).GetY();
	      ct++;
      }
    }

    if(ct > 0){
      pointMean.GetPoint().SetXY(sumX / ct, sumY / ct);
    }
    else{
      System.out.println("ERROR ct==0 in CalculateMean ?? Should never occur");
      pointMean.GetPoint().SetXY(0,0);
    }

    //set the ticks (a confidence level for the cluster/event)
    pointMean.SetTicks(end - begin);

    return pointMean;
  }

  private void PrintMeans(ArrayList<PointMu> meansList)
  {
    int i = 1;
    for(PointMu mean : meansList){
      System.out.println(Integer.toString(i)+":  "+mean.GetPoint().toString()+" "+mean.GetAlpha());
      i++;
    }
  }

  /*
    A subsidiary responsibility of the SignalProcessor class is to clean the output. This
    removes redundant means from the output data. Means are "redundant" if they are the same
    character as the previous mean, or if they are within some very small distance of the previous
    mean point. This is intended to scrub over-detection cases, where the primary Process() function
    triggers too many events for activity in the same region, like if the user looks at 'A' and we receive 
    several triggers/events on 'A' or like on the border between 'A' and other keys.

    This may seem inefficient, like the Process() function should be responsible for not over-detecting. While the
    latter is the case (to a degree), its also the case that if you relax the Process() function and allow it to
    over-detect a little bit, then it can give very fast event-detection, but with less accuracy. But then we can
    just do some fast post-processing to remove the over-detection cases, which is what this function does. Its like
    allowing the Process() function to have a bit of an itchy-trigger finger so it always hits the mark, but then just
    correcting for it firing too many times on a single mark.
  */
  private void MergeMeans(ArrayList<PointMu> intermediateData, ArrayList<PointMu> mergedData)
  {
    int i;
    boolean lastMerge = false;

    if(!mergedData.isEmpty()){
      mergedData.clear();
    }

    for(i = 1; i < intermediateData.size(); i++){
      // Only append output clusters which exceeds some inter-key distance
      // Note that this discrimination continues until sufficient inter-cluster distance is achieved.
      if(HasMinSeparation(intermediateData.get(i-1),intermediateData.get(i))){
        mergedData.add(intermediateData.get(i-1));
      }
      //else, forward-accumulate the reflexive likelihood to preserve repeat char info
      else{
        System.out.println("Merged clusters "+Integer.toString(i-1)+"/"+Integer.toString(i));
        //identical alphas, so just merge the dupes, for instance, merge "AA" to "A"
        if(intermediateData.get(i-1).GetAlpha() == intermediateData.get(i).GetAlpha()){
          intermediateData.get(i-1).SetTicks(intermediateData.get(i-1).GetTicks() + intermediateData.get(i).GetTicks());
          mergedData.add(intermediateData.get(i-1));      
        }
        //else, point with more ticks (higher confidence) wins (outcome is same as prior 'if': these two blocks could be merged, but with less clarity
        else if(intermediateData.get(i-1).GetTicks() > intermediateData.get(i).GetTicks()){
          intermediateData.get(i-1).SetTicks(intermediateData.get(i-1).GetTicks() + intermediateData.get(i).GetTicks());
          mergedData.add(intermediateData.get(i-1));
        }
        else{
          intermediateData.get(i).SetTicks(intermediateData.get(i).GetTicks() + intermediateData.get(i-1).GetTicks());
          mergedData.add(intermediateData.get(i));
        }
        i++; //TODO: this advances the index to skip the next comparison. As a result, this method only merges two
             // consecutive 'merge-able' means, when in reality, there may be multiple ones. For that reason,
             // this should really be iteration (when minSep fails). That way successive errors are absorbed.

        //intermediateData.get(i).GetTicks() += intermediateData.get(i-1).GetTicks();
        if(i >= intermediateData.size()-1){
          lastMerge = true;
        }
      }
    }

    //variable detects if last two clusters were merged or not
    if(!lastMerge && !intermediateData.isEmpty()){
      mergedData.add(intermediateData.get(intermediateData.size()-1));
    }
  }

  /*
    Determines if two point means are "sufficiently" distinct to be called separate.
    Typical usage is verifying whether or not two consecutively detected point means are
    different enough (in terms of distance, char-id, or ticks) to say that they are separate
    input events, and not just over-detection. So this is a post-processing method.
  */
  public boolean HasMinSeparation(PointMu mu1, PointMu mu2)
  {
    if(mu1.GetAlpha() != mu2.GetAlpha()){  //TODO: this is redundant with a check in Process(). Oh well.
      if(Point.DoubleDistance(mu1.GetPoint(),mu2.GetPoint()) < (_keyMapRef.GetMinInterKeyRadius() * 1.5)){
        if(mu1.GetTicks() <= 4 || mu2.GetTicks() <= 4){  //time separation is INF for now
          System.out.println("minkeyrad: "+Double.toString(_keyMapRef.GetMinInterKeyRadius())+" dist: "+Double.toString(Point.DoubleDistance(mu1.GetPoint(),mu2.GetPoint())));
          System.out.println("mindist failed, ticks are ("+mu1.GetAlpha()+","+Integer.toString(mu1.GetTicks())+")  ("+mu2.GetAlpha()+","+Integer.toString(mu2.GetTicks())+")");
          return false;
  	    }
      }
    }

    return true;
  }

  //This function should only be used in testing, not release mode. It simply copies raw data to the output means, assuming the input
  //data is already a pre-processed set of point means (key presses) and not raw eye sensor data.
  public void DummyProcess(ArrayList<Point> rawData, ArrayList<PointMu> meansList)
  {
    if(!meansList.isEmpty()){
      meansList.clear();
    }

    for(Point pt : rawData){
      PointMu pmu = new PointMu(pt,0);
      meansList.add(pmu);
    }

    System.out.println("DummyProcess returning "+Integer.toString(meansList.size())+" output means");
  }
}  //end SignalProcessor class

