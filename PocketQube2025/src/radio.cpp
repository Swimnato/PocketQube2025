#include "radio.h"

#include "camera.h"

#include <LoRa.h>

#define SS 10               // TODO check pin number
#define RESET 9             // TODO check pin number
#define DIO_0 -1            // We will not have this pin so this will disable it
#define TX_POWER 15         // Max input power for FEM (SKY66122-11) is 16dBm
#define SPREADING_FACTOR 12 // set the spreading factor to 12 to maximize distance
#define FREQ 915E6

#define TXRX 7 // CTX pin for Front End Module
#define FEM 5  // Enable disable pin for Front End Module

#define TRANSMIT_REFRESH_DELAY 60000 // time between transmits in ms
#define RECEIVE_REFRESH_DELAY 10000  // time spent in receive in ms

#define PACKET_SIZE 255 // Size of transmitted packet, max is 255 bytes

class RadioManager
{
private:
    radio_st radio_current_st = init_st;

    boolean txComplete = false;
    byte txBuffer[PACKET_SIZE];

    unsigned long timeSinceLastTransition = 0;

    // Radio state machine transitions
    void stateTransition()
    {
        // State transition switch case
        switch (radio_current_st)
        {
        case init_st:
            // State machine init stuff
            mode_sleep();
            radio_current_st = sleep_st;
            break;
        case sleep_st:
            // Will move to tx if enough time has elapsed else stay in sleep
            if (millis() - timeSinceLastTransition >= TRANSMIT_REFRESH_DELAY || millis() < timeSinceLastTransition)
            {
                mode_tx();

                // Pull the file to transmit

                timeSinceLastTransition = millis();
                radio_current_st = tx_st;
            }
            break;
        case tx_st:
            // Will transition to RX after transmission is complete
            if (txComplete)
            { // flag set in tx function
                mode_rx();
                radio_current_st = rx_st;
            }
            break;
        case rx_st:
            // Call RX transition function, will wait in RX for a certain amount of time, then transition
            if (millis() - timeSinceLastTransition >= RECEIVE_REFRESH_DELAY || millis() < timeSinceLastTransition)
            {
                mode_sleep();
                timeSinceLastTransition = millis();
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

    // Run radio state machine actions
    void stateAction()
    {
        // State action switch case
        switch (radio_current_st)
        {
        case init_st:
            // Doesn't need any actions, already all called in setup
            break;
        case sleep_st:
            // No needed sleep actions all will be in transition actions
            break;
        case tx_st:
            // Do transmit things

            // Put file data in the buffer

            // Transmit the buffer
            LoRa.beginPacket();
            LoRa.write(txBuffer, PACKET_SIZE); // TODO transmitting for end of file (won't be full packet size)
            LoRa.endPacket();

            break;
        case rx_st:
            // Do receive things

#ifdef DEBUG
            // print RSSI of packet
            Serial.print("' with RSSI ");
            Serial.println(LoRa.packetRssi());
#endif
            break;
        default:
#ifdef DEBUG
            Serial.println("Undefined state action!");
#endif
            break;
        }
    }

    // Do all the actions to put the radio to sleep
    void mode_sleep()
    {
        // Sleep mode actions
        digitalWrite(FEM, LOW);  // Turn FEM off
        digitalWrite(TXRX, LOW); // Set tx\rx low

        LoRa.sleep();
    }

    // Do all the actions to put the radio in RX
    void mode_rx()
    {
        // Receive mode setup
        digitalWrite(FEM, HIGH); // Turn FEM on
        digitalWrite(TXRX, LOW); // Set tx\rx low for rx mode

        LoRa.receive();
    }

    // Do all the actions to put the radio in TX
    void mode_tx()
    {
        // Transmit mode setup
        digitalWrite(FEM, HIGH);  // Turn FEM on
        digitalWrite(TXRX, HIGH); // Set tx\rx high for tx mode
    }

public:
    // init stuff here
    void radio_init()
    {
        // init LoRa Library
        // LoRa.setSPI(spi); // Set SPI if not default
        LoRa.setPins(SS, RESET, DIO_0);
        if (!LoRa.begin(FREQ))
        {
#ifdef DEBUG
            Serial.println("Starting LoRa failed!");
#endif
        }

        LoRa.setTxPower(TX_POWER);
        LoRa.setSpreadingFactor(SPREADING_FACTOR);
        LoRa.enableCrc(); // enable Cyclic Redundancy Checks to increase successful packets

        // Wont have a callback
        // LoRa.onReceive(onReceive); // Set up callback for receiving

        // Setup FEM pins
        pinMode(FEM, OUTPUT);
        pinMode(TXRX, OUTPUT);

        radio_current_st = init_st;
    }

    // Loop function that runs radio state machine
    void radio_loop()
    {
        stateTransition();

        stateAction();
    }
};
