# This script syllabifies the CMU Pronouncing Dictionary using
# the p2tk automated syllabifier.
#
# Example usage:
# python syllabify-cmudict.py < cmudict.0.6 > cmudict.0.6-syl
#
# by Josh Tauberer, 2008
#
# We read the CMU dictionary line by line from standard input,
# syllabify the line, and then print the result to standard output.

import sys
import syllabifier

language = syllabifier.English # syllabifier.loadLanguage("english.cfg")

print "## This is a syllabified version of the pronunciation dictionary"
print "## below made using the P2TK automated syllabifier. Periods"
print "## indicate syllable boundaries."
print "##"
print "##"

for line in sys.stdin :
	line = line.rstrip()

	if len(line) < 2 or line[0:2] == "##" :
		print line
		continue

	fields = line.split(" ")

	word = fields.pop(0)
	phonemes = fields
		
	print word + "  " + syllabifier.stringify(syllabifier.syllabify(language, phonemes))
