#include "radio.h"

#include <LoRa.h>

#define SS 10
#define RESET 9
#define DIO_0 -1    // We will not have this pin so this will disable it
#define TX_POWER 15 // Max input power for FEM (SKY66122-11) is 16dBm
#define FREQ 915E6

#define TXRX 7 // CTX pin for Front End Module
#define FEM 5  // enable disable pin for Front End Module

radio_st radio_current_st = init_st;

#define SLEEP_REFRESH_DELAY 60000
#define RECEIVE_REFRESH_DELAY 10000
unsigned long timeSinceLastTransition = 0;

static boolean radioSleepTransition();
static void radioSleepAction();

static boolean radioRxTransition();
static void radioRxAction();

void radio_init()
{
    // init stuff here

    // init LoRa Library
    LoRa.setPins(SS, RESET, DIO_0);
    LoRa.setTxPower(TX_POWER);
    if (!LoRa.begin(FREQ))
    {
#ifdef DEBUG
        Serial.println("Starting LoRa failed!");
#endif
    }

    LoRa.onReceive(onReceive); // Set up callback for receiving

    // Setup FEM pins

    radio_current_st = init_st;
}

static void stateTransition()
{
    // State transition switch case
    switch (radio_current_st)
    {
    case init_st:
        // State machine init stuff
        radio_current_st = sleep_st;
        break;
    case sleep_st:
        if (radioSleepTransition())
        {
            radio_current_st = tx_st;
        }
        break;
    case tx_st:
        break;
    case rx_st:
        if (radioRxTransition())
        {
            radio_current_st = sleep_st;
        }
        break;
    default:
#ifdef DEBUG
        Serial.println("Undefined state transition!");
#endif
        break;
    }
}

static void stateAction()
{
    // State action switch case
    switch (radio_current_st)
    {
    case init_st:
        // State machine init stuff
        break;
    case sleep_st:
        radioSleepAction();
        break;
    case tx_st:
        // Do transmit things
        break;
    case rx_st:
        radioRxAction();
        break;
    default:
        // printf("Undefined state transition!");
        break;
    }
}

void radio_loop()
{
    stateTransition();

    stateAction();

    // Check/Call transmit

    // Check/Call receive
}

static boolean radioSleepTransition()
{
    if (millis() - timeSinceLastTransition >= SLEEP_REFRESH_DELAY || millis() < timeSinceLastTransition)
    {
        timeSinceLastTransition = millis();
        return true;
    }

    return false;
}

static void radioSleepAction()
{
    LoRa.sleep();
}

static boolean radioRxTransition()
{
    if (millis() - timeSinceLastTransition >= RECEIVE_REFRESH_DELAY || millis() < timeSinceLastTransition)
    {
        timeSinceLastTransition = millis();
        return true;
    }

    return false;
}

static void radioRxAction()
{
    LoRa.receive();
}

void onReceive(int packetSize)
{
// received a packet
#ifdef DEBUG
    Serial.print("Received packet '");
#endif

    // read packet
    for (int i = 0; i < packetSize; i++)
    {
#ifdef DEBUG
        Serial.print(c);
#endif
    }

// print RSSI of packet
#ifdef DEBUG
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
#endif
}