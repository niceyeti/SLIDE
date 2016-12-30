/*
  This is a separate implementation which uses dynamic programming, instead of the fast-mode method.
  This is experimental, to see how well dynamic programming performs. This is not intended
  to be a space or time-efficient method, its only intended to see how the principle of optimality
  might be used to solve the geometric-sequence comparison problem optimally.

  Compile with:
    javac -g Point.java Key.java KeyMap.java DpTwitch.java
*/

import java.io.*;
import java.nio.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Iterator;
import java.util.Set;
import java.util.Map;
import java.util.Comparator;

public class DpTwitch
{
  //for backtracking
  public enum Direction{UP, LEFT, DIAG}

  //primitive dp matrix cell
  private class MatrixCell{
    public double Score;
    public Direction Backpointer;

    public MatrixCell(){
      Score = 10000000;
      Backpointer = Direction.DIAG;
    }
  }

  //for use or just experimentation
  SignalProcessor _signalProcessor;

  KeyMap _keyMap;
  MatrixCell[][] _matrix;  
  int MAX_ROWS;
  int MAX_COLS;
  Direction _direction;
  
  public DpTwitch(String keyMapFile)
  {
    int i, j;

    MAX_ROWS = 1000;
    MAX_COLS = 100;

    //build the keymap
    _keyMap = new KeyMap(keyMapFile);
    //build the signal processor; this may or may not be used
   	_signalProcessor = new SignalProcessor(_keyMap,14,16,4);
    //build the matrix; no effort for space efficieny here...
    _matrix = new MatrixCell[MAX_ROWS][MAX_COLS];
    for(i = 0; i < MAX_ROWS; i++){
      for(j = 0; j < MAX_COLS; j++){
        _matrix[i][j] = new MatrixCell();
      }
    }
  }

  //Implements the recurrence for each cell, minimizing cost
  private void _scoreCell_DpTwitch(int row, int col, Point datum, Point keyPoint, MatrixCell[][] matrix)
  {
    //TODO: this assumes row and col are positive, non-zero. Needs error check
    if(matrix[row][col-1].Score < matrix[row-1][col].Score){
      //left cell is greater, so take from it and point back to it
      matrix[row][col].Score = matrix[row][col-1].Score + 1.0 * Point.DoubleDistance(datum,keyPoint);
      //matrix[row][col].Score = matrix[row][col-1].Score + Point.CityBlockDistance(datum,keyPoint);
      matrix[row][col].Backpointer = Direction.LEFT;
    }
    else{
      //upper cell is greater, so take from it instead and point up
      matrix[row][col].Score = matrix[row-1][col].Score + 1.0 * Point.DoubleDistance(datum,keyPoint);
      //matrix[row][col].Score = matrix[row-1][col].Score + Point.CityBlockDistance(datum,keyPoint);
      matrix[row][col].Backpointer = Direction.UP;
    }
  }
  
