# nRF24L01
An nRF24L01 library for Arduino, built from scratch, as a school project. It's built in such a way that it can easily be ported over to other microcontrollers.

## Summary

  The nRF24L01+ is a transceiver IC that is often built into cheap transceiver modules. These modules are often used with microcontrollers to provide wireless communication between two or more devices. Arduino is the most common microcontroller used, which this library directly supports.

## nRF24L01+ Modules

### Pinout
| Column 1 | Column 2 |
| ---- | ----- |
| V+ | GND |
| CSN | CE |
| MOSI | SCK |
| IRQ | MISO |

##### V+ #####
:  The supply voltage depends on the voltage of the GPIO you intend to connect to the nRF. If you have a microcontroller with `3.3v` IO, you should supply the module with `1.9-3.6v`. If however, you're using a microcontroller with `5v` IO, you should supply the module with `2.7-3.3v`.


## Installation

## Sample Usage

## Methods

## Porting the Library
