// a PointMu (mu like mean) represents a trigger point or a cluster center for a sequence of x/y inputs.
// This may be either something discrete like a key press, or something stochastic, like "I think there was a click here" and my_ticks determines the likelihood of a click
public class PointMu
{
  char _alpha;
  Point _pt;
  int _ticks;

  public PointMu(){
    _pt = new Point(0,0);
    _ticks = 0;
    _alpha = 'A';
  }
  public PointMu(Point point, int ticks){
    _pt = new Point(point.GetX(), point.GetY());
    _ticks = ticks;
    _alpha = 'A';
  }
  public PointMu(Point point, int ticks, char c){
    _pt = new Point(point.GetX(), point.GetY());
    _ticks = ticks;
    _alpha = c;
  }
  @Override
  public String toString(){
    return _pt.toString()+":"+_alpha;
  }
  public void SetAlpha(char c){
    _alpha = c;
  }
  public void SetPoint(Point pt){
    _pt.SetXY(pt.GetX(),pt.GetY());
  }
  public void SetTicks(int t){
    _ticks = t;
  }
  public char GetAlpha(){
    return _alpha;
  }
  public int GetTicks(){
    return _ticks;
  }
  public Point GetPoint(){
    return _pt;
  }
} //end PointMu class
