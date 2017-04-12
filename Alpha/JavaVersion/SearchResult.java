public class SearchResult implements Comparable<SearchResult>
{
  double _score;
  String _word;  //note this will be used only as a reference to a string in the wordModel, not an actual String, per Java rules

  public SearchResult()
  {
    _score = 0.0;
    _word = "";
  }
  
  public SearchResult(double score, String word)
  {
    _score = score;
    _word = word;
  }
  
  public String GetWord()
  {
    return _word;
  }
  
  public double GetScore()
  {
    return _score;
  }

  public void Print()
  {
    System.out.println(_word+":"+Double.toString(_score));
  }

  @Override
  public String toString()
  {
    return _word+":"+Double.toString(_score);
  }

  @Override
  public int compareTo(SearchResult res)
  {
    return (this.GetScore() >= res.GetScore()) ? 1 : -1;
  }
}
