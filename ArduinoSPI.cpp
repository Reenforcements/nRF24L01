//
//  ArduinoSPI.cpp
//  
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#include <Arduino.h>
#include "ArduinoSPI.hpp"
#include <SPI.h>

namespace nRF24L01 {
    
    void ArduinoSPI::begin() {
        SPI.begin();
        // Convert the pin number to the interrupt
        SPI.usingInterrupt( digitalPinToInterrupt(_IRQPin) );
        pinMode(_SSPin, OUTPUT);
        digitalWrite(_SSPin, HIGH);
    }
    void ArduinoSPI::end() {
        SPI.end();
    }
    
    void ArduinoSPI::beginTransaction() {
        /*
            Up to 10 Mbps, most significant bits first, clock pulses high for writing/reading and changes data on the trailing edge of each clock cycle.
         */
        SPI.beginTransaction( SPISettings(2000000, MSBFIRST, SPI_MODE0) );
        digitalWrite(_SSPin, LOW);
    }
    void ArduinoSPI::endTransaction() {
        digitalWrite(_SSPin, HIGH);
        SPI.endTransaction();
    }
    unsigned char ArduinoSPI::transferByte(unsigned char b) {
        unsigned char result = SPI.transfer(b);
        return result;
    }
    void ArduinoSPI::transferBytes(unsigned char **b, unsigned char size) {
        SPI.transfer(*b, size);
    }
    void ArduinoSPI::delay(unsigned int d) {
        ::delay(d);
    }
    void ArduinoSPI::delayMicroseconds(unsigned int d) {
        ::delayMicroseconds(d);
    }
}
