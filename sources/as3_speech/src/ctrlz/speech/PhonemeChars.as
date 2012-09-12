package ctrlz.speech {

/**
 *	Class description.
 *
 *	@langversion ActionScript 3.0
 *	@playerversion Flash 9.0
 *
 *	@author Zach Archer
 *	@since  07.08.2009
 */
public class PhonemeChars extends Object {
	
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	private static const CODES :String = "AA AE AH AO AW AY EH EY IH IY OW OY UH UW B CH D DH ER F G HH JH K L M N NG P R S SH T TH V W Y Z ZH";
	private static const CHARS :String = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function PhonemeChars()
	{
		trace("PhonemeChars: Static class, do not instantiate");
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	private static var _codesToChars :Object;
	private static var _charsToCodes :Object;
	
	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	public function get codes() :Array { return CODES.split(' '); }
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	public static function getChar( codes:String ) :String {
		if( !_codesToChars ) createLookups();
		var out:String = '';
		for each( var code:String in codes.split(' ') ) {
			out += _codesToChars[code];
		}
		return out;
	}
	public static function getCodeArray( chars:String ) :Array {
		if( !_charsToCodes ) createLookups();
		var out:Array = [];
		for( var i:int=0; i<chars.length; i++ ) {
			out.push( _charsToCodes[ chars.substr(i,1) ] );
		}
		return out;
	}
	
	public static function isVowel( code:String ) :Boolean {
		var match:Array = code.match( /^(AA|AE|AH|AO|AW|AY|EH|EY|IH|IY|OW|OY|UH|UW)$/ );
		return Boolean(match);
	}
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	private static function createLookups() {
		_codesToChars = {};
		_charsToCodes = {};
		var count:int = 0;
		for each( var code:String in CODES.split(' ') ) {
			var char:String = CHARS.substr( count, 1 );
			_codesToChars[code] = char;
			_charsToCodes[char] = code;
			count++;
		}
	}
}

}

