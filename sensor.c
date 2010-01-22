#include <avr/io.h>
#include "sensor.h"

#include "control.h"

uint16_t read_adc(uint8_t channel)
{
	uint8_t i;
	uint16_t result;
 
	ADMUX = channel;                      
	ADMUX |= (1<<REFS1) | (1<<REFS0); // Vref = 2.56V
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0); // Frequenzteiler 32

	// dummy messung
	ADCSRA |= (1<<ADSC);
	while ( ADCSRA & (1<<ADSC) ) {}
	result = ADCW;  

	result = 0; 
	// Eigentliche Messung beginnt jetzt
	for ( i=0; i<4; ++i ) {
		ADCSRA |= (1<<ADSC); // single conversion
		while ( ADCSRA & (1<<ADSC) );
		result += ADCW;	
	}
	ADCSRA &= ~(1<<ADEN);
 
	result /= 4; //Mittelwert und zurueck
	return result;
}

void init_sensor() {

	DDRA &= ~((1 << PA3) & (1 << PA4));

	TCCR0 |= (1 << CS02);
	TIMSK |= (1 << TOIE0 );
  
}


void sensor_irq() {
	//Overflow
	//ADC LESEN
	uint16_t adcvalue;
	uint8_t  erg;

	adcvalue = read_adc(3); //Kanal 3 lesen, SHARP Sensor
	erg = adcvalue/4;  //Durch 4 teilen => Spannung als 8-Bit Wert mal 100 (ohne Komma von 0-255)
	/*   	
		IR-Sensor
		---------

		adcwert * (Uref/1024) = Vout 
		Abstand (cm) y = 22/(Vout-0.13)
		Bremsen bei y < 50cm

		LDR
		---
		
		Hell	-	1K Ohm
		Dunkel	-	500K Ohm
  
	*/
	if ((erg >= 50)) { //ca. 30cm
		hinderniss = 1;
		PORTC |=  (1 << PC3);
		if (!rwd)
			control_cmd('S', 0);
	} else {
		hinderniss = 0;
		PORTC &= ~(1 << PC3);
	}
      
	adcvalue = read_adc(4); //Kanal 4 lesen, LDR
	erg = adcvalue/4; 
	if ((erg >= 130))  {
		PORTC |= (1 << PC0);
	} else {
		PORTC &= ~(1 << PC0);
	}
	PORTC ^= (1 << PC1);
}
/* vim: set sts=0 fenc=utf-8: */
