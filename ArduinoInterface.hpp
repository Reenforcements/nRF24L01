//
//  ArduinoInterface.hpp
//  
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#ifndef ArduinoInterface_hpp
#define ArduinoInterface_hpp

#include "NRF24L01Interface.hpp"

namespace nRF24L01 {
    class ArduinoInterface : public NRF24L01Interface {
    public:
        void begin();
        void end();
        
        void beginTransaction();
        void endTransaction();
        
        unsigned char transferByte(unsigned char b);
        void transferBytes(unsigned char **b, unsigned char size);
        
        void delay(unsigned int d);
        void delayMicroseconds(unsigned int d);
        
        ArduinoInterface(IRQAndSSHolder *s): NRF24L01Interface(s) {
            _IRQPin = s->getIRQPin();
            _SSPin = s->getSSPin();
        }
    private:
        signed char _ssPin = -1;
    };
}

#endif /* ArduinoInterface_hpp */
