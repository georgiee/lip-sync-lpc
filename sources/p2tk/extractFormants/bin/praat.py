class Formant:
  def __init__(self, name = None):
    self.__times = []
    self.__intensities = []
    self.__formants = []
    self.__bandwidths = []
    self.__xmin = None
    self.__xmax = None
    self.__nx = None
    self.__dx = None
    self.__x1 = None
    self.__maxFormants = None

  def n(self):
    return self.__n

  def xmin(self):
    return self.__xmin

  def xmax(self):
    return self.__xmax

  def times(self):
    return self.__times

  def intensities(self):
    return self.__intensities

  def formants(self):
    return self.__formants

  def bandwidths(self):
    return self.__bandwidths

  def read(self, file):
    """ read Formant from Praat .Formant file (saved as a short text file) """
    text = open(file, 'rU')
    text.readline()  # header
    text.readline()
    text.readline()
    self.__xmin = float(text.readline().rstrip())
    self.__xmax = float(text.readline().rstrip())
    self.__nx = int(text.readline().rstrip())
    self.__dx = float(text.readline().rstrip())
    self.__x1 = float(text.readline().rstrip())
    self.__maxFormants = int(text.readline().rstrip())

    for i in range(self.__nx):
      time = i * self.__dx + self.__x1
      intensity = float(text.readline().rstrip())
      nFormants = int(text.readline().rstrip())
      F = []
      B = []
      for j in range(nFormants):
        F.append(float(text.readline().rstrip()))
        B.append(float(text.readline().rstrip()))
      # force at least 3 formants to be returned for each measurment, if Praat didn't find at least three, then we'll disregard this measurement
      if nFormants < 3:
        continue
      self.__times.append(time)
      self.__intensities.append(intensity)
      self.__formants.append(F)
      self.__bandwidths.append(B)

    text.close()

class LPC:
  def __init__(self):
    self.__times = []
    self.__intensities = []
    self.__poles = []
    self.__bandwidths = []
    self.__xmin = None
    self.__xmax = None
    self.__nx = None
    self.__dx = None
    self.__x1 = None
    self.__maxFormants = None

  def times(self):
    return self.__times

  def poles(self):
    return self.__poles

  def bandwidths(self):
    return self.__bandwidths

  def nx(self):
    return self.__nx

  def dx(self):
    return self.__dx

  def x1(self):
    return self.__x1

  def read(self, file):
    """ read Formant from Praat .Formant file (saved as a short text file) """
    text = open(file, 'rU')
    text.readline()  # header
    text.readline()
    text.readline()
    self.__xmin = float(text.readline().rstrip())
    self.__xmax = float(text.readline().rstrip())
    self.__nx = int(text.readline().rstrip())
    self.__dx = float(text.readline().rstrip())
    self.__x1 = float(text.readline().rstrip())
    self.__maxFormants = int(text.readline().rstrip())

    for i in range(self.__nx):
      time = i * self.__dx + self.__x1
      intensity = float(text.readline().rstrip())
      nFormants = int(text.readline().rstrip())
      F = []
      B = []
      for j in range(nFormants):
        F.append(float(text.readline().rstrip()))
        B.append(float(text.readline().rstrip()))
      # force at least 3 formants to be returned for each measurment, if Praat didn't find at least three, then we'll disregard this measurement
      if nFormants < 3:
        continue
      self.__times.append(time)
      self.__intensities.append(intensity)
      self.__poles.append(F)
      self.__bandwidths.append(B)

    text.close()

