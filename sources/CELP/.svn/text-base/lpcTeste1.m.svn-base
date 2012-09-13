T = 2;
n = Fs*T;        % numero de amostras
s = 160;         % tamanho da amostra do LPC 
P = 60;         % Pitch
coef = 12;       % numero de coeficientes a serem usados

clear a
clear g


% Compressao
i=1;
for k = 1:s:n
    %a(i,:) = lpc(y(k:(k+s-1)),coef);                % sem uma janela 
    a(i,:) = lpc(y(k:(k+s-1)).*hamming(s),coef);    % usando uma janela 
    i=i+1;
end

% sinal de exitação 
i=1;
for k = 1:s:n
    e(k:(k+s-1)) = filter([1],a(i,:),y(k:(k+s-1)));  % filtro inverso de h[n]
    g(i) = sqrt(mean(e(k:(k+s-1)).^2));
    e(k:(k+s-1)) = e(k:(k+s-1))/g(i);
    i=i+1;
end    


%trem de impulsos
imp = zeros(n,1);
imp(1:P:end) = 1;
i=1;
for k = 1:s:n
    imp(k:(k+s-1)) = imp(k:(k+s-1));  % filtro inverso de h[n]
    i=i+1;
end 


%reconstrução
i=1;
Z = zeros(coef,1);      % condições iniciais nulas
for k = 1:s:n
     ex = imp(k:(k+s-1));              % exitação é o trem de impulsos
    %ex = rand(s,1);                   % exitação é ruido branco
    
    % aplica o filtro dessa janela na exitação
    [ow,Z] = filter([1],a(i,:),ex,Z);  % condições iniciais em loop
%    [ow]   = filter([1],a(i,:),ex);    % condições iniciais sempre nulas
    o(k:(k+s-1)) = ow*g(i);                               
    i=i+1;
end    
    


%freqz([1],a(10,:))
plot(e);
wavplay(o,Fs)