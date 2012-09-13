#!/usr/bin/env python

"""
Usage:
python extractFormants.py [options] filename.wav filename.TextGrid outputFile

Takes as input a sound file and a Praat .TextGrid file (with word and phone tiers) and outputs automatically extracted F1 and F2 measurements for each vowel (either as a tab-delimited text file or as a Plotnik file)
"""

SCRIPTS_HOME = 'bin'

import sys, os, getopt, math
import praat, esps, plotnik, cmu, vowel
import rpy2.robjects as robjects

class Word:
  transcription = ''
  phones = ''
  xmin = ''
  xmax = ''

class Phone:
  label = ''
  code = ''
  xmin = ''
  xmax = ''
  cd = ''
  fm = ''
  fp = ''
  fv = ''
  ps = ''
  fs = ''

class VowelMeasurement:
  phone = ''
  stress = ''
  word = ''
  f1 = ''
  f2 = ''
  f3 = ''
  b1 = ''
  b2 = ''
  b3 = ''
  t = ''
  code = ''
  cd = ''
  fm = ''
  fp = ''
  fv = ''
  ps = ''
  fs = ''
  text = ''
  beg = ''
  end = ''
  dur = ''

# takes a Praat TextGrid file and returns a list of the words in the file,
# along with their associated phones
def getWordsAndPhones(tg, phoneset):
#  print "\nGenerating a list of all of the words and their associated phones from the TextGrid file"
  words = []
  # initialize the counter for iterating through the phone intervals
  i = 0
  for w in tg[1]:
#    print w.mark(), w.xmin(), w.xmax()
    word = Word()
    word.transcription = w.mark()
    word.xmin = w.xmin()
    word.xmax = w.xmax()
    word.phones = []
#    print "tg[0][%d].xmax()" % i, eval(repr(tg[0][i].xmax())), "word.max", eval(repr(word.xmax))
    while eval(repr(tg[0][i].xmax())) <= eval(repr(word.xmax)):
#      print i
#      print "tg[0][%d].xmax()" % i, eval(repr(tg[0][i].xmax())), "word.max", eval(repr(word.xmax))
      phone = Phone()
      phone.label = tg[0][i].mark()
      phone.xmin = tg[0][i].xmin()
      phone.xmax = tg[0][i].xmax()
      word.phones.append(phone)
      i += 1
      # exit if we've reached the end of the phone interval
      if i == len(tg[0]):
        break
#    print word.transcription, word.xmin, word.xmax
#    for p in word.phones:
#      print p.label, p.xmin, p.xmax
#    print ''
    words.append(word)
#  print "\nAdding Plotnik-style codes for the preceding and following segments for all vowels"
  words = addPlotnikCodes(words, phoneset)
  return words

def addPlotnikCodes(words, phoneset):
  for w in words:

#    print w.transcription
#    for p in w.phones:
#      print p.label, p.xmin, p.xmax
#    print ''
   
    n = getNumVowels(w)
    if n == 0:
      continue

    for i in range(len(w.phones)):
      code = plotnik.cmu2plotnik_code(i, w.phones, w.transcription, phoneset)
      if code:
        w.phones[i].code = code
        w.phones[i].cd = code.split('.')[0]
        w.phones[i].fm = code.split('.')[1][0]
        w.phones[i].fp = code.split('.')[1][1]
        w.phones[i].fv = code.split('.')[1][2]
        w.phones[i].ps = code.split('.')[1][3]
        w.phones[i].fs = code.split('.')[1][4]

  return words

def getNumVowels(word):
  n = 0
  for p in word.phones:
    if isVowel(p.label):
      n += 1
  return n

def isVowel(label):
  # all vowel phone labels will end in either '0', '1', or '2'
  if label[-1] in ['0', '1', '2']:
    return True
  else:
    return False

def hasPrimaryStress(label):
  if label[-1] == '1':
    return True
  else:
    return False

