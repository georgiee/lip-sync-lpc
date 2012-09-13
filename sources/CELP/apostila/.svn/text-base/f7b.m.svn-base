%1.2.7 CELP Analize e sintese, com filtro de predição de pitch
frame_size = 240;       % frame de 30ms
frame_avan = 40;        % cada frame é avançado de 5ms
p = 10;                 % Numero de coef. do filtro LP
codebook_size = 512;   % Numero de vetores no Bloco de exitação
N_comp = 5;            % numero de componentes por frame
LTP_max_delay = 256;    % tamanho do buffer de predição longa


% inicia as variaveis de saida e de estado do filtro
Z = zeros(p,1);                         % condições iniciais nulas
exitacao_buffer = zeros(LTP_max_delay...% buffer de predição longa
                         + frame_avan );
o = zeros((frame_size-frame_avan)/2,1); % incializa variavel de saida
                                        % as primeiras amostras do sinal
                                        % são perdidas na reconstrução

%inicializa o bloco de exitação randomico
codebook = randn(frame_avan,codebook_size);

% adiciona padding ao final para que o dado seja multiplo das janelas
njanelas = (length(y)-frame_size)/frame_avan;
padsize  = (njanelas - floor(njanelas))*frame_size;
y = padarray(y,padsize,0,'post');

% examina cada frame
for i=1:(length(y)-frame_size)/frame_avan   
    
    %------------- Analise CELP ---------------------
    
    % Extrai um frame para analizar os coeficientes LPC
    frame = y((i-1)*(frame_avan)+[1:frame_size]);
    
    
    % calculo dos coeficientes LPC do frame
    A = lpc(frame.*hamming(length(frame)),p);  
    
    % Extrai pedaço central do frame, para escolha da exitação
    speech_frame = frame((frame_size-frame_avan)/2+1:...
                         (frame_size-frame_avan)/2+frame_avan);
                     
          
    % A resposta natural do filtro de reconstrução deve ser retirada 
    % para que a comparação seja justa. Osb: ( A respota natural poderia 
    % ser incorporada na filtragem do codebook)
    speech_natural = filter(1,A,zeros(frame_avan,1),Z);
    speech_frame = speech_frame - speech_natural;
                     
    % Sintetiza o sinal de voz apartir de cada vetor de exitação
    speech_sint = filter(1,A,codebook);
    
    % Localiza a combinação linear de N componentes do codebook que
    % minimiza o erro da reconstrução.
    [gain, indices] = busca_melhor_CL(speech_frame,speech_sint,N_comp);
    
        
    %------------- Sintese CELP ---------------------
    
    % Gera o sinal de exitação apartir do codebook
    exitacao = codebook(:,indices)*gain;
                     
    % Aplica o filtro LP de sintese no sinal de exitação estimado
    [recon,Z] = filter(1,A,exitacao,Z);  % condições iniciais em loop
     
    % encadeando frames de sintese
    o = [o;recon];       
end;

% padding no sinal se saida para ele ter o mesmo tamanho do de entrada
o = padarray(o,frame_size-(frame_size-frame_avan)/2,0,'post');

% tocar o sinal sintetizado
wavplay(o,Fs)