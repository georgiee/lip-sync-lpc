package harness {

/**
 *	Linear Predictive Coding?! How does this stuff work?? I am not good with computer
 *
 *	@langversion ActionScript 3.0
 *	@playerversion Flash 9.0
 *
 *	@author Zach Archer
 *	@since  08.08.2009
 */
import flash.display.*;
import flash.errors.*;
import flash.events.*;
import flash.geom.*;
import flash.media.*;
import flash.net.*;
import flash.text.*;
import flash.utils.*;
import com.zacharcher.math.*;
import ctrlz.speech.*;

public class LPCSynthHarness extends Sprite {
	
	//--------------------------------------
	// CLASS CONSTANTS
	//--------------------------------------
	//private static const LPC_FILE_NAME :String = "retro_mic_test.lpc";
	private static const LPC_FILE_NAME :String = "phonemes.lpc";
	private static const PHONEME_LPC_FILE :String = "phonemes.lpc";
	private static const PHONEME_MARKER_FILE :String = "phonemeMarkers.txt";
	private static const MASTER_VOLUME :Number = 0.001 / 17;
	
	private static const VOWEL_FRAMES:Number = 8.0;
	private static const CONSONANT_FRAMES:Number = 3.0;
	private static const SPACE_FRAMES :Number = 4.0;
	
	private static const MIN_PITCH :Number = 250;
	private static const MAX_PITCH:Number = 10;
	
	private static const HARD_OUTPUT :Boolean = true;	// true == square aliasing, false = triangular ramping
	
	//--------------------------------------
	//  CONSTRUCTOR
	//--------------------------------------
	
	/**
	 *	@constructor
	 */
	public function LPCSynthHarness()
	{
		super();
		trace("Hello world!");
		
		stage.align = StageAlign.TOP;
		stage.scaleMode = StageScaleMode.NO_SCALE;
		
		_pitchEnv = new FreehandEnvelope();
		_pitchEnv.x = 355;
		_pitchEnv.y = 55;
		addChild( _pitchEnv );
				
		for each( var str:String in "pitch,noise,both".split(',') ) {
			var disp:DisplayObject = this[str];
			disp.addEventListener( MouseEvent.CLICK, playButtonClick, false, 0, true );
			disp['tf'].text = str.toUpperCase();
		}
		_playbackType = 'both';
		both.transform.colorTransform = new ColorTransform( -1,-1,-1,1, 255,255,255,0 );
		
		//loadButton.addEventListener( MouseEvent.CLICK, loadLPCNow );
		sayButton.addEventListener( MouseEvent.CLICK, sayItNow );
		creditsButton.addEventListener( MouseEvent.CLICK, creditsClick );
		
		// Before we can say anything, load the phoneme data
		loadingNow.rect.width = 0;
		loadingNow.tf.text = "Loading dictionary";
		isLoadingVisible = true;
		phonemes.text = '';
		
		addEventListener( Event.ENTER_FRAME, initFrame );		
	}
	
	private var _waitFrames:int=0;
	private function initFrame( e:Event ) :void {
		isLoadingVisible = true;
		_waitFrames++;
		if( _waitFrames < 3 ) return;
		removeEventListener( Event.ENTER_FRAME, initFrame );

		var CSScombo:TextFormat = new TextFormat("Arial", 18, 0x000000);
		tf.setStyle( "textFormat", CSScombo );

		_dict = new PhonemeDict();
		_dict.addEventListener( ProgressEvent.PROGRESS, progressEvent, false, 0, true );
		_dict.addEventListener( Event.COMPLETE, dictComplete, false, 0, true );
		_dict.init();
	}
	
	private function dictComplete( e:Event ) :void {
		_phonFrames = new Vector.<LPCFrame>();
		_markerLoader = new URLLoader();
		_markerLoader.addEventListener( Event.COMPLETE, markerLoadComplete );
		_markerLoader.dataFormat = URLLoaderDataFormat.TEXT;
		var req:URLRequest = new URLRequest(PHONEME_MARKER_FILE);
		_markerLoader.load( req );
	}
	
	private function markerLoadComplete( e:Event ) :void {
		trace("Marker load complete!");
		var markerText:String = String(_markerLoader.data);
		var lines:Array = markerText.split("\n");
		_markers = {};
		_markerLengths = {};
		
		var lastPhon:String = '';
		var lastMarker :int = 0;
		for each( var line:String in lines ) {
			var pair:Array = line.split("\t");
			if( pair ) {
				var phon:String = String( pair[0] );
				var marker:int = int( pair[1] );
				_markers[phon] = marker;

				// Store the length of this phoneme, if we can
				if( lastPhon ) {
					_markerLengths[lastPhon] = marker - lastMarker;
				}
				lastPhon = phon;
				lastMarker = marker;
			}
		}
		
		loadLPCNow( _phonFrames, LPC_FILE_NAME );
	}
	
	//--------------------------------------
	//  PRIVATE VARIABLES
	//--------------------------------------
	// LOAD PHONEMES AND MARKERS
	private var _loader :URLLoader;
	private var _loadTargetFrames :Vector.<LPCFrame>;
	private var _markerLoader :URLLoader;
	private var _markers :Object;
	private var _markerLengths :Object;
	private var _phonFrames :Vector.<LPCFrame>;

	// FILTER PARAMS
	private var _order :int;
	private var _sampleRate :Number;
	private var _framesPerSecond :Number;	
	private var _frameSamples :int = 0;	// increases as we step through the frame
	private var _pulseTicker :int = 0;
	private var _lastSample :Number = 0;
	private var _lastAlias :Number = 0;
	private var _zss :Vector.<Number>;	// Feedback "buckets"
	
	// PLAYBACK
	private var _sound :Sound;
	private var _chan :SoundChannel;
	private var _playFrames :Vector.<LPCFrame>;
	private var _playingFrame :int = 0;
	
	// DICTIONARY
	private var _dict :PhonemeDict;
	
	// CONTROLS
	private var _pitchEnv :FreehandEnvelope;
	private var _playbackType :String;
	
	//--------------------------------------
	//  GETTER/SETTERS
	//--------------------------------------
	private function set isLoadingVisible( b:Boolean ) :void {
		var disp:DisplayObject;
		for each( disp in [tf,sayButton,creditsButton,phonemes,pitchControl,_pitchEnv,speedSlider,speed,playLabel,pitch,noise,both] ) {
			disp.visible = !b;
		}
		for each( disp in [loadingNow] ) {
			disp.visible = b;
		}
	}
	
	//--------------------------------------
	//  PUBLIC METHODS
	//--------------------------------------
	
	//--------------------------------------
	//  EVENT HANDLERS
	//--------------------------------------
	private function loadLPCNow( targetFrames:Vector.<LPCFrame>, fileName:String ) :void {
		loadingNow.tf.text = "Loading sound data";
		loadingNow.rect.width = 0;
		isLoadingVisible = true;
		
		_loadTargetFrames = targetFrames;
		
		_loader = new URLLoader();
		_loader.addEventListener( Event.COMPLETE, lpcLoadComplete, false, 0, true );
		_loader.addEventListener( ProgressEvent.PROGRESS, progressEvent, false, 0, true );
		_loader.dataFormat = URLLoaderDataFormat.BINARY;
		var req:URLRequest = new URLRequest(fileName);
		_loader.load( req );		
	}
	
	private function progressEvent( e:ProgressEvent ) :void {
		loadingNow.rect.width = 260 * (e.bytesLoaded / e.bytesTotal);
	}
	
	private function lpcLoadComplete( e:Event ) :void {
		trace("LPC load complete");
		var ba:ByteArray = ByteArray(_loader.data);
		ba.endian = Endian.LITTLE_ENDIAN;
		_order = ba.readShort();
		_sampleRate = ba.readFloat();
		_framesPerSecond = ba.readFloat();
		
		trace("ORDER IS", _order);
		
		for( var i=0; i<9999; i++ ) {
			var frame:LPCFrame = new LPCFrame();
			try {
				frame.power = ba.readFloat();
				frame.unvoiced = ba.readFloat();
				frame.pitch = ba.readFloat();
				frame.err = ba.readFloat();
				for( var j:int=0; j<_order; j++ ) {
					frame.coeffs.push( ba.readFloat() );
				}
			} catch( e:Error ) {
				trace("Read: caught an error:", e);
				break;
			}
			_loadTargetFrames.push( frame );
		}
		
		isLoadingVisible = false;
		if( _playFrames ) playNow();
	}
	
	private function sayItNow( e:MouseEvent=null ) :void {
		var sentence:Array = _dict.getPhonemeCodes( tf.text );
		var showText:String = '"' + sentence.join(' ') + '"';
		phonemes.text = showText.toLowerCase();

		_playFrames = new Vector.<LPCFrame>();

		var emptyFrame:LPCFrame = new LPCFrame();
		emptyFrame.setEmpty();

		// Using these phonemes, build a new set of LPC data
		var c:int;
		var fr:LPCFrame;
		var div:Number;
		for each( var phon:String in sentence ) {
			if( phon == ' ' ) {
				for( c=0; c<SPACE_FRAMES; c++ ) {
					_playFrames.push( emptyFrame );
				}
			} else {
				// Treat vowels & consonants differently
				if( PhonemeChars.isVowel(phon) ) {
					// VOWEL
					// Sample intermittant frames from the vowel? Might sound OK
					div = (_markerLengths[phon]-1) / VOWEL_FRAMES;
					for( c=0; c<VOWEL_FRAMES; c++ ) {
						fr = _phonFrames[ Math.round(_markers[phon] + c*div + (Rand.maybe ? 1.0 : 0.0)) ].clone();
						_playFrames.push( fr );
					}
					
					//_playFrames = _playFrames.concat( _phonFrames.slice( _markers[phon], _markers[phon]+Math.min(20,_markerLengths[phon]) ) );
					
				} else {
					// CONSONANT
					/*
					var slice:Vector.<LPCFrame> = _phonFrames.slice( _markers[phon], _markers[phon]+_markerLengths[phon] );
					for each( var frame:LPCFrame in slice ) {
						fr = frame.clone();
						fr.unvoiced = 1.0;
						_playFrames.push( emptyFrame );
					}
					*/
					var useFrames:Number = (phon == 'ER') ? VOWEL_FRAMES : CONSONANT_FRAMES;
					for( c=1; c<useFrames; c += 1 ) {
						fr = _phonFrames[ Math.round(_markers[phon] + c + (Rand.maybe ? 0.0 : 0.0)) ].clone();
						if( phon != 'ER' ) fr.unvoiced = 1.0;
						_playFrames.push( fr );
					}
				}
			}
		}
		
		// Now apply the pitch to the frames
		for( c=0; c<_playFrames.length; c++ ) {
			_playFrames[c].pitch = Num.interpolate( MIN_PITCH, MAX_PITCH, _pitchEnv.valueAtLerp(Number(c)/_playFrames.length) );
		}
		
		playNow();
	}
	
	private function enterFrame( e:Event ) :void {
		//
	}
	
	private function sampleData( e:SampleDataEvent ) :void {
		var fps:Number = _framesPerSecond * Math.pow(2,speedSlider.value);
		
		var frameWidth:int = Math.round(_sampleRate / fps);
		var srScale :Number = 44100.0 / _sampleRate;
		
		// Synthesize the frame data
		var output:Number;
		var i:int, j:int;
		var sampsCreated:int = 0;
		
		// Step through sample frames until we've generated enough audio, then break (further down in this function.)
		if( _playingFrame >= _playFrames.length ) {
			return;	// We have played all the audio, so exit
		}
		
		var frame:LPCFrame = _playFrames[_playingFrame];

		// Update the pitch in real time
		var pitch:Number = Num.interpolate( MIN_PITCH, MAX_PITCH, _pitchEnv.valueAtLerp(Number(_playingFrame)/_playFrames.length) );		
		frame.pitch = pitch;
		
		var nextFrame:LPCFrame = (_playingFrame < _playFrames.length-1) ? _playFrames[_playingFrame+1] : frame;	// LERPING
		for( var q:int=0; q<9999; q++ ) {
			
			output = 0.0;
			var lerpAmt:Number = Number(_frameSamples)/frameWidth;	// LERPING
			
			var power:Number = Num.interpolate( frame.power, nextFrame.power, lerpAmt );
			//var power = frame.power;
			if( (frame.unvoiced > 0 || _playbackType=='noise') && _playbackType!='pitch' ) {
				// UNVOICED (noise) frame
				output = power * 20.0 * (2.0 * Math.random() - 1.0);	// Bipolar random noise
				
			} else {
				// VOICED (pitched) frame
				_pulseTicker--;
				if( _pulseTicker <= 0 ) {
					pitch = Num.interpolate( frame.pitch, nextFrame.pitch, lerpAmt );
					_pulseTicker = int(pitch + 0.5);	// But pitch is already an int?? Um
					//_pulseTicker = int(frame.pitch + 0.5);
					
					//if( !alt ) {	// IMPULSE TRAIN:
					output = power * pitch;
				}
				/*
				// GLOTTAL PULSE CODE:
	            if( alt && lpc->alt )
	            {
	                j = (int)(pitch+.5) - lpc->ticker + 0;
	                if( j >= 0 && (j*4) < lpc->alt_len )
	                    output = power * pitch * .5f * lpc->alt[j*4] / (float)SHRT_MAX;
	                else output *= .9f;
	            }
				*/
			}
			
			// Bucket cascade
			for( j=0; j<_order; j++ ) {
				output += _zss[j] * frame.coeffs[j];
				//output += _zss[j] * Num.interpolate( frame.coeffs[j], nextFrame.coeffs[j], lerpAmt );
			}
			for( j=_order-1; j>0; j-- ) {
				_zss[j] = _zss[j-1];
			}
			_zss[0] = output;
			
			// Write the sound out.
			// Make sure we alias at the correct sample rate.
			output *= MASTER_VOLUME;
			// Interpolate between the last sample and this one
			var lerp:Number = (_lastSample * _lastAlias) + (output * (1-_lastAlias));
			e.data.writeFloat( lerp );	// left channel
			e.data.writeFloat( lerp );	// right channel
			sampsCreated++;
			// Now we've written some portion of the new sample, so store this
			var alias:Number = 1-_lastAlias;
			
			var s:int;
			for( s=0; s<=100; s++ ) {	// for instead of while, so it can't infinite loop :P
				if( HARD_OUTPUT ) {
					if( alias + 1.0 < srScale ) {
						// If writing another sample would NOT overflow the output sample, then do it
						e.data.writeFloat( output );	// left channel
						e.data.writeFloat( output );	// right channel
						sampsCreated++;
						alias++;
					} else {
						// Writing another sample would overflow the output sample, so just store it
						// for the next round.
						_lastSample = output;
						_lastAlias = srScale - alias;
						break;
					}

				} else {
					// Triangular aliasing
					var a:Number;
					for( a=_lastAlias; a<(srScale-1); a++ ) {
						var out:Number = Num.interpolate( _lastSample, output, a/srScale );
						e.data.writeFloat( out );	// left channel
						e.data.writeFloat( out );	// right channel
						sampsCreated++;
					}
					_lastSample = output;
					_lastAlias = (a - srScale) + 1;
					break;
				}
				
				if( s == 100 ) {
					throw new Error(["Something bad happened: we're aliasing a sample into oblivion",
					s, _lastAlias, alias, output, sampsCreated].join(','));
				}
			}
			
			// Increment the number of samples, check to see if we need a new frame
			_frameSamples++;
			if( _frameSamples >= frameWidth ) {
				_frameSamples = 0;
				_playingFrame++;
				if( _playingFrame >= _playFrames.length ) {
					// Fill the rest of the sound buffer with 0's
					for( s=sampsCreated; s<4096; s++ ) {
						e.data.writeFloat( 0.0 );
						e.data.writeFloat( 0.0 );
					}
					return;
				}
				frame = _playFrames[_playingFrame];

				// Update the pitch in real time
				frame.pitch = Num.interpolate( MIN_PITCH, MAX_PITCH, _pitchEnv.valueAtLerp(Number(_playingFrame)/_playFrames.length) );
			}
		
			// Have we created enough samples to satisfy this audio event?
			if( sampsCreated >= 4096 ) {
				// That's enough for Flash.
				return;
			}
			
		}
		
		/*
		// TODO: This resets the _zss buckets at the start & end of each noise frame.
		// Is this necessary?? My ears currently say no.
		if( i == (frameWidth-1) || i == 0 ) {
			_zss = new Vector.<Number>();
			for( var o:int=0; o<_order; o++ ) {
				_zss.push( 0.0 );
			}
		}
		*/
	}
	
	private function soundComplete( e:Event ) :void {
		trace("Sound complete");
		_sound = null;
		_chan = null;
	}
	
	private var _lastChoice :int = -1;
	private function creditsClick( e:MouseEvent ) :void {
		var cheeky:Array = [
			"Zach Archer made this. I have no idea why.",
			"Rhubarb is a controversial pie ingredient.",
			"I'm sorry, Dave. I'm afraid I can't do that.",
			"Zach Archer is the most insane hacker on Earth.",
			"Quiet, puny human. I'm trying to concentrate.",
			"A syntax error?! This is unprecedented.",
			"These knobs go to eleven.",
			"Elf needs food, badly!",
			"Shots don't hurt other players... Yet."
		];
		
		// Don't repeat the same phrase twice in a row!
		var choice:int = _lastChoice;
		while( choice == _lastChoice ) {
			choice = Rand.int(cheeky.length);
		}
		_lastChoice = choice;
		tf.text = cheeky[ choice ];
		
		sayItNow();
	}
	
	private function playButtonClick( e:MouseEvent ) :void {
		for each( var str:String in "pitch,noise,both".split(',') ) {
			var button:DisplayObject = this[str];
			if( button == e.currentTarget ) {
				button.transform.colorTransform = new ColorTransform( -1,-1,-1,1, 255,255,255,0 );
				_playbackType = str;
			} else {
				button.transform.colorTransform = new ColorTransform();
			}
		}
	}
	
	//--------------------------------------
	//  PRIVATE & PROTECTED INSTANCE METHODS
	//--------------------------------------
	private function playNow() :void {
		trace("Playing now!");
		
		if( !_playFrames ) {
			throw new Error("Cannot play, I have no frames.");
			return;
		}
		_playingFrame = 0;
		_frameSamples = 0;
		_zss = new Vector.<Number>();
		for( var o:int=0; o<_order; o++ ) {
			_zss.push( 0.0 );
		}
		
		// Stop the old sound
		if( _sound ) {
			_sound.removeEventListener( SampleDataEvent.SAMPLE_DATA, sampleData );
			_sound.removeEventListener(Event.COMPLETE, soundComplete);
		}
		if( _chan ) {
			_chan.stop();
		}
		
		// Start a new sound
		_sound = new Sound();
		_sound.addEventListener( SampleDataEvent.SAMPLE_DATA, sampleData, false, 0, true );
		_sound.addEventListener(Event.COMPLETE, soundComplete, false, 0, true);
		_chan = _sound.play();
	}
	
}

}