def checkTiers(tg):
  if len(tg) != 2:
    print "ERROR:  input TextGrid file must contain exactly two tiers"
    sys.exit()
  if tg[0].name() != 'phone':
    print "ERROR:  first tier of input TextGrid file must be 'phone'"
    sys.exit()
  if tg[1].name() != 'word':
    print "ERROR:  second tier of input TextGrid file must be 'word'"
    sys.exit()

# remove from the list of formants those values taken earlier or later than the portion in time corresponding to the vowel
def trimFormants(formants, times, min, max):
#  print times
#  print min, max
  trimmedFormants = []
  trimmedTimes = []
  for i in range(0, len(formants)):
    if times[i] >= min and times[i] <= max:
      trimmedFormants.append(formants[i])
      trimmedTimes.append(times[i])
  return trimmedFormants, trimmedTimes

def lennig(formants, times):
  # initialize this to a number that will be larger than any of the change coefficients
  prev = 1000000
  min_i = -1

  for i in range(1,len(formants) - 1):
    c = (abs(formants[i][0] - formants[i-1][0]) + abs(formants[i][0] - formants[i+1][0])) / formants[i][0] + (abs(formants[i][1] - formants[i-1][1]) + abs(formants[i][1] - formants[i+1][1])) / formants[i][1]
    if c < prev:
      min_i = i
      prev = c
  measurementPoint = times[i]
  return measurementPoint

def anae(v, formants, times):
  F1 = [f[0] for f in formants]
  F2 = [f[1] for f in formants]
  
  if v == 'AE':
    i = F2.index(max(F2))
  elif v == 'AO':
    i = F2.index(min(F2))
  else:
    i = F1.index(max(F1))

  measurementPoint = times[i]
  return measurementPoint

# estimate the transition time from the onset C and to the offset C to be 20msec; if the vowel is shorter than 40msec, say that there are no transition periods
def getTransitionLength(min, max):
  if max - min <= 0.04:
    transition = 0
  else:
    transition = 0.02

  return transition

def getMeasurementPoint(phone, formants, times, measurementPointMethod):
  # measure at 1/3 of the way into the vowel's duration
  if measurementPointMethod == 'third':
    measurementPoint = phone.xmin + (phone.xmax - phone.xmin) / 3
  # measure at 1/4 of the way into the vowel's duration
  elif measurementPointMethod == 'fourth':
    measurementPoint = phone.xmin + (phone.xmax - phone.xmin) / 4
  # measure at 1/2 of the way into the vowel's duration
  elif measurementPointMethod == 'mid':
    measurementPoint = phone.xmin + (phone.xmax - phone.xmin) / 2
  elif measurementPointMethod == 'lennig':
    transition = getTransitionLength(phone.xmin, phone.xmax)
    trimmedFormants, trimmedTimes = trimFormants(formants, times, phone.xmin + transition, phone.xmax - transition)
    measurementPoint = lennig(trimmedFormants, trimmedTimes)
  elif measurementPointMethod == 'anae':
    transition = getTransitionLength(phone.xmin, phone.xmax)
    trimmedFormants, trimmedTimes = trimFormants(formants, times, phone.xmin + transition, phone.xmax - transition)
    measurementPoint = anae(phone.label, trimmedFormants, trimmedTimes)
  else:
    print "ERROR: Unsupported measurement point selection method %s" % measurementPointMethod
    print __doc__
  return measurementPoint

# get the index of the nearest time value from an ordered list of times
def getTimeIndex(t, times):
  # the two following cases can happen if a short vowel is at the beginning or end of a file
  if t < times[0]:
    print "WARNING:  measurement point %f is less than earliest time stamp %f for formant measurements, selecting earliest point as measurement" % (t, times[0])
    # return the index of the first measurement
    return 0

  if t > times[-1]:
    print "WARNING:  measurement point %f is less than latest time stamp %f for formant measurements, selecting latest point as measurement" % (t, times[-1])
    # return the index of the last measurement
    return len(times) - 1

  prev_time = 0.0
  for i in range(len(times)):
    if t > times[i]:
      prev_time = times[i]
      continue
    else:
      if abs(t - prev_time) > abs(t - times[i]):
        return i
      else:
        return i - 1

