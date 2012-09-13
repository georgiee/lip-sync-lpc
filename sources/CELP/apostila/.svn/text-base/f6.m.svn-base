%1.2.6 LP analise e sintese com pitch Variavel
frame_size = 240;   % frame de 30ms
frame_avan = 80;    % cada frame é avançado de 10ms
p = 10;             % Numero de coef. do filtro LP
V_UV = 0.3;         % Coeficiente de detecção vozeado/não

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
    
    f0=pitch(frame,160,20,V_UV);
    
    %------------- Sintese LPC --------------------
    
    % gera 10ms (80 amostras) sinal de exitação estimado,
    exc = zeros(frame_avan,1);      % sinal começa como nulo
    
    if f0 == 0                      % testa se o sinal é vozeado ou não
        % sinal de exitação não-vozeado (ruido branco)
        exc = randn(frame_avan,1); 
        gain = sigma;               % ganho o ganho é iguala ao do residuo já que  o devio pagrão do ruido branco é unitario  
        offset = 0;                 % reseta o offset, caso o proximo frame seja vozeado
       
    else
        %sinal de exitação vozeado (trem de impulsos)
        if offset >= frame_avan         % caso não existão pulsos nesse frame
            offset = offset-frame_avan; % apenas diminue o offset
        else
            exc(offset+1:f0:end) = 1;   % senão adiciona os pulsos necessarios
            if mod(frame_avan-offset,f0) == 0
                offset = 0;             % caso corretamente o offset é zero
            else                        % senão calcula o novo offset
                offset=f0-mod(frame_avan-offset,f0);
            end;
        end;  
        % ganho para que o residuo e seu estimador tenham a mesma variancia
        gain = sigma/sqrt(1/f0);  
    end;
    
                                           
    
    % Aplica o filtro LP de sintese no sinal de exitação estimado
    [recon,Z] = filter(1,A,exc*gain,Z);  % condições iniciais em loop
    
    % encadeando frames de sintese
    if i==1
        o = [recon];   
        f00 = [f0];
    else
        o = [o;recon];       
        f00 = [f00,f0];
    end;
end;

% tocar o sinal sintetizado
wavplay(o,Fs)