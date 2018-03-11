# nRF24L01
An nRF24L01 library for Arduino, built from scratch, as a school project. It's built in such a way that it can easily be ported over to other microcontrollers.

## Summary

  The nRF24L01+ is a transceiver IC that is often built into cheap transceiver modules. These modules are often used with microcontrollers to provide wireless communication between two or more devices. Arduino is the most common microcontroller used, which this library directly supports. Generally, for a two transceiver system, one transceiver is set up as the _primary transmitter_ and the other is set up as the _primary receiver_. The primary receiver can still send data to the transmitter, but there are some requirements when doing this. First, _Automatic acknowledgement packets_ and _ACK payloads_ must be enabled. Second, the receiver can only _reply_ with data after receiving data from the transmitter. It can never initiate its own send operation.

## nRF24L01+ Modules

Here's a picture of a common nRF module. It has an nRF24L01+ chip soldered onto a printed circuit board with a bunch of other necessary components. It also has a long range antenna, which greatly extends the range of the module.

![A picture of an nRF24L01+ module sitting on my desk.](Media/IMG_0818LowQuality.jpg)

The nRF24L01+ claims data rates up to 2Mbps, but I've only been able to achieve about ~0.6Mbps.

### Pinout
| Column 1 | Column 2 |
| ---- | ----- |
| V+ | GND |
| CSN | CE |
| MOSI | SCK |
| IRQ | MISO |

##### V+
The supply voltage depends on the voltage of the GPIO you intend to connect to the nRF. If you have a microcontroller with `3.3v` IO, you should supply the module with `1.9-3.6v`. If however, you're using a microcontroller with `5v` IO, you should supply the module with `2.7-3.3v`.

##### GND
Connect to ground.

##### CE
The CE (Chip Enable) pin is used to tell the nRF to start transmitting or receiving. This can be any digital pin you want.