def predictF1F2(phone, poles, bandwidths, means, covs):
  nPoles = len(poles)
  vowel = phone.cd

  # for debugging
#  print poles
#  print bandwidths
#  print vowel
#  print means[vowel]
#  print covs[vowel]

  values = []
  distances = []
  for i in range(nPoles - 1):
    for j in range(i+1, nPoles):
      x = robjects.FloatVector([poles[i], poles[j], math.log(bandwidths[i]), math.log(bandwidths[j])])
      dist = robjects.r['mahalanobis'](x, means[vowel], covs[vowel])[0]
      values.append([x[0], x[1], x[2], x[3]])
      distances.append(dist)
  winnerIndex = distances.index(min(distances))
  #print values
  #print distances
  #print winnerIndex
  f1 = values[winnerIndex][0]
  f2 = values[winnerIndex][1]
  b1 = math.exp(values[winnerIndex][2])
  b2 = math.exp(values[winnerIndex][3])
  return (f1, f2, b1, b2)

def measureVowel(phone, word, poles, bandwidths, times, measurementPointMethod, formantPredictionMethod, padBeg, padEnd, means, covs):
#  print word.xmin, word.xmax
#  print phone.xmin, phone.xmax
#  print fmt.times()[0], fmt.times()[-1]
#  print fmt.times()
  measurementPoint = getMeasurementPoint(phone, poles, times, measurementPointMethod)
#  print measurementPoint, measurementPoint - phone.xmin
#  i = getTimeIndex(measurementPoint - phone.xmin + padBeg, fmt.times())
  i = getTimeIndex(measurementPoint, times)
#  print i
  if formantPredictionMethod == 'mahalanobis':
    # predict F1 and F2 based on the LPC values at this point in time
    poles = poles[i]
    bandwidths = bandwidths[i]
    f1, f2, b1, b2 = predictF1F2(phone, poles, bandwidths, means, covs)
    f3 = ''
    b3 = ''
  else:
    f1 = poles[i][0]
    f2 = poles[i][1]
    f3 = poles[i][2]
    b1 = bandwidths[i][0]
    b2 = bandwidths[i][1]
    b3 = bandwidths[i][2]

  vm = VowelMeasurement()
  vm.phone = phone.label[:-1]
  vm.stress = phone.label[-1]
  vm.word = word.transcription

  vm.f1 = round(f1, 1)
  vm.f2 = round(f2, 1)
  if f3 != '':
    vm.f3 = round(f3, 1)
  vm.b1 = round(b1, 1)
  vm.b2 = round(b2, 1)
  if b3 != '':
    vm.b3 = round(b3, 1)

  vm.t = round(measurementPoint, 3)
  vm.code = phone.code
  vm.cd = phone.cd
  vm.fm = phone.fm
  vm.fp = phone.fp
  vm.fv = phone.fv
  vm.ps = phone.ps
  vm.fs = phone.fs
  vm.beg = round(phone.xmin, 3)
  vm.end = round(phone.xmax, 3)
  vm.dur = round(phone.xmax - phone.xmin, 3)
  return vm

def loadMeans(inFile):
  means = {}
  for line in open(inFile, 'r').readlines():
    line = line.rstrip('\n')
    vowel = line.split('\t')[0]
    means[vowel] = robjects.FloatVector(line.split('\t')[1:])
  return means

def loadCovs(inFile):
  covs = {}
  for line in open(inFile, 'r').readlines():
    line = line.rstrip('\n')
    vowel = line.split('\t')[0]
    values = robjects.FloatVector(line.split('\t')[1:])
    covs[vowel] = robjects.r['matrix'](values, nrow=4)
  return covs

def checkLocation(file):
  if not os.path.exists(file):
    print "ERROR:  Could not locate %s" % file
    sys.exit()

