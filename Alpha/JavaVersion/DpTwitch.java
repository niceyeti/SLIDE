/*
This is a separate implementation which uses dynamic programming, instead of the fast-mode method.
This is experimental, to see how well dynamic programming performs. This is not intended
to be a space or time-efficient method, its only intended to see how the principle of optimality
might be used to solve the geometric-sequence comparison problem optimally.
*/




public class DpTwitch{

  //for backtracking
  public enum Direction{UP, LEFT, DIAG}

  //primitive dp matrix cell
  private class MatrixCell{
    public double Score;
    public int Backpointer;

    public MatrixCell(){
      score = 10000000;
      backpointer = DIAG;
    }
  }

  KeyMap _keyMap;
  MatrixCell[][] _matrix;  
  int MAX_ROWS;
  int MAX_COLS;
  
  public DpTwitch(string keyMapFile){
    MAX_ROWS = 1000;
    MAX_COLS = 100;

    //build the keymap
    _keyMap = new KeyMap(keyMapFile);
    //build the matrix; no effort for space efficieny here...
    _matrix = new MatrixCell[MAX_ROWS][MAX_COLS];
  }

  /*
  Rather than taking two strings as input sequences for normal optimal-alignment algorithms,
  this compares sequences of geometric coordinates as x/y pairs. The first input sequence is expected
  to be some sensor data, and the second sequence representing the sequence of letter coordinates
  of some word on some ui layout grid. So the input sequence is expected to be significantly 
  longer (unfiltered) than the "hidden" word letter-coordinate sequence.
  */
  public int CompareSequences(ArrayList<Point> inputSequence, ArrayList<Point> wordSequence)
  {
    int i,j;

    //initialize the dp table for distance minimization
    for(i = 0; i < ){

    }








  }


  private _runForward(){

  }




  public static void main(String[] args){

    string keyMapFile = "./resources/ui/keyMap.txt";
    


    


  }
}
