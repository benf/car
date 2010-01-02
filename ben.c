#include <avr/io.h>
#include <stdint.h>

uint16_t a2d(uint8_t mux)
{
				uint8_t i;
				uint16_t result;

				ADMUX = mux;                      // Kanal waehlen
				ADMUX |= (1<<REFS1) | (1<<REFS0); // interne Referenzspannung nutzen

				ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);    // Frequenzvorteiler 
				// setzen auf 8 (1) und ADC aktivieren (1)
				/* nach Aktivieren des ADC wird ein "Dummy-Readout" empfohlen, man liest
					 also einen Wert und verwirft diesen, um den ADC "warmlaufen zu lassen" */
				ADCSRA |= (1<<ADSC);              // eine ADC-Wandlung 
				while ( ADCSRA & (1<<ADSC) ) {
								;     // auf Abschluss der Konvertierung warten 
				}
				result = ADCW;  // ADCW muss einmal gelesen werden,
				// sonst wird Ergebnis der nächsten Wandlung
				// nicht übernommen.

				/* Eigentliche Messung - Mittelwert aus 4 aufeinanderfolgenden Wandlungen */
				result = 0; 
				for( i=0; i<4; i++ )
				{
								ADCSRA |= (1<<ADSC);            // eine Wandlung "single conversion"
								while ( ADCSRA & (1<<ADSC) ) {
												;   // auf Abschluss der Konvertierung warten
								}
								result += ADCW;		    // Wandlungsergebnisse aufaddieren
				}
				ADCSRA &= ~(1<<ADEN);             // ADC deaktivieren (2)

				result /= 4;                     // Summe durch vier teilen = arithm. Mittelwert

				return result;
}

void setLED(uint8_t p)
{
	PORTD |= (p << DDD5);
}

int main (void) {
	DDRD = 0xff;
	PORTD = 0;
	PORTD |= 1 << DDD5;
	//OCR1A = 500;
	//TCCR1A = (1 << COM1A1) | (1 << WGM12) | (1 << WGM11) | (1 << WGM10);
	//TCCR1B = (1 << CS10);

	uint16_t value = 0;
	
	while (1)
	{
		value = a2d(0);
		if(value > 512)
			setLED(1);
		else
			setLED(0);
			
	}
	return 0;
}

