#include "nRF24L01.hpp"
#include "ArduinoInterface.hpp"

nRF24L01::Controller<nRF24L01::ArduinoInterface> *n;

// Any variable modified within an interrupt should be marked "volatile"
// (The readData isn't an interrupt, but its called from the interrupt function)
volatile byte lastPacketSize = 0;
volatile unsigned char dataOut[32];
volatile unsigned long bytesReceived = 0;
void readData() {
    // Get the size of the packet thats in the receiving FIFO
    unsigned char packetSize = n->getNextPacketSize();
    // Read the data from the nRF
    n->readData((unsigned char *)dataOut, packetSize);
    // Inrecement the total number of bytes we received
    bytesReceived += packetSize;
}
void nrfInterrupt() {
    // Get and clear the interrupt bits.
    n->readAndClearInterruptBits();
    // Did the nRF receive data that it wants us to read?
    if (n->didReceivePayload()) {
        // Read the data.
        readData();
    }
}

void setup() {
    // Start serial
    Serial.begin(9600);
    // Create the nRF instance.
    // CE pin = 8
    // Interrupt pin = 2
    // Chip select pin (for SPI) = 10
    n = new nRF24L01::Controller<nRF24L01::ArduinoInterface>(8, 2, 10);
    // The nRF object needs to know what interrupt we're using, but we still need to set up our interrupt routine ourselves.
    attachInterrupt(digitalPinToInterrupt(2), nrfInterrupt, FALLING);
    // Power up the transceiver.
    n->setPoweredUp(true);
    // Set this transceiver as a primary receiver
    n->setPrimaryReceiver();
    // Set the address (must match the address of the transmitter)
    unsigned char addr[] = {0x12, 0x34, 0x56, 0x78, 0x9A};
    n->setAddress(addr, 3);


    // Turn off auto acknowledgement and dynamic payload length.
    n->setAutoAcknowledgementEnabled(false);
    n->setUsesDynamicPayloadLength(false);

    // Set the speed to 2Mbps
    n->setBitrate(2);
    // Set the packet size (aka, number of bytes) that the receiver expects to get from the transmitter.
    // The transmitter always sends 32 bytes, so we should expect 32 bytes each time.
    n->setReceivedPacketLength(32);

}
void loop() {
    // Print the last data we received as well as the total amount of data received.
    Serial.print("Last data received: ");
    for (byte i = 0; i < 32; i++) {
        Serial.print((char)dataOut[i]);
    }
    Serial.print(" total bytes received: ");
    Serial.println(bytesReceived);

    // Make sure there isn't data we missed in the internal receiving FIFO.
    if (n->dataInRXFIFO()) {
        readData();
    }
    // Wait a little
    delay(1000);
}
