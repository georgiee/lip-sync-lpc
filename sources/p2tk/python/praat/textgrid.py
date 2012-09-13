"""
grid.py
by Kyle Gorman (kgorman@ling.upenn.edu)
Python module for Praat TextGrid file manipulation

### TEXTGRID FUNCTIONS ####

Manipulate Praat TextGrids

(n.b. on TextGrid IO: Each tier is represented as a Python list. Each entry in 
the list, interval or point, is a tuple in the list, in proper temporal order 
If necessary, do numeric sort by the first value of the tuple. The last value 
in the tuple of each list entry is the string label associated with it. In the case of
a point tier, the first value of the list-entry tuple is the time of the point, expressed
as a float. In the case of a interval tier, the first and second
values of the list-entry tuple contain start and stop times as floats.)

EXAMPLE(S) ~ 

<code>
from p2tk.python.praat import textgrid
grid = textgrid.read('test.TextGrid') # read in a TextGrid

for tier in grid :
	for interval_start, interval_end, label in tier : # for intervals only
		print interval_start, interval_end, label

textgrid.write(grid, 'test2.TextGrid') # write that TextGrid to a new file
</code>
"""

from os import path,popen,system # used in nearly every program here

def read(inputFile):
    """ 
    def read(inputFile):

    Input: name of Praat TextGrid file
    Output: List of tiers, which are lists of (time,label) tuples (in the case
    of point tiers), or (start,stop,label) tuples (in the case of interval 
    tiers)
    """
    file = open(inputFile,'r') # read it in
    lines = file.readlines() # sadly read it all into memory
    file.close() # close it out
    lines.pop(0) # file type
    lines.pop(0) # object class
    lines.pop(0) # blank
    tiers = [] # data structure that will contain all the tiers
    if 'xmin' in lines[0]: # long TextGrid if this is true
        lines.pop(0) # xmin
        lines.pop(0) # xmax
        lines.pop(0) # tiers?
        junk,nTiers = lines.pop(0).rstrip().split(' = ') # size
        lines.pop(0) # item []
        for i in range(int(nTiers)): # loop over the tiers
            lines.pop(0) # the first tier's start
            if 'IntervalTier' in lines[0]: # we can check directly for interval
                lines.pop(0) # class type
                lines.pop(0) # tier name
                lines.pop(0) # xmin
                lines.pop(0) # xmax
                junk,nInter = lines.pop(0).rstrip().split(' = ') # size
                intervalTier = [] # data struct, gonna be full of tuples
                for j in range(0,int(nInter)): # loop over interval tier itself
                    lines.pop(0) # interval number
                    junk,xmin = lines.pop(0).rstrip().split(' = ') # start
                    junk,xmax = lines.pop(0).rstrip().split(' = ') # stop
                    junk,label = lines.pop(0).rstrip().split(' = ') # label
                    intervalTier.append((float(xmin),float(xmax), \
                                                            label.strip('"')))
                tiers.append(intervalTier) # now write it into big structure
            else: # if point tier
                lines.pop(0) # class type
                lines.pop(0) # tier name
                lines.pop(0) # xmin
                lines.pop(0) # xmax
                junk,nInter = lines.pop(0).rstrip().split(' = ') # size
                pointTier = [] # data struct, gonna be full of tuples
                for j in range(0,int(nInter)): # loop over point tier itself
                    lines.pop(0) # point number
                    junk,time = lines.pop(0).rstrip().split(' = ') # time
                    junk,label = lines.pop(0).rstrip().split(' = ') # label
                    pointTier.append((float(time),label.strip('"'))) # save it
                tiers.append(pointTier) # now write it into big structure
    else: # short TextGrid format
        lines.pop(0) # xmin
        lines.pop(0) # xmax
        lines.pop(0) # tiers
        nTiers = lines.pop(0).rstrip() # size
        for i in range(0,int(nTiers)): # loop over the tiers
            if 'IntervalTier' in lines[0]: 
                lines.pop(0) # class type
                lines.pop(0) # tier name
                lines.pop(0) # xmin
                lines.pop(0) # xmax
                nInter = lines.pop(0).rstrip() # interval size
                intervalTier = [] # data struct, gonna be full of tuples
                for j in range(int(nInter)): # loop over the ier
                    xmin = float(lines.pop(0).rstrip()) # start
                    xmax = float(lines.pop(0).rstrip()) # stop
                    label = lines.pop(0).rstrip() # label
                    intervalTier.append((xmin,xmax,label.strip('"'))) # save
                tiers.append(intervalTier) # now write it into big structure
            else: # point tier
                lines.pop(0) # class type
                lines.pop(0) # tier name
                lines.pop(0) # xmin
                lines.pop(0) # xmax
                nInter = lines.pop(0).rstrip() # interval size
                pointTier = [] # data struct, gonna be full of tuples
                for j in range(int(nInter)): # loop over the ier
                    time = float(lines.pop(0).rstrip()) # time
                    label = lines.pop(0).rstrip() # label
                    pointTier.append((time,label.strip('"'))) # save it
                tiers.append(pointTier) # now write it into big structure
    return tiers # now we are done

