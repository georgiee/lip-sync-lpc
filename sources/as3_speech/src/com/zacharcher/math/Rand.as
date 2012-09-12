/* AS3
	Copyright 2007 .
*/
package com.zacharcher.math {
	
	import com.zacharcher.math.Num;

	/*
	*	Class description.
	*
	*	@langversion ActionScript 3.0
	*	@playerversion Flash 9.0
	*
	*	@author Zach Archer
	*	@since  10.09.2007
	*/
	public class Rand extends Object {
		
		/*
		public function Rand(){
			super();
		}
		*/

		public static function get maybe() :Boolean {
			return (Math.random() < 0.5);
		}

		public static function float( limit1:Number = undefined, optLimit2:Number = undefined ) :Number {
			if( isNaN(optLimit2) ) {
				optLimit2 = 0.0;
				// If no arguments are specified, this is the same as Math.random()
				if( isNaN(limit1) ) {
					return Math.random();
				}
			}
			return Num.interpolate( limit1, optLimit2, Math.random() );
		}

		public static function floatBi( optLimit:Number ) :Number {
			if( isNaN(optLimit) ) {
				optLimit = 1.0;
			}
			return Num.interpolate( -optLimit, optLimit, Math.random() );
		}

		public static function int( limit1:Number, optLimit2:Number = undefined ) :Number {
			if( isNaN(optLimit2) ) {
				return Math.floor( Math.random() * limit1 );
			} else {
				// Increase the space between the numbers by 1; so that Math.floor can
				// possibly return the upper limit
				if( limit1 < optLimit2 ) {
					optLimit2++;
				} else {
					limit1++;
				}
				return Math.floor( Num.interpolate( limit1, optLimit2, Math.random() ) );
			}
		}

		public static function intBi( limit:Number ) :Number {
			limit = Math.abs( limit );	// Ensure positive
			return Math.floor( Num.interpolate( -limit, limit+1, Math.random() ) );
		}
		
		public static function fromArray( ar:Array ) :Object {
			return ar[ Rand.int(ar.length) ];
		}
		
	}
}
