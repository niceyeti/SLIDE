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

	public InferenceAlgorithm()
	{
		//_phiDimension = phiDimension;
		_dp = new DpTwitch("./resources/ui/keyMap.txt");	
	}

	/*
	@xSeq: A variable length sequence of (x,y) coordinate points
	@ySeq: A string sequence of label symbols.
	@weights: The current model weights
	
	Note that this problem is unlike other structured prediction problems, since xSeq and ySeq are of different lengths,
	an xSeq is variable length.
	
	This could be expanded to include yPrevious, the previous word for context.
	*/
	public double[] Phi(ArrayList<Point> xSeq, String word, double[] weights)
	{
		//run dynamic program, and backtrack over dp back pointers to get phi(xSeq,ySeq)
		//_dp.Run(xSeq, ySeq, weights);
		//ArrayList<double> phi = _dp.SumBackTrack();
		
		return _dp.DpPhi(xSeq, word, weights);
	}

	public StructuredResult Infer(ArrayList<Point> xSeq, ArrayList<double> weights)
	{
		//evaluate and sort all words by their score w.r.t. @xSeq
		return _dp.BasicWeightedDp(xSeq, weights);
	}

}