def checkTextGridFile(tgFile):
  checkLocation(tgFile)
  lines = open(tgFile, 'r').readlines()
  if 'File type = "' not in lines[0]:
    print "ERROR:  %s does not appear to be a Praat TextGrid file (the string 'File type=' does not appear in the first line" % tgFile
    sys.exit()

def checkWavFile(wavFile):
  checkLocation(wavFile)

# process the three files which contain lists of filenames, one filename per line
def processInput(wavInput, tgInput, output):
  # remove the trailing newline character from each line of the file, and store the filenames in a list
  wavFiles = [f.replace('\n', '') for f in open(wavInput, 'r').readlines()]
  tgFiles = [f.replace('\n', '') for f in open(tgInput, 'r').readlines()]
  outputFiles = [f.replace('\n', '') for f in open(output, 'r').readlines()]
  return (wavFiles, tgFiles, outputFiles)

def programExists(program):
  p = os.popen('which ' + program)
  if p.readlines() == []:
    return False
  else:
    return True

def outputMeasurements(outputFormat, measurements, outputFile, outputHeader):
  if outputFormat in ['txt', 'text']:
    fw = open(outputFile, 'w')

    if outputHeader == "T":
      # print out the header line
      fw.write('\t'.join(['v', 'str', 'wd', 'f1', 'f2', 'f3', 'b1', 'b2', 'b3', 't', 'beg', 'end', 'dur', 'cd', 'fm', 'fp', 'fv', 'ps', 'fs']))
      fw.write('\n')

    for vm in measurements:
      fw.write('\t'.join([vm.phone, str(vm.stress), vm.word, str(vm.f1), str(vm.f2), str(vm.f3), str(vm.b1), str(vm.b2), str(vm.b3), str(vm.t), str(vm.beg), str(vm.end), str(vm.dur), vm.cd, vm.fm, vm.fp, vm.fv, vm.ps, vm.fs]))
      fw.write('\n')
    fw.close()
  elif outputFormat in ['plotnik', 'Plotnik', 'plt']:
    plt = plotnik.PltFile()
    for vm in measurements:
      plt.measurements.append(vm)
    plt.N = len(plt.measurements)
    plotnik.outputPlotnikFile(plt, outputFile)
  else:
    print "ERROR: Unsupported output format %s" % outputFormat
    print __doc__
    sys.exit(0)

def setDefaultOptions():
  options = {}
  options['case'] = 'upper'
  options['outputFormat'] = 'text'
  options['outputHeader'] = 'T'
  options['formantPredictionMethod'] = 'default'
  options['measurementPointMethod'] = 'third'
  options['speechSoftware'] = 'praat'
  options['nFormants'] = 5
  options['maxFormant'] = 5000
  options['removeStopWords'] = 'F'
  options['measureUnstressed'] = 'T'
  options['minVowelDuration'] = 0.05
  options['windowSize'] = 0.025
  options['preEmphasis'] = 50
  options['multipleFiles'] = 'F'
  options['stopWords'] = ["AND", "BUT", "FOR", "HE", "HE'S", "HUH", "I", "I'LL", "I'M", "IS", "IT", "IT'S", "ITS", "MY", "OF", "OH", "SHE", "SHE'S", "THAT", "THE", "THEM", "THEN", "THERE", "THEY", "THIS", "UH", "UM", "UP", "WAS", "WE", "WERE", "WHAT", "YOU"]
  return options

def checkConfigLine(f, line):
  if '=' not in line:
    print "ERROR:  malformed line in config file %s" % f
    print line
    sys.exit()

def checkConfigOption(f, option):
  allowedOptions = ['case', 'outputFormat', 'outputHeader', 'formantPredictionMethod', 'measurementPointMethod', 'speechSoftware', 'nFormants', 'maxFormant', 'removeStopWords', 'measureUnstressed', 'minVowelDuration', 'windowSize', 'preEmphasis', 'multipleFiles']
  if option not in allowedOptions:
    print "ERROR:  unrecognized option '%s' in config file %s" % (option, f)
    print "The following options are recognized:  ", ", ".join(allowedOptions)
    sys.exit()

