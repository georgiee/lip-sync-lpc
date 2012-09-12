package harness {

/**
 *	Class description.
 *
 *	@langversion ActionScript 3.0
 *	@playerversion Flash 9.0
 *
 *	@author Zach Archer
 *	@since  07.08.2009
 */
import flash.display.*;
import flash.events.*;
import flash.net.*;
import flash.utils.*;
import ctrlz.speech.*;

public class DictCompressor extends Sprite {
	
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function DictCompressor()
	{
		super();
		trace("This is the Dictionary compression harness. W00t w00t! Click for joy.");
		
		stage.addEventListener( MouseEvent.CLICK, click );
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	private var _file :FileReference;
	private var _toSave :ByteArray;
	
	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	private function click( e:MouseEvent ) :void {
		if( _toSave ) {
			trace("Saving...");
			_file = new FileReference();
			_file.save( _toSave, "cmudict.dat" );
			_toSave = null;
			trace("Done.");
			return;
		}
		
		trace("Click! Select the dictionary file to load...");
		_file = new FileReference();
		_file.addEventListener( Event.SELECT, fileSelect, false, 0, true );
		_file.browse();
	}
	
	private function fileSelect( e:Event ) :void {
		trace("Loading now...");
		_file.addEventListener( Event.COMPLETE, loadComplete, false, 0, true );
		_file.load();
	}
	
	private function loadComplete( e:Event ) :void {
		trace("Screening out superfluous lines...")
		var slim:String = '';
		
		var incoming:String = _file.data.toString();
		var badCount:int=0;
		var goodCount:int=0;
		for each( var line:String in incoming.split("\n") ) {
			var items:Array = line.split("\t");
			// words with punctuation / numbers are not accepted
			var match:Array = items[0].match(/^[A-Z']+$/);
			if( match ) {
				// This line passed the test.
				slim += items[0] + "\t";
				// Convert the phoneme codes to chars
				slim += PhonemeChars.getChar( items[1] );
				slim += "\n";
				
				if( goodCount < 10 ) {
					trace("OK:", items[0], '/', items[1], '/', PhonemeChars.getChar( items[1] ) );
					goodCount++;
				}
				
			} else if( badCount < 10 ) {
				trace("REJECTED:", items[0]);
				badCount++;
				if( badCount >= 10 ) {
					trace("Okay, that's enough, you get the idea... it works like it should :P");
				}
			}
		}
		
		trace("Compressing...");
		var ba:ByteArray = new ByteArray();
		ba.writeUTFBytes( slim );
		ba.compress();
		
		_toSave = ba;
		
		trace("Done! Click to save.");
	}
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	
}

}