##### MOSI, MISO, CSN, SCK
Check your [Arduino's pinout for SPI](https://www.arduino.cc/en/Reference/SPI) to see what these pins should be.

The CSN (Chip Select Not) pin is pulled low by SPI to enable the nRF module when it needs to read or write to it. If you're using an Arduino UNO, pin 10 is hard wired for this purpose and used by the SPI library. If you're using a different Arduino, you should look up which digital pin is your SS (slave select) and use that pin.

MOSI and MISO are used for data transfer from the master to the slave and vice versa.

SCK is a clock signal that's used to keep the data transfer in sync.

##### IRQ

This should be connected to an [interrupt on your Arduino](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/). The nRF will output this pin `LOW` when it needs the Arduino to handle an important event. Such events include: "The data send operation successfully completed", "We retried sending the data numerous times, but the other transceiver never received it", or "We received data from a transmitter and you should read it now." The sample code in the "Examples" directory shows an example of how to set this up.

## Installation

Download the library as a zip file (or `git clone` if you'd prefer.) Move the `nRF24L1` folder into the Arduino software's `libraries` directory. Restart the Arduino IDE.

## Sample Usage

Please see the `Sender` and `Receiver` example sketches in the `Examples` directory.

## The Enhanced Shockburst Protocol

The nRF24L01+ sends and receives data using something called `Enhanced Shockburst`, which is a data transmission protocol that does a lot of the heavy lifting for us, making it super easy to send and receive data between two or more microcontrollers. It allows us to send data in convenient packets ranging from 1-32 bytes by merely uploading our data through SPI to the nRF. The nRF automatically handles sending/receiving these packets and will notify us by the IRQ pin of when we should write/read data. 

The receiving nRF can also automatically notify the transmitting nRF that the data was successfully received through a feature called _automatic acknowledge packets_, in which the transmitter will temporarily change to receiving mode so the receiver can send an _ack packet_ saying that it received the data. The receiver can even send a small amount of data back with the ack packet, allowing bidirectional communication.

And much like a computer on a network, each transceiver also has its own address (or addresses) associated with it. In addition to both transceivers being on the same channel, in order to send and receive data, the transmitter must be transmitting to the same address assigned to the receiver. This address can be 3-5 bytes. When a transceiver is a primary receiver, it constantly searches for a valid address in the demodulated signals it receives.

### Packet Structure

Internally, this is what an Enhanced Shockburst packet looks like. The nRF automatically assembles/disassembles these packets for us, so in reality we never touch these ourselves.

| Preamble | Address | Packet Control Field | Payload | CRC | 
| ---- | ----- | ----- | ----- | ----- |
| 1 byte | 3-5 bytes | 9 bits | 1-32 bytes | 1-2 bytes |
| Automatically generated bit sequence that's used by the nRF to synchronize to the incoming stream of bits. | For *transmitters*, this is the address of the receiver we're sending data to. For *receivers*, this is the address that differentiates us from other receivers on the same channel. | These bits are hidden from the user and are used internally for payload length, packet identification, and whether or not to send an ACK upon receiving. | The data that we're sending or receiving. | CRC stands for cyclic redundancy check and helps the nRF figure out if any data was corrupted between being transmitted and received. |  

## Methods

# class `nRF24L01::Controller` 

```
class nRF24L01::Controller
  : public nRF24L01::SpecialPinHolder
```  

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public inline  `[`Controller`](#classn_r_f24_l01_1_1_controller_1af557eb4c8f8ea27f326f3ca04983831a)`(unsigned char CEPin,unsigned char IRQPin,unsigned char CSNPin)` | Controls a single nRF24L01+ module. You should instantiate this in your setup function and store it in a pointer.
`public inline  `[`~Controller`](#classn_r_f24_l01_1_1_controller_1ac9e0ad9db31501f73004471067a45f66)`()` | 
`public inline void `[`setPoweredUp`](#classn_r_f24_l01_1_1_controller_1afe6135efc62a83777f2b223862ad7b4d)`(bool shouldPowerUp)` | Power up or power down the nRF.
`public inline void `[`setPrimaryTransmitter`](#classn_r_f24_l01_1_1_controller_1a9e137e2d0146a74d83cfbed7fc1ee4b8)`()` | Sets this transceiver as a primary transmitter.
`public inline void `[`setPrimaryReceiver`](#classn_r_f24_l01_1_1_controller_1a7ba17cfa8498c8eff95fdcc4608d5431)`()` | Sets this transceiver as a primary receiver.
`public inline void `[`setAutoAcknowledgementEnabled`](#classn_r_f24_l01_1_1_controller_1aaffd15c3217a2b59bac93d37648b5ba6)`(bool enabled)` | Enables or disables auto acknowledgement packets.
`public inline void `[`setUsesDynamicPayloadLength`](#classn_r_f24_l01_1_1_controller_1aa3e5f53a8d9d128949603f85e7bab1b8)`(bool uses)` | Enable or disable dynamic payload length data pipes. (Requires EN_DPL and ENAA_P0-5)
`public inline void `[`setReceivedPacketLength`](#classn_r_f24_l01_1_1_controller_1af87cb3546f4a4aca16786af82b31c7db)`(unsigned char numberBytes)` | Sets the static received packet length. This can be used instead of using dynamic packet lengths.
`public inline void `[`setAddress`](#classn_r_f24_l01_1_1_controller_1a1f68ce5d74aa9ab0cd30b3997c28950b)`(unsigned char address,unsigned char addressSize)` | Sets the internal address of the transceiver. A transmitter and receiver should have the same address.
`public inline void `[`setChannel`](#classn_r_f24_l01_1_1_controller_1ab4725519ead10a47a0959654d3c86969)`(unsigned char channel)` | Sets the channel that the nRF operates on. The channel of the transmitter must match the channel of the receiver.
`public inline void `[`setCRCEnabled`](#classn_r_f24_l01_1_1_controller_1a127dfe2db25033e382680e03b6ff3194)`(bool enabled)` | Enable or disable CRC on incoming data (cyclic redundancy check)
`public inline void `[`setBitrate`](#classn_r_f24_l01_1_1_controller_1a77d8644bf23f5cc1276bbc76104dc11a)`(unsigned char bitrate)` | Sets the data rate of the transceiver
`public inline void `[`setAutoRetransmitCount`](#classn_r_f24_l01_1_1_controller_1aa2b0e98ed2b060797beb2f848442b807)`(unsigned char retryCount)` | Sets the amount of times to try auto retransmitting the packet
`public inline void `[`startSendingPacket`](#classn_r_f24_l01_1_1_controller_1a778085492c7998dd8f4531ff436b6899)`(unsigned char * data,unsigned char size,bool noACK)` | MUST BE PAIRED WITH A CALL TO `concludeSendingPacket` Starts the process of sending a packet using the nRF.
`public inline void `[`concludeSendingPacket`](#classn_r_f24_l01_1_1_controller_1ad4f8ee61183fefee6787a3d5acfeb3c4)`()` | Ends a packet send operation. Call this in your IRQ interrupt.
`public inline unsigned char `[`getNextPacketSize`](#classn_r_f24_l01_1_1_controller_1a94725746f5bb59f0b27e2cabb49cf54a)`()` | Reads the size of the next packeted queued in the receiving queue on the nRF (if there is a next packet)
`public inline void `[`readData`](#classn_r_f24_l01_1_1_controller_1a0701ea733fd3ff4837ca3fcb6ba596bb)`(unsigned char * dataOut,unsigned char length)` | Reads a packet of data from the nRF.
`public inline void `[`flushRXFIFO`](#classn_r_f24_l01_1_1_controller_1a0428e367814d960924f50fee902f384d)`()` | Clears all the data from the RX FIFO
`public inline unsigned int `[`getStatusAndConfigRegisters`](#classn_r_f24_l01_1_1_controller_1a9c915e9626dae0f9883d251556d458a6)`()` | Returns the 8 bit status and 8 bit config registers as a 16 bit unsigned integer.
`public inline unsigned char `[`getFIFOStatus`](#classn_r_f24_l01_1_1_controller_1afb9a2c226273d22a04fc50fcb5b653e1)`()` | Get the status of the FIFO register
`public inline bool `[`dataInRXFIFO`](#classn_r_f24_l01_1_1_controller_1af5fad9e40c1982b88856c2a91ef16880)`()` | Checks the receiving FIFO
`public inline void `[`readAndClearInterruptBits`](#classn_r_f24_l01_1_1_controller_1ae1cc619b240af4a17a51ddbd01d70c2b)`()` | Gets the interrupt bits from the STATUS register and subsequently clears the interrupts bits from the STATUS register. You should call this methods from your interrupt before calling `didReceivePayload`, `didSendPayload`, and `didHitMaxRetry`. You only need to call this once per interrupt trigger though.
`public inline bool `[`didReceivePayload`](#classn_r_f24_l01_1_1_controller_1a7e25537441e27b4b9715e90eb6e9e973)`()` | Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.
`public inline bool `[`didSendPayload`](#classn_r_f24_l01_1_1_controller_1a79863530a7735cd3b772a148db0cfff5)`()` | Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.
`public inline bool `[`didHitMaxRetry`](#classn_r_f24_l01_1_1_controller_1afcafc30b51ff9fcec7cfd27bc24febd7)`()` | Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.

## Members

#### `public inline  `[`Controller`](#classn_r_f24_l01_1_1_controller_1af557eb4c8f8ea27f326f3ca04983831a)`(unsigned char CEPin,unsigned char IRQPin,unsigned char CSNPin)` 

Controls a single nRF24L01+ module. You should instantiate this in your setup function and store it in a pointer.

#### Parameters
* `CEPin` The microcontroller pin hooked up to the CE pin on the nRF. 

* `IRQPin` The microcontroller pin hooked up to the IRQ pin of the nRF. This pin should also be set up as an interrupt so you can handle important events from the nRF as they happen. 

* `CSNPin` The chip select not pin (also called the SS or slave select pin.) This pin is used by SPI to enable the nRF when it wants to send/receive data through SPI. 

#### Returns
An instance of `[Controller](#classn_r_f24_l01_1_1_controller)`.

#### `public inline  `[`~Controller`](#classn_r_f24_l01_1_1_controller_1ac9e0ad9db31501f73004471067a45f66)`()` 

#### `public inline void `[`setPoweredUp`](#classn_r_f24_l01_1_1_controller_1afe6135efc62a83777f2b223862ad7b4d)`(bool shouldPowerUp)` 

Power up or power down the nRF.

#### Parameters
* `shouldPowerUp` `true` to power up or `false` to power down.

#### `public inline void `[`setPrimaryTransmitter`](#classn_r_f24_l01_1_1_controller_1a9e137e2d0146a74d83cfbed7fc1ee4b8)`()` 

Sets this transceiver as a primary transmitter.

#### `public inline void `[`setPrimaryReceiver`](#classn_r_f24_l01_1_1_controller_1a7ba17cfa8498c8eff95fdcc4608d5431)`()` 

Sets this transceiver as a primary receiver.

#### `public inline void `[`setAutoAcknowledgementEnabled`](#classn_r_f24_l01_1_1_controller_1aaffd15c3217a2b59bac93d37648b5ba6)`(bool enabled)` 

Enables or disables auto acknowledgement packets.

#### Parameters
* `enabled` `true` to enable or `false` to disable.

#### `public inline void `[`setUsesDynamicPayloadLength`](#classn_r_f24_l01_1_1_controller_1aa3e5f53a8d9d128949603f85e7bab1b8)`(bool uses)` 

Enable or disable dynamic payload length data pipes. (Requires EN_DPL and ENAA_P0-5)

#### Parameters
* `uses` `true` to enable, `false` to disable

#### `public inline void `[`setReceivedPacketLength`](#classn_r_f24_l01_1_1_controller_1af87cb3546f4a4aca16786af82b31c7db)`(unsigned char numberBytes)` 

Sets the static received packet length. This can be used instead of using dynamic packet lengths.

#### Parameters
* `numberBytes` The number of bytes that this receiver should receive each time from the transmitter.

#### `public inline void `[`setAddress`](#classn_r_f24_l01_1_1_controller_1a1f68ce5d74aa9ab0cd30b3997c28950b)`(unsigned char address,unsigned char addressSize)` 

Sets the internal address of the transceiver. A transmitter and receiver should have the same address.

#### Parameters
* `address` 3-5 bytes 

* `addressSize` The number of bytes in the address

#### `public inline void `[`setChannel`](#classn_r_f24_l01_1_1_controller_1ab4725519ead10a47a0959654d3c86969)`(unsigned char channel)` 

Sets the channel that the nRF operates on. The channel of the transmitter must match the channel of the receiver.

#### Parameters
* `channel` An integer from 0 to 127.

#### `public inline void `[`setCRCEnabled`](#classn_r_f24_l01_1_1_controller_1a127dfe2db25033e382680e03b6ff3194)`(bool enabled)` 

Enable or disable CRC on incoming data (cyclic redundancy check)

#### Parameters
* `enabled` `true` to enable or `false` to disable.

#### `public inline void `[`setBitrate`](#classn_r_f24_l01_1_1_controller_1a77d8644bf23f5cc1276bbc76104dc11a)`(unsigned char bitrate)` 

Sets the data rate of the transceiver

#### Parameters
* `bitrate` 0 for 250kbps, 1 for 1Mbps, and 2 for 2Mbps

#### `public inline void `[`setAutoRetransmitCount`](#classn_r_f24_l01_1_1_controller_1aa2b0e98ed2b060797beb2f848442b807)`(unsigned char retryCount)` 

Sets the amount of times to try auto retransmitting the packet

#### Parameters
* `char` retryCount An integer from 0 - 15

#### `public inline void `[`startSendingPacket`](#classn_r_f24_l01_1_1_controller_1a778085492c7998dd8f4531ff436b6899)`(unsigned char * data,unsigned char size,bool noACK)` 

MUST BE PAIRED WITH A CALL TO `concludeSendingPacket`*** Starts the process of sending a packet using the nRF.

#### Parameters
* `data` The data to send. 

* `size` The number of bytes to send. 

* `noACK` Requires dynamic ACK to be enabled. If enabled, setting this parameter to true will disable ACK for this single packet.

#### `public inline void `[`concludeSendingPacket`](#classn_r_f24_l01_1_1_controller_1ad4f8ee61183fefee6787a3d5acfeb3c4)`()` 

Ends a packet send operation. Call this in your IRQ interrupt.

#### `public inline unsigned char `[`getNextPacketSize`](#classn_r_f24_l01_1_1_controller_1a94725746f5bb59f0b27e2cabb49cf54a)`()` 

Reads the size of the next packeted queued in the receiving queue on the nRF (if there is a next packet)

#### Returns
The number of bytes in the next packet (if there's a packet waiting.)

#### `public inline void `[`readData`](#classn_r_f24_l01_1_1_controller_1a0701ea733fd3ff4837ca3fcb6ba596bb)`(unsigned char * dataOut,unsigned char length)` 

Reads a packet of data from the nRF.

#### Parameters
* `dataOut` The array to hold the data read from the nRF. 

* `length` The length of the array given.

#### `public inline void `[`flushRXFIFO`](#classn_r_f24_l01_1_1_controller_1a0428e367814d960924f50fee902f384d)`()` 

Clears all the data from the RX FIFO

#### `public inline unsigned int `[`getStatusAndConfigRegisters`](#classn_r_f24_l01_1_1_controller_1a9c915e9626dae0f9883d251556d458a6)`()` 

Returns the 8 bit status and 8 bit config registers as a 16 bit unsigned integer.

#### Returns
((status << 8) | config)

#### `public inline unsigned char `[`getFIFOStatus`](#classn_r_f24_l01_1_1_controller_1afb9a2c226273d22a04fc50fcb5b653e1)`()` 

Get the status of the FIFO register

#### Returns
The FIFO register contents

#### `public inline bool `[`dataInRXFIFO`](#classn_r_f24_l01_1_1_controller_1af5fad9e40c1982b88856c2a91ef16880)`()` 

Checks the receiving FIFO

#### Returns
`true` if there's any data in the FIFO, otherwise false.

#### `public inline void `[`readAndClearInterruptBits`](#classn_r_f24_l01_1_1_controller_1ae1cc619b240af4a17a51ddbd01d70c2b)`()` 

Gets the interrupt bits from the STATUS register and subsequently clears the interrupts bits from the STATUS register. You should call this methods from your interrupt before calling `didReceivePayload`, `didSendPayload`, and `didHitMaxRetry`. You only need to call this once per interrupt trigger though.

#### Returns
None, see didReceivePayload, didSendPayload, and didHitMaxRetry

#### `public inline bool `[`didReceivePayload`](#classn_r_f24_l01_1_1_controller_1a7e25537441e27b4b9715e90eb6e9e973)`()` 

Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.

#### Returns
`true` if this is the reason the interrupt was triggered.

#### `public inline bool `[`didSendPayload`](#classn_r_f24_l01_1_1_controller_1a79863530a7735cd3b772a148db0cfff5)`()` 

Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.

#### Returns
`true` if this is the reason the interrupt was triggered.

#### `public inline bool `[`didHitMaxRetry`](#classn_r_f24_l01_1_1_controller_1afcafc30b51ff9fcec7cfd27bc24febd7)`()` 

Call this method after calling `readAndClearInterruptBits` to see if this was the reason the interrupt was triggered.

#### Returns
`true` if this is the reason the interrupt was triggered.

## Porting the Library

The library was designed to be easily ported to other microcontrollers. In order to add support for another microcontroller, create a new class that inherits from and implements all the virtual methods of `NRF24L01Interface`. For an example, please see the `ArduinoInterface` class. The nRF24L01+ uses [SPI mode 0](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Mode_numbers).