def checkAllowedValues(f, option, value, allowedValues):
  if value not in allowedValues:
    print "ERROR:  unrecognized value '%s' for option '%s' in config file %s" % (value, option, f)
    print "The following values are recognized for option '%s'" % option, ", ".join(allowedValues)
    sys.exit()

# need to add checks also for options that take numeric values...
def checkConfigValue(f, option, value):
  if option == 'case':
    allowedValues = ['lower', 'upper']
    checkAllowedValues(f, option, value, allowedValues)
  if option == 'outputFormat':
    allowedValues = ['txt', 'text', 'plotnik', 'Plotnik']
    checkAllowedValues(f, option, value, allowedValues)
  if option == 'formantPredictionMethod':
    allowedValues = ['default', 'mahalanobis']
    checkAllowedValues(f, option, value, allowedValues)
  if option == 'measurementPointMethod':
    allowedValues = ['fourth', 'third', 'mid', 'lennig', 'anae']
    checkAllowedValues(f, option, value, allowedValues)
  if option == 'speechSoftware':
    allowedValues = ['praat', 'Praat', 'esps', 'ESPS']
    checkAllowedValues(f, option, value, allowedValues)
  if option in ['removeStopWords', 'measureUnstressed', 'outputHeader', 'multipleFiles']:
    allowedValues = ['T', 'F']
    checkAllowedValues(f, option, value, allowedValues)

def parseConfig(options, f):
  for line in open(f, 'rU').readlines():
    checkConfigLine(f, line)

    option = line.split('=')[0].strip()
    checkConfigOption(f, option)

    value = line.split('=')[1].strip()
    checkConfigValue(f, option, value)

    options[option] = value
  return options

def parseStopWordsFile(f):
  stopWords = []
  for line in open(f, 'r').readlines():
    word = line.rstrip('\n')
    stopWords.append(word)
  return stopWords

def changeCase(word, case):
  if case == 'lower':
    w = word.lower()
  # assume 'upper' here
  else:
    w = word.upper()
  return w

def getSoundEditor():
  # use sox for manipulating the files if we have it, since it's faster
  if programExists('sox'):
    soundEditor = 'sox'
  elif programExists('praat'):
    soundEditor = 'praat'
  else:
    print "ERROR:  neither 'praat' nor 'sox' can be found in your path"
    print "One of these two programs must be available for processing the audio file"
    sys.exit()
  return soundEditor

def extractPortion(wavFile, vowelWavFile, beg, end, soundEditor):
  if soundEditor == 'sox':
    os.system('sox ' + wavFile + ' ' + vowelWavFile + ' trim ' + str(beg) + ' ' + str(end - beg))
  elif soundEditor == 'praat':
    os.system('praat ' + SCRIPTS_HOME + '/extractSegment.praat ' + wavFile  + ' ' + vowelWavFile + ' ' + str(beg) + ' ' + str(end))
  else:
    pass

def checkSpeechSoftware(speechSoftware):
  if speechSoftware in ['ESPS', 'esps']:
    if not programExists('formant'):
      print "ERROR:  ESPS was specified as the speech analysis program, but the command 'formant' is not in your path"
      sys.exit()
    else:
      return 'esps'
  elif speechSoftware in ['praat', 'Praat']:
    if not programExists('praat'):
      print "ERROR:  Praat was specified as the speech analysis program, but the command 'praat' is not in your path"
    else:
      return 'praat'
  else:
    print "ERROR:  unsupported speech analysis software %s" % speechSoftware
    sys.exit()

def convertTimes(times, offset):
  convertedTimes = [t + offset for t in times]
  return convertedTimes

