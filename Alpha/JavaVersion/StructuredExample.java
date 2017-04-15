import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Collections; //sorting
import java.util.ArrayList;

/*
A primitive class just for holding a training example, in this case
an x-sequence and a y-sequence, both of which are 
*/
public class StructuredExample
{
	public String Word;
	public ArrayList<SignalDatum> XSequence;
	public ArrayList<Point> YSequence;

	public StructuredExample(ArrayList<SignalDatum> xSeq, ArrayList<Point> ySeq, String word)
	{
		Word = word;
		XSequence = xSeq;
		YSequence = ySeq;
	}
}

