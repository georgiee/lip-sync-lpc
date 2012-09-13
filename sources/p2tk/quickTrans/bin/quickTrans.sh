#!/bin/sh
#
# Use this script to start up an interface that enables transcription of utterance chunks extracted from an audio file.  The interface recognizes the command 'r', used to repeat the current utterance chunk.  All other text input is taken as a transcription of the utterance chunk, terminated by a newline.  To exit the interface, type Ctrl-C.  The transcription for each utterance chunk is appended to the transcription file after each transcription is made; the interface allows the user to quit in the middle of the file, and start up again in the same place at a later time.
#
# Usage:
#
# $ sh quickTrans.sh filename.wav
#
# Output:
#
# a file filename.trans containing one line for each utterance chunk of the form:
#
# start_time	end_time	transcription

if [ $# -ne 1 ]
then
  echo "Usage:  transcribe.sh filename.wav"
  exit 2
fi

if [ ! -f $1 ]
then
  echo "ERROR:  file $1 does not exist"
  exit 1
fi

source ./config

# the full path of the current working directory
p=`pwd`

# get the extension used for the audio file
ext=`echo $p/$1 | sed 's/.*\.\([a-z][a-z]*\)/\1/'`

# file for storing the transciption
transFile=`echo $p/$1 | sed "s/\.$ext$/.trans/"`

uttFile=`echo $p/$1 | sed "s/\.$ext$/.utt/"`

if [ -f $transFile ]
then
  # the ending time for the last transcribed utterance
  time=`tail -1 $transFile | cut -f2`
  if [ "$time" != "" ]
  then
    echo "$transFile already exists -- starting from the next untranscribed utterance"
    # output the times for only those utterances remaining to be transcribed
    sed "1,/$time/ d" $uttFile > temp.utt
  else
    # output all of the utterances
    cat $uttFile > temp.utt
  fi
else
  touch $transFile
  # output all of the utterances
  cat $uttFile > temp.utt
fi

# get the number of utterances to be transcribed
n=`wc temp.utt | sed 's/  */ /g' | cut -d' ' -f2`

# for each utterance
i=1
while [ $i -le $n ]
do
  # if the user types an 'r' character, then the current utterance is repeated
  trans=r
  while [ "$trans" = "r" ]
  do
    # get the start and end times
    beg=`sed "${i}! d" temp.utt | cut -f2`
    end=`sed "${i}! d" temp.utt | cut -f4`
  
    wavFile=../$TEMP/${beg}_${end}.wav
    # play the audio file in praat 
    echo ""
    echo "playing $wavFile"
    # use printf here to suppress the newline character ("echo -n" will not work if the shell uses the builtin echo command)
    printf "Transcription: "
    praat $CODE_DIR/play.praat $wavFile
    read trans
  done
  echo "$beg\t$end\t$trans" >> $transFile
  let i=i+1
done
