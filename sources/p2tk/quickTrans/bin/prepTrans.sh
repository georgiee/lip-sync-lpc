#!/bin/sh
#
# Usage:
#
# $ prep_trans.sh filename.wav
#
# Output:
#
# 1) filename.IntensityTier (a Praat IntensityTier file that will be used to determine the timestamps for the utterance chunks in the audio file)
# 2) filename.utt (a list of timestamps for the beginning and end of the utterance chunks to be extracted)

if [ $# -ne 1 ]
then
  echo "Usage: prep_trans.sh filename.wav"
  exit 2
fi

if [ ! -f $1 ]
then
  echo "ERROR:  file $1 does not exist"
  exit 1
fi

# create a temporary directory for storing the extracts to be transcribed
if [ ! -d $TEMP ]
then
  mkdir $TEMP
fi

source ./config

# the full path of the current working directory (it's easier to use Praat from the command line if you specify the complete path for the file arguments)
p=`pwd`

# get the extension used for the audio file
ext=`echo $p/$1 | sed 's/.*\.\([a-z][a-z]*\)/\1/'`

intensityFile=`echo $p/$1 | sed "s/\.$ext$/.IntensityTier/"`
uttFile=`echo $p/$1 | sed "s/\.$ext$/.utt/"`

echo "running intensity.praat on $p/$1"
echo ""
praat $CODE_DIR/intensity.praat $p/$1 $intensityFile

echo "running intensity2times.py on $intensityFile with the following configuration variables:"
echo "minPauseDuration=$minPauseDuration"
echo "stDevFraction=$stDevFraction"
echo "begPad=$begPad"
echo "endPad=$endPad"
echo ""
python $CODE_DIR/intensity2times.py $intensityFile $minPauseDuration $stDevFraction $begPad $endPad > $uttFile

echo "removing old audio files from tmp/"
echo ""
rm $TEMP/*

echo "running extractUtterances.praat"
echo ""
praat $CODE_DIR/extractUtterances.praat $p/$1 $uttFile ../$TEMP
