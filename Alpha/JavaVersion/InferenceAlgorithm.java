import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Collections; //sorting
import java.util.ArrayList;



/*
InferenceAlgorithm would make a nice abstract class, whose concrete instances
could be swapped easily to test different inference methods (graphical, 
dynamic programming, etc).

For now, we can keep this concrete, since we're only targeting one inference method: dp.

PRimary public methods: 
	Phi(x,y): The joint feature function
	Infer(x): The prediction method, given x sequence of euclidean points
*/

public class InferenceAlgorithm
{
	//int _phiDimension;
	//dp-twitch currently holds the kitchen sink of code, methods, keymaps, etc for testing algorithms
	DpTwitch _dp;
	Vocab _vocab;

	public InferenceAlgorithm(int phiDimension, String vocabPath)
	{
		//_phiDimension = phiDimension;
		_dp = new DpTwitch("./resources/ui/keyMap.txt", phiDimension);
		_vocab = new Vocab(vocabPath);
	}

	/*
	@xSeq: A variable length sequence of (x,y) coordinate points
	@ySeq: A string sequence of label symbols.
	@weights: The current model weights
	
	Note that this problem is unlike other structured prediction problems, since xSeq and ySeq are of different lengths,
	an xSeq is variable length.
	
	This could be expanded to include yPrevious, the previous word for context.
	*/
	public double[] Phi(ArrayList<SignalDatum> xSeq, String word, double[] weights)
	{
		//run dynamic program, and backtrack over dp back pointers to get phi(xSeq,ySeq)
		return _dp.DpPhi(xSeq, word, weights);
	}

	/*
	Since this is a ranking problem, I decided to return the entire ranked list determine by the inference
	procedure, rather than only the top scoring y_hat. Its easier to track metrics this way, and more
	information can be leveraged from the results as well.
	*/
	public ArrayList<SearchResult> Infer(ArrayList<SignalDatum> xSeq, double[] weights)
	{	//(ArrayList<Point> inputSequence, ArrayList<Point> wordSequence, double[] weights, double threshold)
		//evaluate and sort all words by their score w.r.t. @xSeq
		return _dp.WeightedDpInference(xSeq, weights, _vocab);
	}
}

