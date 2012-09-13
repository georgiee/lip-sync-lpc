function [gain, indices] = busca_melhor_CL(speech_frame,speech_sint,N)
%
% [gain, indices] = busca_melhor_CL(speech_frame,speech_sint,N_comp)
%
%  Busca a combinação linear de N vetores em 'speech_sint' que melhor
%  representa o vetor 'speech_frame'. O resultado é dado nos vetores 'gain'
%  e indices. O algoritimo de busca é do tipo 'greedy'.
%
%  O vetor correspondente combinação linear resultate pode ser obtido por:
%  vetor_estimado = speech_sint(:,indices)*gain;

% pre- allocação
indices = zeros(1,N);
gain    = zeros(N,1);

for i = 1 : N;     %  algoritimo guloso   
    %----- Calculo do vetor que minimiza a potencia erro ------

    power = sum(speech_sint.^2);        % vetor (colunas) com as potencias  
                                        % de cada sinal sintetizado
                                        
    acorr =(speech_frame'*speech_sint); % vetor (colunas) com a  
                                        % autocorrelação de cada sinal 
                                        % sintetizado com o original
                                        
    acorr_norm = (acorr.^2)./power;     % Autocorrelação normalizada

    [junk,indices(i)] = max(acorr_norm); % acha o indice do vetor que minimiza

    gain(i)=acorr(indices(i))./...      % ganho para minimixar o erro
                 power(indices(i));     
    %----------------------------------------------------------
    
    speech_frame = speech_frame - speech_sint(:,indices(i)).*gain(i);
end;
