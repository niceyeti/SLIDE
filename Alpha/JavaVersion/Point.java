public class Point
{
  int m_X;
  int m_Y;

  public Point(){
    m_X = 0;
    m_Y = 0;
  }
  public Point(int x, int y){
    SetXY(x,y);
  }
  
	public Point(Point p){
		InitPoint(p);
	}

  public void InitPoint(Point p){
    m_X = p.GetX();
    m_Y = p.GetY();
  }
  public int GetX(){
    return m_X;
  }
  public int GetY(){
    return m_Y;
  }
  public void SetXY(int X, int Y){
    m_X = X;
    m_Y = Y;
  }
  public void SetY(int Y){
    m_Y = Y;
  }
  public void SetX(int X){
    m_X = X;
  }
	@Override
	public String toString(){
		return "("+Integer.toString(m_X)+","+Integer.toString(m_Y)+")";
	}

	public boolean Equals(Point other){
		return other.GetX() == m_X && other.GetY() == m_Y;
	}

  public static double DoubleDistance_Doom(Point p1, Point p2){
    return 0.0;
  }
  
  public static double CityBlockDistance(Point p1, Point p2){
  	return Math.abs(p1.GetX() - p2.GetX()) + Math.abs(p1.GetY() - p2.GetY());
  }

  public static double DoubleDistance(Point p1, Point p2){
    return Math.sqrt(Math.pow((double)(p1.GetX() - p2.GetX()),2.0) + Math.pow((double)(p1.GetY() - p2.GetY()),2.0));
  }
} //end Point class

