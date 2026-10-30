// structs do pacote, constantes e enums
#pragma once

// definindo os tipos de mensagens
#define MSG_INIT 0   // Fase 1 sincronizar: Teste e fechamento do anel
#define MSG_START 1  // Fase 2 confirmar sincronismo
// mensagens do jogo
#define MSG_COLETA_APOSTAS 2  // coleta as apostas
#define MSG_PAGAMENTO 3       // paga premios/resultados
#define MSG_PASSA_ROLETA 4    // passagem de bastao - importante arrumar o sincronismo

// TIPOS
#define APOSTA_NENHUMA 'X'
#define APOSTA_NUMERO 'N'
#define APOSTA_PRETO 'P'
#define APOSTA_VERMELHO 'V'
#define APOSTA_PAR 'A'
#define APOSTA_IMPAR 'I'

// estado do jogador-  ********************************aqui eu acho que vou colocar fixo as fixa
struct EstadoJogador {
    unsigned int fichas;         // Saldo atual de fichas
    bool apostou;                // se ja apostou na odada
    unsigned int valor_aposta;   // qtd de fichas apostadas
    char tipo_aposta;            // as que estao definidas acima
    unsigned int num_escolhido;  // caso escolheu numero usa, se nao inutil, tenho que pensar mais aqui **********
    int resultado_rodada;        // fim da rodada - usar para fazer o balanco
};


// protocolo do anel
struct PacoteAnel {
    int tipo_mensagem;       // tipo de mensagem que eu estpu enviando no anel
    char id_roleta_atual;    // tipo de roleta que esta enviando a mensagem, para saber se e o meu turno ou nao
    char id_proxima_roleta;  // to usando id, mas o professor recomendou usar  ip ou mac

    // dados fixos para os 4 nós (0='A', 1='B', 2='C', 3='D')
    EstadoJogador jogadores[4];
};