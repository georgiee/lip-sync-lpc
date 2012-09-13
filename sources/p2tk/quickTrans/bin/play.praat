# Usage:
#
# praat play.praat filename.wav

form Get_arguments
  text wavfile
endform

Read from file... 'wavfile$'
wavname$ = selected$ ("Sound")
select Sound 'wavname$'
Play
