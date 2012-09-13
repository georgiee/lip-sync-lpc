% 1.2.3 LP sintese de 30ms de som não-vozeado
p = 10; % 10 coeficientes de predição

input_frame = y(1800:2039);  %frame de 30ms de uma consoante 'teSte numero 1'
figure;
plot(input_frame);
figure;
pwelch(input_frame);

% calculo dos coeficientes LPC e do desvio padrão do erro
[A,sigma2] = lpc(input_frame,p); 
sigma = sqrt(sigma2);               

% grafico do filtro de sintese sobreposto ao do periodograma
hold on;
[Hi,Wi] = freqz(1,A,1024);
plot(Wi/pi,20*log10(sigma*abs(Hi)));


%Sinal de exitação estimado
exi = randn(1,240);     % sinal de exitação é ruido branco
gain = sigma;           % ganho o ganho é iglua ao do residuo já que 
                        % o devio pagrão do ruido branco é unitario
 
% reconstrução do frame apartir do sinal de exitação e dos coef. LP
sinal_recon = filter(1,A,exi*gain);% sinal de exitação passado pelo filtro LP
figure;
plot(sinal_recon);
figure;
pwelch(sinal_recon);

