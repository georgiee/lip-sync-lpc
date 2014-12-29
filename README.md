# Lip Sync Research
I tried to create a basic vowel detection for a flash project, to decide if an expected word was spoken and to sync lips to a sound currently spoken by a user. 
Conclusion: Use Linear Predictive Coding (LPC) to find vowels or synthesizes speech.
But it was work than I expected. I used ALF(AS3) and numpy(python) to validate my AS3 results. I havehad more trust in numpy.
I also used a wonderful tool called PRAAT to verify my results and look into a working vowel detection.

I failed to complete the vowel detection within the given project timeline so I used a simple fallback: Measure the volume and open the mouth/lips accordingly. Good enough. But I was a little bit disappointed. It felt that I was very, very close to a workign vowel detection. **Note to myself: Continue in JavaScript!**

Here some notes from 2012-11 :)

Finally I got it! 2012 11 13 !! GK
http://www.mail-archive.com/clam-devel@llistes.projectes.lafarga.org/msg00586.html
To get the formants, you have to solve the linear model of the LPC coefficients, which I think is just finding the roots of a polynomial or the poles of the filter. The LPC coefficients are real valued, but the roots are complex. The arg/angle of the root/pole is the formant frequency and the magnitude is the formant bandwidth.

http://www.mathworks.de/de/help/signal/ug/formant-estimation-with-lpc-coefficients.html
Obtain the linear prediction coefficients. To specify the model order, use the general 
rule that the order is two times the expected number of formants plus 2. In the frequency range, [0,Fs/2], you expect 3 formants. Therefore, set the model order equal to 8. Find the roots of the prediction polynomial returned by lpc.

Because the LPC coefficients are real-valued, the roots occur in complex conjugate pairs.
Retain only the roots with one sign for the imaginary part and determine the angles corresponding to the roots.
http://www.mathworks.de/de/help/signal/ug/formant-estimation-with-lpc-coefficients.html


## Some notes
### Collection of documents and tools

+ Vowel (IPA) Formant f1  Formant f2
+ u 320 Hz  800 Hz
+ o 500 Hz  1000 Hz
+ ɑ 700 Hz  1150 Hz
+ a 1000 Hz 1400 Hz
+ ø 500 Hz  1500 Hz
+ y 320 Hz  1650 Hz
+ ɛ 700 Hz  1800 Hz
+ e 500 Hz  2300 Hz
+ i 320 Hz  2500 Hz



### linear prediction coefficients
Almost all of the semantically important information in speech lies below 4000 − 5000 Hz, as demonstrated by the intelligibility of telephones, so an audio sample rate of 10kHz  is sufficient for analysis applications such as lip-sync. 

Mund Position reichen doch 10-15

Previous synchronized speech animation has typically used approximately 10-15 distinct mouth keyframes [11,12,5] (although synthetic speech approaches [15,14] have used many more distinct mouth positions).
via ______lipsync91.pdf

40 OPTIMUM deutsche Sprache nur wichtig bei Sprachgenerierung.

Mund Positionen:
http://animation.about.com/od/flashanimationtutorials/a/animationphonem.htm

LPC 
Deutsch hat etwa 40 Phoneme (ca. 20 Vokalphoneme und ca. 20 konsonantische Phoneme), deutsche Dialekte oft erheblich mehr.
http://de.wikipedia.org/wiki/Phonem#Phoneme_und_Phonemklassen_der_deutschen_Lautsprache

http://www.speech.cs.cmu.edu/cgi-bin/cmudict
Englisch: (40)
AA	0
AE	26
AH	47
AO	68
AW	90
AY	115
EH	145
EY	169
IH	192
IY	215
OW	239
OY	261
UH	286
UW	314
B	342
CH	354
D	367
DH	383
ER	404
F	419
G	430
HH	439
JH	452
K	472
L	488
M	502
N	525
NG	542
P	560
R	568
S	583
SH	598
T	612
TH	622
V	633
W	648
Y	661
Z	672
ZH	689
END	704
