# Segments an IntensityTier file into utterance chunks.  The algorithm for determining the location of the chunks looks for areas of the audio file that have every intensity value lower than a user-defined maximum intensity level (threshold) and have a duration longer than a user-defined minimum pause length (minPauseDuration). If a portion of the audio file matches these two criteria, it is considered a pause between two utterances, and the utterance boundaries are marked accordingly.
#
# The values that generally seem to work pretty well for my audio files are:
#
# minPauseDuration = 0.2 sec
# threshold = intensityMean - intensityStDev * 0.33
#
# Usage:
#
# python intensity2times.py filename.IntensityTier minPauseDuration intensityStDev begPad endPad
#
# Output to stdout of the form:
#
# BEG     0.045   END     0.333
# BEG     0.472   END     0.792
# BEG     0.994   END     1.272
# BEG     1.442   END     1.794

import sys
from math import sqrt

def mean(X):
  m = float(sum(X)) / len(X)
  return m

def variance(X):
  m = mean(X)
  D = [x - m for x in X]
  D2 = [d**2 for d in D]
  var = float(sum(D2)) / (len(X) - 1)
  return var

def stdev(X):
  var = variance(X)
  return sqrt(var)

def combine_utts(L, x):
  utts = []
  # starting from the second utterance, get the diff. between the start time of the current utt. and the end time of the previous utt.
  for i in range(len(L))[1:]:
    diff = L[i][0] - L[i-1][1]
    if diff >= x:
      utt = (L[i-1])
      utts.append(utt)
    else:
      L[i] = (L[i-1][0], L[i][1])
  utts.append(L[i])
  return utts

# pad the start and end times for the utterances to make the transitions easier to transcribe
def pad_utts(L, beg_pad, end_pad):
  # add the first utterance as is, since subtracting might bring us before the actual start time of the file
  utts = [L[0]]
  for utt in L[1:-1]:
    utts.append((utt[0]-beg_pad, utt[1]+end_pad))
  # add the last utterance as is, since adding might bring us after the actual end time of the file
  utts.append(L[-1])
  return utts

fr = open(sys.argv[1], 'rU')

# default value for minimum inter-utterance pause length
min = 0.2
if len(sys.argv) > 2:
  min = float(sys.argv[2])

stDevFraction = 0.33
if len(sys.argv) > 3:
  stDevFraction =  float(sys.argv[3])

begPad = 0.05
if len(sys.argv) > 4:
  begPad =  float(sys.argv[4])

endPad = 0.25
if len(sys.argv) > 5:
  endPad =  float(sys.argv[5])

lines = fr.readlines()
n = 1
# the total number of intensity samples
max = int(lines[5].strip())
intensities = []
times = []
while n <= max:
  time = float(lines[n*2+4].strip())
  times.append(time)
  intensity = float(lines[n*2+5].strip())
  intensities.append(intensity)
  n += 1

m = mean([x for x in intensities if x > 0])
sd = stdev([x for x in intensities if x > 0])

threshold = m - sd * stDevFraction

#print m, sd, threshold

# the start time of an utterance
beg = -1

# a list for storing utterance start and end times as tuples
utts = []

for i in range(len(intensities)):
  if intensities[i] <= 0:
    continue
  if beg < 0:
    if intensities[i] >= threshold:
      beg = times[i]
      # save this for the final utterance
      #last_beg = beg
    else:
      continue
  else:
    if intensities[i] >= threshold:
      continue
    else:
      end = times[i]
      utts.append((beg, end))
      beg = -1

if beg > 0:
  utts.append((beg, times[-1]))

utts = combine_utts(utts, min)

utts = pad_utts(utts, begPad, endPad)

for utt in utts:
  print "BEG\t%.3f\tEND\t%.3f" % (utt[0], utt[1])
