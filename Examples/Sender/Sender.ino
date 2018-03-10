
#include "nRF24L01.hpp"
#include "ArduinoInterface.hpp"

// Its important to make the pointer to the nRF up here and instantiate the instance in the "setup" function.
nRF24L01::Controller<nRF24L01::ArduinoInterface> *n;

// Any variables that get changed in an interrupt should be marked "volatile".
volatile unsigned char lastInterruptBits = 0;
volatile bool readyForMoreData = true;
// The IRQ pin will output LOW when the nRF has something important for us to handle.
// This interrupt gets triggered when the IRQ pin goes low.
void nrfInterrupt() {
    // Set the CE pin LOW to complete the send.
    n->concludeSendingPacket();
    // Read and clear the interrupt bits.
    n->readAndClearInterruptBits();

    if(n->didSendPayload()) {
        // The payload successfully sent!
        readyForMoreData = true;
    } else if(n->didHitMaxRetry()) {
        // The nRF tried to resend the payload multiple times but the other side never received it.
        // (Note: this should only happen when ACK is enabled on both sides. In this
        //   example program, ACK is disabled, so we'll never get to this condition.)
        readyForMoreData = true;
    }
}

unsigned int totalCount = 0;
void setup() {
    // Create the nRF instance.
    // CE pin = 8
    // Interrupt pin = 2
    // Chip select pin (for SPI) = 10
    n = new nRF24L01::Controller<nRF24L01::ArduinoInterface>(8, 2, 10);
    // The nRF object needs to know what interrupt we're using, but we still need to set up our interrupt routine ourselves.
    attachInterrupt(digitalPinToInterrupt(2), nrfInterrupt, FALLING);
    

    // Power up the transceiver.
    n->setPoweredUp(true);
    // Set this transceiver as the primary transmitter.
    n->setPrimaryTransmitter();
    // Set the address of the transmitter.
    // This address must match on the receiving side.
    unsigned char addr[] = {0x12, 0x34, 0x56, 0x78, 0x9A};
    n->setAddress(addr, 5);
    
    // Turn off auto acknowledgement and dynamic payload length.
    n->setAutoAcknowledgementEnabled(false);
    n->setUsesDynamicPayloadLength(false);

    // Set the speed to 2Mbps
    n->setBitrate(2);
    // ACK is off, so auto retransmit shold also be off since we have no way of knowing packets were received anyways.
    n->setAutoRetransmitCount(0);

    while (true) {
        readyForMoreData = false;
        unsigned char text[32] = "Hello, this is the nRF sending!";
        // Start a sending operation. The operation should finish with our interrupt being called.
        // This method sets the CE pin HIGH, but we need to set it LOW again after the send operation finishes.
        // The interrupt will be called when the operation is finished, so we call "concludeSendingPacket" inside there.
        n->startSendingPacket(text, 32);
        // Wait until "readyForMoreData" is set true inside the interrupt.
        while(readyForMoreData == false);
        delay(1000);
    }
}

