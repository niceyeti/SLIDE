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
	private class MatrixCell
	{
		public double Score; //recursive score
		public double Dist; //geometric distance
		public Direction Backpointer;

		public MatrixCell()
		{
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
  int PHI_DIM;
  Direction _direction;
  
  public DpTwitch(String keyMapFile)
  {
    int i, j;

    MAX_ROWS = 1000;
    MAX_COLS = 100;
	PHI_DIM = 3;

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
  
  public DpTwitch(String keyMapFile, int phiDim)
  {
    int i, j;

    MAX_ROWS = 1000;
    MAX_COLS = 100;
	PHI_DIM = phiDim;

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
  //This recurrence is somewhat invalid, doesn't propagate minimal scores properly.
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
  
	private void _scoreCell_DpTwitch3(int row, int col, Point datum, Line line, MatrixCell[][] matrix)
	{
		double dist = line.PointLineDistance(datum);

		if(matrix[row-1][col-1].Score < matrix[row-1][col].Score && matrix[row-1][col-1].Score < matrix[row][col-1].Score){
			//diagonal substitution
			matrix[row][col].Score = matrix[row-1][col-1].Score + 1.0 * dist;
			matrix[row][col].Backpointer = Direction.DIAG;
		}
		else if(matrix[row][col-1].Score < matrix[row-1][col].Score){
			//insertion: left cell is greater, so take from it and point back to it
			matrix[row][col].Score = matrix[row][col-1].Score + 1.0 * dist;
			matrix[row][col].Backpointer = Direction.LEFT;
		}
		else{
			//deletion: upper cell is greater, so take from it instead and point up
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

  public void TestWordPointwise(String inputFile, String hiddenWord)
  {
    int i;
    double score;

    //grab the test data
    ArrayList<Point> testPoints = BuildTestData(inputFile);
    ArrayList<Point> wordPoints = _keyMap.WordToPointSequence(hiddenWord);

    System.out.println("TestWordPointwiseStream testing "+Integer.toString(testPoints.size())+" raw data points");

    score = ComparePointSequences(testPoints,wordPoints,-1.0);
    
    System.out.println("Score: "+Double.toString(score));
  }

  /*
  Rather than taking two Strings as input sequences for normal optimal-alignment algorithms,
  this compares sequences of geometric coordinates as x/y pairs. The first input sequence is expected
  to be some sensor data, and the second sequence representing the sequence of letter coordinates
  of some word on some ui layout grid. So the input sequence is expected to be significantly 
  longer (unfiltered) than the "hidden" word letter-coordinate sequence.
  
  Note that with some recurrences--and in fact the ones shown most successful--this function is 'barely' dynamic
  programming, and could possibly be approximated with a stack of some kind.
  
  @threshold: If this holds a positive value, sequence comparison will bail and return INF-distance measure (just some
  large distance score). This averts the needless computation of highly-distant words.
  */
  public double ComparePointSequences(ArrayList<Point> inputSequence, ArrayList<Point> wordSequence, double threshold)
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
		_scoreCell_DpTwitch(i,j,inputSequence.get(i),wordSequence.get(j),_matrix);
        //_scoreCell_DpTwitch2(i,j,inputSequence.get(i),wordSequence.get(j),_matrix);
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
  
	public double CompareLinearSequences(ArrayList<Point> inputSequence, ArrayList<Line> lineSequence, double threshold)
	{
		int i,j;
		int INF = 10000000;
		double rowMin = 0;

		if(inputSequence.size() <= 1){
			System.out.println("ERROR input sequence too short for dpTwitch"+inputSequence.size());
			return -1;
		}
		if(lineSequence.size() <= 1){
			System.out.println("ERROR line sequence too short for dpTwitch: "+lineSequence.size());
			return -1;
		}

		//initialize the dp table, for distance minimization
		_matrix[0][0].Backpointer = _direction.UP;
		_matrix[0][0].Score = lineSequence.get(0).PointLineDistance(inputSequence.get(0));
		//_matrix[0][0].Score = Point.DoubleDistance(inputSequence.get(0), lineSequence.get(0));
		for(j = 1; j < lineSequence.size(); j++){
			//init the first row
			_matrix[0][j].Backpointer = Direction.LEFT;
			_matrix[0][j].Score = lineSequence.get(j).PointLineDistance(inputSequence.get(0)) + _matrix[0][j-1].Score;
			//_matrix[0][j].Score = Point.DoubleDistance(inputSequence.get(0), wordSequence.get(j)) + _matrix[0][j-1].Score;
		}
		for(i = 1; i < inputSequence.size(); i++){
			//init the first column
			_matrix[i][0].Backpointer = Direction.UP;
			_matrix[i][0].Score = lineSequence.get(0).PointLineDistance(inputSequence.get(i)) + _matrix[i-1][0].Score;
			//_matrix[i][0].Score = Point.DoubleDistance(inputSequence.get(i), wordSequence.get(0)) + _matrix[i-1][0].Score;
		}

		//run the forward algorithm
		for(i = 1; i < inputSequence.size(); i++){
			rowMin = INF;
			for(j = 1; j < lineSequence.size(); j++){
				//the recurrence
				_scoreCell_DpTwitch3(i, j, inputSequence.get(i), lineSequence.get(j), _matrix);
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
		return _matrix[inputSequence.size()-1][lineSequence.size()-1].Score;
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
  every word in the vocabulary. This is brute-force, inefficient dynamic programming over
  the point sequence given by some word, which is a poor approximation of trace-to-word distance.
  */
  public void TestPointwiseDP(String inputFile)
  {
    int i = 0;
    double dist;
    double minDist = 9999999;
    Vocab vocab = new Vocab("./resources/languageModels/vocab.txt");
    ArrayList<Point> testPoints = BuildTestData(inputFile);
    ArrayList<SearchResult> results = new ArrayList<SearchResult>();

	//experimental: optionally filter the input points, and check the effect on performance
	//ArrayList<PointMu> pointMus = _signalProcessor.Process(testPoints);
	//testPoints = PointMu.PointMuListToPointList(pointMus);
	testPoints = _signalProcessor.SlidingMeanFilter(testPoints,2);
	//testPoints = _signalProcessor.RedundancyFilter(testPoints);
	System.out.println("num test points: "+testPoints.size());

    for(String word : vocab){
      ArrayList<Point> hiddenSequence = _keyMap.WordToPointSequence(" "+word+" ");
      dist = ComparePointSequences(testPoints,hiddenSequence,-1);
      if(dist >= 0){
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
  
	/*
	Given a file containing an input stream of sensor points, compares that sequence with
	every word in the vocabulary. This version runs the linear trace-to-word distance measure,
	using some heuristics for reducing computational complexity of comparing linear distances.
	The linear sequence dynamic programming model should provide about the best/most-precise signal-trace
	distance metric possible.
	*/
	public void TestLinearDP(String inputFile)
	{
		int i = 0;
		double dist;
		double minDist = 9999999;
		Vocab vocab = new Vocab("./resources/languageModels/vocab.txt");
	//ArrayList<Point> rawInput = BuildTestData(inputFile);
		ArrayList<Point> testPoints = BuildTestData(inputFile);
		ArrayList<SearchResult> results = new ArrayList<SearchResult>();

		//experimental: optionally filter the input points, and check the effect on performance
	//ArrayList<PointMu> pointMus = _signalProcessor.Process(rawInput);
	//ArrayList<Point> testPoints = PointMu.PointMuListToPointList(pointMus);
		//testPoints = _signalProcessor.SlidingMeanFilter(testPoints,5);
		System.out.println("num test points: "+testPoints.size());

		for(String word : vocab){
			//TODO: These could be pre-computed and stored, giving faster run times
			ArrayList<Point> hiddenSequence = _keyMap.WordToPointSequence(word);
			ArrayList<Line> lineSequence = Line.PointsToLineSequence(hiddenSequence);

			dist = CompareLinearSequences(testPoints,lineSequence,-1.0);
			//an experiment: define dist as average dist, to help make it length invariant
			//dist = dist / (double)lineSequence.size();
			//if(dist >= 0){
				SearchResult result = new SearchResult(dist,word);
				results.add(result);
				if(dist < minDist){
					minDist = dist;
				}
			//}
				//System.out.println(">"+word+"<");
			if(word.equals("MISSISSIPPI")){
				System.out.println("missip: "+dist);
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
			/*
			if(i > 10000){
				break;
			}
			*/
			//System.out.print("\r\n");
		}
	}
 
 	/*
 	The perceptron-weighted dp update method. This is entirely experimental.
 	
 	TODO: Currently this only uses left and up feature weights, which is likely sufficient for now.
 	*/
	private void _scoreCell_BasicWeighted(int row, int col, SignalDatum datum, Point keyPoint, MatrixCell[][] matrix, double[] weights)
	{
		double upScore, leftScore;
		//double curDist = -Point.DoubleDistance(datum.point, keyPoint);

		//System.out.println(datum.xdev+" "+datum.ydev);
		matrix[row][col].Dist = Point.DoubleDistance(datum.point, keyPoint);
		leftScore = matrix[row][col-1].Score + weights[_direction.LEFT.ordinal()] * matrix[row][col].Dist;
		upScore = matrix[row-1][col].Score + weights[_direction.UP.ordinal()] * matrix[row][col].Dist;

		if(upScore > leftScore){
			matrix[row][col].Score = upScore;
			matrix[row][col].Backpointer = Direction.UP;
		}
		else{
			matrix[row][col].Score = leftScore;
			matrix[row][col].Backpointer = Direction.LEFT;
		}

		/*
		//TODO: this assumes row and col are positive, non-zero. Needs error check
		if(matrix[row][col-1].Score < matrix[row-1][col].Score){
			//left cell is lesser (deletion from word), so update from it and point back to it
			matrix[row][col].Score = matrix[row][col-1].Score + weights[_direction.LEFT.ordinal()] * matrix[row][col].Dist;
			//matrix[row][col].Score = matrix[row][col-1].Score + Point.CityBlockDistance(datum,keyPoint);
			matrix[row][col].Backpointer = Direction.LEFT;
		}
		else{
			//upper cell is lesser (deletion from signal), so take from it instead and point up
			matrix[row][col].Score = matrix[row-1][col].Score + weights[_direction.UP.ordinal()] * matrix[row][col].Dist;
			//matrix[row][col].Score = matrix[row-1][col].Score + Point.CityBlockDistance(datum,keyPoint);
			matrix[row][col].Backpointer = Direction.UP;
		}
		*/
	}
	
  	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// All the following are experimental methods for testing weighted, structured-perceptron based learning using
	///// dynamic programming. None of this code is formalized, since its experimental.
	
	/*
	BasicWeightedDp: The first test for basic perceptron-weighted dynamic programming. This method is intended
	to establish viability, whether or not perceptron-weighted-dp even works.
	
	Keep this method simple, as a prototype. The weights should encompass only penalties for insertion, deletion, 
	and substitiution. The dynamic program is the same as above pointwise methods, except that the penalty weights
	are expected to change over iterations.
	
	Returns: A StructuredResult object.
	
	TODO: Could just store and return top-scoring word over all words, rather than the expensive sort() call.
	*/
	public ArrayList<SearchResult> BasicWeightedDpInference(ArrayList<SignalDatum> xSeq, double[] weights, Vocab vocab)
	{
		int i = 0;
		double dist;
		double maxScore = -99999999;
		//ArrayList<Point> testPoints = BuildTestData(inputFile);
		//ArrayList<Point> testPoints = xSeq;
		ArrayList<SearchResult> results = new ArrayList<SearchResult>();

		//experimental: optionally filter the input points, and check the effect on performance
		//ArrayList<PointMu> pointMus = _signalProcessor.Process(testPoints);
		//testPoints = PointMu.PointMuListToPointList(pointMus);
		//simple, modest filtering
		//testPoints = _signalProcessor.SlidingMeanFilter(testPoints,2);
		//testPoints = _signalProcessor.RedundancyFilter(testPoints);
		System.out.println("num test points: "+xSeq.size());

		for(String word : vocab){
			ArrayList<Point> hiddenSequence = _keyMap.WordToPointSequence(" "+word+" ");
			dist = BasicWeightedDp(xSeq, hiddenSequence, weights, -1);
			SearchResult result = new SearchResult(dist,word);
			results.add(result);

			/* space/speed optimizations; not used during experimentation
			if(dist >= 0){
				SearchResult result = new SearchResult(dist,word);
				results.add(result);
				if(dist < minDist){
					minDist = dist;
				}
			}
			*/

			i++;
			/*
			if(i > 30){
			break;
			}
			*/
			if(i % 10000 == 9999){
				System.out.print("\r"+Integer.toString(i)+" of "+Integer.toString(vocab.size())+" "+Integer.toString(results.size())+" results");
			}
		}
		System.out.println("Results size: "+results.size());

		/*
		i = 0;
		for(SearchResult result : results){
			System.out.print(Integer.toString(i+1)+":  ");
			result.Print();
			i++;
			//if(i > 100){
			//	break;
			//}
			//System.out.print("\r\n");
		}
		*/

		//for maximization formulations
		Collections.sort(results, Collections.reverseOrder());
		//for minimization formulations
		//Collections.sort(results);
		
		i = 0;
		System.out.println("\nTop 20 of "+Integer.toString(results.size())+" results: ");
		for(SearchResult result : results){
			System.out.print(Integer.toString(i+1)+":  ");
			result.Print();
			i++;
			if(i > 100){
				break;
			}
			//System.out.print("\r\n");
		}
		
		//return new StructuredResult(results.get(0).GetWord(), results.get(0).GetScore());
		return results;
	}
	
	/*
	Unique to dynamic-programming+perceptron experiments. Runs forward
	algorithm according to weights, then x vector summed over the x values
	given by the back pointers.
	*/
	public double[] DpPhiBasic(ArrayList<SignalDatum> xSeq, String word, double[] weights)
	{	
		ArrayList<Point> wordSequence = _keyMap.WordToPointSequence(" "+word+" ");
		//run forward program
		BasicWeightedDp(xSeq, wordSequence, weights, -1);
		//backtrack to derive phi
		return _phiBacktrack_Basic(xSeq.size()-1, wordSequence.size()-1);
	}
	
	/*
	precondition: BasicWeightedDp has been called, initializing the cell backpointers, dist, and score for some input word.
	*/
	private double[] _phiBacktrack_Basic(int startRow, int startCol)
	{
		int row, col;
		double[] phi = new double[PHI_DIM];

		//zero the vector
		for(int i = 0; i < phi.length; i++){
			phi[i] = 0.0;
		}
		
		row = startRow;
		col = startCol;
		while(row > 0 || col > 0){ //this loop construction works, assuming matrix backpointers at edges have been initialized such that row/col indices never go negative
			if(_matrix[row][col].Backpointer == _direction.UP){
				//phi[_direction.UP.ordinal()] += _matrix[row][col].Score;
				//phi[_direction.UP.ordinal()]   += 1.0;
				phi[_direction.UP.ordinal()] += _matrix[row][col].Dist;
				row--;
			}
			else if(_matrix[row][col].Backpointer == _direction.LEFT){
				//phi[_direction.LEFT.ordinal()] += _matrix[row][col].Score;
				//phi[_direction.LEFT.ordinal()] += 1.0;
				phi[_direction.LEFT.ordinal()] += _matrix[row][col].Dist;
				col--;
			}
			/*
			NOT YET USED
			else if(_matrix[row][col].Backpointer == _direction.SUB){
				phi[_direction.SUB.ordinal()] += _matrix[row][col].Score;
				row--;
				col--;
			}
			*/
		}
		
		return phi;
	}

	private double[] _phiBacktrack(int startRow, int startCol)
	{
		int row, col;
		double[] phi = new double[PHI_DIM];

		//zero the vector
		for(int i = 0; i < phi.length; i++){
			phi[i] = 0.0;
		}
		
		row = startRow;
		col = startCol;
		while(row > 0 || col > 0){ //this loop construction works, assuming matrix backpointers at edges have been initialized such that row/col indices never go negative
			if(_matrix[row][col].Backpointer == _direction.UP){
				//phi[_direction.UP.ordinal()] += _matrix[row][col].Score;
				phi[_direction.UP.ordinal()]   += 1.0;
				row--;
			}
			else if(_matrix[row][col].Backpointer == _direction.LEFT){
				//phi[_direction.LEFT.ordinal()] += _matrix[row][col].Score;
				phi[_direction.LEFT.ordinal()] += 1.0;
				col--;
			}
			/*
			NOT YET USED
			else if(_matrix[row][col].Backpointer == _direction.SUB){
				phi[_direction.SUB.ordinal()] += _matrix[row][col].Score;
				row--;
				col--;
			}
			*/
		}
		
		return phi;
	}

	
	/*
	Runs the weighted dynamic program. Note that a post-condition of this method is that the dynamic programming table
	has its backpointers initialized, such that immediately after this call one could backtrack to get the phi() vector
	for this 
	*/
	public double BasicWeightedDp(ArrayList<SignalDatum> inputSequence, ArrayList<Point> wordSequence, double[] weights, double threshold)
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
		_matrix[0][0].Dist = Point.DoubleDistance(inputSequence.get(0).point, wordSequence.get(0));
		_matrix[0][0].Score = -1.0 * _matrix[0][0].Dist;
		_matrix[0][0].Backpointer = _direction.UP;

		//init the first row		
		for(j = 1; j < wordSequence.size(); j++){
			_matrix[0][j].Dist = Point.DoubleDistance(inputSequence.get(0).point, wordSequence.get(j));
			_matrix[0][j].Score = _matrix[0][j].Dist * weights[_direction.LEFT.ordinal()] + _matrix[0][j-1].Score;
			_matrix[0][j].Backpointer = Direction.LEFT;
		}
		//init the first column
		for(i = 1; i < inputSequence.size(); i++){
			_matrix[i][0].Dist = Point.DoubleDistance(inputSequence.get(i).point, wordSequence.get(0));
			_matrix[i][0].Score = _matrix[i][0].Dist * weights[_direction.UP.ordinal()] + _matrix[i-1][0].Score;
			_matrix[i][0].Backpointer = Direction.UP;
		}

		//run the forward algorithm
		for(i = 1; i < inputSequence.size(); i++){
			rowMin = INF;
			for(j = 1; j < wordSequence.size(); j++){
				//the recurrence
				_scoreCell_BasicWeighted(i, j, inputSequence.get(i), wordSequence.get(j), _matrix, weights);
				if(_matrix[i][j].Score < rowMin){
					rowMin = _matrix[i][j].Score;
				}
			}
			if(threshold > 0 && rowMin > threshold){
				return INF;
			}
		}

		//_printMatrix(inputSequence.size(),wordSequence.size());

		//for optimal global alignment, the bottom-rightmost cell will have the score for this alignment
		return _matrix[inputSequence.size()-1][wordSequence.size()-1].Score;
	}

	/*
	///CANONICAL DP METHOD TESTING
	public static void main(String[] args)
	{
		String wordFile = "./resources/testing/performance/word12.txt";
		String word = "MISSISSIPPI";
		String keyMapFile = "./resources/ui/keyMap.txt";
		DpTwitch twitch = new DpTwitch(keyMapFile);

		
		//twitch.TestWordPointwise(wordFile,word);
		//twitch.TestWordPointwise(wordFile,"BILOXI");
		//twitch.TestWordPointwise(wordFile,"ALABAMA");
		//twitch.TestWordPointwise(wordFile,"ABC");
		
		twitch.TestPointwiseDP(wordFile);
		//twitch.TestLinearDP(wordFile);
	}
	*/
}
