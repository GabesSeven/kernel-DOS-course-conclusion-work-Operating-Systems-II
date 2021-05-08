#include<system.h>
#include<stdio.h>
typedef struct registros{
	unsigned bx1, es1;
}regis;

typedef union k{
	regis x;
	char far *y;
}APONTA_REG_CRIT;

APONTA_REG_CRIT a;

typedef struct desc_p{
        char nomep[35];
        enum{ativo, bloq_p, terminado} estado;
        PTR_DESC contexto;
        struct desc_p *fila_sem; 
        struct desc_p *prox_desc;
}DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;
PTR_DESC_PROC prim = NULL;
PTR_DESC d_esc;

PTR_DESC_PROC far procura_prox_ativo();
void far volta_dos();

typedef struct {
	int s;  
	PTR_DESC_PROC Q; 
}semaforo;

void far inicia_semaforo(semaforo *sem, int n) {
	sem->s = n;
	sem->Q = NULL;
}

void far p(semaforo *sem){
	PTR_DESC_PROC aux;
	disable();
	if(sem->s > 0)
		sem->s--;
	else {
		prim->estado = bloq_p;
		if(sem->Q == NULL){
			sem->Q = prim;
			prim->fila_sem = NULL;	
		}
		else{ 
			aux = sem->Q;
			while(aux->fila_sem != NULL)
				aux = aux->fila_sem;	
			aux->fila_sem = prim;
			prim->fila_sem = NULL;
		}
		aux = prim;
		if((prim = procura_prox_ativo()) == NULL)
			volta_dos();
		transfer(aux->contexto, prim->contexto);
		}
	enable();
}

void far v(semaforo *sem){
	PTR_DESC_PROC aux;
	disable();
	if(sem->Q != NULL){
		sem->Q->estado = ativo;
		aux = sem->Q->fila_sem;
		sem->Q = aux;	
	} else 
		sem->s++;
	enable();	
}

void far criar_processo (void far (*end_proc)(), char nome_proc[]){
	PTR_DESC_PROC p_aux, aux;
	p_aux = (PTR_DESC_PROC) malloc (sizeof(DESCRITOR_PROC));
	if (p_aux == NULL)
	   exit(1);
	
	strcpy (p_aux->nomep, nome_proc);
	p_aux->estado = ativo;
	p_aux->contexto = cria_desc();
	newprocess (end_proc , p_aux->contexto);
	
	if (prim == NULL){
		prim = p_aux;
		prim->prox_desc = NULL;
	} else if(prim->prox_desc == NULL){
		prim->prox_desc = p_aux;
		p_aux->prox_desc = prim;
	} else{
		aux = prim;
		while(aux->prox_desc != prim) 
			aux = aux->prox_desc;
			
		p_aux->prox_desc = prim;
		aux->prox_desc = p_aux;
	}
}

PTR_DESC_PROC far procura_prox_ativo(){
	PTR_DESC_PROC aux;
	aux = prim->prox_desc;
	while(aux->prox_desc != prim){
		if(aux->estado == ativo){
			return aux;
		}	
		else 
			aux = aux->prox_desc;
	}
	return NULL;
} 

void far volta_dos(){
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

void far escalador(){
	p_est->p_origem = d_esc;
	p_est->p_destino = prim->contexto; 
	p_est->num_vetor = 8;
	_AH=0x34;
	_AL=0x00;
	geninterrupt(0x21);
	a.x.bx1=_BX;
	a.x.es1=_ES;
	
	while(1){
		iotransfer();
		disable(); 
		if(!*a.y){
			prim->contexto = p_est->p_destino;
			prim=procura_prox_ativo();
			p_est->p_destino = prim->contexto;
		}
		enable(); 
	}
}

void far dispara_sistema(){ 
	PTR_DESC d_aux;
	d_aux = cria_desc();
	d_esc = cria_desc();
	newprocess(escalador, d_esc);
	transfer (d_aux, d_esc);
}

void far termina_processo1(){
	PTR_DESC_PROC p_aux;
	disable(); 
	prim->estado = terminado;
	p_aux = prim; 
	if((prim = procura_prox_ativo()) == NULL) volta_dos();
	transfer(p_aux->contexto, prim->contexto); 
} 
