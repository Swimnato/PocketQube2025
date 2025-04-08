#include "radio.h"

#include <LoRa.h>

#define SS 10       // TODO check pin number
#define RESET 9     // TODO check pin number
#define DIO_0 -1    // We will not have this pin so this will disable it
#define TX_POWER 15 // Max input power for FEM (SKY66122-11) is 16dBm
#define FREQ 915E6

#define TXRX 7 // CTX pin for Front End Module
#define FEM 5  // Enable disable pin for Front End Module

#define TRANSMIT_REFRESH_DELAY 60000 // time between transmits in ms
#define RECEIVE_REFRESH_DELAY 10000  // time spent in receive in ms

class RadioManager
{
private:
    radio_st radio_current_st = init_st;

    boolean txComplete = false;

    unsigned long timeSinceLastTransition = 0;

    void stateTransition()
    {
        // State transition switch case
        switch (radio_current_st)
        {
        case init_st:
            // State machine init stuff
            radio_current_st = sleep_st;
            break;
        case sleep_st:
            // Will move to tx if enough time has elapsed else stay in sleep
            if (millis() - timeSinceLastTransition >= TRANSMIT_REFRESH_DELAY || millis() < timeSinceLastTransition)
            {
                // Transition actions, turn on FEM and radio
                // TODO

                timeSinceLastTransition = millis();
                radio_current_st = tx_st;
            }
            break;
        case tx_st:
            // Will transition to RX after transmission is complete
            if (txComplete)
            { // flag set in tx function
                radio_current_st = rx_st;
            }
            break;
        case rx_st:
            // Call RX transition function, will wait in RX for a certain amount of time, then transition
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

    void stateAction()
    {
        // State action switch case
        switch (radio_current_st)
        {
        case init_st:
            // State machine init stuff
            // Doesn't need any actions, already all called in setup
            break;
        case sleep_st:
            // Run sleep actions
            radioSleepAction();
            break;
        case tx_st:
            // Do transmit things
            break;
        case rx_st:
            // Do receive things
            radioRxAction();
            break;
        default:
            // printf("Undefined state transition!");
            break;
        }
    }

    void radioSleepAction()
    {
        LoRa.sleep();
    }

    boolean radioRxTransition()
    {
        if (millis() - timeSinceLastTransition >= RECEIVE_REFRESH_DELAY || millis() < timeSinceLastTransition)
        {
            timeSinceLastTransition = millis();
            return true;
        }

        return false;
    }

    void radioRxAction()
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

public:
    // init stuff here
    void radio_init()
    {

        // init LoRa Library
        LoRa.setPins(SS, RESET, DIO_0);
        LoRa.setTxPower(TX_POWER);
        if (!LoRa.begin(FREQ))
        {
#ifdef DEBUG
            Serial.println("Starting LoRa failed!");
#endif
        }

        // Wont have a callback
        // LoRa.onReceive(onReceive); // Set up callback for receiving

        // Setup FEM pins

        radio_current_st = init_st;
    }

    // Loop function that runs radio state machine
    void radio_loop()
    {
        stateTransition();

        stateAction();
    }
};
