%1.2.4 LP analise e sintese com pitch fixo
frame_size = 240;   % frame de 30ms
frame_avan = 80;    % cada frame é avançado de 10ms
p = 10;             % Numero de coef. do filtro LP

% inicia as variaveis de saida e de estado do filtro
o =0;
Z = zeros(p,1);    % condições iniciais nulas


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
    % gera 10ms (80 amostras) sinal de exitação estimado
    imp = zeros(frame_avan,1);     % trem de impulsos pitch de 40
    imp(1:40:end) = 1;             % ganho para que o residuo e seu
    gain = sigma/std(imp);         % estimador tenham o mesmo desvio padrão
    
    % Aplica o filtro LP de sintese no sinal de exitação estimado
    [recon,Z] = filter(1,A,imp*gain,Z);  % condições iniciais em loop
    
    % encadeando frames de sintese
    o = [o;recon];    
end;

% tocar o sinal sintetizado
wavplay(o,Fs)