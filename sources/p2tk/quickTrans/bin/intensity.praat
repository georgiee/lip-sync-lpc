# Usage:
#
# praat intensity.praat filename.wav filename.IntensityTier
#
# Output:
#
# A Praat IntensityTier file corresponding to the audio file filename.wav.

form Get_arguments
  text wavfile
  text outfile
endform

Read from file... 'wavfile$'
To Intensity... 75 0 yes
Down to IntensityTier
Write to short text file... 'outfile$'
