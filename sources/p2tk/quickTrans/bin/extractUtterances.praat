# This Praat script reads an input file, filename.utt, containing timestamps for the utterance chunks of the form:
#
# BEG     0.755   END     1.663
# BEG     2.216   END     3.135
# BEG     3.571   END     4.351
#
# The script then extracts these chunks from the audio file, filename.wav, and outputs them to tempdir.
#
# Usage:
#
# praat extract_utterances.praat filename.wav filename.utt tempdir

form Get_arguments
  text wavfile
  text uttfile
  text tempdir
endform

Read Strings from raw text file... 'uttfile$'
uttname$ = selected$ ("Strings")
n_utts = Get number of strings

Open long sound file... 'wavfile$'
wavname$ = selected$ ("LongSound")

for i from 1 to n_utts
  select Strings 'uttname$'
  utt$ = Get string... i

  # store the beg and end times as numerical variables
  beg = extractNumber(utt$, "BEG")
  end = extractNumber(utt$, "END")
  select LongSound 'wavname$'
  Extract part... 'beg' 'end' Rectangular 1 no
  Rename... window
  select Sound window

  # now store the beg and end times as strings (to preserve the 0's for filename consistency)
  beg$ = extractWord$(utt$, "BEG")
  end$ = extractWord$(utt$, "END")
  Write to WAV file... 'tempdir$'/'beg$'_'end$'.wav
  echo extracted BEG 'beg$' END 'end$'

  select all
  minus LongSound 'wavname$'
  minus Strings 'uttname$'
  Remove
endfor

echo 
echo The number of utterances is 'n_utts'.
