#include <LoRa.h>

#define SS 10
#define RESET 9
#define DIO0 -1 // We will not have this pin so this will disable it

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
    if (!LoRa.begin(915E6))
    {
        Serial.println("Starting LoRa failed!");
    }
    LoRa.setPins(SS, RESET, DIO0);

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
