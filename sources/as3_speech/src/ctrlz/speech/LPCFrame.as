package ctrlz.speech {

/**
 *	Class description.
 *
 *	@langversion ActionScript 3.0
 *	@playerversion Flash 9.0
 *
 *	@author Zach Archer
 *	@since  09.08.2009
 */
import flash.errors.*;
import flash.utils.*;

public class LPCFrame extends Object {
	
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function LPCFrame()
	{
		super();
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	// Obtained from csound::lpanal
	public var power :Number = 0;
	public var unvoiced :Number = 0;
	public var err :Number = 0;
	public var pitch :Number = 0;
	
	public var coeffs :Vector.<Number> = new Vector.<Number>();

	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	public function clone() :LPCFrame {
		var out:LPCFrame = new LPCFrame();
		out.power = power;
		out.unvoiced = unvoiced;
		out.err = err;
		out.pitch = pitch;
		out.coeffs = new Vector.<Number>();
		for each( var n:Number in coeffs ) {
			out.coeffs.push( n.valueOf() );
		}
		return out;
	}
	
	public function parseAnalysisData( ba:ByteArray ) :void {
		ba.endian = Endian.LITTLE_ENDIAN;
		unvoiced = ba.readDouble();
		power = ba.readDouble();
		err = ba.readDouble();
		pitch = ba.readDouble();
		
		// Read all coefficients
		var i:int;
		for( i=0; i<100; i++ ) {
			var c:Number;
			try {
				c = ba.readDouble();
			} catch( e:EOFError ) {
				break;
			}
			coeffs.push( c );
		}
		
		// This is a hunch:
		// Divide each coefficient by the number of coefficients, else horrible feedback.
		for( i=0; i<coeffs.length; i++ ) {
			coeffs[i] /= coeffs.length * 2;
		}
		
		//if(Math.random()<0.01) trace("?", coeffs);
	}
	
	public function setEmpty( coeffCount:int=50 ) :void {
		for( var c:int=0; c<coeffCount; c++ ) {
			coeffs.push( 0 );
		}
		pitch = 1;
	}
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	
}

}