  private void _scoreCell_DpTwitch2(int row, int col, Point datum, Point keyPoint, MatrixCell[][] matrix)
  {
	double dist = Point.CityBlockDistance(datum,keyPoint);
	//double dist = Point.DoubleDistance(datum,keyPoint);

	if(matrix[row-1][col-1].Score < matrix[row-1][col].Score && matrix[row-1][col-1].Score < matrix[row][col-1].Score){
		matrix[row][col].Score = matrix[row-1][col-1].Score + 1.0 * dist;
		matrix[row][col].Backpointer = Direction.DIAG;
	}
	else if(matrix[row][col-1].Score < matrix[row-1][col].Score){
		//left cell is greater, so take from it and point back to it
		matrix[row][col].Score = matrix[row][col-1].Score + 1.0 * dist;
		matrix[row][col].Backpointer = Direction.LEFT;
	}
	else{
		//upper cell is greater, so take from it instead and point up
		matrix[row][col].Score = matrix[row-1][col].Score + 1.0 * dist;
		matrix[row][col].Backpointer = Direction.UP;
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

  public void TestWord(String inputFile, String hiddenWord)
  {
    int i;
    double score;

    //grab the test data
    ArrayList<Point> testPoints = BuildTestData(inputFile);
    ArrayList<Point> wordPoints = _keyMap.WordToPointSequence(hiddenWord);

    System.out.println("TestWordStream testing "+Integer.toString(testPoints.size())+" raw data points");

    score = CompareSequences(testPoints,wordPoints,-1.0);
    
    System.out.println("Score: "+Double.toString(score));
  }

  /*
  Rather than taking two Strings as input sequences for normal optimal-alignment algorithms,
  this compares sequences of geometric coordinates as x/y pairs. The first input sequence is expected
  to be some sensor data, and the second sequence representing the sequence of letter coordinates
  of some word on some ui layout grid. So the input sequence is expected to be significantly 
  longer (unfiltered) than the "hidden" word letter-coordinate sequence.
  
  @threshold: If this holds a positive value, sequence comparison will bail and return INF-distance measure (just some
  large distance score). This averts the needless computation of highly-distant words.
  */
  public double CompareSequences(ArrayList<Point> inputSequence, ArrayList<Point> wordSequence, double threshold)
  {
    int i,j;
    int INF = 10000000;
	double rowMin = 0;

    if(inputSequence.size() <= 1){
      System.out.println("ERROR sequence1 too short for dpTwitch"+inputSequence.size());
      return -1;
    }
    if(wordSequence.size() <= 1){
      System.out.println("ERROR sequence2 too short for dpTwitch: "+wordSequence.size());
      return -1;
    }

    //initialize the dp table, for distance minimization
    _matrix[0][0].Backpointer = _direction.UP;
    _matrix[0][0].Score = Point.DoubleDistance(inputSequence.get(0), wordSequence.get(0));
    for(j = 1; j < wordSequence.size(); j++){
      //init the first row
      _matrix[0][j].Score = Point.DoubleDistance(inputSequence.get(0), wordSequence.get(j)) + _matrix[0][j-1].Score;
      _matrix[0][j].Backpointer = Direction.LEFT;
    }
    for(i = 1; i < inputSequence.size(); i++){
      //init the first column
      _matrix[i][0].Score = Point.DoubleDistance(inputSequence.get(i), wordSequence.get(0)) + _matrix[i-1][0].Score;
      _matrix[i][0].Backpointer = Direction.UP;
    }

    //run the forward algorithm
    for(i = 1; i < inputSequence.size(); i++){
      rowMin = INF;
      for(j = 1; j < wordSequence.size(); j++){
        //the recurrence
        _scoreCell_DpTwitch2(i,j,inputSequence.get(i),wordSequence.get(j),_matrix);
        if(_matrix[i][j].Score < rowMin){
        	rowMin = _matrix[i][j].Score;
        }
      }
      if(threshold > 0 && rowMin > threshold){
      	return INF;
      }
    }

    //_printMatrix(inputSequence.size(),wordSequence.size());

    //for optimal global alignment, the bottom-rightmost cell will have the optimal score for this alignment
    return _matrix[inputSequence.size()-1][wordSequence.size()-1].Score;
  }

  private void _printMatrix(int numRows, int numCols)
  {
    int i, j;

    //print the matrix
    for(i = 0; i < numRows; i++){
      for(j = 0; j < numCols; j++){
        //System.out.print(_matrix[i][j].Score);
        if(_matrix[i][j].Backpointer == Direction.UP){
          System.out.print(" ^ ");
        }
        if(_matrix[i][j].Backpointer == Direction.LEFT){
          System.out.print(" < ");
        }
      }
      System.out.print("\r\n");
    }
  }

  /*
  Given a file containing an input stream of sensor points, compares that sequence with
  every word in the vocabulary. This is brute-force, inefficient.
  */
  public void Test(String inputFile)
  {
    int i = 0;
    double dist;
    double minDist = 9999999;
    Vocab vocab = new Vocab("./resources/languageModels/vocab.txt");
    ArrayList<Point> rawInput = BuildTestData(inputFile);
    ArrayList<SearchResult> results = new ArrayList<SearchResult>();

	//experimental: optionally filter the input points, and check the effect on performance
	ArrayList<PointMu> pointMus = _signalProcessor.Process(rawInput);
	ArrayList<Point> testPoints = PointMu.PointMuListToPointList(pointMus);
	//testPoints = _signalProcessor.SlidingMeanFilter(testPoints,12);
	System.out.println("num test points: "+testPoints.size());

    for(String word : vocab){
      ArrayList<Point> hiddenSequence = _keyMap.WordToPointSequence(word);
      dist = CompareSequences(testPoints,hiddenSequence,-1.0);
      if(dist > 0){
        SearchResult result = new SearchResult(dist,word);
        results.add(result);
        if(dist < minDist){
        	minDist = dist;
        }
      }
      
      i++;
	  /*
      if(i > 30){
        break;
      }
      */
      if(i % 1000 == 999){
	    System.out.println(i);  
      }
    }
    
    Collections.sort(results);
    
    i = 0;
    System.out.println("Top 20 of "+Integer.toString(results.size())+" results: ");
    for(SearchResult result : results){
      System.out.print(Integer.toString(i+1)+":  ");
      result.Print();
      i++;
      if(i > 100){
        break;
      }
      //System.out.print("\r\n");
    }
  }

  public static void main(String[] args)
  {
    String wordFile = "./resources/testing/performance/word12.txt";
    String word = "MISSISSIPPI";
    String keyMapFile = "./resources/ui/keyMap.txt";
    DpTwitch twitch = new DpTwitch(keyMapFile);

    twitch.TestWord(wordFile,word);
    twitch.TestWord(wordFile,"BILOXI");
    twitch.TestWord(wordFile,"ALABAMA");
    twitch.TestWord(wordFile,"ABC");

    twitch.Test(wordFile);
    
  }
}
