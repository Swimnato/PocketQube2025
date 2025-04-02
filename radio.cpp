#include <LoRa.h>

#define SS 10
#define RESET 9
#define DIO_0 -1    // We will not have this pin so this will disable it
#define TX_POWER 15 // Max input power for FEM (SKY66122-11) is 16dBm
#define FREQ 915E6

#define TXRX 7 // CTX pin for Front End Module
#define FEM 5  // enable disable pin for Front End Module

// Missile state machine states
typedef enum
{
    init_st,
    sleep_st,
    tx_st,
    rx_st,
} radio_st;

radio_st radio_current_st;

void radio_init()
{
    // init stuff here

    // init LoRa Library
    LoRa.setPins(SS, RESET, DIO_0);
    LoRa.setTxPower(TX_POWER);
    if (!LoRa.begin(FREQ))
    {
        Serial.println("Starting LoRa failed!");
    }

    // Setup FEM pins

    radio_current_st = init_st;
}

void radio_loop()
{
    // State transition switch case
    switch (radio_current_st)
    {
    case init_st:
        // State machine init stuff
        radio_current_st = sleep_st;
        break;
    case sleep_st:
        break;
    case tx_st:
        break;
    case rx_st:
        break;
    default:
        // printf("Undefined state transition!");
        break;
    }

    // State action switch case
    switch (radio_current_st)
    {
    case init_st:
        // State machine init stuff
        radio_current_st = sleep_st;
        break;
    case sleep_st:
        // Do sleep things
        break;
    case tx_st:
        // Do transmit things
        break;
    case rx_st:
        // Do receive things
        break;
    default:
        // printf("Undefined state transition!");
        break;
    }
    // Check/Call transmit

    // Check/Call receive
}
