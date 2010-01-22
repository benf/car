#ifndef _SENSOR_H_
#define _SENSOR_H_

void init_sensor();
uint16_t read_adc(uint8_t channel);
void sensor_irq();

#endif
