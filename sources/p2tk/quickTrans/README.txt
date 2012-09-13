I. GENERAL OVERVIEW
II. SYSTEM REQUIREMENTS
III. INSTALLATION
IV. USAGE
V. CHANGING CONFIGURATION VARIABLES TO IMPROVE PERFORMANCE
VI. FAQ


I. GENERAL OVERVIEW

quickTrans is a collection of shell scripts, python scripts, and Praat scripts to be used for transcribing audio files.  It is meant to be simple and lightweight, and should enable the user to transcribe speech quickly.  The general procedure is to first automatically segment the audio file into short files corresponding to individual utterances (breath groups), and transcribe each of these.  Ideally, each of these utterances will fit into the user's short-term memory, and no repetition or back-tracking in the audio file will be necessary.  In my experience, the degree to which this is successful depends heavily on characteristics of the audio file being transcribed (amount of background noise, amount of overlap between multiple interlocutors, etc.).  Configuration parameters can be adjusted by the user in an attempt to adapt the system to the specific audio file being transcribed.

I have used this tool successfully for transcribing sociolinguistic interviews--my average transcription time is about five times real time (e.g., a 10 minute interview will take me 50 minutes to transcribe).  This rate obviously depends heavily on the characteristics of the audio file (and your typing speed!)  It has usually been necessary to also concurrently have the audio file open in a sound editor such as Praat in order to play back segments larger than the single utterances.  This is especially necessary when the audio file contains a lot of background noise, etc.


II. SYSTEM REQUIREMENTS

quickTrans requires a Unix (or Unix-like environment) to run.  On a Mac, it can be run in a Terminal window;  under Windows, I have run it in Cygwin.  The following software is required to run quickTrans:

1) Bourne shell (should be available by default on your system)
2) Python (can be downloaded here:  http://www.python.org)
3) Praat (can be downloaded here:  http://www.praat.org)

quickTrans has been tested on Mac Os 10.5.7 with Praat 5.0.38 and Python 2.5.1.


III. INSTALLATION

After downloading quickTrans.tar.gz, you will need to uncompress and expand the file.  Some systems will do this automatically for you.  If you need to do it manually, go to the directory where quickTrans.tar.gz is located, and type the following command:

$ tar -xvzf quickTrans.tar.gz

This command will create the directory quickTrans where all of the software is located.


IV. USAGE

NB:  These instructions assume you have a basic familiarity with commands in a Unix shell, including 'cd' for changing directories and 'cp' for copying files.

Copy the audio file you would like to transcribe to the quickTrans/ directory.  Assuming your audio file is named 'filename.wav', run the following command:

$ sh bin/prepTrans.sh filename.wav

The script prepTrans.sh prepares the audio file for transcription by segmenting it into small chunks corresponding to individual utterances.  These short chunks are saved in the tmp/ directory.  (NB:  In order for prepTrans.sh and quickTrans.sh to work, the command 'praat' needs to be accessible.  If you have never used Praat from the command line before, see the FAQ "How do I add 'praat' to my search path?")

To transcribe the individual utterances, run the following command:

$ sh bin/quickTrans.sh filename.wav

This starts up a simple interface that enables sequential transcription of the utterance chunks extracted from the audio file.  For each utterance to be trancribed, you will see a prompt displaying the name of the extract to be transcribed followed by a line for your transcription, e.g.:

playing tmp/0.056_2.147.wav
Transcription:

At the 'Transcription:' prompt, the user can do one of three possible actions:

1) type 'r' (for 'repeat') followed by a carriage return to hear the utterance again
2) type your transcription of the utterance followed by a carriage return (this will cause the next utterance to be played)
3) type Ctrl-C to exit the interface

Your transcription will be saved to the file 'filename.trans' (i.e., the original filename extension of the audio file will be replaced by '.trans'), with one line of the transcription file per utterance.  If you exit the quickTrans interface in the middle of a file, the next time you start transcribing it will pick up where you left off.


V. CHANGING CONFIGURATION VARIABLES TO IMPROVE PERFORMANCE

The configuration variables that I usually use are stored in the file 'config'.  They are:

* minPauseDuration=0.2
the minimum pause duration (in sec) required to end one utterance and start the next one

