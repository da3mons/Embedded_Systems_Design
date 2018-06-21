/* Projeto Final da Disciplina de Projeto de Sistemas Embarcados*/
#include <asf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "pt.h"
#define N 5		/* Número de amostras */


/* Configuração do USART */
struct usart_module usart_instance;
struct usart_config usart_conf;

/* Declaração das funções */
void configure_eeprom(void);
void temp_low();
void temp_high();
void temp_default();

/* Variáveis Globais */
volatile char temp_lida[50], aux[50];
float temp_res;
float vec_temp[N];
int final_temp;
int ack = 0, set = 0;
static struct pt pt_sender, pt_receiver;

/*
 * ProtoThread para ler temperatura da memória
 * Espera flag que informa que temperatura já foi escrita na memória
 * Atualiza flag para outra ProtoThread
 */
PT_THREAD(sender(struct pt *pt))
{
	PT_BEGIN(pt);
		PT_WAIT_UNTIL(pt, ack == 1);
			eeprom_emulator_read_page(0, aux);
			set = 1;
			ack = 0;
	PT_END(pt);
}

/*
 * ProtoThread para comparar a temperatura
 * Espera flag que informa que temperatura já foi lida da memória
 */
PT_THREAD(receiver(struct pt *pt))
{
	PT_BEGIN(pt);
		PT_WAIT_UNTIL(pt, set == 1);
		set = 0;
		if (final_temp <= 15)
		{
			temp_low();
		}
		else if (final_temp >= 26)
		{
			temp_high();
		}
		else
		{
			temp_default();
		}
	PT_END(pt);
}
	
/* Configuração da memória (paginação) */
void configure_eeprom(void)
{
	/* Setup EEPROM emulator service */
	enum status_code error_code = eeprom_emulator_init();

	if (error_code == STATUS_ERR_NO_MEMORY) {
		while (true) {
			/* No EEPROM section has been set in the device's fuses */
		}
	}
	else if (error_code != STATUS_OK) {
		/* Erase the emulated EEPROM memory (assume it is unformatted or
		 * irrecoverably corrupt) */
		printf("Memory error!!!\n");
		eeprom_emulator_erase_memory();
		eeprom_emulator_init();
	}
}

/* Temperatura menor ou igual a 15°C */
void temp_low()
{
	gfx_mono_init();
	gfx_mono_draw_string("Temp. Ambiente:", 1, 7, &sysfont);
	gfx_mono_draw_string(aux, 93, 7, &sysfont);
	gfx_mono_draw_string(" C", 105, 7, &sysfont);
	gfx_mono_draw_string("Ar Condicionado:", 1, 20, &sysfont);
	gfx_mono_draw_string("23 C", 100, 20, &sysfont);
}

/* Temperatura maior ou igual a 26ºC */
void temp_high()
{
	gfx_mono_init();
	gfx_mono_draw_string("Temp. Ambiente:", 1, 7, &sysfont);
	gfx_mono_draw_string(aux, 93, 7, &sysfont);
	gfx_mono_draw_string(" C", 105, 7, &sysfont);
	gfx_mono_draw_string("Ar Condicionado:", 1, 20, &sysfont);
	gfx_mono_draw_string("20 C", 100, 20, &sysfont);
}

/* Temperatura padrão (entre 15ºC e 26ºC) */
void temp_default()
{
	gfx_mono_init();
	gfx_mono_draw_string("Ar Condicionado", 15, 4, &sysfont);
	gfx_mono_draw_string("Desligado", 32, 14, &sysfont);
	gfx_mono_draw_string("Temp. Ambiente:", 0, 24, &sysfont);
	gfx_mono_draw_string(aux, 93, 25, &sysfont);
	gfx_mono_draw_string("C", 107, 25, &sysfont);
}

int main(void){

	system_init();
		/* Configurações das funções */
		gfx_mono_init();							/*Configura displays */
		usart_get_config_defaults(&usart_conf);		/* Configura USART como default para leitura dos pinos */
		stdio_serial_init(&usart_instance, EDBG_CDC_MODULE, &usart_conf);		/* Inicia comunicação com o Periférico de Temperatura */
		usart_enable(&usart_instance);		/* Habilita USART */
		at30tse_init();						/* Inicia Periférico, já configurado para medição de temperatura */
		configure_eeprom();					/* Configura memória */
	
	int i, i_vec;
	i_vec = 0;
	ack = 0;
	set = 0;	
	
	/* Inicia Threads */
	PT_INIT(&pt_sender);
	PT_INIT(&pt_receiver);
	
	
	while(1)
	{	
		if (i_vec == N){ 		/* Se já atingiu o numero de amostras */
				final_temp = 0;
				i_vec = 0;
				for(i = 0; i < N; ++i){			/* Efetua a média das amostras */
					final_temp += vec_temp[i];
				}
				final_temp = final_temp / N;
				itoa((int)final_temp, temp_lida, 10);		/* Conversão para caractere */
				eeprom_emulator_write_page(0, temp_lida);	/* Salva na memória */
				eeprom_emulator_commit_page_buffer();
				ack = 1;	/* Atualiza flag para as threads */
				
			}else{
				sender(&pt_sender);
				receiver(&pt_receiver);
					
				temp_res = at30tse_read_temperature();	/* Lê sensor de temperatura */
				for(i=0;i<1000000;i++);			/* Tempo de espera até obter próxima amostra */
				vec_temp[i_vec] = temp_res;
				i_vec++;
			}
	}		
}
