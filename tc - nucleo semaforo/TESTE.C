#include<nucleo.h>
#include<stdio.h>
FILE *arq;
int buffer[6], mensagem = 0, pont_prod = 0, pont_consum = 0, max = 6;
semaforo mutex, cheio, vazio;
void far deposita(int msg, int prod){
	if((&vazio)->s < 0){
		fprintf(arq, "%s%d%s%d%s", "BUFFER CHEIO: produtor",prod," nao inseriu mensagem ",msg,"\n");
		printf("BUFFER CHEIO: produtor%d nao inseriu mensagem %d", prod, msg);
	} else {
		fprintf(arq, "%s%d%s%d%s", "produtor",prod," inseriu mesagem ",msg,"\n");
		printf("produtor%d inseriu mesagem %d", prod, msg);
		mensagem++;
		buffer[pont_prod++] = msg;
		if(pont_prod > max)
			pont_prod = 0;
	}
}
void far retira(int prod){
	if((&cheio)->s > max){
		fprintf(arq, "%s%d%s", "BUFFER VAZIO: consumidor", prod, "nao retirou mensagem\n");
		printf("BUFFER VAZIO: consumidor%d nao retirou mensagem", prod);
	} else {
		fprintf(arq, "%s%d%s%d%s", "consumidor",prod," retirou mesagem ",buffer[pont_consum],"\n");
		printf("consumidor%d retirou mesagem %d", prod, buffer[pont_consum]);
		pont_consum++;
		if(pont_consum > max)
			pont_consum = 0;
	}
}
void far produtor1(){
	int i = 0;
	while(i < 10){
		p(&vazio);
		p(&mutex);
		deposita(mensagem, 1);
		v(&mutex);
		v(&cheio);
		i++;
	}
	termina_processo1();
}
void far produtor2(){
	int i = 0;
	while(i < 10){
		p(&vazio);
		p(&mutex);
		deposita(mensagem, 2);
		v(&mutex);
		v(&cheio);
		i++;
	}
	termina_processo1();
}
void far consumidor1(){
	int i = 0;
	while(i < 10){
		p(&cheio);
		p(&mutex);
		retira(1);
		v(&mutex);
		v(&vazio);
		i++;
	}
	termina_processo1();
}
void far consumidor2(){
	int i = 0;
	while(i < 10){
		p(&cheio);
		p(&mutex);
		retira(2);
		v(&mutex);
		v(&vazio);
		i++;
	}
	termina_processo1();
}
main(){
	arq = fopen("C:RESUL.txt","w");
	criar_processo(produtor1, "p1");
	criar_processo(produtor2, "p2");
	criar_processo(consumidor1, "c1");
	criar_processo(consumidor2, "c2");
	inicia_semaforo(&cheio, 0);
	inicia_semaforo(&vazio, max);
	inicia_semaforo(&mutex, 1);
	dispara_sistema();
	fclose(arq);
}
