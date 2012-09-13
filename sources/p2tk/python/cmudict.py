# This module loads the CMU Pronouncing Dictionary,
# or any other dictionary in the same format, into
# memory and provides a function to convert a word
# into a list of its pronunciations in the dictionary.
#
# by Josh Tauberer, 2008
#
# Example usage:
#
# import p2tk.python.cmudict
# d = cmudict.loadDictionary("path/to/cmudict")
#     or
# d = cmudict.loadDictionary() # loads cmudict that is next to this module
# p = cmudict.lookup(d, "abolishionists")
# print p
# 
# It prints out a list of the three pronunciations
# in the dictionary. Each pronunciation is a list
# of phonemes, as they appear in the dictionary:
#
# [ ['AE2', 'B', 'AH0', 'L', 'IH1', 'SH', 'AH0', 'N', 'AH0', 'S', 'T', 'S'],
#   ['AE2', 'B', 'AH0', 'L', 'IH1', 'SH', 'AH0', 'N', 'AH0', 'S', 'S'],
#   ['AE2', 'B', 'AH0', 'L', 'IH1', 'SH', 'AH0', 'N', 'AH0', 'S'] ]

import os.path

failOnNotFound = True

def loadDictionary(filename = None) :
	# If the file name is not specified, use cmudict.0.6 in the directory
	# that contains this module.
	if filename== None :
		filename = os.path.dirname(__file__) + "/cmudict.0.6"
	
	dict =  { }

	f = file(filename, "r")

	for line in f :
		line = line.rstrip()

		if len(line) < 2 or line[0:2] == "##" :
			continue
	
		fields = line.split(" ")

		word = fields.pop(0)
		phonemes = filter(lambda x : x != "", fields)
		
		if word[-1] == ")" :
			if word[-3] != "(" :
				raise ValueError, "File format error: " + line
			# This is a multiple pronunciation. Hack off the (#).
			word = word[0:-3] 
			
		if not word in dict :
			dict[word] = []
		dict[word].append(phonemes)
		
	f.close()

	return dict

def lookup(dict, word) :
	word = word.upper()
	if not failOnNotFound and not word in dict :
		return None
	return dict[word]

if __name__ == "__main__" :
	d = loadDictionary("cmudict.0.6")
	print lookup(d, "abolitionists")