class MFCC:
  def __init__(self):
    self.__times = []
    self.__mfccs = []
    self.__xmin = None
    self.__xmax = None
    self.__nx = None
    self.__dx = None
    self.__x1 = None
    self.__fmin = None
    self.__fmin = None
    self.__maximumNumberOfCoefficients = None

  def xmin(self):
    return self.__xmin

  def xmax(self):
    return self.__xmax

  def nx(self):
    return self.__nx

  def dx(self):
    return self.__dx

  def x1(self):
    return self.__x1

  def fmin(self):
    return self.__fmin

  def fmax(self):
    return self.__fmax

  def times(self):
    return self.__times

  def mfccs(self):
    return self.__mfccs

  def read(self, file):
    """ read Formant from Praat .Formant file (saved as a short text file) """
    text = open(file, 'rU')
    text.readline()  # header
    text.readline()
    text.readline()
    self.__xmin = float(text.readline().rstrip())
    self.__xmax = float(text.readline().rstrip())
    self.__nx = int(text.readline().rstrip())
    self.__dx = float(text.readline().rstrip())
    self.__x1 = float(text.readline().rstrip())
    self.__fmin = float(text.readline().rstrip())
    self.__fmax = float(text.readline().rstrip())
    self.__maximumNumberOfCoefficients = int(text.readline().rstrip())

    for i in range(self.__nx):
      time = i * self.__dx + self.__x1
      nCoefficients = int(text.readline().rstrip())
      M = []
      # the first one is c0, the energy coefficient
      M.append(float(text.readline().rstrip()))
      for j in range(nCoefficients):
        M.append(float(text.readline().rstrip()))
      self.__times.append(time)
      self.__mfccs.append(M)

    text.close()

