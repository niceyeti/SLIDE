
/*
  This is just a bag-of-strings class for words in some vocabulary.
  Its a common enough pattern for other objects to need an object that reads
  in a vocab file and then provides access to it, so I separated this into 
  its own class instead of using native collections.
*/

import java.io.*;
import java.util.Set;  //for word set w/in DirectInference class
import java.util.HashSet;
import java.util.Iterator;
import java.util.ArrayList;

public class Vocab implements Iterable<String>
{
  HashSet<String> _wordSet;

  public Vocab(){
    _wordSet = new HashSet<String>();
  }

  public Vocab(String vocabFile){
    _wordSet = new HashSet<String>();
    BuildWordSet(vocabFile);
  }

  /*  Expect word model as flat file word database, one (unique) word per line.
      Requirements for the word model (at least for fastmode) are no punctuation,
      and all upper case. This is because the fastmode is more conversaitonal than
      grammatical, selecting complete words rather than contractions or other. Its
      really more of a phonetic word model than a written word model. Forward all complaints
      to the American Spell Check society.
  */
  private void BuildWordSet(String vocabFile)
  {
    System.out.println("Building word model from >"+vocabFile+"< for DirectInference class ctor...");

    try{
      //_wordSet = new Set<String>();
      BufferedReader br = new BufferedReader(new FileReader(vocabFile));
      String line;

      if(_wordSet.size() > 0){
        _wordSet.clear(); //clear the word model if it contains existing items
      }

      while((line = br.readLine()) != null){
        // process the line
        String upper = line.toUpperCase();
        _wordSet.add(upper);
      }

      br.close();
      System.out.println("Building word model completed. wordSet.size()="+Integer.toString(_wordSet.size()));
    }
    catch(java.io.FileNotFoundException ex){
      System.out.println("ERROR FileNotFoundException thrown for vocab file: "+vocabFile);
    }
    catch(java.io.IOException ex){
      System.out.println("ERROR IOException caught for vocab file "+vocabFile+" due to cause: "+ex.getCause());
    }
  }

  //Exposes set iteration to clients, so they can iterate over words directly
  public Iterator<String> iterator(){
    return _wordSet.iterator();
  }

  public int size(){
    return _wordSet.size();
  }

}



