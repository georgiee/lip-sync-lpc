import sys
import string
import re

glide_regex = re.compile('{[a-z0-9]*}')
style_regex = re.compile('-[0-9]-')
comment_regex = re.compile('-- .*')
count_regex = re.compile('[0-9]$')
# primary stress, secondary stress, or unstressed
stress_regex = re.compile('[0-2]$')

A2P = {'AA':'5', 'AE':'3', 'AH':'6', 'AO':'53', 'AW':'42', 'AY':'41', 'EH':'2', 'ER':'94', 'EY':'21', 'IH':'1', 'IY':'11', 'OW':'62', 'OY':'61', 'UH':'7', 'UW':'72'}
A2P_FINAL = {'IY':'12', 'EY':'22', 'OW':'63'}
A2P_R = {'EH':'2', 'AE':'3', 'IH':'14', 'IY':'14', 'EY':'24', 'AA':'44', 'AO':'64', 'OW':'64', 'UH':'74', 'UW':'74', 'AH':'6', 'AW':'42', 'AY':'41', 'OY':'61'}
MANNER = {'s':'1', 'a':'2', 'f':'3', 'n':'4', 'l':'5', 'r':'6'}
PLACE = {'l':'1', 'a':'4', 'p':'5', 'b':'2', 'd':'3', 'v':'6'}
VOICE = {'-':'1', '+':'2'}

def arpabet2plotnik(ac, trans, prec_p, foll_p, phoneset):
#  print ac, trans, prec_p, foll_p
  if foll_p == '' and ac in ['IY', 'EY', 'OW']:
    pc = A2P_FINAL[ac]
  elif foll_p != '' and ac == 'AY' and phoneset[foll_p].cvox == '-':
    pc = '47'
  elif trans in ['FATHER', 'MA', 'PA', 'SPA', 'CHICAGO', 'PASTA', 'BRA', 'UTAH', 'TACO']:
    pc = '43'
  elif prec_p != '' and ac == 'UW' and phoneset[prec_p].cplace == 'a':
    pc = '73'
  elif foll_p != '' and phoneset[foll_p].ctype == 'r' and ac != 'ER':
    pc = A2P_R[ac]
  else:
    pc = A2P[ac]
  return pc

# this is a hack based on the fact that we know that the CMU transcriptions for vowels all indicate the level of stress in their final character (0, 1, or 2); will rewrite them later to be more portable...
def is_v(p):
  if p[-1] in ['0', '1', '2']:
    return True
  else:
    return False

def get_n_foll_syl(i, phones):
  n = 0
  for p in phones[i+1:]:
    if is_v(p.label):
      n += 1
  return n

def get_n_foll_c(i, phones):
  n = 0
  for p in phones[i+1:]:
    if is_v(p.label):
      break
    elif n == 1 and p.label in ['Y', 'W', 'R', 'L']:  # e.g. 'figure', 'Wrigley', etc.
      break
    else:
      n += 1
  return n

class PltFile:
  first_name = ''
  last_name = ''
  age = ''
  city = ''
  state = ''
  sex = ''
  ts = ''
  N = ''
  S = ''
  measurements = []

class VowelMeasurement:
  F1 = 0
  F2 = 0
  F3 = ''
  code = ''
  stress = 1
  text = ''
  word = ''
  trans = ''
  fname = ''
  comment = ''
  glide = ''
  style = ''
  t = 0

# input: Plotnik word as originally entered (with parentheses, token numbers, glide annotations, etc.)
# output: normal transcription
def word2trans(word):
  trans = word.replace('(', '')
  trans = trans.replace(')', '')
  # the glide annotation, if it exists is outside the count, so this must be done first
  trans = re.sub(glide_regex, '', trans)
  trans = re.sub(count_regex, '', trans)
  trans = str.upper(trans)
  return trans

def word2fname(word):
  fname = word.replace('(', '')
  fname = fname.replace(')', '')
  fname = fname.replace('-', '')
  fname = re.sub(glide_regex, '', fname)
  fname = str.upper(fname)
  if len(fname) > 8:
    last = fname[-1]
    if last in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']:
      fname = fname[0:7] + last
    else:
      fname = fname[0:8]
  return fname

