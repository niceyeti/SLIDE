import java.io.*;
import java.util.Map;
import java.util.HashMap;

//TODO: this is a temporary abstraction. Use the Team Force data structure, 
public class KeyMap
{
  Map<Character,Key> m_keyMap;
  double m_minInterKeyRadius; // defined as one-half the distance between the two nearest alpha keys
  double m_leftExtreme, m_rightExtreme, m_upperExtreme, m_lowerExtreme; //defines the ui boundaries for alpha chars

  //should not use the default ctor; only use the one that inits the map based on a file of key coordinates
  public KeyMap(){
    m_keyMap = new HashMap<Character,Key>();
    m_minInterKeyRadius = 0;
    m_leftExtreme = 0;
    m_rightExtreme = 0;
    m_lowerExtreme = 0;
    m_upperExtreme = 0;
  }

  public KeyMap(String keyCoordinateFileName){
    m_keyMap = new HashMap<Character,Key>();
    initKeyMap(keyCoordinateFileName);
  }

  public double GetMinInterKeyRadius(){
    return m_minInterKeyRadius;
  }

  //util, lookup a Point (value) given char c (a map key)
  //TODO: error checking on null returns, if c is not in map keys      
  public Point GetPoint(char key){
    Point ret = null;

    if(m_keyMap.containsKey(key)){
      ret = m_keyMap.get(key).GetPoint();
      if(ret == null){
        System.out.println("ERROR key >"+key+"< found in map, with null Point reference");
        ret = m_keyMap.get('G').GetPoint();  //return something we know is in the map
      }
    }
    else{
      System.out.println("ERROR key >"+key+"< not found in map!");
    }
    return ret;
  }

  /*
    Iterate over the map looking for key nearest to some point p.
    Note this returns the nearest ALPHA key, not the nearest key among all the keys.

    TODO: This could be a lot faster than linear search. It will be called for every mean from
    the SingularityBuilder, so eliminating it might speed things a bit.  Could at least return 
    as soon as we find a min-distance that is less than the key width (or width/2), such that
    no other key could be closer than this value. Something like that...
  */
  public char FindNearestAlphaKey(Point pt)
  {
    char c = 'G';
    double dist, min;

    //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
    min = 99999.0;
    for(Key key : m_keyMap.values()){
      if(Utilities.IsAlpha(key.GetId())){
        dist = Point.DoubleDistance(key.GetPoint(),pt);
        if(min > dist){
          min = dist;
          if(min < m_minInterKeyRadius){  //optimization. return when dist is below some threshold, such that no other key could be nearer
            return key.GetId();
          }
          c = key.GetId();
        }
      }
    }

    return c;
  }

  //TODO: all of the key map initialization and update (eg, if the user resizes the screen or moves the client window) needs to be done according to the Team Gleason project
  public void initKeyMap(String keyCoordinateFileName)
  {
    //the calling order of these inits matters, since the first inits all of the entries, second fills the location data of the entries, etc
    initKeyNeighbors();
    initKeyCoordinates(keyCoordinateFileName);
    initDerivedGeometryVals(); //initializes various geometric properties (m_minInterKeyRadius, etc) used in some of the inference logic
  }

