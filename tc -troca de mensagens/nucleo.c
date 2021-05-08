#include <system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declaração do tipo mensagem */
typedef struct address {
  int flag;
  char nome_emissor[35];
  char mensa[25];
  struct address *ptr_msg;
} mensagem;

typedef mensagem *PTR_MENSAGEM;

/* Declaração do descritor de processo */
typedef struct desc_p {
  char nome[35];
  enum {ativo, terminado, bloqrec, bloqrecSeletivo, bloqenv, bloq_P} estado;
  PTR_DESC contexto;
  struct desc_p *prox_desc;
  struct desc_p *fila_sem;

  PTR_MENSAGEM ptr_msg;
  int tam_fila;
  int qtde_msg_fila;

  char nomeRecebeSeletivo[35]; /* Armazena o nome do processo do qual se está esperando receber uma mensagem */
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

typedef struct registros {
  unsigned bx1, es1;
} regis;

typedef union k {
  regis x;
  char far *y;
} APONTA_REG_CRIT;

APONTA_REG_CRIT a;

PTR_DESC_PROC PRIM = NULL; /* Aponta para o processo que esta sendo executado */
PTR_DESC d_esc;

/* Semáforo */
typedef struct {
  int s;
  PTR_DESC_PROC Q;
} semaforo;

void insere_Descritor(PTR_DESC_PROC descritor) {
  PTR_DESC_PROC atual;

  if(PRIM == NULL) {
    PRIM = descritor;
    PRIM->prox_desc = PRIM;
    return;
  }

  atual = PRIM;

  while(atual->prox_desc != PRIM) {
    atual = atual->prox_desc;
  }

  /* Atual agora é o último elemento da lista circular */
  atual->prox_desc = descritor;
  descritor->prox_desc = PRIM;
}

PTR_MENSAGEM cria_fila_mensa(int max_fila) {
  PTR_MENSAGEM fila = NULL, novo_elemento;
  int i = 0;

  while(i < max_fila) {
    novo_elemento = (PTR_MENSAGEM) malloc(sizeof(mensagem));
    novo_elemento->flag = 0;
    novo_elemento->ptr_msg = fila;
    fila = novo_elemento;
    i++;
  }

  return fila;
};

void far criar_Processo(char nome_proc[35], void far (*end_proc)(), int max_fila) {
  PTR_DESC_PROC p_aux;

  p_aux = (PTR_DESC_PROC) malloc(sizeof(DESCRITOR_PROC));

  /* Preenchimento do descritor */
  strcpy(p_aux->nome, nome_proc);
  p_aux->estado = ativo;
  p_aux->contexto = cria_desc();
  p_aux->fila_sem = NULL;
  p_aux->tam_fila = max_fila;
  p_aux->qtde_msg_fila = 0;
  p_aux->ptr_msg = cria_fila_mensa(max_fila);
  newprocess(end_proc, p_aux->contexto);

  insere_Descritor(p_aux);
}

PTR_DESC_PROC procura_prox_ativo() {
  PTR_DESC_PROC elemento = PRIM->prox_desc;

  while(elemento != PRIM) {
    if(elemento->estado == ativo) {
      return elemento;
    } else {
      elemento = elemento->prox_desc;
    }
  }

  if(PRIM->estado == ativo) {
    return PRIM;
  } else {
    return NULL;
  }
}

void far volta_dos() {
  disable();
  setvect(8, p_est->int_anterior);
  enable();
  exit(0);
}

void far escalador() {
  p_est->p_origem = d_esc;
  p_est->p_destino = PRIM->contexto;
  p_est->num_vetor = 8;

  /* inicia ponteiro para R.C. do DOS */
  _AH = 0x34;
  _AL = 0x00;
  geninterrupt(0x21);
  a.x.bx1 = _BX;
  a.x.es1= _ES;

  while(1) {
    iotransfer();
    disable();

    if(!*a.y) {
      if((PRIM = procura_prox_ativo()) == NULL) volta_dos();
      p_est->p_destino = PRIM->contexto;
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


/* Implementação - Semáforos */
void far inicia_semaforo(semaforo *sem, int n) {
  sem->s = n;
  sem->Q = NULL;
}

void insereProcessoFilaSemaforo(semaforo *sem, PTR_DESC_PROC processo) {
  PTR_DESC_PROC elementoAtual;

  disable();

  if(sem->Q == NULL) {
    sem->Q = processo;
    return;
  }

  elementoAtual = sem->Q;
  while(elementoAtual->fila_sem != NULL) {
    elementoAtual = elementoAtual->fila_sem;
  }

  elementoAtual->fila_sem = processo;
}

PTR_DESC_PROC removeProcessoFilaSemaforo(semaforo *sem) {
  PTR_DESC_PROC primeiro;

  if(sem->Q == NULL) {
    return NULL;
  }

  primeiro = sem->Q;
  sem->Q = primeiro->fila_sem;

  primeiro->fila_sem = NULL;
  return sem->Q;
}

void far P(semaforo *sem) {
  PTR_DESC_PROC p_aux;
  disable();

  if(sem->s > 0) {
    sem->s = sem->s - 1;
  } else {
    PRIM->estado = bloq_P;
    insereProcessoFilaSemaforo(sem, PRIM);
    p_aux = PRIM;

    if((PRIM = procura_prox_ativo()) == NULL) volta_dos();

    transfer(p_aux->contexto, PRIM->contexto);
  }

  enable();
}

void far V(semaforo *sem) {
  disable();

  if(sem->Q != NULL) {
    (sem->Q)->estado = ativo;
    removeProcessoFilaSemaforo(sem);
  } else {
    sem->s = sem->s + 1;
  }

  enable();
}

/* Localiza o descritor do processo com o nome especificado */
PTR_DESC_PROC procuraProcesso(char *nome) {
  PTR_DESC_PROC d_aux;

  if(strcmp(nome, PRIM->nome) == 0) {
    return PRIM;
  }

  d_aux = PRIM->prox_desc;
  while(d_aux != PRIM) {
    if(strcmp(nome, d_aux->nome) == 0) {
      return d_aux;
    }

    d_aux = d_aux->prox_desc;
  }

  return NULL;
}

/* Localiza uma mensagem com a flag desejada na fila especificada */
PTR_MENSAGEM localizaMensagem(PTR_MENSAGEM fila, int flagDesejada) {
  PTR_MENSAGEM aux = fila;
  while(aux->flag != flagDesejada) {
    aux = aux->ptr_msg;
  }

  return aux;
}

/* Localiza uma mensagem do emissor especificado e com a flag desejada na fila
   Caso nenhuma mensagem seja encontrada, o valor retornado e NULL */
PTR_MENSAGEM localizaMensagemSeletivo(PTR_MENSAGEM fila, char *nome_emissor, int flagDesejada) {
  PTR_MENSAGEM aux = fila;
  while(aux != NULL) {
    if((aux->flag == flagDesejada) && (strcmp(aux->nome_emissor, nome_emissor) == 0)) {
      return aux;
    }
    aux = aux->ptr_msg;
  }

  return NULL;
}

int far envia(char *nome_destino, char *msg) {
  PTR_DESC_PROC d_aux, p_aux;
  PTR_MENSAGEM slot;

  disable();
  d_aux = procuraProcesso(nome_destino);

  if(d_aux == NULL) {
    enable();
    return 1;
  }

  if(d_aux->tam_fila == d_aux->qtde_msg_fila) {
    enable();
    return 2;
  }

  slot = localizaMensagem(d_aux->ptr_msg, 0);
  disable();
  slot->flag = 1;
  strcpy(slot->nome_emissor, PRIM->nome);
  strcpy(slot->mensa, msg);
  (d_aux->qtde_msg_fila)++;

  PRIM->estado = bloqenv;

  if(d_aux->estado == bloqrec) {
    d_aux->estado = ativo;
  }

  if(d_aux->estado == bloqrecSeletivo) {
    if(strcmp(d_aux->nomeRecebeSeletivo, PRIM->nome) == 0) {
      d_aux->estado = ativo;
    }
  }

  p_aux = PRIM;
  if((PRIM = procura_prox_ativo()) == NULL) {
    volta_dos();
  }

  transfer(p_aux->contexto, PRIM->contexto);
  return 0;
}

void far recebe(char *msg, char *p_emissor) {
  PTR_DESC_PROC p_aux, d_aux;
  PTR_MENSAGEM slot;

  disable();
  if(PRIM->qtde_msg_fila == 0) {
    /* Fila vazia */
    PRIM->estado = bloqrec;
    p_aux = PRIM;
    if((PRIM = procura_prox_ativo()) == NULL) {
      volta_dos();
    }
    transfer(p_aux->contexto, PRIM->contexto);
  }
  disable();

  slot = localizaMensagem(PRIM->ptr_msg, 1);
  slot->flag = 0;

  strcpy(p_emissor, slot->nome_emissor);
  strcpy(msg, slot->mensa);

  (PRIM->qtde_msg_fila)--;

  d_aux = procuraProcesso(slot->nome_emissor);

  if(d_aux->estado == bloqenv) {
    d_aux->estado = ativo;
  }

  enable();
}

/* Recebe Seletivo - Para a execução do processo até que exista uma mensagem do processo emissor especificado */
void far recebe_seletivo(char *nome_processo, char *msg, char *p_emissor) {
  PTR_DESC_PROC p_aux, d_aux;
  PTR_MENSAGEM slot;

  disable();

  /* Verifica se já existe uma mensagem do emissor desejado */
  slot = localizaMensagemSeletivo(PRIM->ptr_msg, nome_processo, 1);

  if(slot == NULL) {
    /* Não existe nenhuma mensagem do emissor desejado - então bloqueia o processo atual até que chegue uma mensagem do emissor */
    PRIM->estado = bloqrecSeletivo;

    /* Armazena o nome do processo do qual se deseja obter uma mensagem */
    strcpy(PRIM->nomeRecebeSeletivo, nome_processo);

    /* Passa o controle para outro processo */
    p_aux = PRIM;
    if((PRIM = procura_prox_ativo()) == NULL) {
      volta_dos();
    }
    transfer(p_aux->contexto, PRIM->contexto);
  }

  disable();

  /* Localiza a mensagem do emissor desejado */
  slot = localizaMensagemSeletivo(PRIM->ptr_msg, nome_processo, 1);
  slot->flag = 0;

  /* Armazena os valores nos parâmetros de saída */
  strcpy(p_emissor, slot->nome_emissor);
  strcpy(msg, slot->mensa);

  (PRIM->qtde_msg_fila)--;

  /* Desbloqueia o processo emissor */
  d_aux = procuraProcesso(slot->nome_emissor);
  if(d_aux->estado == bloqenv) {
    d_aux->estado = ativo;
  }

  enable();
}
