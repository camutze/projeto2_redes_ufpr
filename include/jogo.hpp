#pragma once

#include "protocolo.hpp"

#define FICHAS_INICIAIS 100  // saldo inicial de fichas para cada jogador

// regras da roleta e apostas. Inicialmente as minimas, depois se der tempo fazemos o restante

// Converte o ID ('A', 'B', 'C', 'D') para o indice do array (0, 1, 2, 3)
// para acessar o vetor
int obter_indice_jogador(char id);

// Preenche o saldo inicial de fichas para todos os nós (executado na 1ª rodada)
void inicializar_partida(PacoteAnel* pacote, unsigned int fichas_iniciais);

// Le do teclado a modalidade e o valor da aposta do jogador local
void capturar_aposta_usuario(PacoteAnel* pacote, char meu_id);

// Sorteia o numero da roleta (0 a 36) e calcula os ganhos/perdas de cada nó
void processar_roleta_e_premios(PacoteAnel* pacote);

// Imprime na tela a tabela com o saldo atualizado e resultado da rodada
void exibir_placar(const PacoteAnel* pacote, char meu_id);

// 1 se algum jogador faliu, 0 caso contrario
int alguem_faliu(const PacoteAnel* pacote);