# write a Praat TextGrid
def write(list,outputFile,format=None):
    """ 
    def write(list,outputFile,format=None):

    Input: list of tiers, which are lists of (time,label) tuples (in the 
    case of point tiers) in temporal order and/or lists of (start,stop,label) 
    tuples (in the case of interval tiers), an output filename, and an output
    filename(,format i.e. TextGrid is 'short' format if non-null)
    Output: none, but TextGrid is printed to output file 
    """
    file = open(outputFile,'w') # open for writing
    xmin,xmax = (),None # positive and negative infinity
    if (format): # nonnull, so short
        file.write('File type = "ooTextFile"\n') # write header first line
        file.write('Object class = "TextGrid"\n\n') # 2nd and third line 
        for tier in list: # loop over tiers
            if len(tier[0]) > 2: # interval
                if tier[0][0] < xmin: # if a smaller xmin
                    xmin = tier[0][0] # save it
                if tier[-1][2] > xmax: # if a bigger xmax
                    xmax = tier[-1][1] # save it
            else: # point tier
                if tier[0][0] < xmin: # if a smaller xmin
                    xmin = tier[0][0] # save it
                if tier[-1][0] > xmax: # if a bigger xmax
                    xmax = tier[-1][0] # save it
        file.write(str(xmin) + '\n') # xmin
        file.write(str(xmax) + '\n') # xmax
        file.write('<exists>\n') # tiers line
        file.write(str(len(list)) + '\n') # number of tiers
        tCounter = 1 # keep track of the number of tiers
        for tier in list: # loop over tiers
            if len(tier[0]) > 2: # is it start/stop or just point?
                file.write('"IntervalTier"\n') # class label
                file.write('"' + str(tCounter) + '"\n') # n/m
                file.write(str(xmin) + '\n') # xmin
                file.write(str(xmax) + '\n') # xmax
                file.write(str(len(tier)) +'\n') # number of intervals
                iCounter = 1 # keep track of the number of intervals
                for intrvl in tier: # loop over vals
                    file.write(str(intrvl[0]) + '\n') # xmin
                    file.write(str(intrvl[1]) + '\n') # xmax
                    file.write('"' + intrvl[2] + '"\n') # label
                    iCounter = iCounter + 1 # increment interval counter
            else: # type is point tier
                file.write('"TextTier"\n') # class label
                file.write('"' + str(tCounter) + '"\n') 
                file.write(str(xmin) + '\n') # xmin
                file.write(str(xmax) + '\n') # xmax
                file.write(str(len(tier)) +'\n')
                pCounter = 1 # keep track of the number of points
                for point in tier: # loop over vals
                    file.write(str(point[0]) + '\n')
                    file.write('"' + point[1] + '"\n') 
                    pCounter = pCounter + 1 # increment point counter
            tCounter = tCounter + 1 # increment tier counter
    else: # long format TextGrid
        file.write('File type = "ooTextFile"\n') # write header first line
        file.write('Object class = "TextGrid"\n\n') # 2nd and third line 
        for tier in list: # loop over tiers
            if len(tier[0]) > 2: # is it interval?
                if tier[0][0] < xmin: # if a smaller xmin
                    xmin = tier[0][0] # save it
                if tier[-1][1] > xmax: # if a bigger xmax
                    xmax = tier[-1][1] # save it
            else: # point tier
                if tier[0][0] < xmin: # if a smaller xmin
                    xmin = tier[0][0] # save it
                if tier[-1][0] > xmax: # if a bigger xmax
                    xmax = tier[-1][0] # save it
        file.write('xmin = ' + str(xmin) + '\n') # xmin
        file.write('xmax = ' + str(xmax) + '\n') # xmax
        file.write('tiers? <exists>\n') # tiers line
        file.write('size = ' + str(len(list)) + '\n') # number of tiers
        file.write('item []:\n') # last piece of header
        tCounter = 1 # keep track of the number of tiers
        for tier in list: # loop over tiers
            file.write('    item [' + str(tCounter) + ']:\n') # item number
            if len(tier[0]) > 2: # is it start/stop or just point?
                file.write('        class = "IntervalTier"\n') # class label
                file.write('        name = "' + str(tCounter) + '"\n') # n/m
                file.write('        xmin = ' + str(xmin) + '\n') # xmin
                file.write('        xmax = ' + str(xmax) + '\n') # xmax
                file.write('        intervals: size = ' + str(len(tier)) +'\n')
                iCounter = 1 # keep track of the number of intervals
                for intrvl in tier: # loop over vals
                    file.write('        intervals [' + str(iCounter) + ']:\n')
                    file.write('            xmin = ' + str(intrvl[0]) + '\n')
                    file.write('            xmax = ' + str(intrvl[1]) + '\n')
                    file.write('            text = "' + intrvl[2] + '"\n') 
                    iCounter = iCounter + 1 # increment interval counter
            else: # type is point tier
                file.write('        class = "TextTier"\n') # class label
                file.write('        name = "' + str(tCounter) + '"\n') 
                file.write('        xmin = ' + str(xmin) + '\n') # xmin
                file.write('        xmax = ' + str(xmax) + '\n') # xmax
                file.write('        points: size = ' + str(len(tier)) +'\n')
                pCounter = 1 # keep track of the number of points
                for point in tier: # loop over vals
                    file.write('        points [' + str(pCounter) + ']:\n')
                    file.write('            time = ' + str(point[0]) + '\n')
                    file.write('            mark = "' + point[1] + '"\n') 
                    pCounter = pCounter + 1 # increment point counter
            tCounter = tCounter + 1 # increment tier counter
    file.close() # close it out

