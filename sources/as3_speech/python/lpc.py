#!/usr/bin/env python
# encoding: utf-8
"""
lpc.py

Created by Zach Archer on 2009-08-21.
Copyright (c) 2009 . All rights reserved.
"""

import sys
import os
import math
import numpy
from array import array 

class Frame:
	def __init__(self):
		self.power = 0
		self.unvoiced = 0
		self.err = 0
		self.pitch = 0
		self.coefs = array('f')

	def analyzeBytes(self,bytes,params):
		order = params['order']
		windowWidth = params['windowWidth']
		windowData = []
		if( params['window'] ):
			windowData = params['windowData']
		#order,minFrameSamples,window
		
		#
		# CONVERT BYTE STREAM TO 16-BIT SAMPLES
		#
		samps = array('f')
		origSR = params['originalSampleRate']
		alias = origSR / params['sampleRate']	# We must collect this many samples from the bytestream
		activeSample = 0.0
		activeAlias = 0.0	# We have collected this many samples in the activeSample
		intRange = int(min( windowWidth*alias, len(bytes)/2 ))
		for b in range(0,intRange):
			# Scrape a new sample off the bytestream
			sample = (ord(bytes[b*2]) * 256) + ord(bytes[b*2+1]);
			# samples are signed
			if( sample >= (128*256) ):
				sample -= (256*256)
			
			if( (activeAlias + 1.0) < alias ):
				# If this sample won't overflow the activeSample, just toss it in with the others
				activeSample += sample
				activeAlias += 1.0
			else:
				# This sample will overflow (or match) the permitted alias amount.
				activeSample += sample * (alias - activeAlias)
				# Apply the windowing function, if given
				if( len(windowData) ):
					activeSample *= windowData[ len(samps) ]
				samps.append( activeSample / alias )
				
				# Now set whatever's left over as the new active sample
				activeAlias = 1.0-(alias-activeAlias)
				activeSample = sample * activeAlias
		
		if( len(samps) < math.floor(windowWidth) ):
			#print "active samp is", activeSample, activeAlias
			samps.append( activeSample )
			
		if( len(samps) < math.floor(windowWidth) ):
			print("This frame is short, going to pad with zeroes...", len(samps), windowWidth )
			for q in range( len(samps), int(windowWidth)+2 ):
				samps.append( 0 )
			
		corr = array('f')
		self.autocorrelate(samps,corr,params)	# Sets the pitch & populates corr with the yVector
		if( self.unvoiced ):
			order = params['unvoicedOrder']
			
		# construct the R matrix
		r = numpy.zeros((order,order))
		for i in range( 1, order+1 ):
			for j in range( 1, order+1 ):
				r[i-1][j-1] = corr[ abs(int(i-j)) ]
		
		# invert to get the res matrix
		# why is this incredibly slow at 30x30??? yikes
		res = numpy.linalg.inv( r )
		
		# Find the coefficients A = P*R^(-1)
		for i in range( 0, order ):
			self.coefs.append( 0.0 )
			for j in range( 0, order ):
				self.coefs[i] += res[i][j] * corr[j+1]
		
		# Append any missing coefficients (unvoiced) with zeros
		if( self.unvoiced ):
			for i in range( params['unvoicedOrder'], params['order'] ):
				self.coefs.append( 0.0 )
		
		# Now compute the power
		zs = []
		for i in range( 0, order ):
			zs.append( samps[ order-i-1 ] )
		
		hopeSize = len(samps) - order
		
		# Find the MSE
		tempPower = 0.0
		for i in range( order, hopeSize + order ):
			temp = 0
			for j in range( 0, order ):
				temp += zs[j] * self.coefs[j]
			
			# Inefficient code, replaced by the equilavent of zs.unshift:
			#for j in range( order-1, 0, -1 ):
			#	zs[j] = zs[j-1]
			#zs[0] = samps[i]
			zs.insert( 0, samps[i] )	# unshift
			
			error = zs[0] - temp
			tempPower += error * error
		
		self.power = math.sqrt(tempPower) / hopeSize

		
	def autocorrelate(self,samps,yVector,params):
		for n in range( 0, len(samps) ):
			temp = 0
			for i in range( 0, len(samps)-n-1 ):
				temp += samps[i] * samps[i+n]
			yVector.append( temp )
			
		# set temp to the first element of y
		temp = yVector[0]
		#j = int(len(samps) * 0.02)	# j is now the maximum possible pitch (shortest distance)
		j = int(params['sampleRate']/params['maxPitch'])	# j is now the maximum possible pitch (shortest distance)
		# Find the pitch with the strongest energy. Loop to the point y stops decreasing
		while( j < len(samps) and yVector[j] < temp ):
			temp = yVector[j]
			j += 1
		j = min( j, len(samps)-1 )	# sanity check
		
		temp = 0
		# Find the max between j and the nyquist frequency
		for i in range( j, len(samps)/2 ):
			if( yVector[i] > temp ):
				j = i
				temp = yVector[i]
		
		# "why are we doing this?"   I often ask myself the same question.
		norm = 1.0 / len(samps)
		k = len(samps)
		
		# This is a de-emphasis function, reduces the brightness that was added
		# during correlation. (Short sample intervals have more sample comparisons / multiplication,
		# will have higher yVector amplitudes.)
		for i in range( 0, len(samps) ):
			yVector[i] *= (k-i) * norm
		
		if( yVector[0] != 0 and yVector[j] / yVector[0] < params['noiseLevel'] ):
			self.unvoiced = 1.0	# unvoiced
		
		"""
		# My voice can go this low :P
		if( j > len(samps) / 4 ):
			self.unvoiced = 1.0 # unvoiced
		"""
			
		# j is the pitch.
		# if j is negative, it is UNVOICED.
		self.pitch = j
		
		