def getVowelMeasurement(vowelFileStem, p, w, speechSoftware, formantPredictionMethod, measurementPointMethod, nFormants, maxFormant, windowSize, preEmphasis, padBeg, padEnd):
  vowelWavFile = vowelFileStem + '.wav'

  if speechSoftware == 'esps':
    esps.runFormant(vowelWavFile)
    if formantPredictionMethod == 'mahalanobis':
      lpc = esps.LPC()
      lpc.read(vowelFileStem + '.pole')
    else:
      fmt = esps.Formant()
      fmt.read(vowelFileStem + '.pole', vowelFileStem + '.fb')

    # clean up the temporary files we created for this vowel
    esps.rmFormantFiles(vowelFileStem)

  # assume praat here
  else:
    if formantPredictionMethod == 'mahalanobis':
      os.system('praat ' + SCRIPTS_HOME + '/extractFormants.praat ' + vowelWavFile + ' 6 5000 0.02 50 all')
      lpc = praat.LPC()
      lpc.read(vowelFileStem + '.Formant')
    else:
      os.system('praat ' + SCRIPTS_HOME + '/extractFormants.praat ' + vowelWavFile + ' ' + str(nFormants) + ' ' + str(maxFormant) + ' ' + str(windowSize) + ' ' + str(preEmphasis) + ' burg')
      fmt = praat.Formant()
      fmt.read(vowelFileStem + '.Formant')

    os.remove(vowelFileStem + '.Formant')

  if formantPredictionMethod == 'mahalanobis':
    convertedTimes = convertTimes(lpc.times(), p.xmin - padBeg)
    poles = lpc.poles()
    bandwidths = lpc.bandwidths()
    vm = measureVowel(p, w, poles, bandwidths, convertedTimes, measurementPointMethod, formantPredictionMethod, padBeg, padEnd, means, covs)
  # assume 'default' here
  else:
    convertedTimes = convertTimes(fmt.times(), p.xmin - padBeg)
    formants = fmt.formants()
    bandwidths = fmt.bandwidths()
    vm = measureVowel(p, w, formants, bandwidths, convertedTimes, measurementPointMethod, formantPredictionMethod, padBeg, padEnd, '', '')
  
  os.remove(vowelWavFile)
  return vm

# if the phone is at the beginning of the sound file, we need to make sure that the added window will not extend past the beginning of the file, since this will mess up extractPortion(); if it does, truncate the added window to the available space
def getPadding(phone, windowSize, maxTime):
  if phone.xmin - windowSize < 0:
    padBeg = phone.xmin
  else:
     padBeg = windowSize

  if phone.xmax + windowSize > maxTime:
    padEnd = maxTime - phone.xmax
  else:
    padEnd = windowSize

  return (padBeg, padEnd)

