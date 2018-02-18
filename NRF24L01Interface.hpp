//
//  NRF24L01Interface.hpp
//  
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#ifndef NRF24L01Interface_hpp
#define NRF24L01Interface_hpp

namespace nRF24L01 {
    
    class IRQAndSSHolder {
    public:
        virtual unsigned char getIRQPin() const = 0;
        virtual unsigned char getSSPin() const = 0;
        virtual unsigned char getCEPin() const = 0;
    };
    
    
    class NRF24L01Interface {
    public:
        virtual void begin() = 0;
        virtual void end() = 0;
        
        virtual void beginTransaction() = 0;
        virtual void endTransaction() = 0;
        
        virtual unsigned char transferByte(unsigned char b) = 0;
        virtual void transferBytes(unsigned char **b, unsigned char size) = 0;
        
        virtual void delay(unsigned int d) = 0;
        virtual void delayMicroseconds(unsigned int d) = 0;
        
        NRF24L01Interface(IRQAndSSHolder *s) {
            _IRQPin = s->getIRQPin();
            _SSPin = s->getSSPin();
        }
    protected:
        unsigned char _IRQPin;
        unsigned char _SSPin;
        unsigned char _CEPin;
    };
}

#endif /* NRF24L01Interface_hpp */
