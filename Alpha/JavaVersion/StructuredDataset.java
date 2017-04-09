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
			example = _readTrainingExample(path)
			_trainingExamples.add();
		}
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
		StructureExample example;

		try{
			BufferedReader br = new BufferedReader(new FileReader(exampleFilePath));
			String line;
			String[] tokens;

			xSequence = new ArrayList<Point>();
			//read the target word from the first line of the file, and convert to y-sequence
			String word = br.readLine();
			ySequence = _keyMap.WordToPointSequence(word);
			//read sensor stream of (x,y) points from subsequent lines
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
						xSequence.add(pt);
					}
					catch(NumberFormatException ex){
						System.out.println("Number format exception >"+ex.getCause().getMessage()+"< caught for test input file >"+testFilePath+"<  Coordinates incorrectly formatted");
					}
				}
			}
			
			example = new StructuredExample(xSequence, ySequence, );
			
			br.close();
		}
		catch(java.io.FileNotFoundException ex){
			System.out.println("ERROR FileNotFoundException thrown for file: "+testFilePath);
		}
		catch(java.io.IOException ex){
			System.out.println("ERROR IOException caught for file "+testFilePath+" due to cause: "+ex.getCause());
		}

		return example;
	}

}
