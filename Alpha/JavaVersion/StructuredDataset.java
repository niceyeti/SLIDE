import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Collections; //sorting
import java.util.ArrayList;


/*
Container class for constructing 
*/

public class StructuredDataset
{
	ArrayList<String> _samples;
	KeyMap _keyMap;
	ArrayList<StructuredExample> _trainingExamples;

	public StructuredDataset(String keyMapPath)
	{
		_samples = new ArrayList<String>();
		_keyMap = new KeyMap(keyMapPath);
	}
	
	//@trainingFiles: 
	public void BuildTrainingData(String[] trainingFiles)
	{
		_trainingExamples = new ArrayList<StructuredExample>();
	
		for(String path : trainingFiles)
		{
			StructuredExample example = _readTrainingExample(path);
			example.Print();
			_trainingExamples.add(example);
		}
	}

	public ArrayList<StructuredExample> GetTrainingExamples()
	{
		return _trainingExamples;
	}
	
	public double GetSize()
	{
		return _trainingExamples.size();
	}

	/*
	Reads in a StructuredExample file, for which the first line contains the target word, and the subsequent lines
	contain the 

	*/
	public StructuredExample _readTrainingExample(String exampleFilePath)
	{
		ArrayList<Point> xSequence;
		ArrayList<Point> ySequence;
		String word;
		StructuredExample example = null;

		try{
			BufferedReader br = new BufferedReader(new FileReader(exampleFilePath));
			String line;
			String[] tokens;

			xSequence = new ArrayList<Point>();
			//read the target word from the first line of the file, and convert to y-sequence
			word = br.readLine();
			ySequence = _keyMap.WordToPointSequence(word);
			//read sensor stream of (x,y) points from subsequent lines
			while((line = br.readLine()) != null){
				tokens = line.trim().split("\t");
				if(tokens.length != 2){
					System.out.println("ERROR incorrect number of tokens for this line >"+line+"< in test input file "+exampleFilePath);
				}
				else if(!Utilities.IsIntString(tokens[0]) || !Utilities.IsIntString(tokens[1])){
					System.out.println("ERROR non-integer tokens for this line >"+line+"< in test input file "+exampleFilePath);
				}
				else{
					try{
						int x = Integer.parseInt(tokens[0]);
						int y = Integer.parseInt(tokens[1]);
						Point pt = new Point(x,y);
						xSequence.add(pt);
					}
					catch(NumberFormatException ex){
						System.out.println("Number format exception >"+ex.getCause().getMessage()+"< caught for test input file >"+exampleFilePath+"<  Coordinates incorrectly formatted");
					}
				}
			}
			
			ArrayList<SignalDatum> data = SignalProcessor.DecorateRawSignal(xSequence);
			
			example = new StructuredExample(data, ySequence, word);
			
			br.close();
		}
		catch(java.io.FileNotFoundException ex){
			System.out.println("ERROR FileNotFoundException thrown for file: "+exampleFilePath);
		}
		catch(java.io.IOException ex){
			System.out.println("ERROR IOException caught for file "+exampleFilePath+" due to cause: "+ex.getCause());
		}

		return example;
	}

}