  /*
    FastMode needs a few geometric values such as the minimum distance between keys, and the layout boundaries.
    The layout boundaries are used to determine if a sensor value (an X/Y pair) is omitted, and min-key distance
    can be used in various ways for logic questions and for generating/smoothing probability distributions. Like maybe
    you have a raw trigger point (x,y), and a list of its nearest neighbor keys, each with its own (x,y) center. A probability
    for the "intended key" can be based in various ways upon whether or not the trigger point is within the m_minInterKeyRadius distance.
    Or say you look up the neighbors of the nearest key, and want to use the minInterKey distance to omit neighbor keys that are greater
    than some scaled m_minInterKeyRadius, such that somewhat far neighbor keys aren't evaluated or are assigned a rapidly diminishing probability.
    A few basic geometry values can be used in this way to scale probabilities in an intuitive way, and to assign precise values when answering
    questions like, given some trigger point and its nearest neighbors, determine the most likely "intended" key press. And other such
    "regional" queries for the interface.

    NOTE: the values are only based in alpha characters, since only these drive the fastmode inference process.
  */
  public void initDerivedGeometryVals()
  {
    int curX, curY;
    String alphaStr = new String("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

     if(m_keyMap.isEmpty()){
       System.out.println("ERROR key map not yet created");
     }

     //order of these calls matters
     initMinInterKeyRadius();
     initKeyBoundaries();
  }

  //sets the minimum interkey distance, defined as one half the center-to-center distance between the nearest two keys
  private void initMinInterKeyRadius()
  {
    double dist;

    m_minInterKeyRadius = 999999;
    //find the minimum interkey distance, the dist between the centers of the two nearest keys.
    for(Key outerKey : m_keyMap.values()){
      for(Key innerKey : m_keyMap.values()){
        if(Utilities.IsAlpha(outerKey.GetId()) && Utilities.IsAlpha(innerKey.GetId())){
          if(outerKey.GetId() != innerKey.GetId()){ //dont compare keys with themselves!
            dist = Point.DoubleDistance(outerKey.GetPoint(),innerKey.GetPoint()) / 2.0;
            if(dist < m_minInterKeyRadius){
              m_minInterKeyRadius = dist;
            }
          }
        }
      }
    }  //post-loop: m_minInterKeyRadius holds one half the distance between the two nearst alpha keys
  }

  //calculates the left, right, upper, and lower-most boundaries based on the alpha characters in the layout
  private void initKeyBoundaries()
  {
    int curX, curY;

    //to add as much confusion as possible, the upper left window corner is defined as (0,0), so "m_upperExtreme" will be less than m_lowerExtreme
    m_leftExtreme = 99999; m_rightExtreme = 0; m_upperExtreme = 99999; m_lowerExtreme = 0;
    for(Key kbKey : m_keyMap.values()){
      if(Utilities.IsAlpha(kbKey.GetId())){ //verify its an alpha
        curX = kbKey.GetPoint().GetX();
        curY = kbKey.GetPoint().GetY();

        if(m_leftExtreme > curX){
          m_leftExtreme = curX;
        }
        if(m_rightExtreme < curX){
          m_rightExtreme = curX;
        }
        if(m_upperExtreme > curY){
          m_upperExtreme = curY;
        }
        if(m_lowerExtreme < curY){
          m_lowerExtreme = curY;
        }
      }
    }

    //We just found the most extreme left/right/lower/upper values among the centers of the keys,
    //but its the boundaries we're after. So add the m_minInterKeyRadius to each to expand the boundaries a little.
    m_lowerExtreme += m_minInterKeyRadius;
    m_upperExtreme -= m_minInterKeyRadius;
    m_rightExtreme += m_minInterKeyRadius;
    m_leftExtreme -= m_minInterKeyRadius;

    //check if we've underflowed the boundaries of the client window, and adjust
    if(m_upperExtreme < 0){
      m_upperExtreme = 0;
    }
    if(m_leftExtreme < 0){
      m_leftExtreme = 0;
    }
    //should check the right and lower, but there is no equivalent way to see if they extend too far.
    //TODO: get client window size and make sure right and m_lowerExtreme are within it as well
    System.out.println("KeyMap m_minInterKeyRadius: "+Double.toString(m_minInterKeyRadius));
    System.out.println("KeyMap bounds (x:y): "+Double.toString(m_leftExtreme)+","+Double.toString(m_rightExtreme)+":"+Double.toString(m_upperExtreme)+","+Double.toString(m_lowerExtreme));
  }

  //Init the coordinates of keys, based on an input file containing pixel/int x/y coordinates for keys.
  // Precondition: keyMap entries must have already been created for all keys. This function just fills in the coordinate data for existing entries.
  public void initKeyCoordinates(String keyCoordinateFileName)
  {
    try{
      BufferedReader br = new BufferedReader(new FileReader(keyCoordinateFileName));
      String line;
      String[] tokens;

      //error-check: keymap must be populated with key-val entries in order to set coordinates for the existing entries
      if(m_keyMap.isEmpty()){
        System.out.println("ERROR key map not yet loaded with key-value entries in initKeyCoordinates()");
        return;
      }

      while((line = br.readLine()) != null){
        // process the line
        tokens = line.split("\t");
        if(tokens.length != 3){
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< incorrectly formatted with "+Integer.toString(tokens.length)+" tokens per line: "+line);
        }
        else if(!Utilities.IsIntString(tokens[1]) || !Utilities.IsIntString(tokens[2])){ //verify integer inputs
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< coordinates incorrectly formatted: "+tokens[1]+"/"+tokens[2]);
        }
        else{
           try{
             int x = Integer.parseInt(tokens[1]);
             int y = Integer.parseInt(tokens[2]);
             char c = tokens[0].toUpperCase().charAt(0);
             
             //enter these coordinates for this char
             if(m_keyMap.containsKey(c)){
               m_keyMap.get(c).GetPoint().SetXY(x,y);
             }
             else{
               System.out.println("ERROR char >"+Character.toString(c)+"< not found in keyMap. Key coordinate file entries misaligned with key initialization in initKeyNeighbors()");
             }
           }
           catch(NumberFormatException e){
             System.out.println("Number format exception >"+e.getCause().getMessage()+"< caught for key map coordinate file >"+keyCoordinateFileName+"<  Coordinates incorrectly formatted");
           }
        }
      }
      br.close();
    }
    catch(java.io.FileNotFoundException ex){
      System.out.println("ERROR FileNotFoundException thrown in KeyMap.initKeyCoordinates for file: "+keyCoordinateFileName);
    }
    catch(java.io.IOException ex){
      System.out.println("ERROR IOException thrown in KeyMap.initKeyCoordinates reading file: "+keyCoordinateFileName);
      System.out.println("Cause: "+ex.getCause());
    }
  }

  //initializes the keymap based on the on-screen keyboard layout. If the layout changes, this must also change
  //This function only initilializes the key entries in the map and the key layout relative to neighbor keys. Establishing
  //each key's coordinates is done in initKeyCoordinates()
  //TODO: punctuation is not defined! OR worse, defined here but not in the key coordinate file. Also missing spacebar here
  public void initKeyNeighbors()
  {
    m_keyMap.put('Q', new Key('Q', new String("WA")));
    m_keyMap.put('W', new Key('W', new String("QASE")));
    m_keyMap.put('E', new Key('E', new String("RDSW")));
    m_keyMap.put('R', new Key('R', new String("TFDE")));
    m_keyMap.put('T', new Key('T', new String("YGFR")));
    m_keyMap.put('Y', new Key('Y', new String("UHGT")));
    m_keyMap.put('U', new Key('U', new String("IJHY")));
    m_keyMap.put('I', new Key('I', new String("OKJU")));
    m_keyMap.put('O', new Key('O', new String("PLKI")));
    m_keyMap.put('P', new Key('P', new String("LO;")));
    m_keyMap.put('A', new Key('A', new String("QWSZ")));
    m_keyMap.put('S', new Key('S', new String("AWEDXZ")));
    m_keyMap.put('D', new Key('D', new String("SERFCX")));
    m_keyMap.put('F', new Key('F', new String("DRTGVC")));
    m_keyMap.put('G', new Key('G', new String("FTYHBV")));
    m_keyMap.put('H', new Key('H', new String("GYUJNB")));
    m_keyMap.put('J', new Key('J', new String("HUIKMN")));
    m_keyMap.put('K', new Key('K', new String("JIOL,M")));
    m_keyMap.put('L', new Key('L', new String("KOP;.,")));
    m_keyMap.put(';', new Key(';', new String("PL.?\"")));
    m_keyMap.put('\"', new Key('\"', new String(";?")));
    m_keyMap.put('Z', new Key('Z', new String("ASX")));
    m_keyMap.put('X', new Key('X', new String("ZSDC")));
    m_keyMap.put('C', new Key('C', new String("XDFV")));
    m_keyMap.put('V', new Key('V', new String("CFGB")));
    m_keyMap.put('B', new Key('B', new String("VGHN")));
    m_keyMap.put('N', new Key('N', new String("BHJM")));
    m_keyMap.put('M', new Key('M', new String("NJK,")));
    m_keyMap.put(',', new Key(',', new String("MKL.")));
    m_keyMap.put('.', new Key('.', new String(",;?L")));
    m_keyMap.put('?', new Key('?', new String(".;\"")));
    m_keyMap.put(' ', new Key(' ', new String("!")));  //TODO: Space character is an exception, having no neighbors. Is this safe elsewhere, wherever neighbors are expected?
  }

  //Util, returns true if a point is within the active region of the ALPHA characters on the keyboard layout
  public boolean InBounds(Point pt){
    if((pt.GetX() < m_rightExtreme) && (pt.GetX() > m_leftExtreme)){
      if((pt.GetY() < m_lowerExtreme) && (pt.GetY() > m_upperExtreme)){
        return true;
      }
    }
    return false;
  }
} //end keyMap class