if __name__ == '__main__':
  try:
    opts, args = getopt.getopt(sys.argv[1:], '', ["means=", "covariances=", "phoneset=", "outputFormat=", "config=", "stopWords="])
    wavInput, tgInput, output = args
  except:
    (type, value, traceback) = sys.exc_info()
    print value
    print __doc__
    sys.exit(0)

  # by default, assume that these files are located in the current directory
  meansFile = 'means.txt'
  covsFile = 'covs.txt'
  phonesetFile = 'cmu_phoneset.txt'
  configFile = ''
  stopWordsFile = ''

  for o, a in opts:
    if o == "--means":
      meansFile = a
    elif o == "--covariances":
      covsFile = a
    elif o == "--phoneset":
      phonesetFile = a
    elif o == "--outputFormat":
      outputFormat = a
    elif o == "--config":
      configFile = a
    elif o == "--stopWords":
      stopWordsFile = a
    else:
      print "ERROR:  unrecognized option %s" % o
      print __doc__
      sys.exit(0)

  # set the default options that will be used if no config file is specified
  options = setDefaultOptions()

  # if the user specifies a config file, get the values for the options contained in it
  if configFile != '':
    options = parseConfig(options, configFile)

  if stopWordsFile != '':
    stopWords = parseStopWordsFile(stopWordsFile)
  else:
    stopWords = options['stopWords']

  # assign the options to individual variables and to type conversion if necessary
  case = options['case']
  outputFormat = options['outputFormat']
  outputHeader = options['outputHeader']
  formantPredictionMethod = options['formantPredictionMethod']
  measurementPointMethod = options['measurementPointMethod']
  speechSoftware = options['speechSoftware']
  nFormants = int(options['nFormants'])
  maxFormant = int(options['maxFormant'])
  removeStopWords = options['removeStopWords']
  measureUnstressed = options['measureUnstressed']
  minVowelDuration = float(options['minVowelDuration'])
  windowSize = float(options['windowSize'])
  preEmphasis = float(options['preEmphasis'])
  multipleFiles = options['multipleFiles']

  phoneset = cmu.read_phoneset(phonesetFile)

  # make sure the specified speech analysis program is in our path
  speechSoftware = checkSpeechSoftware(speechSoftware)

  # determine what program we'll use to extract portions of the audio file
  soundEditor = getSoundEditor()

  # if we're using the Mahalanobis distance metric for vowel formant prediction, we need to load files with the mean and covariance values
  if formantPredictionMethod == 'mahalanobis':
    means = loadMeans(meansFile)
    covs = loadCovs(covsFile)

  # put the list of stop words in upper or lower case to match the word transcriptions
  newStopWords = []
  for w in stopWords:
    w = changeCase(w, case)
    newStopWords.append(w)
  stopWords = newStopWords

  if multipleFiles == 'T':
    wavFiles, tgFiles, outputFiles = processInput(wavInput, tgInput, output)
  else:
    wavFiles = [wavInput]
    tgFiles = [tgInput]
    outputFiles = [output]

  for (wavFile, tgFile, outputFile) in zip(wavFiles, tgFiles, outputFiles):
    # make sure that we can find the input files, and that the TextGrid file is formatted properly
    checkWavFile(wavFile)
    checkTextGridFile(tgFile)
  
    # this will be used for the temporary files that we write
    fileStem = wavFile.replace('.wav', '')
  
    # load the information from the TextGrid file with the word and phone alignments
    tg = praat.TextGrid()
    tg.read(tgFile)
    checkTiers(tg)
    words = getWordsAndPhones(tg, phoneset)
    maxTime = tg.xmax()
    measurements = []
  
    for w in words:
      # convert to upper or lower case, if necessary
      w.transcription = changeCase(w.transcription, case)
  
      # don't process this word if it's in the list of stop words
      if removeStopWords == 'T' and w.transcription in stopWords:
        continue
  
      #print w.transcription, w.xmin, w.xmax
  
      # if the word doesn't contain any vowels, then we won't analyze it
      numVowels = getNumVowels(w)
      if numVowels == 0:
        continue
  
      for p in w.phones:
        # skip this phone if it's not a vowel
        if not isVowel(p.label):
          continue
  
        # skip this vowel if it doesn't have primary stress and the user only wants to measure stressed vowels
        if measureUnstressed == 'F' and not hasPrimaryStress(p.label):
          continue
  
        dur = p.xmax - p.xmin
  
        # don't measure this vowel if it's shorter than the minimum length threshold (this avoids an ESPS error due to there not being enough samples for the LPC, and it leaves out vowels that are reduced)
        if dur < minVowelDuration:
          continue
  
        vowelFileStem = fileStem + '_' + p.label
        vowelWavFile = vowelFileStem + '.wav'
  
        print ''
        print "Extracting formants for vowel %s in word %s" % (p.label, w.transcription)
  
        padBeg, padEnd = getPadding(p, windowSize, maxTime)
  
        extractPortion(wavFile, vowelWavFile, p.xmin - padBeg, p.xmax + padEnd, soundEditor)
  
        vm = getVowelMeasurement(vowelFileStem, p, w, speechSoftware, formantPredictionMethod, measurementPointMethod, nFormants, maxFormant, windowSize, preEmphasis, padBeg, padEnd)
        measurements.append(vm)
  
    # don't output anything if we didn't take any measurements (this prevents the creation of empty output files)
    if len(measurements) > 0:
      outputMeasurements(outputFormat, measurements, outputFile, outputHeader)
      print "\nVowel measurements output in %s format to the file %s" % (outputFormat, outputFile)
