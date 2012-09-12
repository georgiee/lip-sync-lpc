#!/usr/bin/env python
# encoding: utf-8
"""
phonemeAnalysis.py

Created by Zach Archer on 2009-08-20.
Copyright (c) 2009 __MyCompanyName__. All rights reserved.
"""

import psyco
psyco.full()

import sys
import getopt
import math
import re
import pp
import os
from array import array

import numpy
import lpc

#framesPerSecond = 30
originalSampleRate = 44100
#frameWidth = sampleRate / framesPerSecond
#windowMult = 2.0	# when using a window, set this to 2.0, so the window regions will overlap
#window = numpy.blackman(frameWidth*windowMult)	# set this to [] for no windowing

help_message = '''
Usage: analyze_lpc.py [INPUT_FILE]
'''


class Usage(Exception):
	def __init__(self, msg):
		self.msg = msg


def main(argv=None):
	if argv is None:
		argv = sys.argv
	try:
		try:
			opts, args = getopt.getopt(argv[1:], "hx:s:f:n:o:u:w:p", ["help","maxPitch=","sampleRate=","framesPerSecond=","noiseLevel=","order=","unvoicedOrder=","window=","phonemes"])
		except getopt.error, msg:
			raise Usage(msg)

		# I need at least one argument -- the file name to analyze
		if( len(args) == 0 ):
			raise Usage(help_message)
	
		# option processing
		params = {'sampleRate':8000.0, 'order':10, 'unvoicedOrder':4, 'framesPerSecond':40.0, 'maxPitch':2000.0, 'noiseLevel':0.2, 'window':'', 'phonemes':False}	# defaults
		for option, value in opts:
			#if option == "-v":
			#	verbose = True
			if option in ("-h", "--help", "-?"):
				raise Usage(help_message)
			#if option in ("-o", "--output"):
			#	output = value
			#if option in ("-lp", "--lowpitch"):
			#	params['lowpitch'] = float(value)
			if option in ("-x", "--maxPitch"):
				params['maxPitch'] = float(value)
			if option in ("-s", "--sampleRate"):
				params['sampleRate'] = float(value)
			if option in ("-f", "--framesPerSecond"):
				params['framesPerSecond'] = float(value)
			if option in ("-n", "--noiseLevel"):
				params['noiseLevel'] = float(value)
			if option in ("-o", "--order"):
				params['unvoicedOrder'] = params['order'] = int(value)
			if option in ("-u", "--unvoicedOrder"):
				params['unvoicedOrder'] = int(value)
			if option in ("-w", "--window"):
				params['window'] = str(value)
			if option in ("-p", "--phonemes"):
				params['phonemes'] = True
	
	except Usage, err:
		print >> sys.stderr, sys.argv[0].split("/")[-1] + ": " + str(err.msg)
		print >> sys.stderr, "\t for help use --help"
		return 2
		
	
	print "Starting..."

	# Various pre-processing is needed first
	# How wide is each frame?
	frameWidth = params['sampleRate'] / params['framesPerSecond']
	windowWidth = frameWidth	
	if( params['window'] ):
		windowWidth *= 2	# sampling windows should overlap
		if( params['window'] in ("bartlett","blackman","hamming","hanning","kaiser") ):
			fn = getattr( numpy, params['window'] )
			params['windowData'] = fn( windowWidth );
		else:
			raise Exception("--window argument must be one of the following: bartlett,blackman,hamming,hanning,kaiser")
	params['frameWidth'] = frameWidth
	params['windowWidth'] = windowWidth
	params['originalSampleRate'] = originalSampleRate
	
	# Load the sound file
	fileName = args[0]
	# How large is this sound file?
	fileSize = os.path.getsize(fileName)
	print "File size is", fileSize, ", frame width is", frameWidth, ", window width is", windowWidth
	print "Opening file", fileName, "..."
	FILE = open(fileName,"r")
	
	# Change the filename extension, generate .lpc data
	outFileName = fileName
	outFileName = re.sub("\.dat", ".lpc", outFileName)
	outFileName = re.sub("\.raw", ".lpc", outFileName)
	if( fileName == outFileName ):
		raise AssertionError("fileName is the same as outFileName:", fileName, outFileName)
	
	# tuple of all parallel python servers to connect with
	ppservers = ()
	# Creates jobserver with automatically detected number of workers
	job_server = pp.Server(ppservers=ppservers)
	print "Starting pp with", job_server.get_ncpus(), "workers..."

	#
	# Create the packets to crunch
	#
	print "Building jobs..."
	jobs = []
	origScale = params['originalSampleRate'] / params['sampleRate']
	for p in range(0, int(fileSize/(frameWidth*origScale*2)) ):
		seekPoint = math.floor( p*frameWidth*2*origScale )
		if( seekPoint % 2 ): 
			seekPoint += 1
		FILE.seek( seekPoint )
		bytes = FILE.read( int(math.ceil(windowWidth*2*origScale)) )	# Gather enough bytes for a windowing function with overlap
		jobs.append( job_server.submit( analyzePackets, (bytes,params), (), ('math','lpc','numpy','array') ) )

	# Prepare to save
	OUT = open(outFileName,"w")
	# order is a 16-bit int
	shortAr = array( 'h', [params['order']] )
	shortAr.tofile( OUT )
	floatAr = array( 'f', [params['sampleRate'],params['framesPerSecond']] )
	floatAr.tofile( OUT )

	# 
	# COLLECT THE OUTPUT
	#
	frames = []
	codes = "AA AE AH AO AW AY EH EY IH IY OW OY UH UW B CH D DH ER F G HH JH K L M N NG P R S SH T TH V W Y Z ZH"
	phonemes = codes.split(' ')
	markers = {}
	
	if( params['phonemes'] ):
		PHON_FILE = open("phonemeMarkers.txt","w")
	
	silentFrames = 999	# Keep track of silent frames between phoneme sounds
	for job in (jobs):
		frame = job()
		
		saveFrame = True	# Save the frame, unless we have some reason to ignore it
		
		# Phoneme separation: We may elect to not save a frame
		if( params['phonemes'] ):
			if( frame.power < 0.1 ):
				# This is a silent frame between phonemes, so prepare to ignore it.
				silentFrames += 1
				if( silentFrames >= 3 ):
					# After 3 silent frames, it must be a silent section between phonemes. Don't save the frame!
					saveFrame = False
			else:
				# If we have just ended a run of silent frames, then save a marker for the starting frame of the phoneme. 
				if( silentFrames >= 3 ):
					if( len(phonemes) == 0 ):
						print "** Uh oh... A new phoneme is starting, but I can't shift anything off the list."
						jobs = []
						break
					else:
						phon = phonemes.pop(0)
						markers[phon] = len(frames)
						print "* PHONEME", phon, len(frames)
						PHON_FILE.write( phon + "\t" + str(len(frames)) + "\n" )
				
				silentFrames = 0
			
			
		if( saveFrame ):
			print( str(len(frames))+":", frame.pitch, frame.power, frame.unvoiced )
			frames.append( frame )
		
			#
			# WRITE THE FRAME ANALYSIS
			#
			# coef's, etc are 32-bit floats
			floats = [frame.power, frame.unvoiced, frame.pitch, frame.err]
			floats.extend( frame.coefs )
			floatAr = array( 'f', floats )
			floatAr.tofile( OUT )
			
	OUT.close()
	if( params['phonemes'] ):
		PHON_FILE.write( "END\t" + str(len(frames)) + "\n" )
		PHON_FILE.close()
	
	print("Done!!!")


def analyzePackets( bytes, params ):
	frame = lpc.Frame()
	frame.analyzeBytes( bytes, params )
	return frame
	

if __name__ == "__main__":
	sys.exit(main())
