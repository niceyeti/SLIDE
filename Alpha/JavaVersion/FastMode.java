import java.io.*;
import java.nio.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.Map;
import java.util.Comparator;


//math?

/*
  TODO: file path escape for different operating systems (can Java resolve these internally???)
  TODO: all the public/private qualifiers and class-file break down
  TODO: coding standards, make em purty. Change the capital case methods to camel case, like the java libs, if desired...
  TODO: check and evaluate enforcement of uppercase requirement; use uppercase char encodings throughout, but perhaps catch potential lowercase leaks at their root
  TODO: keymap update stuff. m_keyMap is stateful, since the user may change the location and size of keys, so these will need to be update then
  TODO: check operator precedence usage compared with c++, especially in compound conditionals, etc

*/


public class FastMode{

  KeyMap m_keyMap;
  SignalProcessor m_signalProcessor;
  DirectInference m_directInference;

  //dont use the default constructor, as it hardcodes the keymap file path
  public FastMode()
  {
    String keyFilePath = "./resources/ui/keyMap.txt";
    System.out.println("Default FastMode constructor called. Ctor taking keymap file path parameter is perferred.");
    System.out.println("Attempting to build keymap with hardcoded path: "+keyFilePath);
    m_keyMap = new KeyMap(keyFilePath);
    m_signalProcessor = new SignalProcessor(m_keyMap,14,16,4);
    m_directInference = new DirectInference("./resources/languageModels/vocab.txt",m_keyMap);
  }

  public FastMode(String resourceDir)
  {
    if(Utilities.PathExists(resourceDir)){
      System.out.println("Building keymap");
      m_keyMap = new KeyMap(resourceDir+"ui/keyMap.txt");
      System.out.println("Building m_signalProcessor");
      m_signalProcessor = new SignalProcessor(m_keyMap,14,16,4);
      System.out.println("Building DirectInference");
      m_directInference = new DirectInference(resourceDir+"languageModels/vocab.txt",m_keyMap);
    }
    else{
      System.out.println("ERROR resourceDir >"+resourceDir+"< not found! FastMode construction failed");
    }
  }

  //Constructs test input data from some input file containing a raw stream (X,Y) coordinates, one coordinate per line.
  //This function checks some of the file formatting, but also strongly assumes it is valid.
  //Output is stored in the pointSequence.
  public ArrayList<Point> BuildTestData(String testFilePath)
  {
    ArrayList<Point> pointSequence = new ArrayList<Point>();

    try{
      BufferedReader br = new BufferedReader(new FileReader(testFilePath));
      String line;
      String[] tokens;

      while((line = br.readLine()) != null){
        tokens = line.trim().split("\t");
        if(tokens.length != 2){
          System.out.println("ERROR incorrect number of tokens for this line >"+line+"< in test input file "+testFilePath);
        }
        else if(!Utilities.IsIntString(tokens[0]) || !Utilities.IsIntString(tokens[1])){
          System.out.println("ERROR non-integer tokens for this line >"+line+"< in test input file "+testFilePath);
        }
        else{
          try{
            int x = Integer.parseInt(tokens[0]);
            int y = Integer.parseInt(tokens[1]);
            Point pt = new Point(x,y);
            pointSequence.add(pt);
          }
          catch(NumberFormatException ex){
            System.out.println("Number format exception >"+ex.getCause().getMessage()+"< caught for test input file >"+testFilePath+"<  Coordinates incorrectly formatted");
          }
        }
      }
      br.close();
    }
    catch(java.io.FileNotFoundException ex){
      System.out.println("ERROR FileNotFoundException thrown for file: "+testFilePath);
    }
    catch(java.io.IOException ex){
      System.out.println("ERROR IOException caught for file "+testFilePath+" due to cause: "+ex.getCause());
    }

    return pointSequence;
  }

  //Main class method: takes in a raw sensor stream, finds key events (means) within it, and then runs geometric word inference over these means
  public ArrayList<SearchResult> Process(ArrayList<Point> rawPoints)
  {
    //run sig processor
    ArrayList<PointMu> means = m_signalProcessor.Process(rawPoints);
    //run inference procedure over means
    ArrayList<SearchResult> results = m_directInference.Process(means);

    return results;
  }

  //Driver for consuming a file of raw input points, converting them to means, and finally running direct-inference method on those means.
  public void TestWordStream(String testFilePath)
  {
    //grab the test data
    ArrayList<Point> testPoints = BuildTestData(testFilePath);

    System.out.println("TestWordStream testing "+Integer.toString(testPoints.size())+" raw data points");

    //simulate signal processing and inference using these inputs
    ArrayList<SearchResult> results = Process(testPoints);

    //print top results
    if(results.size() > 0){
      PrintResults(6,results);
    }
    else{
      System.out.println("No results in TestWordStream() for file: "+testFilePath);
    }
  }

  //prints first k results in result list; if k=0, then print all results
  public void PrintResults(int k, ArrayList<SearchResult> results)
  {
    int i = 0;
    System.out.println("First "+Integer.toString(k)+" results:");
    while(i < k && i < results.size()){
      System.out.println(results.get(i));
      i++;
    }
  }

  /*
    Passes user event parameters to the SignalProcessor class.
    Usage might be to have optimizer scripts that repeatedly call fastModeObject.PerformanceTests()
    in order to find better and better event parameters.
  */
  public void SetEventParameters(int dxThresh, int innerDxThresh, int triggerThresh)
  {
    m_signalProcessor.SetEventParameters(dxThresh,innerDxThresh,triggerThresh);
  }

