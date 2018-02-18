//
//  ArduinoSPI.hpp
//  
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#ifndef ArduinoSPI_hpp
#define ArduinoSPI_hpp

#include "NRF24L01Interface.hpp"

namespace nRF24L01 {
    class ArduinoSPI : public NRF24L01Interface {
    public:
        void begin();
        void end();
        
        void beginTransaction();
        void endTransaction();
        
        unsigned char transferByte(unsigned char b);
        void transferBytes(unsigned char **b, unsigned char size);
        
        void delay(unsigned int d);
        void delayMicroseconds(unsigned int d);
        
        ArduinoSPI(IRQAndSSHolder *s): NRF24L01Interface(s) {
            _IRQPin = s->getIRQPin();
            _SSPin = s->getSSPin();
        }
    private:
        signed char _ssPin = -1;
    };
}

#endif /* ArduinoSPI_hpp */
