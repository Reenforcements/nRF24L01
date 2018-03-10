# nRF24L01
An nRF24L01 library for Arduino, built from scratch, as a school project. It's built in such a way that it can easily be ported over to other microcontrollers.

## Summary

  The nRF24L01+ is a transceiver IC that is often built into cheap transceiver modules. These modules are often used with microcontrollers to provide wireless communication between two or more devices. Arduino is the most common microcontroller used, which this library directly supports.

## nRF24L01+ Modules

Here's a picture of a common nRF module. It has an nRF24L01+ chip soldered onto a printed circuit board with a bunch of other necessary components. It also has a long range antenna, which greatly extends the range of the module.

![A picture of an nRF24L01+ module sitting on my desk.](Media/IMG_0818LowQuality.jpg)

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

This should be connected to an [interrupt on your Arduino](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/). If you need help, please see the examples in the "Examples" directory.

## Installation

Download the library as a zip file (or `git clone` if you'd prefer.) Move the `nRF24L1+` folder into the Arduino software's `libraries` directory. Restart the Arduino IDE.

## Sample Usage

## Methods

## Porting the Library
