package harness {

/**
 *	Class description.
 *
 *	@langversion ActionScript 3.0
 *	@playerversion Flash 9.0
 *
 *	@author Zach Archer
 *	@since  27.08.2009
 */
import flash.display.*;
import flash.events.*;
import flash.geom.*;
import com.zacharcher.math.*;

public class FreehandEnvelope extends Sprite {
	
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	private static const WIDTH:int = 200;
	private static const HEIGHT:int = 130;
	private static const BG_COLOR:uint = 0x000000;
	private static const LINE_COLOR:uint = 0x009999;
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function FreehandEnvelope()
	{
		super();
		
		_bmp = new Bitmap( new BitmapData( WIDTH, HEIGHT, false, 0x000000 ) );
		addChild( _bmp );

		_values = new Vector.<int>();
		var ar:Array = [37,36,35,34,33,32,32,31,31,31,31,31,31,32,32,33,33,34,35,36,37,38,39,40,41,42,43,44,45,46,46,47,48,49,49,50,50,51,51,52,52,53,53,54,55,55,56,57,57,58,59,60,60,61,62,63,64,65,65,66,66,67,67,68,68,68,68,68,67,67,66,66,65,64,64,63,62,61,60,59,58,57,57,56,55,54,54,53,53,53,52,52,52,52,52,51,51,51,54,54,53,53,52,52,52,52,52,52,52,51,51,51,51,51,50,50,50,50,50,50,51,53,54,56,58,61,62,63,66,69,74,78,82,85,89,89,92,96,100,104,105,105,106,107,108,108,108,109,109,109,109,109,109,109,109,109,109,109,109,107,107,107,105,105,105,105,105,105,105,105,105,105,105,105,107,107,107,107,107,107,107,107,107,109,110,111,111,112,114,115,117,117,117,118,118,118,119,119,120,125];
		while( ar.length ) {
			_values.push( ar.shift() );
		}
		
		redrawBitmap();
		
		addEventListener( Event.ADDED_TO_STAGE, addedToStage );
	}
	private function addedToStage( e:Event ) :void {
		removeEventListener( Event.ADDED_TO_STAGE, addedToStage );
		addEventListener( MouseEvent.MOUSE_DOWN, mouseDown, false, 0, true );
		stage.addEventListener( MouseEvent.MOUSE_MOVE, mouseMove, false, 0, true );
		stage.addEventListener( MouseEvent.MOUSE_UP, mouseUp, false, 0, true );
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	private var _bmp :Bitmap;
	private var _values :Vector.<int>;
	private var _isDragging :Boolean = false;
	private var _lastPt :Point;
	
	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	
	// At the given horizontal interpolation (0...1), return the pitch value as a range in (0...1)
	public function valueAtLerp( n:Number ) :Number {
		var horiz:Number = n * (WIDTH-1);
		var idx:int = Math.min( Math.floor(horiz), WIDTH-2 );
		var val:Number = Num.interpolate( _values[idx], _values[idx+1], horiz-Math.floor(horiz) );
		return (HEIGHT-val) / HEIGHT; 
	}
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	private function mouseDown( e:MouseEvent ) :void {
		_isDragging = true;
		_lastPt = new Point( mouseX, mouseY );
		redrawBitmap();
	}
	private function mouseMove( e:MouseEvent ) :void {
		if( !_isDragging ) return;
		
		var pt:Point = new Point( Math.max(0,Math.min(WIDTH,mouseX)), mouseY );
		if( !_bmp.bitmapData.rect.containsPoint(pt) ) return;	// Dragged outside the rectangle, so ignore point
		
		// Interpolate from the last point to the current one. Set the values
		var left:Point;
		var right:Point;
		if( _lastPt.x == pt.x ) {	// Same X
			left = right = pt;
		} else if( _lastPt.x < pt.x ) {	// Moving right
			left = _lastPt;
			right = pt;
		} else {	// Moving left
			left = pt;
			right = _lastPt;
		}
		
		// Interpolate across the values
		for( var i:int=left.x; i<=right.x; i++ ) {
			var lerp:Number = (i-left.x) / (right.x-left.x);
			_values[i] = Num.interpolate( left.y, right.y, lerp );
		}
		
		_lastPt = pt;
		redrawBitmap();
	}
	private function mouseUp( e:MouseEvent ) :void {
		_isDragging = false;
	}
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	private function redrawBitmap() :void {
		_bmp.bitmapData.fillRect( _bmp.bitmapData.rect, BG_COLOR );
		for( var i:int=0; i<WIDTH; i++ ) {
			_bmp.bitmapData.fillRect( new Rectangle( i, _values[i], 1, HEIGHT ), LINE_COLOR );
		}
	}
}

}

