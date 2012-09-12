/* AS3
	Copyright 2007 .
*/
package com.zacharcher.math {
	
	/*
	*	Class description.
	*
	*	@langversion ActionScript 3.0
	*	@playerversion Flash 9.0
	*
	*	@author Zach Archer
	*	@since  20.08.2007
	*/
	import flash.geom.*;

	import com.zacharcher.math.Num;

	public class Geom {

		// Converts degrees (0..360) to radians (2*PI..0)
		// Remember, degrees increment clockwise; radians increment counter-clockwise.
		public static function deg2rad( deg:Number ) :Number {
			var rad:Number = (-deg/360) * (Math.PI*2);
			return Num.wrap( rad, Math.PI*2 );	// Construe to 0.0...Math.PI*2
		}
		public static function rad2deg( rad:Number ) :Number {
			var deg:Number = (-rad/Math.PI) * 180;
			return Num.wrap( deg, 360 );	// Construe to 0.0...360.0
		}
		
		// Rotate 
		public static function rotatePoint( p:Point, rads:Number ) :Point {
			var dist:Number = p.length;
			var angle:Number = Math.atan2( -p.y, p.x );
			return Point.polar( dist, -angle + rads );
		}

	}	
}
