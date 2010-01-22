// \author Maik Rungberg
#ifndef _SENSOR_H_
#define _SENSOR_H_

volatile uint8_t hinderniss;
volatile uint8_t rwd;

void init_sensor();
uint16_t read_adc(uint8_t channel);
void sensor_irq();

#endif
/* vim: set sts=0 fenc=utf-8: */
