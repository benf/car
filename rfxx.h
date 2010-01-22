// \author Benjamin Franzke
#ifndef _RFXX_H_
#define _RFXX_H_

void rfxx_init(void);

void rf12_init(uint8_t transfer);

uint16_t rfxx_wrt_cmd(uint16_t cmd);

void rf12_send(uint8_t data);

uint8_t rf12_recv(void);

void rf12_recv_data(uint8_t *data, uint8_t num);

void rf12_send_data(uint8_t *data, uint8_t num);

#if 0
void RF02B_SEND(uint8_t data);
uint8_t RF01_RDFIFO(void);



void rf02_send_data(uint8_t *data, uint8_t num);

void rf12_init_send(void);


void rf01_init(void);

void rf02_init(void);
#endif

#endif /* _RFXX_H_ */
/* vim: set sts=0 fenc=utf-8: */