class TextGrid:
    """ represents Praat TextGrids as list of different types of tiers """

    def __init__(self, name = None): 
        self.__tiers = []
        self.__n = 0
        self.__xmin = None
        self.__xmax = None
        self.__name = name # this is just for the MLF case

    def __str__(self):
        return '<TextGrid with %d tiers>' % self.__n

    def __iter__(self):
        return iter(self.__tiers)

    def __len__(self):
        return self.__n

    def __getitem__(self, i):
        """ return the (i-1)th tier """
        return self.__tiers[i] 

    def xmin(self):
        return self.__xmin

    def xmax(self):
        return self.__xmax

    def append(self, tier):
        self.__tiers.append(tier)
        self.__xmax = max(tier.xmax(), self.__xmax)

    def read(self, file):
        """ read TextGrid from Praat .TextGrid file (short format)"""
        text = open(file, 'r')
        text.readline() # header
        text.readline()
        text.readline()
        self.__xmin = float(text.readline().rstrip())
        self.__xmax = float(text.readline().rstrip())
        text.readline()
        self.__n = int(text.readline().rstrip())
        for i in range(self.__n): # loop over grids
            # [1:-1] strips off the " characters surrounding all labels
            if text.readline().rstrip()[1:-1] == 'IntervalTier': 
                inam = text.readline().rstrip()[1:-1]
                imin = float(text.readline().rstrip())
                imax = float(text.readline().rstrip())
                itier = IntervalTier(inam, imin, imax)
                n = int(text.readline().rstrip())
                for j in range(n):
                    jmin = float(text.readline().rstrip())
                    jmax = float(text.readline().rstrip())
                    jmrk = text.readline().rstrip()[1:-1]
                    itier.append(Interval(jmin, jmax, jmrk))
                self.append(itier) 
            else: # pointTier
                inam = text.readline().rstrip()[1:-1]
                imin = float(text.readline().rstrip())
                imax = float(text.readline().rstrip())
                itier = PointTier(inam, imin, imax)
                n = int(text.readline().rstrip())
                for j in range(n):
                    jtim = float(text.readline().rstrip())
                    jmrk = text.readline().rstrip()[1:-1]
                    itier.append(Point(jtim, jmrk))
                self.append(itier)
        text.close()

    def readLong(self, file):
        """ read TextGrid from Praat .TextGrid file (long format)"""
        text = open(file, 'r')
        text.readline() # header
        text.readline()
        text.readline()
        self.__xmin = float(text.readline().rstrip().split()[2])
        self.__xmax = float(text.readline().rstrip().split()[2])
        text.readline()
        m = int(text.readline().rstrip().split()[2]) # will be self.__n soon
        text.readline()
        for i in range(m): # loop over grids
            text.readline()
            if text.readline().rstrip().split()[2] == '"IntervalTier"': 
                inam = text.readline().rstrip().split()[2][1:-1]
                imin = float(text.readline().rstrip().split()[2])
                imax = float(text.readline().rstrip().split()[2])
                itier = IntervalTier(inam, imin, imax) # redundant FIXME
                n = int(text.readline().rstrip().split()[3])
                for j in range(n):
                    text.readline().rstrip().split() # header junk
                    jmin = float(text.readline().rstrip().split()[2])
                    jmax = float(text.readline().rstrip().split()[2])
                    jmrk = text.readline().rstrip().split()[2][1:-1]
                    itier.append(Interval(jmin, jmax, jmrk))
                self.append(itier) 
            else: # pointTier
                inam = text.readline().rstrip().split()[2][1:-1]
                imin = float(text.readline().rstrip().split()[2])
                imax = float(text.readline().rstrip().split()[2])
                itier = PointTier(inam, imin, imax) # redundant FIXME
                n = int(text.readline().rstrip().split()[3])
                for j in range(n):
                    text.readline().rstrip() # header junk
                    jtim = float( text.readline().rstrip().split()[2])
                    jmrk = text.readline().rstrip().split()[2][1:-1]
                    itier.append(Point(jtim, jmrk))
                self.append(itier)
        text.close()

    def write(self, text):
        """ write it into a text file that Praat can read """
        text = open(text, 'w')
        text.write('File type = "ooTextFile"\n')
        text.write('Object class = "TextGrid"\n\n')
        text.write('xmin = %f\n' % self.__xmin)
        text.write('xmax = %f\n' % self.__xmax)
        text.write('tiers? <exists>\n')
        text.write('size = %d\n' % self.__n)
        text.write('item []:\n')
        for (tier, n) in zip(self.__tiers, range(1, self.__n + 1)):
            text.write('\titem [%d]:\n' % n)
            if tier.__class__ == IntervalTier: 
                text.write('\t\tclass = "IntervalTier"\n')
                text.write('\t\tname = "%s"\n' % tier.name())
                text.write('\t\txmin = %f\n' % tier.xmin())
                text.write('\t\txmax = %f\n' % tier.xmax())
                text.write('\t\tintervals: size = %d\n' % len(tier))
                for (interval, o) in zip(tier, range(1, len(tier) + 1)): 
                    text.write('\t\t\tintervals [%d]:\n' % o)
                    text.write('\t\t\t\txmin = %f\n' % interval.xmin())
                    text.write('\t\t\t\txmax = %f\n' % interval.xmax())
                    text.write('\t\t\t\ttext = "%s"\n' % interval.mark())
            else: # PointTier
                text.write('\t\tclass = "TextTier"\n')
                text.write('\t\tname = "%s"\n' % tier.name())
                text.write('\t\txmin = %f\n' % tier.xmin())
                text.write('\t\txmax = %f\n' % tier.xmax())
                text.write('\t\tpoints: size = %d\n' % len(tier))
                for (point, o) in zip(tier, range(1, len(tier) + 1)):
                    text.write('\t\t\tpoints [%d]:\n' % o)
                    text.write('\t\t\t\ttime = %f\n' % point.time())
                    text.write('\t\t\t\tmark = "%s"\n' % point.mark())
        text.close()

