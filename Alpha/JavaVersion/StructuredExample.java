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
	
	//Outputs the example in a python parseable dict string, json like format, for easy reading in python using eval()
	public String Print()
	{
		int i;
		String output = "";
	
	
		output = "{\'Word\': \'"+Word+"\',";
		output += "\'Y-sequence\': [";
		i = 0;
		while(i < YSequence.size()-2){
			output += YSequence.get(i).ToString();
			output += ",";
			i++;
		}
		//close the list with the last one
		output += YSequence.get( YSequence.size() - 1 ).ToString();
		output += "],";


		output += "\'X-sequence\': [";
		i = 0;
		while(i < XSequence.size()-2){
			output += XSequence.get(i).ToString();
			output += ",";
			i++;
		}
		//close the list with the last one
		output += XSequence.get( YSequence.size() - 1 ).ToString();
		output += "]}";
	
		System.out.println(output);
	
		return output;
	}
	
}

