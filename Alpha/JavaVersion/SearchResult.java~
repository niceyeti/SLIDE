public class SearchResult implements Comparable<SearchResult>
{
  double m_score;
  String m_word;  //note this will be used only as a reference to a string in the wordModel, not an actual String, per Java rules

  public SearchResult(){
    m_score = 0.0;
    m_word = "";
  }
  public SearchResult(double score, String word){
    m_score = score;
    m_word = word;
  }
  public String GetWord(){
    return m_word;
  }
  public double GetScore(){
    return m_score;
  }
  @Override
  public String toString(){
    return m_word+":"+Double.toString(m_score);
  }

  @Override
  public int compareTo(SearchResult res) {
    int ret;
    ret = (this.GetScore() > res.GetScore()) ? 1 : -1;
    return ret;
  }
}
