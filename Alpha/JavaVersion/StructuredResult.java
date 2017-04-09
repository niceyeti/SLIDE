public class StructuredResult
{
	public String Word;
	public double Score;

	//note phi is not included, since we don't want to calculate it for every example
	public StructuredResult(String word, double score)
	{
		Word = word;
		Score = score;
	}
}
