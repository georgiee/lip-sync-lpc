%1.2.5 LP analise e sintese com ruido branco
frame_size = 240;   % frame de 30ms
frame_avan = 80;    % cada frame é avançado de 10ms
p = 10;             % Numero de coef. do filtro LP
f0 = 40;            % Pitch de reprodução (em amostras)

% inicia as variaveis de saida e de estado do filtro
Z = zeros(p,1);     % condições iniciais nulas
offset = 0;         % offset do sinal de exitação começa em zero


% adiciona padding ao final para que o dado seja multiplo das janelas
njanelas = (length(y)-frame_size)/frame_avan;
padsize  = (njanelas - floor(njanelas))*frame_size;
y = padarray(y,padsize,0,'post');

% examina cada frame
for i=1:(length(y)-frame_size)/frame_avan   
    
    %-------------- Analize LPC --------------------
    % Extrai um frame para analizar
    frame = y((i-1)*(frame_avan)+[1:frame_size]);
    
    % calculo dos coeficientes LPC, aplica uma janela de hamming no frame
    [A,sigma2] = lpc(frame.*hamming(length(frame)),p);  
    sigma = sqrt(sigma2);            %desvio padrão do sinal de exitação
    
    
    %------------- Sintese LPC --------------------
    % gera 10ms (80 amostras) sinal de exitação estimado,
    exc = randn(frame_avan,1);      % sinal de exitação é ruido branco

    % ganho para que o residuo e seu estimador tenham a mesma variancia
    gain = sigma;           % ganho o ganho é iguala ao do residuo já que 
                            % o devio pagrão do ruido branco é unitario                                           
    
    % Aplica o filtro LP de sintese no sinal de exitação estimado
    [recon,Z] = filter(1,A,exc*gain,Z);  % condições iniciais em loop
    
    % encadeando frames de sintese
    if i==1
        o = [recon];   
        e = [exc];         
    else
        o = [o;recon];   
        e = [e;exc];   
    end;
end;

% tocar o sinal sintetizado
wavplay(o,Fs)