#ifndef _CONTROL_H_
#define _CONTROL_H_

#define DDR_ENGINE    DDRD
#define PORT_ENGINE   PORTD
#define ENGINE_LEFT   PD0
#define ENGINE_RIGHT  PD1
#define ENGINE_ENABLE PD5

#define DDR_DIRECTION DDRA
#define DIRECTION     PORTA
#define DIR_LEFT      PA0
#define DIR_RIGHT     PA1
#define DIR_EN        PA2

volatile uint8_t action;
volatile uint8_t param;

void init_control();
void control_cmd(uint8_t _action, int8_t _param);

#endif /* _CONTROL_H_ */
