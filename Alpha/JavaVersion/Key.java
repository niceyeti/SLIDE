
//Every key has a char id, a location (Point), and a list of its neighbors (as a string)
public class Key  {
    char _id;
    Point _center;
    String _neighbors; //raw neighbors are just stored as a string
 
    public Key(){
      _id = '\0';
      _neighbors = "";
      _center = new Point(0,0);
    }
    public Key(char c, String neighborStr){
      _id = c;
      _neighbors = neighborStr;
      _center = new Point(0,0);
    }
    public String GetNeighbors(){
      return _neighbors;
    }
    public char GetId(){
      return _id;
    }
    public Point GetPoint(){
      return _center;
    }
    public void SetNeighbors(String neighbors){
      _neighbors = neighbors;
    }
    public void SetPoint(Point p){
      _center.SetXY(p.GetX(), p.GetY());
    }
    public void SetPoint(int x, int y){
      _center.SetXY(x,y);
    }

  
  public static void main(String[] args){
    Key key = new Key('B',"ABCDEGH");
    Point pt = new Point();
    if(pt == null){
      System.out.println("null exception!");
    }else{
      pt.SetXY(22,33);
      //key.SetPoint(pt);
      key.SetPoint(22,33);
      System.out.println("Key coordinates: "+Integer.toString(key.GetPoint().GetX())+","+Integer.toString(key.GetPoint().GetY()));
    }
  }
  

}
