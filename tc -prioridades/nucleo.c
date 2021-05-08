#include <system.h>
#include <stdio.h>
#include <stdlib.h>

/* Declaração do descritor de processo */
typedef struct desc_p {
  char nome[35];
  enum {ativo, terminado} estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
  int prioridade; /* Prioridade do processo, variando de 0 (menor prioridade) até 4 (maior prioridade) */
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

/* Definição dos ponteiros para região crítica do DOS */
typedef struct registros {
  unsigned bx1, es1;
} regis;

typedef union k {
  regis x;
  char far *y;
} APONTA_REG_CRIT;

APONTA_REG_CRIT a;


PTR_DESC_PROC PRIM = NULL; /* Aponta para o processo que esta sendo executado */
PTR_DESC d_esc; /* Co-rotina do escalador */

PTR_DESC_PROC lista_processo[5] = {NULL, NULL, NULL, NULL, NULL}; /* Listas de processos, separados por prioridade */

/* Insere o descritor de processo no final da lista correspondente à sua prioridade */
void insere_Descritor(PTR_DESC_PROC descritor) {
  PTR_DESC_PROC fila = lista_processo[descritor->prioridade];
  descritor->prox_desc = NULL;
  if(fila == NULL) {
    lista_processo[descritor->prioridade] = descritor;
    return;
  }

  while(fila->prox_desc != NULL) {
    fila = fila->prox_desc;
  }

  /* Fila agora aponta para o último elemento da lista */
  fila->prox_desc = descritor; /* Insere o descritor no final da lista */
}

void far criar_Processo(char nome_proc[35], void far (*end_proc)(), int prioridade) {
  PTR_DESC_PROC p_aux;

  p_aux = (PTR_DESC_PROC) malloc(sizeof(DESCRITOR_PROC));
  if(p_aux == NULL)
    exit(1);

  /* Ajusta as prioridades de acordo com o limite inferior e o limite superior */
  if(prioridade < 0) {
    prioridade = 0;
  } else if(prioridade > 4) {
    prioridade = 4;
  }

  /* Preenchimento do descritor */
  strcpy(p_aux->nome, nome_proc);
  p_aux->estado = ativo;
  p_aux->contexto = cria_desc();
  p_aux->prioridade = prioridade;

  /* Como a lista não é mais circular, o ponteiro para o próximo descritor deve ser inicializado com NULL */
  p_aux->prox_desc = NULL;
  newprocess(end_proc, p_aux->contexto);

  insere_Descritor(p_aux);
}

/* Retorna o próximo processo ativo na lista de prioridade atual */
PTR_DESC_PROC procura_prox_ativo(PTR_DESC_PROC inicio) {
  PTR_DESC_PROC elemento = inicio;

  while(elemento != NULL) {
    if(elemento->estado == ativo) {
      return elemento;
    } else {
      elemento = elemento->prox_desc;
    }
  }

  return NULL;
}

void far volta_dos() {
  disable();
  setvect(8, p_est->int_anterior);
  enable();
  exit(0);
}

/* Armazena a lista que possui índice indicado por indice2 no final da lista indicada por indice1 */
void far junta_listas(int indice1, int indice2) {
  PTR_DESC_PROC ultimo;

  if(lista_processo[indice1] == NULL) {
    lista_processo[indice1] = lista_processo[indice2];
    return;
  }

  ultimo = lista_processo[indice1];
  while(ultimo->prox_desc != NULL) {
    ultimo = ultimo->prox_desc;
  }

  ultimo->prox_desc = lista_processo[indice2];
}

void far escalador() {
  int i, indice = 4;
  int prioridadeAtual; /* Armazena o número da lista de processos que está em execução no momento */
  PTR_DESC_PROC aux, inicio, proximo;
  p_est->p_origem = d_esc;
  p_est->num_vetor = 8;

  /* Verifica o primeiro processo com a maior prioridade e o armazena em PRIM */
  while(lista_processo[indice] == NULL) {
    indice--;
  }
  PRIM = lista_processo[indice];
  prioridadeAtual = indice;

  p_est->p_destino = PRIM->contexto;

  /* inicia ponteiro para R.C. do DOS */
  _AH = 0x34;
  _AL = 0x00;
  geninterrupt(0x21);
  a.x.bx1 = _BX;
  a.x.es1= _ES;

  while(1) {
    iotransfer();
    disable();

    if(!*a.y) { /* Troca o processo se não está em uma região crítica do DOS */
      aux = procura_prox_ativo(PRIM->prox_desc); /* Procura um processo ativo na mesma fila de prioridade do processo atual */
      if(aux != NULL) {
        /* Como existe outro processo ativo na fila atual, simplesmente troca o processo para ele */
        PRIM = aux;
        p_est->p_destino = PRIM->contexto;
      } else {
        /* Não existe outro processo ativo na mesma lista após o processo atual, então é necessário passar para a próxima lista */
        do {
          if(prioridadeAtual != 0) {
            junta_listas(prioridadeAtual - 1, prioridadeAtual);
            lista_processo[prioridadeAtual] = NULL;
            prioridadeAtual--;
          } else { /* Está na lista com menor prioridade, não há como mudar para outra lista */
            /* aux = procura_prox_ativo(PRIM->prox_desc); */
            aux = NULL;
            break;
          }
        } while((aux = procura_prox_ativo(lista_processo[prioridadeAtual])) == NULL);

        if(aux != NULL) {
          PRIM = aux;
          p_est->p_destino = PRIM->contexto;
        } else {
          /* Nenhum processo restante na lista de menor prioridade - restaura as listas */

          inicio = lista_processo[0];

          /* Zera todas as listas */
          i = 0;
          while(i < 5) {
            lista_processo[i] = NULL;
            i++;
          }

          /* Reinsere todos os elementos */
          while(inicio != NULL) {
            proximo = inicio->prox_desc;
            inicio->prox_desc = NULL;
            insere_Descritor(inicio);

            inicio = proximo;
          }

          /* Busca um descritor ativo com maior prioridade existente */
          prioridadeAtual = 4;

          while( ((aux = procura_prox_ativo(lista_processo[prioridadeAtual])) == NULL) && (prioridadeAtual != 0) ) {
            prioridadeAtual--;
          }

          if(aux != NULL) {
            PRIM = aux;
            p_est->p_destino = PRIM->contexto;
          } else {
            volta_dos();
          }
        }
      }
      /*
      if((PRIM = procura_prox_ativo()) == NULL) volta_dos();
      p_est->p_destino = PRIM->contexto;
      */
    }

    enable();
  }
}

void far dispara_sistema() {
  PTR_DESC d_aux;
  d_aux = cria_desc();
  d_esc = cria_desc();
  newprocess(escalador, d_esc);
  transfer(d_aux, d_esc);
}

void far terminar_Processo() {
  disable();
  PRIM->estado = terminado;
  enable();
  while(1);
}
