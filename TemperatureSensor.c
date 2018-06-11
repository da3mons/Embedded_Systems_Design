#include <asf.h>


void adc_configure(void);
uint16_t adc_reader(uint8_t adc_num, uint8_t adc_chan);

uint16_t result;	// Recebe a temperatura final
uint16_t cu;
struct adc_module adc_instance;

/*
*	CONFIGURAÇÃO
*/
void adc_configure(void) {
	
	struct	adc_config config_adc;	// Estrutura para configurar ADC
	
	adc_get_config_defaults(&config_adc);		// Configura ADC com valores iniciais
		
	config_adc.clock_source = GCLK_GENERATOR_0;	
	//config_adc.resolution = 12;		// Taxa de conversão (+bits +lento)
	config_adc.positive_input = ADC_POSITIVE_INPUT_TEMP;	/* Configura a entrada positiva para
																	* leitura da temperatura
																	*/
	config_adc.reference = ADC_REFERENCE_INT1V;
	config_adc.negative_input = ADC_NEGATIVE_INPUT_GND;
	/*
	* Inicialização do ADC:
	* 1) Ponteiro para instancia da estutura adc_module	(MODULO ADC)
	* 2) Ponteiro para o modulo ADC (ADC1, ADC2, ACD3, ....)	(SELEÇÃO DO ADC)
	* 3) Ponteiro para instancia da estutura adc_config (CONFIGURAÇÃO ADC)
	*/
	adc_init(&adc_instance, ADC, &config_adc);
	
	/*
	* ADC_AVGCTRL_SAMPLENUM_4: Possivel numero de amostras a serem acumuladas
	* só pode ser usado quando ADC_RESOLUTION_CUSTOM é usada.
	*
	*/
	
	ADC->AVGCTRL.reg = ADC_AVGCTRL_ADJRES(2) | ADC_AVGCTRL_SAMPLENUM_4;
	adc_enable(&adc_instance);	// Habilita o módulo ADC
}


int main(void)
{
	
	system_init();
	
	adc_configure();
	
	adc_start_conversion(&adc_instance);
	

	 // Wait for conversion to be done and read out result 
	 while (adc_read(&adc_instance, &result) == STATUS_BUSY){}

	 cu = 2*result;
	 
	 while (1) {
		
	 }
}
