# Usage:
#
# praat extract_segment.praat infile.wav outfile.wav beg_time end_time

form Get_arguments
  text infile
  text outfile
  real beg
  real end
endform

Read from file... 'infile$'
Extract part... 'beg' 'end' Rectangular 1 no
Write to WAV file... 'outfile$'
echo extracted BEG 'beg' END 'end' from 'infile$'
