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
import flash.events.*;
import flash.net.*;
import flash.utils.*;
import ctrlz.speech.*;

public class PhonemeDict extends EventDispatcher {
		
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function PhonemeDict()
	{
		super();
	}
	
	public function init() :void {
		var req:URLRequest = new URLRequest("cmudict.dat");
		
		_loader = new URLLoader();
		_loader.addEventListener( IOErrorEvent.IO_ERROR, ioError, false, 0, true );
		_loader.addEventListener( SecurityErrorEvent.SECURITY_ERROR, securityError, false, 0, true );
		_loader.addEventListener( Event.COMPLETE, loaderComplete, false, 0, true );
		_loader.dataFormat = URLLoaderDataFormat.BINARY;
		trace("Loading now...");
		_loader.load( req );
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	private var _loader :URLLoader;
	private var _dict:Object = {};
	
	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	public function getPhonemeCodes( txt:String ) :Array {
		var out:Array = [];
		for each( var word:String in txt.split(/\s+/g) ) {
			var ucWord:String = word.toUpperCase();
			
			// Remove punctuation and weird chars
			ucWord = ucWord.replace( /[^\w\d']/g, '' );
			
			if( !_dict[ ucWord ] ) continue;	// Word not found!
			
			// If this is not the first word in the array, add a separator
			if( out.length ) out.push( ' ' );
			var wordCodes:Array = PhonemeChars.getCodeArray( _dict[ ucWord ] );
			out = out.concat( wordCodes );
		}
		return out;
	}
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	private function ioError( e:IOErrorEvent ) :void {
		trace("IO ERROR", e);
	}
	private function securityError( e:SecurityErrorEvent ) :void {
		trace("Security error", e);
	}
	private function progressEvent( e:ProgressEvent ) :void {
		dispatchEvent( e );
	}
	private function loaderComplete( e:Event ) :void {
		trace("Complete!", e);
				
		// Attempt to decompress the .gzip file
		var ba:ByteArray = ByteArray(_loader.data);
		ba.uncompress();

		var str:String = ba.toString();
		var lines:Array = str.split("\n");
		for each( var line:String in lines ) {
			var pair:Array = line.split("\t");
			_dict[pair[0]] = pair[1];
		}
		
		trace( "DICTIONARY TEST:", _dict['ZACH'], '==', PhonemeChars.getCodeArray(_dict['ZACH']) );
		
		dispatchEvent( e );
		
		/*
		var gunk:Array = str.split("\n");
		for( var i:int=0; i<10; i++ ) {
			trace(gunk[i]);
		}
		*/
	}
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	
}

}

