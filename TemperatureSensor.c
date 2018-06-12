#include <asf.h>


void adc_configure(void);
uint16_t adc_reader(uint8_t adc_num, uint8_t adc_chan);

uint16_t tf;	// Recebe a temperatura final
uint16_t twotf; // teste
float temp;	// Recebe temperatura convertida para Celcius

struct adc_module adc_instance;	/* Estrutura para ADC*/

/*
*	CONFIGURAÇÃO
*/
void adc_configure(void) {
	
	struct	adc_config config_adc;	// Estrutura para configurar ADC
	
	adc_get_config_defaults(&config_adc);		// Configura ADC com valores iniciais
		
	config_adc.clock_source = GCLK_GENERATOR_0;	
	//config_adc.resolution = 12;		// Taxa de conversão (+bits +lento)    <NECESSÁRIO ????? >
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

float calculate_temperature(uint16_t raw_code)
{
 float VADC; /* Voltage calculation using ADC result for Coarse Temp calculation */
 float VADCM; /* Voltage calculation using ADC result for Fine Temp calculation. */
 float INT1VM; /* Voltage calculation for reality INT1V value during the ADC conversion */

 VADC = ((float)raw_code * INT1V_VALUE_FLOAT)/ADC_12BIT_FULL_SCALE_VALUE_FLOAT;

 /* Coarse Temp Calculation by assume INT1V=1V for this ADC conversion */
 coarse_temp = tempR + (((tempH - tempR)/(VADCH - VADCR)) * (VADC - VADCR));

 /* Calculation to find the real INT1V value during the ADC conversion */
 INT1VM = INT1VR + (((INT1VH - INT1VR) * (coarse_temp - tempR))/(tempH - tempR));

 VADCM = ((float)raw_code * INT1VM)/ADC_12BIT_FULL_SCALE_VALUE_FLOAT;

 /* Fine Temp Calculation by replace INT1V=1V by INT1V = INT1Vm for ADC conversion */
 fine_temp = tempR + (((tempH - tempR)/(VADCH - VADCR)) * (VADCM - VADCR));

 return fine_temp;
}

int main(void)
{
	
	system_init();
	
	adc_configure();
	
	adc_start_conversion(&adc_instance);
	

	 /*
	 *	Espera até que conversão seja feita e Resultado salvo em tempetaura
	 */
	 while (adc_read(&adc_instance, &tf) == STATUS_BUSY){}
	
	/* Testa se é possivel modificar valor da variável */ 
	twotf = tf * 2;
	/* Conversão da temperatura para Celcius */
	temp = calculate_temperature(tf);
	printf("\nThe current temperature is = %f degree Celsius", temp);
	
	while (1) {}
}
