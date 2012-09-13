function [P] = pitch(speech_frame,Pmax,Pmin,V_UV);
%  [P] = pitch(speech_frame);
%
%   A função retorna o periodo de pitch (em amostras) de um sinal de voz
%   Zero é retornado caso no frame não seja detectado periocidade
%
%   Pmax     - pitch maximo (em amostras)
%   Pmin     - pitch minimo (em amostras)
%   V_UV     - fator de decisão 
%
%  Ex.: [P] = pitch(speech_frame,160,16,1.5);


% Calcula a autocorrelação normalizada
C = xcorr(speech_frame,Pmax,'coeff');  % Calcula só até um delay de Pmax
C=C(Pmax+1:2*Pmax+1);         % retira só a parte direita da autocorrelação

% Busca pelo valor maximo de autocorrelação (pitch)
[Cmax,i]=max(C(Pmin:end));    % só procura em valores validos para o pitch
P = (Pmin+i-2);               % correção para o valor retornado pela busca

% Decide se o sinal é Vozeado ou não-Vozeado
if  Cmax < V_UV             % Mede a razão entre a corelação do sinal com 
    P = 0;                  % si mesmo e da corelação do pitch
end;                        % Obs: a correlação foi normalizada por R(0)

% Grafico de autocorrelação
% subplot(211);
% plot(speech_frame);
% subplot(212);
% plot([0:Pmax],C);
% hold on;
% plot(P,Cmax,'r*')
% title(sprintf('Pitch at: %d , max %f ',P,Cmax));
% hold off