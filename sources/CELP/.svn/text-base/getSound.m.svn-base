Fs = 8000;      % frequencia de amostragem

recObj = audiorecorder(Fs,16,1);
disp('Pressione enter para COMEÇAR gravar')
[~]=input('');                      
record(recObj);
disp('Pressione enter para PARAR gravar');
[~]=input('');                      
stop(recObj);

y= getaudiodata(recObj);
T = length(y)/Fs;                    % tempo de gravação em segundos

% Pre-filtragem
pre = [1 -0.9];
yf = filter(pre,1,y);

specgram(y,512,800,hamming(40));