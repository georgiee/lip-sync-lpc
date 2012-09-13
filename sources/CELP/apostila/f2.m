% 1.2.2 LP sintese de 30ms de som vozeado
p = 10; % 10 coeficientes de predição

input_frame = y(3900:4139);  % frame de 30ms de uma vogal 'teste nUmero 1'
figure;
plot(input_frame);
figure;
periodogram(input_frame,[],512);

% calculo dos coeficientes LPC e do desvio padrão do erro
[A,sigma2] = lpc(input_frame,p); 
sigma = sqrt(sigma2);               

% grafico do filtro de sintese sobreposto ao do periodograma
hold on;
[Hi,Wi] = freqz(1,A,1024);
plot(Wi/pi,20*log10(sigma*abs(Hi)));

%Calculo e grafico do residuo e do seu espectro
LP_residuo = filter(A,1,input_frame); %calculo do residuo, usando filtro inverso
figure;
plot(LP_residuo);
figure;
periodogram(LP_residuo,[],512);


%Sinal de exitação estimado
imp = zeros(240,1);     % trem de inpulsos pitch de 64
imp(1:64:end) = 1;
gain = sigma/std(imp);  % ganho deve se igualar a variancia do residuo pela estimação
figure;
plot(imp*gain);
figure;
periodogram(imp*gain,[],8192);

% reconstrução do frame apartir do sinal de exitação e dos coef. LP
sinal_recon = filter(1,A,imp*gain);% sinal de exitação passado pelo filtro LP
figure;
plot(sinal_recon);
figure;
periodogram(sinal_recon,[],8192);