* stDevFraction=0.33
used for determining whether a sample of the audio file is part of an utterance or a pause; if the intensity of the sample is below this threshold:

threshold = intensityMean - instensityStDev * stDevFraction

I.e., if the intensity of a sample is lower the value that is 1/3 (by default) of a standard deviation below the global intensity mean for the file, then it is considered to be not part of an utterance.

* begPad=0.05
the amount of time that is added from the audio file to the beginning of each utterance (this aids in transcription by providing a little context)

* endPad=0.15
the amount of time that is added from the audio file to the end of each utterance (this aids in transcription by providing a little context)

These have worked well for most files;  however, I have had to modify them sometimes to make the transcription easier.  Audio files with lots of background noise, low intensity levels for a speaker (e.g., because the microphone was too far away), lots of overlap among speakers, etc. may require modification of these settings in order to cause utterances that are appropriate for transcription to be extracted.  Here are a few common problems that may arise, and suggested modifications of the configuration variables:

1) "The utterances are too long.  I can't transcribe them easily because I can't keep them in my short term memory."

This is usually caused by the presence of background noise in the file or because multiple speakers overlap each other.  Try decreasing the value of minPauseDuration (this should cause some of the utterances to be shorter in duration and increase the overall number of utterances extracted).  In my experience, I have found that utterances should be shorter than 3 sec for the most efficient transcription.

2) "Some speech from my audio file is not being extracted into utterances, and my transcription will be incomplete because of this."

This is usually caused by a speaker speaking very quietly or having their microphone too far away from their mouth (i.e., their intensity level is much lower than the overall mean intensity level).  Try lowering the value for intensityStDev--this will cause samples with smaller intensity values to be treated as part of an utterance.


VI. FAQ

* "How do I add Praat to my search path?"

(This response is tailored to Mac users.)

quickTrans uses the speech analysis program Praat for segmenting the audio file into utterances based on intensity levels, and for playing back the utterances for transcription.  In order to use Praat, the command 'praat' must be in your search path.  Depending on your system and how Praat was installed, it may or may not be there.  In order to find out, you can type:

$ which praat

If there is no output, or if you get a message saying that 'praat' wasn't found, then you need to add 'praat' to your search path.  Additionally, you could try running Praat from the command line:

$ praat

If Praat starts, then it's in your search path.  If you get a message such as 'command not found', then it isn't.

To add Praat to your search path, you first need to determine the location of the Praat command on your system.  On a Mac, the default installation location is:

/Applications/Praat.app/Contents/MacOS/Praat

Type the following command to see if it exists there:

$ ls /Applications/Praat.app/Contents/MacOS/Praat

If you don't get an error message, then this is the location you need to add to your path.  If you do get a message such as 'No such file or directory', then you can try to determine its actual location with the 'locate' command:

$ locate Praat

Notice the upper-case 'Praat' here--the Mac version of Praat is installed with a capital 'P'.  When I type the following command on my system, I get:

/Applications/Praat.app
/Applications/Praat.app/Contents
/Applications/Praat.app/Contents/Info.plist
/Applications/Praat.app/Contents/MacOS
/Applications/Praat.app/Contents/MacOS/Praat
/Applications/Praat.app/Contents/PkgInfo
/Applications/Praat.app/Contents/Resources
/Applications/Praat.app/Contents/Resources/Praat.icns

The one with 'Praat' as the right-most part of the filename is the location of the program that you will need to add to your path.

There are two ways to add Praat to your path (both assume you are using the Bourne shell or one of its variants):

1) Temporarily adding Praat to your search path:

To see your search path, type the command:

$ echo $PATH

To add the location for Praat to it, type:

$ PATH=$PATH:/Applications/Praat.app/Contents/MacOS

(Assuming that the location of the 'Praat' command is /Applications/Praat.app/Contents/MacOS/Praat.)

2) Permanently adding Praat to your search path:

Add the line:

PATH=$PATH:/Applications/Praat.app/Contents/MacOS

to the file .bash_login in your home directory.  Here is one way to do this:

$ cd $HOME
$ echo 'PATH=$PATH:/Applications/Praat.app/Contents/MacOS' >> .bash_login
