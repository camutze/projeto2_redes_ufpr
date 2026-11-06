#include "jogo.hpp"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>  //unicamente para resolver o problema do usuario digitar letras a mais e o programa travar, ignorando o resto do buffe

int obter_indice_jogador(char id) {
    char id_upper = std::toupper(id);  // Converte para maiúscula
    return id_upper - 'A';             // Retorna 0 para 'A', 1 para 'B', 2 para 'C' e 3 para 'D'
}

void inicializar_partida(PacoteAnel* pacote, unsigned int fichas_iniciais) {
    // pacote->sincronizado = true;  // se chego aqui significa que o anel esta sincronizado e posso enviar mensagens de jogo

    for (int i = 0; i < 4; i++) {
        pacote->jogadores[i].fichas = fichas_iniciais;
        pacote->jogadores[i].apostou = false;
        pacote->jogadores[i].valor_aposta = 0;
        pacote->jogadores[i].tipo_aposta = APOSTA_NENHUMA;
        pacote->jogadores[i].num_escolhido = 0;
        pacote->jogadores[i].resultado_rodada = 0;
    }
}

void capturar_aposta_usuario(PacoteAnel* pacote, char meu_id) {
    int idx = obter_indice_jogador(meu_id);
    unsigned int valor = 0, num = 0;
    
    char opcao;

    EstadoJogador* meu_slot = &pacote->jogadores[idx];
    
    std::cout << "\033[2J\033[1;1H";

    std::cout << "\n==================================================\n";
    std::cout << " SUA VEZ DE APOSTAR (Saldo atual: " << meu_slot->fichas << " fichas)\n";
    std::cout << "==================================================\n";

    do {
        std::cout << "Modalidades de Aposta:\n";
        std::cout << "  [N] Numero exato (Paga 35x)\n";
        std::cout << "  [P] Cor Preta (Paga 1x)\n";
        std::cout << "  [V] Cor Vermelha (Paga 1x)\n";
        std::cout << "  [A] Numero Par (Paga 1x)\n";
        std::cout << "  [I] Numero Impar (Paga 1x)\n";
        std::cout << "  [X] Nao apostar nesta rodada\n";
        std::cout << "Escolha a modalidade: ";

        // validando o usuario para nao ter que ficar sincronizando o anel toda vez que o usuario erra
        std::cin >> opcao;
        // LIMPEZA DO BUFFER: Remove qualquer letra extra que o usuário tenha digitado por erro
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        opcao = std::toupper(opcao);

    } while (opcao != 'N' && opcao != 'P' && opcao != 'V' && opcao != 'A' && opcao != 'I' && opcao != 'X');

    if (opcao == 'X') {
        meu_slot->tipo_aposta = APOSTA_NENHUMA;
        meu_slot->valor_aposta = 0;
        meu_slot->apostou = true;
        std::cout << "[JOGO] Voce optou por nao apostar.\n";
        return;
    }

    do {
        std::cout << "Informe o valor da aposta (1 a " << meu_slot->fichas << "): ";
        std::cin >> valor;
    } while (valor < 1 || valor > meu_slot->fichas);

    meu_slot->valor_aposta = valor;
    meu_slot->tipo_aposta = opcao;

    if (opcao == 'N') {
        do {
            std::cout << "Escolha o numero (0 a 36): ";
            std::cin >> num;
        } while (num > 36);
        meu_slot->num_escolhido = num;
    }

    meu_slot->apostou = true;
    std::cout << "[JOGO] Aposta registrada com sucesso!\n";
}

// https://observador.pt/prognosticos/roleta-europeia/
//  Funcao auxiliar para identificar numeros vermelhos na roleta europeia
static bool e_vermelho(int num) {
    int vermelhos[] = {1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36};
    for (int v : vermelhos)
        if (v == num) return true;
    return false;
}

void processar_roleta_e_premios(PacoteAnel* pacote) {
    // Sorteia um numero de 0 a 36
    std::srand(std::time(nullptr));
    int sorteado = std::rand() % 37;

    std::cout << "\n==================================================\n";
    std::cout << " ROLETA GIRANDO... NUMERO SORTEADO: " << sorteado << "\n";
    if (sorteado == 0) {
        std::cout << " Cor: VERDE (Zero)\n";
    } else {
        std::cout << " Cor: " << (e_vermelho(sorteado) ? "VERMELHO" : "PRETO") << "\n";
        std::cout << " Paridade: " << (sorteado % 2 == 0 ? "PAR" : "IMPAR") << "\n";
    }
    std::cout << "==================================================\n";

    for (int i = 0; i < 4; i++) {
        EstadoJogador* j = &pacote->jogadores[i];

        if (j->tipo_aposta == APOSTA_NENHUMA || j->valor_aposta == 0) {
            j->resultado_rodada = 0;
            continue;
        }

        bool ganhou = false;
        unsigned int multiplicador = 1;

        if (j->tipo_aposta == APOSTA_NUMERO && j->num_escolhido == (unsigned int)sorteado) {
            ganhou = true;
            multiplicador = 35;
        } else if (sorteado != 0) {  // O Zero faz perder apostas de cores/paridade
            if (j->tipo_aposta == APOSTA_VERMELHO && e_vermelho(sorteado))
                ganhou = true;
            else if (j->tipo_aposta == APOSTA_PRETO && !e_vermelho(sorteado))
                ganhou = true;
            else if (j->tipo_aposta == APOSTA_PAR && (sorteado % 2 == 0))
                ganhou = true;
            else if (j->tipo_aposta == APOSTA_IMPAR && (sorteado % 2 != 0))
                ganhou = true;
        }

        if (ganhou) {
            int ganho = j->valor_aposta * multiplicador;
            j->fichas += ganho;
            j->resultado_rodada = ganho;
        } else {
            j->fichas -= j->valor_aposta;
            j->resultado_rodada = -((int)j->valor_aposta);
        }

        // Reseta o estado para a proxima rodada
        j->apostou = false;
    }
}

void exibir_placar(const PacoteAnel* pacote, char meu_id) {
    char id_jogador;

    std::cout << "\n--------------------------------------------------\n";
    std::cout << " PLACAR DA RODADA (Roleta Atual: " << pacote->id_roleta_atual << ")\n";
    std::cout << "--------------------------------------------------\n";

    for (int i = 0; i < 4; i++) {
        id_jogador = 'A' + i;
        const EstadoJogador* j = &pacote->jogadores[i];

        std::cout << " Jogador " << id_jogador;
        if (id_jogador == meu_id) std::cout << " (VOCE)";  // Marca o jogador atual
        std::cout << ": Saldo = " << j->fichas << " fichas | ";

        if (j->resultado_rodada > 0) {
            std::cout << "Resultado: +" << j->resultado_rodada << " fichas!\n";
        } else if (j->resultado_rodada < 0) {
            std::cout << "Resultado: " << j->resultado_rodada << " fichas\n";
        } else {
            std::cout << "Resultado: 0 (Nao apostou)\n";
        }
    }
    std::cout << "--------------------------------------------------\n\n";
}

int alguem_faliu(const PacoteAnel* pacote) {
    for (int i = 0; i < 4; i++)
        if (pacote->jogadores[i].fichas == 0) return 1;  // Retorna 1 se algum jogador faliu

    return 0;
}