  //TODO: updating the key map (such as if the user resizes the ui region) is untested
  public void UpdateKeyMap(String keyCoordinateFileName)
  {
    if(m_keyMap != null){
      System.out.println("Updating m_keyMap...");
      m_keyMap.initKeyMap(keyCoordinateFileName);
    }
  }

  //See testing/regression/README
  public void RegressionTests(String sourceDir)
  {
    if(sourceDir.length() > 0 && sourceDir.charAt(sourceDir.length()-1) != '/'){
      sourceDir += "/";
    }

    if(Utilities.PathExists(sourceDir)){
      System.out.println("*************************************************************");
      System.out.println(">> Executing regression tests in "+sourceDir);

      //test valid word stream
      System.out.println("\n>> TEST: valid word");
      TestWordStream(sourceDir+"validWord.txt");
      
      //test a valid stream with only one coordinate in it
      System.out.println("\n>> TEST: stream with only one VALID coordinate/input");
      TestWordStream(sourceDir+"lengthOneValid.txt");

      //Some boundary Tests. This one test one below valid number of inputs
      System.out.println("\n>> TEST: stream with three VALID coordinate/inputs (one below valid number of inputs)");
      TestWordStream(sourceDir+"lengthThreeValid.txt");

      //Test valid input boundary (4 raw points)
      System.out.println("\n>> TEST: stream with four VALID coordinate/input (boundary of valid number of inputs)");
      TestWordStream(sourceDir+"lengthFourValid.txt");

      //Test a file with only one invalid data point (this is somewhat redundant with the empty input list test, since the input list will just be empty)
      System.out.println("\n>> TEST: stream with only one INVALID coordinate/input");
      TestWordStream(sourceDir+"lengthOneInvalid.txt");

      //Test empty input stream
      System.out.println("\n>> TEST: empty stream");
      TestWordStream(sourceDir+"lengthZero.txt");

      //Test very long input. Dont expect any results, since the distance from any word will be enormous, and over the omission threshold.
      System.out.println("\n>> TEST: very long input stream (of valid points)");
      TestWordStream(sourceDir+"longInput.txt");

      //Test a stream with all invalid points. Again, somewhat redundant with empty input stream test, since the stream will just be empty.
      System.out.println("\n>> TEST: stream with multiple points, all of which are INVALID");
      TestWordStream(sourceDir+"allInvalid.txt");

      System.out.println("\n>> Tests pass if no crashes, exceptions, or abnormal results");
      System.out.println("*************************************************************");
    }
    else{
      System.out.println("ERROR Path not found: "+sourceDir+" in RegressionTests()");
    }
  }

  /*
    Main test driver, given an input file containing a stream of (x,y) points generated from a sample run (a word)
    The test file dir, file names, and count are hard-coded. This is just for testing. But you can
    run this function without the hardware to test different input parameters to the signal processing, different inference
    logic, etc.
    TODO: send the tests with a read only mirror directory of the test inputs.

    This is a "performance test" only in terms of how it is currently being used. The source dir contains actual inputs from
    EyeTribe hardware, so you can consume these to perform any kind of tests you want. Don't get hyperactive about formal software testing,
    as the hardware itself may change, settings (eg, Hz), or different hardware may be used all together. I only focused on
    "things don't crash" and then testing to find the approximately-best parameters for signal processing and then direct inference.
  */
  public void PerformanceTests(String sourceDir)
  {
    int i;
    String testFilePrefix = "word";
    String testFileSuffix = ".txt";

    for(i = 1; i <= 12; i++){
      String fname = new String(sourceDir+testFilePrefix+Integer.toString(i)+testFileSuffix);
      System.out.println("Next test file: "+fname);
      TestWordStream(fname);
    }
    System.out.println("Performance test complete, no more test files.");
  }

  public static void main(String[] args){

    //TODO: map the pathEscape to different OS's: eg Windows requires '\\', linux requires '/'
    String sysPathEscape = "/";

    System.out.println("Args:");  
    for(int i = 0; i < args.length; i++){
      System.out.println(Integer.toString(i)+": "+args[i]);     
    }

    //TODO: striaghten out object input parameters so this is easy to use for clients
    if(args.length < 2){
      System.out.println("ERROR incorrect number of parameters. cmd line usage:  ./FastMode $RESOURCE_DIR_PATH $OPTS");
      return;
    }
    if(!Utilities.PathExists(args[0])){
      System.out.println("ERROR parameter >"+args[0]+"< does not exist. cmd line usage:  ./FastMode $RESOURCE_DIR_PATH $OPTS");
      return;
    }

    String resourceDir = args[0].trim();
    String opt = args[1].trim();
    if(resourceDir.length() > 0 && resourceDir.charAt(resourceDir.length()-1) != '/'){ //check if we need to append a slash to directory path
      resourceDir = resourceDir + sysPathEscape; 
    }
    System.out.println("Building FastMode dependencies from input resource dir "+resourceDir+"  ...");

    //Build fastmode
    FastMode fastMode = new FastMode(resourceDir);

    //map command line options: performance testing, maintenance/regression testing, etc
    if(opt.equals("-p")){
      //execute performance tests: assume normal/valid input, for demo'ing and for testing different input parameters
      System.out.println("Beginning performance tests...");
      fastMode.PerformanceTests(resourceDir+"testing/performance/"); 
    }
    else if(opt.equals("-r")){
      //executes a set of basic regression tests
      System.out.println("Beginning regression tests...");
      fastMode.RegressionTests(resourceDir+"testing/regression/");
    }
    else{
      System.out.println("ERROR option >"+opt+"< not recognized. Use -p to run performance tests, -r for regression tests.");
    }
  }
}  // end FastMode class

