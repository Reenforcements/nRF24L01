//
//  ArduinoInterface.cpp
//  
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#include <Arduino.h>
#include "ArduinoInterface.hpp"
#include <SPI.h>

namespace nRF24L01 {
    
    void ArduinoInterface::begin() {
        SPI.begin();
        // Convert the pin number to the interrupt
        SPI.usingInterrupt( digitalPinToInterrupt(_IRQPin) );
        pinMode(_SSPin, OUTPUT);
        digitalWrite(_SSPin, HIGH);
    }
    void ArduinoInterface::end() {
        SPI.end();
    }
    
    void ArduinoInterface::beginTransaction() {
        /*
            Up to 10 Mbps, most significant bits first, clock pulses high for writing/reading and changes data on the trailing edge of each clock cycle.
         */
        SPI.beginTransaction( SPISettings(2000000, MSBFIRST, SPI_MODE0) );
        digitalWrite(_SSPin, LOW);
    }
    void ArduinoInterface::endTransaction() {
        digitalWrite(_SSPin, HIGH);
        SPI.endTransaction();
    }
    unsigned char ArduinoInterface::transferByte(unsigned char b) {
        unsigned char result = SPI.transfer(b);
        return result;
    }
    void ArduinoInterface::transferBytes(unsigned char **b, unsigned char size) {
        SPI.transfer(*b, size);
    }
    void ArduinoInterface::delay(unsigned int d) {
        ::delay(d);
    }
    void ArduinoInterface::delayMicroseconds(unsigned int d) {
        ::delayMicroseconds(d);
    }
}
