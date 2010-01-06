#define SOFT_SPI  0
#if 1
#define RFXX_nIRQ_PORT  PORTB
#define RFXX_nIRQ_PIN   PINB
#define RFXX_nIRQ       PB2

#else

#define RFXX_nIRQ_PORT  PORTD
#define RFXX_nIRQ_PIN   PIND
#define RFXX_nIRQ       PD2

#endif

#define RFXX_FSK        PB1

#define PORT_SPI        PORTB
#define PIN_SPI         PINB
#define DDR_SPI         DDRB
#define SPI_SS          PB4
#define SPI_MOSI        PB5
#define SPI_MISO        PB6
#define SPI_SCK         PB7

/* vim: set sts=0 fenc=utf-8: */
