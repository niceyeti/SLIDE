
/*
A class for conditioned signal inputs. The signal points could be decorated arbitrarily
in addition to the (x,y) position reading, with values like stdev-x, stdev-y, etc.

Keep this primitive, no funny stuff. Put calculations in signalprocessor perhaps.
*/

import java.io.*;
import java.nio.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Iterator;
import java.util.Set;
import java.util.Map;
import java.util.Comparator;

public class SignalDatum
{
	public Point point;
	public double xdev;
	public double ydev;
	//public double stdevXY;

	public SignalDatum()
	{
		point = new Point();
		xdev = 0.0;
		ydev = 0.0;
	}

	public String ToString()
	{
		return "("+point.GetX()+","+point.GetY()+","+xdev+","+ydev+")";
	}

	public SignalDatum(Point pt, double sigma_x, double sigma_y)
	{
		point = pt;
		xdev = sigma_x;
		ydev = sigma_y;
	}
}

