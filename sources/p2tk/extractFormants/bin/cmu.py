import re

class Phone:
  label = ''
  vc = ''
  vlng = ''
  vheight = ''
  vfront = ''
  vrnd = ''
  ctype = ''
  cplace = ''
  cvox = ''

# this reads the dictionary in order by line; thus, if a single word has multiple pronunciations, the one listed last is returned into the dictionary
def read_dict(f):
  lines = open(f, 'r').readlines()
  dict = {}
  pat = re.compile('  *')
  for line in lines:
    line = line.rstrip()
    line = re.sub(pat, ' ', line)
    word = line.split(' ')[0]
    phones = line.split(' ')[1:]
    if word not in dict:
      dict[word] = [phones]
    else:
      dict[word].append(phones)
  return dict

def read_phoneset(f):
  lines = open(f, 'r').readlines()
  phoneset = {}
  for line in lines[1:]:
    p = Phone()
    line = line.rstrip('\n')
    label = line.split()[0]
    p.label = label
    p.vc = line.split()[1]
    p.vlng = line.split()[2]
    p.vheight = line.split()[3]
    p.vfront = line.split()[4]
    p.vrnd = line.split()[5]
    p.ctype = line.split()[6]
    p.cplace = line.split()[7]
    p.cvox = line.split()[8]
    phoneset[label] = p
  return phoneset