def merge(tg1, tg2):
    """ 
    def merge(tg1,tg2):

    Input: two lists of tiers, which are lists of (time,label) tuples (in the 
    case of point tiers) in temporal order and/or lists of (start,stop,label) 
    tuples (in the case of interval tiers)
    Output: a new list of tiers which merges the corresponding tiers from tg1
    and tg2. in any region where both TextGrids have intervals for non-empty
    labels, then tg2 takes precedence.
    """

    ret = []
    if len(tg1) != len(tg2) :
        raise ValueError, "The two TextGrids don't have the same number of tiers."
		
    for tier in range(len(tg1)) :
        newtier = []
        ret.append(newtier)
        
        i1 = 0
        i2 = 0
        i2end = len(tg2[tier])
        
        # Advance i2 past any empty-string intervals
        while i2 < len(tg2[tier]) and tg2[tier][i2][-1] == "":
            i2 += 1
        
        # Advance i2end backwards past any empty-string intervals
        while i2end > 0 and tg2[tier][i2end-1][-1] == "":
            i2end -= 1

        # Write out all of the points and intervals in tg1 that start
        # before the first non-empty-string point or interval in tg2
        while i1 < len(tg1[tier]):
            if i2 == len(tg2[tier]) or tg1[tier][i1][0] < tg2[tier][i2][0]:
                # If this is an interval that ends after the start of
                # whatever is here on tg2, then move the end time back.
                item = list(tg1[tier][i1])
                if i2 < len(tg2[tier]) and len(tg1[tier][i1]) == 3 and tg1[tier][i1][1] > tg2[tier][i2][0]:
                    item[1] = tg2[tier][i2][0]
                
                newtier.append(item)
                i1 += 1
            else:
                break
                
        # Write out the remainder of tg2 up to i2end.
        for i in range(i2, i2end):
            newtier.append(list(tg2[tier][i]))
        
        # Get the last time on tg2.
        e = tg2[tier][i2end-1][0]
        if len(tg2[tier][i2end-1]) == 3: # if interval, end value
            e = tg2[tier][i2end-1][1]
        
        # Write out the remainder of tg1.
        while i1 < len(tg1[tier]):
            t = tg1[tier][i1][0]
            if len(tg1[tier][i1]) == 3: # if interval, end value
        	    t = tg1[tier][i1][1]
        	
            if len(tg2[tier]) == 0 or t > e:
                # If this is an interval that starts before the end of
                # tg2, then move the start time forward.
                item = list(tg1[tier][i1])
                if len(tg2[tier]) != 0 and len(tg1[tier][i1]) == 3 and tg1[tier][i1][0] < e:
                    item[0] = e
                
                newtier.append(item)
            i1 += 1

    return ret
