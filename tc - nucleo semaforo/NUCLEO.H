#include<system.h>
#include<stdio.h>
typedef struct desc_p{
        char nomep[35];
        enum{ativo, terminado} estado;
        PTR_DESC contexto;
        struct desc_p *prox_desc;
}DESCRITOR_PROC;
typedef DESCRITOR_PROC *PTR_DESC_PROC;
typedef struct {
	int s;    
	PTR_DESC_PROC Q; 
}semaforo;
extern PTR_DESC_PROC far procura_prox_ativo();
extern void far criar_processo(void far (*end_proc)(), char nome_proc[]);
extern void far escalador();
extern void far dispara_sistema();
extern void far termina_processo();
extern void far termina_processo1();
extern void far p(semaforo *sem);
extern void far v(semaforo *sem);
extern void far inicia_semaforo(semaforo *sem, int n);
