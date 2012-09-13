%1.2.7 CELP Analize e sintese, com filtro de predição de pitch
frame_size = 240;       % frame de 30ms
frame_avan = 40;        % cada frame é avançado de 5ms
p = 10;                 % Numero de coef. do filtro LP
codebook_size = 512;   % Numero de vetores no Bloco de exitação
N_comp = 5;            % numero de componentes por frame
LTP_max_delay = 256;    % tamanho do buffer de predição longa


% inicia as variaveis de saida e de estado do filtro
Z = zeros(p,1);                         % condições iniciais nulas
exitacao_buffer =zeros(LTP_max_delay+...% buffer de predição longa                         
                       frame_avan,1);
exitacao = zeros(frame_avan,1);
o = zeros((frame_size-frame_avan)/2,1); % incializa variavel de saida
                                        % as primeiras amostras do sinal
                                        % são perdidas na reconstrução
                                        
plot(exitacao_buffer);

%inicializa o bloco de exitação randomico
codebook = randn(frame_avan,codebook_size);

% adiciona padding ao final para que o dado seja multiplo das janelas
njanelas = (length(y)-frame_size)/frame_avan;
padsize  = (njanelas - floor(njanelas))*frame_size;
y = padarray(y,padsize,0,'post');

% examina cada frame
for i=1:(length(y)-frame_size)/frame_avan     
    
    % prenche rotaciona o buffer de exitações 
    exitacao_buffer = [exitacao_buffer(frame_avan+1: ...
                       LTP_max_delay+frame_avan);exitacao];
%        figure(1);plot(exitacao_buffer);hold on;plot(exitacao,'r');hold off;    % plot do buffer
    
    % cria um codebook, com um vetor para cada pitch
     for j=1:LTP_max_delay+1
         LTP_codeblock(:,j) =  exitacao_buffer(j+(0:frame_avan-1));
         %figure(2);plot(LTP_codeblock(:,j));
         %ir = 0;
     end
    
    
    %------------- Analise CELP ---------------------
    
    % Extrai um frame para analizar os coeficientes LPC
    frame = y((i-1)*(frame_avan)+[1:frame_size]);    
    
    % calculo dos coeficientes LPC do frame
    A = lpc(frame.*hamming(length(frame)),p);  
    
    % Extrai pedaço central do frame, para escolha da exitação
    speech_frame = frame((frame_size-frame_avan)/2+1:...
                         (frame_size-frame_avan)/2+frame_avan);
                     
          
    % A resposta natural do filtro de reconstrução deve ser retirada 
    % para que a comparação seja justa. 
    speech_natural = filter(1,A,zeros(frame_avan,1),Z);
    speech_frame = speech_frame - speech_natural;
                     
    % encontra o valor de pitch (P) e ganho (b) mais eficinte
    speech_sint = filter(1,A,LTP_codeblock);
    [b,P] = busca_melhor_CL(speech_frame,speech_sint,1);    
        
    % gera a predicao correspondente e retira do sinal 
    speech_frame = speech_frame - speech_sint(:,P).*b;
    
    % Encontra o melhor vetor de exitação
    speech_sint = filter(1,A,codebook);
    [gain, indices] = busca_melhor_CL(speech_frame,speech_sint,N_comp);
    
        
    %------------- Sintese CELP ---------------------
    
   
    % Gera o sinal de exitação apartir do codebook
    exitacao = codebook(:,indices)*gain + speech_sint(:,P).*b;
        
                     
    % Aplica o filtro LP de sintese no sinal de exitação estimado
    [recon,Z] = filter(1,A,exitacao,Z);  % condições iniciais em loop
     
    % encadeando frames de sintese
    o = [o;recon];       
    

end;

% padding no sinal se saida para ele ter o mesmo tamanho do de entrada
o = padarray(o,frame_size-(frame_size-frame_avan)/2,0,'post');

% tocar o sinal sintetizado
wavplay(o,Fs)