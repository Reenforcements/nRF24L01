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
        
        /**
         Controls a single nRF24L01+ module. You should instantiate this in your setup function and store it in a pointer.

         @param CEPin The microcontroller pin hooked up to the CE pin on the nRF.
         @param IRQPin The microcontroller pin hooked up to the IRQ pin of the nRF. This pin should also be set up as an interrupt so you can handle important events from the nRF as they happen.
         @param CSNPin The chip select not pin (also called the SS or slave select pin.) This pin is used by SPI to enable the nRF when it wants to send/receive data through SPI.
         @return An instance of `Controller`.
         */
        Controller(unsigned char CEPin, unsigned char IRQPin, unsigned char CSNPin = 10): _CEPin(CEPin), _IRQPin(IRQPin), _CSNPin(CSNPin), _poweredUp(false), _mode(Mode::None), _ACKEnabled(true), _lastInterruptBits(0) {
            _NRF24L01Interface = new T(static_cast<SpecialPinHolder*>(this));
            // Wait for radio to power on.
            _NRF24L01Interface->delay(100);
            // Begin the SPI and pass this object so SPI can get our interrupt pin
            _NRF24L01Interface->begin();
            readAndClearInterruptBits();
            flushRXFIFO();
        }
        ~Controller() {
            delete _NRF24L01Interface;
        }
        
        
        /**
         Power up or power down the nRF.

         @param shouldPowerUp `true` to power up or `false` to power down.
         */
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
        
        
        /**
         Sets this transceiver as a primary transmitter.
         */
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
        
        
        /**
         Sets this transceiver as a primary receiver.
         */
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
        
        
        
        /**
         Enables or disables auto acknowledgement packets.

         @param enabled `true` to enable or `false` to disable.
         */
        void setAutoAcknowledgementEnabled(bool enabled) {
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::EN_AA);
            _NRF24L01Interface->transferByte(enabled ? BITS_EN_AA : 0x00);
            _NRF24L01Interface->endTransaction();

        }
        
        
        /**
         Enable or disable dynamic payload length data pipes. (Requires EN_DPL and ENAA_P0-5)

         @param uses `true` to enable, `false` to disable
         */
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
        
        
        /**
         Sets the static received packet length. This can be used instead of using dynamic packet lengths.

         @param numberBytes The number of bytes that this receiver should receive each time from the transmitter.
         */
        void setReceivedPacketLength(unsigned char numberBytes) {
            _receivedPacketLength = numberBytes & 0b00111111;
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::RX_PW_P0);
            _NRF24L01Interface->transferByte( _receivedPacketLength );
            _NRF24L01Interface->endTransaction();
        }
        
        
        /**
         Sets the internal address of the transceiver. A transmitter and receiver should have the same address.

         @param address 3-5 bytes
         @param addressSize The number of bytes in the address
         */
        void setAddress(unsigned char address[], unsigned char addressSize) {
            unsigned char storage[5];
            unsigned char *tempAddress = storage;
            
            //SETUP_AW
            unsigned char newAddressSize = 0b11;
            switch(addressSize) {
                case 3:
                    newAddressSize = 0b01;
                    break;
                case 4:
                    newAddressSize = 0b10;
                    break;
                case 5:
                default:
                    newAddressSize = 0b11;
                    break;
            }
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::SETUP_AW);
            _NRF24L01Interface->transferByte(newAddressSize);
            _NRF24L01Interface->endTransaction();
            
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
        
        /**
         Sets the channel that the nRF operates on. The channel of the transmitter must match the channel of the receiver.

         @param channel An integer from 0 to 127.
         */
        void setChannel(unsigned char channel) {
            channel = channel & Bits::BITS_RF_CH;
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::REGISTER_RF_CH);
            _NRF24L01Interface->transferByte(channel);
            _NRF24L01Interface->endTransaction();
        }
        
        
        /**
         Enable or disable CRC on incoming data (cyclic redundancy check)

         @param enabled `true` to enable or `false` to disable.
         */
        void setCRCEnabled(bool enabled) {
            //EN_CRC
            // Read the CONFIG register
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
            unsigned char config = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            
            // Write the CONFIG register with the PWR_UP bit on.
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::CONFIG);
            _NRF24L01Interface->transferByte(enabled ? config | Bits::EN_CRC : config & (~(Bits::EN_CRC)) );
            _NRF24L01Interface->endTransaction();
        }
        
        
        /**
         Sets the data rate of the transceiver

         @param bitrate 0 for 250kbps, 1 for 1Mbps, and 2 for 2Mbps
         */
        void setBitrate(unsigned char bitrate) {
            unsigned char bits = 0;
            switch(bitrate) {
                case 0:
                    bits = Bits::RF_DR_LOW;
                    break;
                case 1:
                    bits = 0;
                    break;
                case 2:
                default:
                    bits = Bits::RF_DR_HIGH;
                    break;
            }
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::RF_SETUP);
            unsigned char rfsetup = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            
            rfsetup = (rfsetup & (~(Bits::RF_DR_LOW | Bits::RF_DR_HIGH))) | bits;
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::RF_SETUP);
            _NRF24L01Interface->transferByte(rfsetup);
            _NRF24L01Interface->endTransaction();
        }
        
        
        /**
         Sets the amount of times to try auto retransmitting the packet

         @param char retryCount An integer from 0 - 15
         */
        void setAutoRetransmitCount(unsigned char retryCount) {
            //SETUP_RETR
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::SETUP_RETR);
            unsigned char setupretr = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::W_REGISTER | Registers::SETUP_RETR);
            _NRF24L01Interface->transferByte( (setupretr & Bits::ARD) | (retryCount & Bits::ARC) );
            _NRF24L01Interface->endTransaction();
        }
        
        /**
         ***MUST BE PAIRED WITH A CALL TO `concludeSendingPacket`*** 
         Starts the process of sending a packet using the nRF.

         @param data The data to send.
         @param size The number of bytes to send.
         @param noACK Requires dynamic ACK to be enabled. If enabled, setting this parameter to true will disable ACK for this single packet.
         */
        void startSendingPacket(unsigned char *data, unsigned char size, bool noACK = false) {
            // Choose a write command based on whether or not we want an ACK
            unsigned char writeCommand = noACK ? W_TX_PAYLOAD_NO_ACK : W_TX_PAYLOAD;//(_ACKEnabled) ? Commands::W_TX_PAYLOAD : Commands::W_TX_PAYLOAD_NO_ACK;
            
            // Fill the TX FIFO with the data.
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(writeCommand);
            _NRF24L01Interface->transferBytes(&data, size);
            _NRF24L01Interface->endTransaction();
            
            // Pulse the CE pin to send.
            _NRF24L01Interface->writeCEHigh();
        }
        
        
        /**
         Ends a packet send operation. Call this in your IRQ interrupt.
         */
        void concludeSendingPacket() {
            _NRF24L01Interface->writeCELow();
        }
        
        
        /**
         Reads the size of the next packeted queued in the receiving queue on the nRF (if there is a next packet)

         @return The number of bytes in the next packet (if there's a packet waiting.)
         */
        unsigned char getNextPacketSize() {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_RX_PL_WID);
            unsigned char packetSize = _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
            return packetSize;
        }
        
        
        /**
         Reads a packet of data from the nRF.

         @param dataOut The array to hold the data read from the nRF.
         @param length The length of the array given.
         */
        void readData(unsigned char *dataOut, unsigned char length = 0) {
            
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_RX_PAYLOAD);
            _NRF24L01Interface->transferBytes(&dataOut, length != 0 ? length : _receivedPacketLength);
            _NRF24L01Interface->endTransaction();
        }
        
        
        /**
         Clears all the data from the RX FIFO
         */
        void flushRXFIFO() {
            //FLUSH_RX
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::FLUSH_RX);
            _NRF24L01Interface->transferByte(0x00);
            _NRF24L01Interface->endTransaction();
        }
        
        /**
         Returns the 8 bit status and 8 bit config registers as a 16 bit unsigned integer.

         @return ((status << 8) | config)
         */
        unsigned int getStatusAndConfigRegisters() {
            _NRF24L01Interface->beginTransaction();
            unsigned char status = _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::CONFIG);
            unsigned char config = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            return (((unsigned int)status) << 8) | ((unsigned int)config);
        }
        
        
        /**
         Get the status of the FIFO register

         @return The FIFO register contents
         */
        unsigned char getFIFOStatus() {
            //FIFO_STATUS
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::FIFO_STATUS);
            unsigned char fifo = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            return fifo;
        }
        
        
        /**
         Checks the receiving FIFO

         @return `true` if there's any data in the FIFO, otherwise false.
         */
        bool dataInRXFIFO() {
            _NRF24L01Interface->beginTransaction();
            _NRF24L01Interface->transferByte(Commands::R_REGISTER | Registers::FIFO_STATUS);
            unsigned char fifo = _NRF24L01Interface->transferByte(Commands::NOP);
            _NRF24L01Interface->endTransaction();
            return (fifo & 0b00000010) > 0 | (fifo & 0b00000001) == 0;
        }
        
        
        /**
         Gets the interrupt bits from the STATUS register and subsequently clears the interrupts bits from the STATUS register. You should call this methods from your interrupt before calling `didReceivePayload`, `didSendPayload`, and `didHitMaxRetry`. You only need to call this once per interrupt trigger though.

         @return None, see didReceivePayload, didSendPayload, and didHitMaxRetry
         */
        void readAndClearInterruptBits() {
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
            
            _lastInterruptBits = mask & status;
        }
        
        
        /**
         Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.

         @return `true` if this is the reason the interrupt was triggered.
         */
        bool didReceivePayload() {
            return ((_lastInterruptBits & INTERRUPT_BIT_RX_DR) > 0);
        }
        
        /**
         Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.
         
         @return `true` if this is the reason the interrupt was triggered.
         */
        bool didSendPayload() {
            return ((_lastInterruptBits & INTERRUPT_BIT_TX_DS) > 0);
        }
        
        /**
         Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.
         
         @return `true` if this is the reason the interrupt was triggered.
         */
        bool didHitMaxRetry() {
            return ((_lastInterruptBits & INTERRUPT_BIT_MAX_RT) > 0);
        }
        
        static const unsigned char INTERRUPT_BIT_RX_DR = 1 << 6;
        static const unsigned char INTERRUPT_BIT_TX_DS = 1 << 5;
        static const unsigned char INTERRUPT_BIT_MAX_RT = 1 << 4;
        
    private:
        // Private member variables
        T *_NRF24L01Interface;
        bool _poweredUp;
        volatile unsigned char _IRQPin;
        volatile unsigned char _CSNPin;
        volatile unsigned char _CEPin;
        volatile unsigned char _receivedPacketLength;
        volatile unsigned char _lastInterruptBits;
        volatile Mode _mode;
        volatile bool _ACKEnabled;
        
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