class IntervalTier:
    """ represents IntervalTier as a list plus some features: min/max time, 
    size, and tier name """

    def __init__(self, name = None, xmin = None, xmax = None):
        self.__n = 0
        self.__name = name
        self.__xmin = xmin
        self.__xmax = xmax
        self.__intervals = []

    def __str__(self):
        return '<IntervalTier "%s" with %d points>' % (self.__name, self.__n)

    def __iter__(self):
        return iter(self.__intervals)

    def __len__(self):
        return self.__n

    def __getitem__(self, i):
        """ return the (i-1)th interval """
        return self.__intervals[i]

    def xmin(self):
        return self.__xmin

    def xmax(self):
        return self.__xmax

    def name(self):
        return self.__name

    def append(self, interval):
        self.__intervals.append(interval)
        self.__xmax = interval.xmax()
        self.__n += 1

    def read(self, file):
        text = open(file, 'r')
        text.readline() # header junk 
        text.readline()
        text.readline()
        self.__xmin = float(text.readline().rstrip().split()[2])
        self.__xmax = float(text.readline().rstrip().split()[2])
        self.__n = int(text.readline().rstrip().split()[3])
        for i in range(self.__n):
            text.readline().rstrip() # header
            imin = float(text.readline().rstrip().split()[2]) 
            imax = float(text.readline().rstrip().split()[2])
            imrk = text.readline().rstrip().split()[2].replace('"', '') # txt
            self.__intervals.append(Interval(imin, imax, imrk))
        text.close()

    def write(self, file):
        text = open(file, 'w')
        text.write('File type = "ooTextFile"\n')
        text.write('Object class = "IntervalTier"\n\n')
        text.write('xmin = %f\n' % self.__xmin)
        text.write('xmax = %f\n' % self.__xmax)
        text.write('intervals: size = %d\n' % self.__n)
        for (interval, n) in zip(self.__intervals, range(1, self.__n + 1)):
            text.write('intervals [%d]:\n' % n)
            text.write('\txmin = %f\n' % interval.xmin())
            text.write('\txmax = %f\n' % interval.xmax())
            text.write('\ttext = "%s"\n' % interval.mark())
        text.close()

class PointTier:
    """ represents PointTier (also called TextTier for some reason) as a list 
    plus some features: min/max time, size, and tier name """

    def __init__(self, name = None, xmin = None, xmax = None):
        self.__n = 0
        self.__name = name
        self.__xmin = xmin
        self.__xmax = xmax
        self.__points = []

    def __str__(self):
        return '<PointTier "%s" with %d points>' % (self.__name, self.__n)

    def __iter__(self):
        return iter(self.__points)
    
    def __len__(self):
        return self.__n
    
    def __getitem__(self, i):
        """ return the (i-1)th tier """
        return self.__points[i]

    def name(self):
        return self.__name

    def xmin(self):
        return self.__xmin

    def xmax(self): 
        return self.__xmax

    def append(self, point):
        self.__points.append(point)
        self.__xmax = point.xmax()
        self.__n += 1

    def read(self, file):
        text = open(file, 'r')
        text.readline() # header junk 
        text.readline()
        text.readline()
        self.__xmin = float(text.readline().rstrip().split()[2])
        self.__xmax = float(text.readline().rstrip().split()[2])
        self.__n = int(text.readline().rstrip().split()[3])
        for i in range(self.__n):
            text.readline().rstrip() # header
            itim = float(text.readline().rstrip().split()[2])
            imrk = text.readline().rstrip().split()[2].replace('"', '') # txt
            self.__points.append(Point(imrk, itim))
        text.close()

    def write(self, file):
        text = open(file, 'w')
        text.write('File type = "ooTextFile"\n')
        text.write('Object class = "TextTier"\n\n')
        text.write('xmin = %f\n' % self.__xmin)
        text.write('xmax = %f\n' % self.__xmax)
        text.write('points: size = %d\n' % self.__n)
        for (point, n) in zip(self.__points, range(1, self.__n + 1)):
            text.write('points [%d]:\n' % n)
            text.write('\ttime = %f\n' % point.time())
            text.write('\tmark = "%s"\n' % point.mark())
        text.close()

class Interval:
    """ represent an Interval """
    def __init__(self, xmin, xmax, mark):
        self.__xmin = xmin
        self.__xmax = xmax
        self.__mark = mark
    
    def __str__(self):
        return '<Interval "%s" %f:%f>' % (self.__mark, self.__xmin, self.__xmax)

    def xmin(self):
        return self.__xmin

    def xmax(self):
        return self.__xmax

    def mark(self):
        return self.__mark

class Point:
    """ represent a Point """
    def __init__(self, time, mark):
        self.__time = time
        self.__mark = mark
    
    def __str__(self):
        return '<Point "%s" at %f>' % (self.__mark, self.__time)

    def time(self):
        return self.__time

    def mark(self):
        return self.__mark
