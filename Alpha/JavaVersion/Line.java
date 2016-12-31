
/*
Class for storing objects describing 2d lines in linear algebraic terms as n and p.
This storage format is entirely for efficient distance measures from points to a given line,
which is given by: 
	dist = n dot x + p
In 2d, this means only two multiplications and two additions, which is good for a high-frequency dp function.

This class was built entirely to represent this pointwise distance equation with as much built-in
pre-computation as possible, to reduce dynamic programming burden of distance measures.

P1, P2 = points defining the line
(x0, y0) = some other point in space

distance(P1, P2, (x0,y0)) = \(y2-y1)x0 - (x2-x1)y0 + x2*y1 -y2*x1\  /  sqrt((y2-y1)**2 + (x2-x1)**2)

Let mag = sqrt((y2-y1)**2 + (x2-x1)**2), then:

	a = (y2-y1) / mag
	b = (x2-x1) / mag
	c = (x2*y1 -y2*x1) / mag

With these pre-computed and normalized values, any distance to point x is given by:
	
	Using the absolute value rule: |a|/b == |a/b|
	and letting n = <a,b>, p = c
	then dist = |n dot x + c|
*/

import java.util.ArrayList;

public class Line
{
	double _a;
	double _b;
	double _c;
	double _mag;

	public Line(Point p1, Point p2){
		double x1 = p1.GetX();
		double y1 = p1.GetY();
		double x2 = p2.GetX();
		double y2 = p2.GetY();

		_mag = Math.sqrt(Math.pow(y2-y1,2.0) + Math.pow(x2-x1, 2.0));
		if(_mag == 0.0){
			System.out.println("ERROR p1 == p2 in Line() ctor; expect div-zero/nan result");
		}
		
		_a = (y2-y1) / _mag;
		_b = (x2-x1) / _mag;
		_c = (x2*y1 - y2*x1) / _mag;
	}

	public double PointLineDistance(Point point){
		return Math.abs( _a * point.GetX() + _b * point.GetY() + _c);
	}
	
	/*
	Given a sequence of n-points, converts them to a sequence of n-1 line segments,
	where each line segment is described by vector n and constant p. This is just convenient
	notation for getting the distance from a point x to the line, which is given by: x dot n + p.
	Storing the segments as n and p means these values are precomputed, speeding up dynamic programming.
	
	NOTE: consecutive, equal points are problematic (zero length lines) and are skipped until a unique point is found.
	*/
	public static ArrayList<Line> PointsToLineSequence(ArrayList<Point> points){
		ArrayList<Line> lines = new ArrayList<Line>();
		Point prev, cur;
		
		for(int i = 0; i < points.size()-1; i++){
			prev = points.get(i);
			//shuttle to next non-equal point
			while(i < (points.size()-1) && prev.Equals(points.get(i+1))){
				i++;
			}
			//post-loop: i points at non-equal next point, or is out of bounds
			if(i < points.size()-1){ //add point pair, if in-bounds
				Line line = new Line(prev, points.get(i+1));
				lines.add(line);
			}
		}

		return lines;
	}
	
}


