// Implementacao de socket, bind, sendto e recvfrom

#include "rede.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

int criar_socket_udp() {
    // https://wiki.inf.ufpr.br/maziero/doku.php?id=pua:comunicacao_em_rede
    // AF_INET = IPv4, SOCK_DGRAM = UDP, 0 = Protocolo padrao
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
        perror("[ERRO REDE] Falha ao criar o socket UDP");

    return sock_fd;
}

int configurar_timeout(int sock_fd, int segundos) {
    struct timeval tv;
    tv.tv_sec = segundos;
    tv.tv_usec = 0;

    // Define SO_RCVTIMEO para o recvfrom retornar -1 em caso de timeout
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("[ERRO REDE] Falha ao configurar timeout no socket");
        return -1;
    }
    return 0;
}

int vincular_porta(int sock_fd, int porta_local) {
    struct sockaddr_in endereco_local;                   // estrutura padrao do C/POSIX usada para armazenar enderecos IPv4 e portas, maziero wiki
    memset(&endereco_local, 0, sizeof(endereco_local));  // zera tudo pra evitar lixo de memoria
    // memset(endereco_local, 0, sizeof());

    endereco_local.sin_family = AF_INET;           // define a familia Ipv4
    endereco_local.sin_addr.s_addr = INADDR_ANY;   // Aceita conexoes de qualquer interface
    endereco_local.sin_port = htons(porta_local);  // Converte a porta para Network Byte Order

    if (bind(sock_fd, (struct sockaddr*)&endereco_local, sizeof(endereco_local)) < 0) {
        perror("[ERRO REDE] Falha ao fazer bind na porta local, veja se a mesma nao esta sendo usada por outro processo\n");
        return -1;
    }
    return 0;
}

int enviar_pacote(int sock_fd, const PacoteAnel* pacote, const char* ip_destino, int porta_destino) {
    struct sockaddr_in endereco_destino;
    socklen_t tamanho_destino;
    ssize_t bytes_enviados;
    bytes_enviados = -1;  // tive que forcar o erro

    memset(&endereco_destino, 0, sizeof(endereco_destino));

    endereco_destino.sin_family = AF_INET;             // define a familia Ipv4, mesma coisa
    endereco_destino.sin_port = htons(porta_destino);  // na documentacao pede pra inverter a ordem dos bytes da porta

    // testa se o IP 'e Valido
    if (inet_pton(AF_INET, ip_destino, &endereco_destino.sin_addr) <= 0) {
        perror("[ERRO REDE] Endereco IP de destino invalido");
        return -1;
    }

    tamanho_destino = sizeof(endereco_destino);

    // Envia o bloco de memoria exato da struct PacoteAnel
    bytes_enviados = sendto(sock_fd, pacote, sizeof(PacoteAnel), 0,
                            (struct sockaddr*)&endereco_destino,
                            tamanho_destino);  // aqui passa com copia e no recvfrom como referencia

    if (bytes_enviados < 0) {
        perror("[ERRO REDE] Falha ao enviar pacote via sendto");
        return bytes_enviados;
    }

    return 0;
}

int receber_pacote(int sock_fd, PacoteAnel* pacote) {
    struct sockaddr_in endereco_origem;
    socklen_t tamanho_origem = sizeof(endereco_origem);
    ssize_t bytes_recebidos;
    bytes_recebidos = -1;

    // Preenche a struct apontada pelo ponteiro pacote com os bytes vindos do socket
    bytes_recebidos = recvfrom(sock_fd, pacote, sizeof(PacoteAnel), 0,
                               (struct sockaddr*)&endereco_origem,
                               &tamanho_origem);

    if (bytes_recebidos < 0) return -1;  // apaguei a mensgem pois estava poluindo o terminal

    return (int)bytes_recebidos;
}

const char* obter_ip_local() {
    // socket UDP temporario
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in alvo_ficticio;

    if (sock < 0) return "127.0.0.1";  // Se falhar, assume loopback

    // mando para um destino ficticio
    std::memset(&alvo_ficticio, 0, sizeof(alvo_ficticio));  // zera a struct para evitar lixo de memoria
    alvo_ficticio.sin_family = AF_INET;
    alvo_ficticio.sin_addr.s_addr = inet_addr("8.8.8.8");
    alvo_ficticio.sin_port = htons(53);

    // connect apenas consulta a tabela de rotas do Linux
    if (connect(sock, (struct sockaddr*)&alvo_ficticio, sizeof(alvo_ficticio)) < 0) {
        close(sock);
        return "127.0.0.1";
    }

    // descobre qual IP a placa de rede atribuiu para essa saida
    struct sockaddr_in endereco_local;
    socklen_t tamanho = sizeof(endereco_local);
    getsockname(sock, (struct sockaddr*)&endereco_local, &tamanho);

    // fecha o socket temporario
    close(sock);

    // converte a estrutura de IP para char e retorna
    return inet_ntoa(endereco_local.sin_addr);
}