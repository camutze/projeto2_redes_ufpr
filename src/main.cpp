#include <cstdlib>
#include <cstring>
#include <iostream>

#include "jogo.hpp"
#include "protocolo.hpp"
#include "rede.hpp"

// usar no proximo #using namespace std;

void imprime_cabecalho(char* meu_id, const char** ip_local, int* porta_local, const char** ip_sucessor, int* porta_sucessora) {
    std::cout << "==================================================\n";
    std::cout << " TRABALHO 2 - REDES DE COMPUTADORES - UFPR\n";
    std::cout << "==================================================\n";
    std::cout << " Autor: Carlos e Kaio\n";
    std::cout << "==================================================\n";
    std::cout << " ID do no: " << *meu_id << "\n";
    std::cout << " IP local: ";
    std::cout << *ip_local << "\n";
    std::cout << " Porta local: " << *porta_local << "\n";
    std::cout << " IP do sucessor: " << *ip_sucessor << "\n";
    std::cout << " Porta do sucessor: " << *porta_sucessora << "\n";
    std::cout << "==================================================\n\n";
}

int main(int argc, char* argv[]) {
    PacoteAnel pacote;
    int sock_fd, porta_local, porta_sucessora, bytes, idx_atual;
    const char* ip_sucessor;
    const char* ip_local;
    char meu_id, proximo_id;

    // Valida se o utilizador passou os 4 argumentos obrigatorios
    if (argc < 5) {
        std::cout << "Uso correto: " << argv[0] << " <ID> <PORTA_LOCAL> <IP_SUCESSOR> <PORTA_SUCESSORA>\n";
        std::cout << "Exemplo:     " << argv[0] << " A 6767 192.168.0.1 6768\n";
        std::cout << "\nIP DA SUA MAQUINA: \n";
        system("hostname -I | awk '{print $1}'");

        return 1;
    }

    // Incializa variaveis com os argumentos
    ip_local = obter_ip_local();
    meu_id = argv[1][0];                      // A
    porta_local = std::atoi(argv[2]);         // 6767 kaio que fez essa exigencia
    ip_sucessor = argv[3];                    // 192.168.18,78
    porta_sucessora = std::atoi(argv[4]);     // 6768 - da pra usar a mesma porta se o ip for diferente
    std::memset(&pacote, 0, sizeof(pacote));  // mesma coisa, zera tudo pra evitar lixo de memoria

    imprime_cabecalho(&meu_id, &ip_local, &porta_local, &ip_sucessor, &porta_sucessora);
    // cria o socket UDP
    sock_fd = criar_socket_udp();
    if (sock_fd < 0) return 1;

    // vincula a porta local para escutar mensagens
    if (vincular_porta(sock_fd, porta_local) < 0) return 1;

    // define timeout para o recvfrom nao congelar para sempre
    configurar_timeout(sock_fd, TIMEOUT);

    if (meu_id == 'A') {
        std::cout << "Pressione ENTER para testar e fechar o anel...";
        std::cin.get();

        pacote.tipo_mensagem = MSG_INIT;
        pacote.id_roleta_atual = meu_id;

        std::cout << "[SINCRONIZANDO] Disparando MSG_INIT para o B...\n";
        // enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
    } else
        std::cout << "[SINCRONIZANDO] Aguardando sinal de conexao do anel...\n";

    while (true) {
        bytes = receber_pacote(sock_fd, &pacote);  // Recebe um pacote e preenche a struct PacoteAnel com os dados recebidos

        // Se o recvfrom retornar -1, significa que ocorreu Timeout (nenhum pacote chegou)
        if (bytes < 0) {
            // Se eu sou o No A e estou a tentar fechar o anel, retransmito a MSG_INIT
            if (meu_id == 'A' && pacote.tipo_mensagem == MSG_INIT) {
                std::cout << "[TIMEOUT] Anel ainda nao respondeu. Retransmitindo MSG_INIT...\n";
                enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
            } else if (pacote.sincronizado == true) {
                std::cout << "[TIMEOUT] Pacote sincronizado - Aguardando Apostas...\n";
            }
            continue;  // Volta ao inicio do while para tentar escutar novamente
        }

        if (alguem_faliu(&pacote)) {
            std::cout << " Algum jogador faliu! Encerrando o jogo...\n";
            return 0;
        }

        
        // Se um pacote valido chegou, tratamos de acordo com o tipo
        switch (pacote.tipo_mensagem) {
            case MSG_INIT:
                std::cout << "[REDE] Pacote MSG_INIT recebido.\n";
                if (meu_id == 'A') {
                    std::cout << "[SINCRONIZADO] Anel verificado com sucesso! Disparando MSG_START...\n";
                    pacote.tipo_mensagem = MSG_START;
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                } else {
                    std::cout << "[SINCRONIZANDO] Repassando MSG_INIT para o sucessor...\n";
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                }
                break;

            case MSG_START:
                std::cout << "[REDE] Pacote MSG_START recebido!\n";
                if (meu_id != 'A') {
                    std::cout << "Anel SINCRONIZADO. Entrando em modo de jogo!\n";
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                } else {
                    std::cout << "\n==================================================\n";
                    std::cout << " REDE PRONTA E SINCRONIZADA! O JOGO VAI COMECAR.\n";
                    std::cout << "==================================================\n\n";

                    // Inicializa a partida
                    inicializar_partida(&pacote, FICHAS_INICIAIS);
                    pacote.tipo_mensagem = MSG_COLETA_APOSTAS;
                    pacote.id_roleta_atual = 'A';

                    // A maquina A faz sua aposta e passa para o proximo no
                    capturar_aposta_usuario(&pacote, meu_id);
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                }
                break;

            case MSG_COLETA_APOSTAS:
                if (pacote.id_roleta_atual == meu_id) {
                    // O pacote deu a volta e a roleta atual recebeu de volta!
                    std::cout << "\n[ROLETA] Todas as apostas foram coletadas! Girando a roleta...\n";
                    processar_roleta_e_premios(&pacote);

                    pacote.tipo_mensagem = MSG_PAGAMENTO;
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                } else {
                    // Jogador comum: abre terminal de aposta e repassa pro próximo
                    capturar_aposta_usuario(&pacote, meu_id);
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                }
                break;

            case MSG_PAGAMENTO:
                // Todos os nós (Roleta e Jogadores) mostram o placar atualizado

                exibir_placar(&pacote, meu_id);

                if (pacote.id_roleta_atual == meu_id) {
                    // A roleta confirma que todos viram o placar e passa o bastão pro próximo
                    std::cout << "[ROLETA] Placar exibido. Passando a roleta para o proximo no...\n";

                    idx_atual = obter_indice_jogador(meu_id);

                    // a logica funcinona assim:
                    // se eu sou o A idx=0, o proximo e o B idx=1, se eu sou o D idx=3, o proximo e o A idx=0
                    proximo_id = 'A' + ((idx_atual + 1) % 4);

                    pacote.tipo_mensagem = MSG_PASSA_ROLETA;
                    pacote.id_proxima_roleta = proximo_id;
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                } else {
                    // Jogador comum repassa a mensagem de pagamento
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                }
                break;

            case MSG_PASSA_ROLETA:
                pacote.id_roleta_atual = pacote.id_proxima_roleta;

                if (pacote.id_roleta_atual == meu_id) {
                    std::cout << "\n==================================================\n";
                    std::cout << " AGORA VOCE E A ROLETA DA RODADA! (" << meu_id << ")\n";
                    std::cout << "==================================================\n";

                    pacote.tipo_mensagem = MSG_COLETA_APOSTAS;
                    capturar_aposta_usuario(&pacote, meu_id);
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                } else {
                    std::cout << "[SISTEMA] Nova Roleta da rodada: " << pacote.id_roleta_atual << "\n";
                    enviar_pacote(sock_fd, &pacote, ip_sucessor, porta_sucessora);
                }
                break;

            case MSG_OVER:
                std::cout << "[SISTEMA] Pacote MSG_OVER recebido. Encerrando o jogo...\n";
                return 0;

            default:
                std::cout << "[AVISO] Pacote com tipo desconhecido: " << pacote.tipo_mensagem << "\n";
                break;
        }
    }

    return 0;
}