uint16_t rfxx_wrt_cmd(uint16_t aCmd);

void RF02B_SEND(uint8_t data);

void rf12_send(uint8_t data);

void rfxx_init(void);

uint8_t RF01_RDFIFO(void);

uint8_t rf12_recv(void);

void rf12_recv_data(uint8_t *data, uint8_t num);

void rf12_send_data(uint8_t *data, uint8_t num);

void rf02_send_data(uint8_t *data, uint8_t num);

void rf12_init_send(void);

void rf12_init(uint8_t transfer);

void rf01_init(void);

void rf02_init(void);

/* vim: set sts=0 fenc=utf-8: */
