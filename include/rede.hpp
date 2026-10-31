#pragma once

// sockets UDP
#include "protocolo.hpp"

// Cria o descriptor do socket UDP (SOCK_DGRAM)
int criar_socket_udp();

// Define um tempo limite de espera para o recvfrom nao bloquear para sempre
int configurar_timeout(int sock_fd, int segundos);

// Associa o socket a porta local do no da rede (bind). Registra a sua porta de escuta no sistema operacional
int vincular_porta(int sock_fd, int porta_local);

// Envia a struct do pacote diretamente para o IP e Porta do sucessor (sendto)
int enviar_pacote(int sock_fd, const PacoteAnel* pacote, const char* ip_destino, int porta_destino);

// Aguarda e recebe um pacote vindo de qualquer no da rede (recvfrom)
int receber_pacote(int sock_fd, PacoteAnel* pacote);