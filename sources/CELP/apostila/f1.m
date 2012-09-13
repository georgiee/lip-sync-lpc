% 1.2.1 Examinando o arquivo de voz
load audio;
figure;
plot(y);
figure;
specgram(y,512,800,hamming(40));
wavplay(y,Fs);