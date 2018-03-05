//
//  nRF24L01.hpp
//
//
//  Created by Austyn Larkin on 2/11/18.
//
//

#ifndef nRF24L01_hpp
#define nRF24L01_hpp

#include "NRF24L01Interface.hpp"

namespace nRF24L01 {
    template <class T>
    class Controller: public SpecialPinHolder {
    public:
        Controller(unsigned char CEPin, unsigned char IRQPin, unsigned char CSNPin = 10): _CEPin(CEPin), _IRQPin(IRQPin), _CSNPin(CSNPin), _poweredUp(false), _mode(Mode::None), _ACKEnabled(true) {
            _NRF24L01Interface = new T(static_cast<SpecialPinHolder*>(this));
            // Wait for radio to power on.
            _NRF24L01Interface->delay(100);
            // Begin the SPI and pass this object so SPI can get our interrupt pin
            _NRF24L01Interface->begin();
            getAndClearInterruptBits();
        }
        ~Controller() {
            delete _NRF24L01Interface;
        }
        
        void setPoweredUp(bool shouldPowerUp) {
            if(shouldPowerUp) {
                if(!_poweredUp) {
                    // Power up the transceiver
                    
                    // Read the CONFIG register
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
                    unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
                    _NRF24L01Interface->endTransaction();
                    
                    // Write the CONFIG register with the PWR_UP bit on.
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::CONFIG);
                    _NRF24L01Interface->transferByte(config | Bits::PWR_UP);
                    _NRF24L01Interface->endTransaction();
                    
                    // Wait 1.5ms
                    _NRF24L01Interface->delay(2);
                    
                    _poweredUp = true;
                }
            } else {
                if(_poweredUp) {
                    // Power down the transceiver
                    
                    // Read the CONFIG register
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
                    unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
                    _NRF24L01Interface->endTransaction();
                    
                    // Write the CONFIG register with the PWR_UP bit off.
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::CONFIG);
                    _NRF24L01Interface->transferByte(config & (~Bits::PWR_UP));
                    _NRF24L01Interface->endTransaction();
                    
                    _poweredUp = false;
                }
            }
        }
        
        enum class Mode : unsigned char {
            None = 0,
            PTX = 1,
            PRX = 2
        };
        
        void setPrimaryTransmitter() {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
            unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::CONFIG);
            // Set PRIM_RX to 0 to be a primary transmitter
            _NRF24L01Interface->transferByte(config & (~Bits::PRIM_RX));
            _NRF24L01Interface->endTransaction();
            
            _mode = Mode::PTX;
        }
        
        void setPrimaryReceiver() {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
            unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::CONFIG);
            // Set PRIM_RX to 0 to be a primary transmitter
            _NRF24L01Interface->transferByte(config | Bits::PRIM_RX);
            _NRF24L01Interface->endTransaction();
            
            // Hold CE high
            _NRF24L01Interface->writeCEHigh();
            
            _mode = Mode::PRX;
        }
        
        void setAutoAcknowledgementEnabled(bool enabled) {
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::EN_AA);
            _NRF24L01Interface->transferByte(enabled ? BITS_EN_AA : 0x00);
            _NRF24L01Interface->endTransaction();

        }
        
        // Enable dynamic payload length data pipes. (Requires EN_DPL and ENAA_P0-5)
        void setUsesDynamicPayloadLength(bool uses) {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::DYNPD);
            _NRF24L01Interface->transferByte(uses ? Bits::DPL_P : 0x00);
            _NRF24L01Interface->endTransaction();
            
            // Read the feature register
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::FEATURE);
            unsigned char feature = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::FEATURE);
            _NRF24L01Interface->transferByte( uses ? feature | Bits::EN_DPL : (feature & (~Bits::EN_DPL) ) );
            _NRF24L01Interface->endTransaction();
        }
        
        // Sets the static received packet length. This can be used instead of using dynamic packet lengths.
        void setReceivedPacketLength(unsigned char numberBytes) {
            _receivedPacketLength = numberBytes & 0b00111111;
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::RX_PW_P0);
            _NRF24L01Interface->transferByte( _receivedPacketLength );
            _NRF24L01Interface->endTransaction();
        }
        
        void setAddress(unsigned char address[], unsigned char addressSize) {
            unsigned char storage[5];
            unsigned char *tempAddress = storage;
            
            switch(_mode) {
                case Mode::PTX: {
                    // TODO: Change the address length register
                   
                    storage[0] = address[0];
                    storage[1] = address[1];
                    storage[2] = address[2];
                    storage[3] = address[3];
                    storage[4] = address[4];
                    
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::TX_ADDR);
                    _NRF24L01Interface->transferBytes(&tempAddress, addressSize);
                    _NRF24L01Interface->endTransaction();
                    
                    storage[0] = address[0];
                    storage[1] = address[1];
                    storage[2] = address[2];
                    storage[3] = address[3];
                    storage[4] = address[4];
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::RX_ADDR_P0);
                    _NRF24L01Interface->transferBytes(&tempAddress, addressSize);
                    _NRF24L01Interface->endTransaction();
                    break;
                }
                case Mode::PRX: {
                    storage[0] = address[0];
                    storage[1] = address[1];
                    storage[2] = address[2];
                    storage[3] = address[3];
                    storage[4] = address[4];
                    
                    _NRF24L01Interface->beginTransaction();
                    _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::RX_ADDR_P0);
                    _NRF24L01Interface->transferBytes(&tempAddress, addressSize);
                    _NRF24L01Interface->endTransaction();
                    break;
                }
                default:
                    break;
            }
        }
        
        void setChannel(unsigned char channel) {
            channel = channel & Bits::BITS_RF_CH;
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::REGISTER_RF_CH);
            _NRF24L01Interface->transferByte(channel);
            _NRF24L01Interface->endTransaction();
        }
        
        void sendData(unsigned char *data, unsigned char size, bool noACK = false) {
            // Choose a write command based on whether or not we want an ACK
            unsigned char writeCommand = noACK ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD;//(_ACKEnabled) ? Commands::W_TX_PAYLOAD : Commands::W_TX_PAYLOAD_NO_ACK;
            
            // Fill the TX FIFO with the data.
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(writeCommand);
            _NRF24L01Interface->transferBytes(&data, size);
            _NRF24L01Interface->endTransaction();
            
            // Pulse the CE pin to send.
            _NRF24L01Interface->writeCEHigh();
            _NRF24L01Interface->delayMicroseconds(10);
            _NRF24L01Interface->writeCELow();
        }
        
        unsigned char getNextPacketSize() {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_RX_PL_WID);
            unsigned char packetSize = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            return packetSize;
        }
        
        void readData(unsigned char *dataOut, unsigned char length = 0) {
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_RX_PAYLOAD);
            _NRF24L01Interface->transferBytes(&dataOut, length != 0 ? length : _receivedPacketLength);
            _NRF24L01Interface->endTransaction();
        }
        
        void cancelSending() {
            _NRF24L01Interface->writeCELow();
        }
        
        void test_readAddress(unsigned char address[], unsigned char addressSize) {
            // Read the address back out
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::TX_ADDR);
            _NRF24L01Interface->transferBytes(&address, addressSize);
            _NRF24L01Interface->endTransaction();
        }
        
        unsigned int getStatusAndConfigRegisters() {
            _NRF24L01Interface->beginTransaction();
            unsigned char status = _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
            unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            return (((unsigned int)status) << 8) | ((unsigned int)config);
        }
        
        unsigned char getFIFOStatus() {
            //FIFO_STATUS
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::FIFO_STATUS);
            unsigned char fifo = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            return fifo;
        }
        
        unsigned char getAndClearInterruptBits() {
            const unsigned char mask = (RX_DR | TX_DS | MAX_RT);
            
            // Get the status register
            _NRF24L01Interface->beginTransaction();
            unsigned char status = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            
            // Clear the interrupt bits
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::STATUS);
            _NRF24L01Interface->transferByte(mask);
            _NRF24L01Interface->endTransaction();
            return mask & status;
        }
        
        static const unsigned char INTERRUPT_BIT_RX_DR = 1 << 6;
        static const unsigned char INTERRUPT_BIT_TX_DS = 1 << 5;
        static const unsigned char INTERRUPT_BIT_MAX_RT = 1 << 4;
        
    private:
        // Private member variables
        T *_NRF24L01Interface;
        bool _poweredUp;
        unsigned char _IRQPin;
        unsigned char _CSNPin;
        unsigned char _CEPin;
        unsigned char _receivedPacketLength;
        Mode _mode;
        bool _ACKEnabled;
        
        // Command constants
        enum Commands : unsigned char {
            R_REGISTER = 0b00000000,
            W_REGISTER = 0b00100000,
            
            R_RX_PAYLOAD = 0b01100001,
            W_TX_PAYLOAD = 0b10100000,
            
            FLUSH_TX = 0b11100001,
            FLUSH_RX = 0b11100010,
            REUSE_TX_PL = 0b11100011,
            
            // a Read the size of the top R_RX_PAYLOAD in the FIFO
            R_RX_PL_WID = 0b01100000,
            // a
            W_ACK_PAYLOAD = 0b10101000,
            // a
            W_TX_PAYLOAD_NO_ACK = 0b10110000,
            
            NOP = 0xFF
        };
        
        // Reigster map table
        enum Registers : unsigned char {
            CONFIG = 0x00,
            EN_AA = 0x01,
            
            
            SETUP_AW = 0x03,
            SETUP_RETR = 0x04,
            REGISTER_RF_CH = 0x05,
            RF_SETUP = 0x06,
            STATUS = 0x07,
            OBSERVE_TX = 0x08,
            
            RX_ADDR_P0 = 0x0A,
            TX_ADDR = 0x10,
            
            RX_PW_P0 = 0x11,
            
            FIFO_STATUS = 0x17,
            
            DYNPD = 0x1C,
            FEATURE = 0x1D
        };
        
        // Bit constants
        enum Bits : unsigned char {
            // CONFIG
            MASK_RX_DR = 1 << 6,
            MASK_TX_DS = 1 << 5,
            MASK_MAX_RT = 1 << 4,
            EN_CRC = 1 << 3,
            CRCO = 1 << 2,
            PWR_UP = 1 << 1,
            // 1: PRX, 0: PTX
            PRIM_RX = 1 << 0,
            
            // EN_AA
            BITS_EN_AA = 0b00111111,
            
            // SETUP_AW
            AW = 0b00000011,
            
            // SETUP_RETR
            // a
            ARD = 0b11110000,
            ARC = 0b00001111,
            
            // RF_CH
            BITS_RF_CH = 0b01111111,
            
            // RF_SETUP
            CONT_WAVE = 1 << 7,
            // Bit 6 Reserved
            RF_DR_LOW = 1 << 5,
            PLL_LOCK = 1 << 4,
            RF_DR_HIGH = 1 << 3,
            RF_PWR = 0b00001110,
            
            // STATUS
            RX_DR = 1 << 6,
            TX_DS = 1 << 5,
            MAX_RT = 1 << 4,
            RX_P_NO = 0b00001110,
            TX_FULL__STATUS = 1 << 0,
            
            // OBSERVE_TX
            PLOS_CNT = 0b11110000,
            ARC_CNT = 0b00001111,
            
            // FIFO_STATUS
            TX_REUSE = 1 << 6,
            TX_FULL_FIFO__STATUS = 1 << 5,
            TX_EMPTY = 1 << 4,
            RX_FULL = 1 << 1,
            RX_EMPTY = 1 << 0,
            
            // DYNPD
            DPL_P = 0b00111111,
            
            // FEATURE
            EN_DPL = 1 << 2,
            EN_ACK_PAY = 1 << 1,
            EN_DYN_ACK = 1 << 0
        };
        
        /**
         Used by the SPI interface to get the interrupt pin number

         @return The interrupt pin number passed to the constructor of the Controller.
         */
        unsigned char getIRQPin() const {
            return _IRQPin;
        }
        unsigned char getCEPin() const {
            return _CEPin;
        }
        unsigned char getCSNPin() const {
            return _CSNPin;
        }
    };
}

#endif /* nRF24L01_hpp */


/*
 Interfacing notes
 
 //Add channel support
 /*
 At 2Mbps the channel occupies a bandwidth wider than the resolution of the RF channel frequency setting. To ensure non-overlapping channels in 2Mbps mode, the channel spacing must be 2MHz or more. At 1Mbps and 250kbps the channel bandwidth is the same or lower than the resolution of the RF frequency.
 */


