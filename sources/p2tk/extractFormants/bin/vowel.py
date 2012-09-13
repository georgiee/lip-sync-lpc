def isDiphthong(v):
  if v == "AW" or v == "AY" or v == "OY":
    return 1
  else:
    return 0

def isUpgliding(v):
  if v == "IY" or v == "EY" or v == "AY" or v == "OY" or v == "UW" or v == "OW" or v == "AW":
    return 1
  else:
    return 0

def isIngliding(v):
  if v == "AE" or v == "AO":
    return 1
  else:
    return 0

def isShort(v):
  if v in ["AA", "EH", "IH", "UH", "AH"]:
    return 1
  else:
    return 0