# returns the index of the stressed vowel, or '' if 0 or more than one exist
def get_stressed_v(phones):
  primary_count = 0
  for p in phones:
    if p[-1] == '1':
      primary_count += 1
      i = phones.index(p)
  # if there is more than vowel with primary stress in the transcription, then we don't know which one to look at, so return ''
  if primary_count != 1:
    return ''
  else:
    return i

def cmu2plotnik_code(i, phones, trans, phoneset):
  if not is_v(phones[i].label):
    return None

  # if the vowel is the final phone in the list, then there is no following segment
  if i+1 == len(phones):
    foll_p = ''
    fm = '0'
    fp = '0'
    fv = '0'
    fs = '0'
  else:
    # get the following segment, and strip the stress code off if it's a vowel
    foll_p = re.sub(stress_regex, '', phones[i+1].label)
    ctype = phoneset[foll_p].ctype
    cplace = phoneset[foll_p].cplace
    cvox = phoneset[foll_p].cvox
    # convert from the CMU codes to the Plotnik codes
    fm = MANNER.get(ctype, '0')
    fp = PLACE.get(cplace, '0')
    fv = VOICE.get(cvox, '0')
    n_foll_syl = get_n_foll_syl(i, phones)
    n_foll_c = get_n_foll_c(i, phones)
    if n_foll_c <= 1 and n_foll_syl == 1:
      fs = '1'
    elif n_foll_c <= 1 and n_foll_syl >= 2:
      fs = '2'
    elif n_foll_c > 1 and n_foll_syl == 0:
      fs = '3'
    elif n_foll_c > 1 and n_foll_syl == 1:
      fs = '4'
    elif n_foll_c > 1 and n_foll_syl >= 2:
      fs = '5'
    else:
      fs = '0'

  # if the vowel is the first phone in the list, then there is no preceding segment
  if i == 0:
    prec_p = ''
    ps = '0'
  else:
    # get the preceding segment, and strip the stress code off if it's a vowel
    prec_p = re.sub(stress_regex, '', phones[i-1].label)
    if prec_p in ['B', 'P', 'V', 'F']:
      ps = '1'
    elif prec_p in ['M']:
      ps = '2'
    elif prec_p in ['D', 'T', 'Z', 'S', 'TH', 'DH']:
      ps = '3'
    elif prec_p in ['N']:
      ps = '4'
    elif prec_p in ['ZH', 'SH', 'JH', 'CH']:
      ps = '5'
    elif prec_p in ['G', 'K']:
      ps = '6'
    elif i > 1 and prec_p in ['L', 'R'] and phones[i-2] in ['B', 'D', 'G', 'P', 'T', 'K', 'V', 'F', 'Z', 'S', 'SH', 'TH']:
      ps = '8'
    elif prec_p in ['L', 'R', 'ER0', 'ER2', 'ER1']:
      ps = '7'
    elif prec_p in ['W', 'Y']:
      ps = '9'
    else:
      ps = '0'
    
  code = arpabet2plotnik(phones[i].label[:-1], trans, prec_p, foll_p, phoneset)
  code += '.'
  code += fm
  code += fp
  code += fv
  code += ps
  code += fs
  return code

def process_measurement_line(line):
  vm = VowelMeasurement()
  vm.F1 = float(line.split(',')[0])
  vm.F2 = float(line.split(',')[1])
  try:
    vm.F3 = float(line.split(',')[2])
  except ValueError:
    vm.F3 = ''
  vm.code = line.split(',')[3]
  vm.stress = line.split(',')[4]

  vm.text = line.split(',')[5]

  vm.word = vm.text.split()[0]
  vm.trans = word2trans(vm.word)
  vm.fname = word2fname(vm.word)

  res = re.findall(glide_regex, vm.text)
  if len(res) > 0:
    temp = res[0].replace('{', '')
    temp = temp.replace('}', '')
    vm.glide = temp

  res = re.findall(style_regex, vm.text)
  if len(res) > 0:
    temp = res[0].replace('-', '')
    temp = temp.replace('-', '')
    vm.style = temp

  res = re.findall(comment_regex, vm.text)
  if len(res) > 0:
    temp = res[0].replace('-- ', '')
    vm.comment = temp
    if temp == 'glide':
      vm.glide = 'g'
  else:
    res = style_regex.split(vm.text)
    if len(res) > 1:
      vm.comment = res[1].strip()

  return vm

def get_first_name(line):
  first_name = line.split(',')[0].split()[0]
  return first_name

