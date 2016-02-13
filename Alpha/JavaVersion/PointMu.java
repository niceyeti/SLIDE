// a PointMu (mu like mean) represents a trigger point or a cluster center for a sequence of x/y inputs.
// This may be either something discrete like a key press, or something stochastic, like "I think there was a click here" and my_ticks determines the likelihood of a click
public class PointMu
{
  char m_alpha;
  Point m_pt;
  int m_ticks;

  public PointMu(){
    m_pt = new Point(0,0);
    m_ticks = 0;
    m_alpha = 'A';
  }
  public PointMu(Point point, int ticks){
    m_pt = new Point(point.GetX(), point.GetY());
    m_ticks = ticks;
    m_alpha = 'A';
  }
  public PointMu(Point point, int ticks, char c){
    m_pt = new Point(point.GetX(), point.GetY());
    m_ticks = ticks;
    m_alpha = c;
  }
  @Override
  public String toString(){
    return m_pt.toString()+":"+m_alpha;
  }
  public void SetAlpha(char c){
    m_alpha = c;
  }
  public void SetPoint(Point pt){
    m_pt.SetXY(pt.GetX(),pt.GetY());
  }
  public void SetTicks(int t){
    m_ticks = t;
  }
  public char GetAlpha(){
    return m_alpha;
  }
  public int GetTicks(){
    return m_ticks;
  }
  public Point GetPoint(){
    return m_pt;
  }
} //end PointMu class