def get_last_name(line):
  try:
    last_name = line.split(',')[0].split()[1]
  except IndexError:
    last_name = ''
  return last_name

def get_age(line):
  try:
    age = line.split(',')[1].strip()
  except IndexError:
    age = ''
  return age

def get_sex(line):
  try:
    sex = line.split(',')[2].strip()
  except IndexError:
    sex = ''
  # only some files have sex listed in the first line
  if sex not in ['m', 'f']:
    sex = ''
  return sex

def get_city(line):
  sex = get_sex(line)
  if sex in ['m', 'f']:
    try:
      city = line.split(',')[3].strip()
    except IndexError:
      city = ''
  else:
    try:
      city = line.split(',')[2].strip()
    except IndexError:
      city = ''
  return city

def get_state(line):
  sex = get_sex(line)
  if sex in ['m', 'f']:
    try:
      state = line.split(',')[4].strip().split()[0]
    except IndexError:
      state = ''
  else:
    try:
      state = line.split(',')[3].strip().split()[0]
    except IndexError:
      state = ''
  return state

def get_ts(line):
  if ' TS ' in line:
    ts = line.strip().split(' TS ')[1]
  elif ' ts ' in line:
    ts = line.strip().split(' ts ')[1]
  else:
    ts = ''
  return ts

def get_n(line):
  try:
    n = int(line.strip().split(',')[0])
  except IndexError:
    n = ''
  return n

def get_s(line):
  try:
    s = float(line.strip().split(',')[1])
  except IndexError:
    s = ''
  return s

def process_plt_file(filename):
  f = open(filename, 'rU')
  line = f.readline().strip()
  
  # skip initial blank lines
  while line == '':
    line = f.readline()
    # EOF was reached, so this file only contains blank lines
    if line == '':
      sys.exit()
    else:
      line = line.strip()

  Plt = PltFile()

  Plt.first_name = get_first_name(line)
  Plt.last_name = get_last_name(line)
  Plt.age = get_age(line)
  Plt.sex = get_sex(line)
  Plt.city = get_city(line)
  Plt.state = get_state(line)
  Plt.ts = get_ts(line)

  line = f.readline().strip()
  Plt.N = get_n(line)
  Plt.S = get_s(line)

#  print ','.join([ts, first_name, last_name, age, sex, city, state])
#  print ','.join([n, s])

  line = f.readline().strip()

  # skip any blank lines between header and formant measurements
  while line == '':
    line = f.readline()
    # this file only contains blank lines
    if line == '':
      sys.exit()
    else:
      line = line.strip()

  Plt.measurements = []

  # proceed until we reach the blank line separating the formant data from the means
  while line != '':
     # some files don't contain this blank line, so look to see if the first value in the line is '1'; if it is, this must be the beginning of the means list, and not an F1 measurement
    if line.split(',')[0] == '1':
      break
    vm = process_measurement_line(line)
    Plt.measurements.append(vm)
    line = f.readline().strip()

  if len(Plt.measurements) != Plt.N:
    print "ERROR:  N's do not match for %s" % filename
    return None
  else:
    return Plt

# unstressed vowels are labeled with '0' in the CMU pronouncing dictionary, but '3' in Plotnik
def convertStress(stress):
  if stress == '0':
    stress = '3'
  return stress

# Plotnik requires the duration to be represented in msec as an integer
def convertDur(dur):
  dur = int(round(dur * 1000))
  return dur

def outputPlotnikFile(Plt, f):
  pltFields = {'f1':0, 'f2':1, 'f3':2, 'code':3, 'stress':4, 'word':5}
  fw = open(f, 'w')
  if Plt.sex == '':
    fw.write(Plt.first_name+' '+Plt.last_name+', '+Plt.age+', '+Plt.city+', '+Plt.state+' '+Plt.ts)
  else:
    print Plt.first_name+' '+Plt.last_name+', '+Plt.age+', '+Plt.sex+','+Plt.city+', '+Plt.state+' '+Plt.ts
  fw.write('\n')
  fw.write(str(Plt.N)+','+str(Plt.S))
  fw.write('\n')
  for vm in Plt.measurements:
    stress = convertStress(vm.stress)
    dur = convertDur(vm.dur)
    fw.write(','.join([str(round(vm.f1, 1)), str(round(vm.f2, 1)), str(vm.f3), vm.code, stress + '.' + str(dur), vm.word + ' ' + str(vm.t)]))
    fw.write('\n')
