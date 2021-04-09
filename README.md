# CEDHeatingCalendars
An Arduino UNO based system intended to control 8 zones On/Off per 30mn slice with pilot wire

Hardware is made of
- Arduino Micro (UNO would work the same)
- DS3231 Clock
- PCF8574 remote IO
- AT24C256B EEPROM
- LCD2004 liquid crystal display
- W5500 Ethernet interface
- 6 push buttons for interacting with system through LCD
All modules except W5500 are connected with 2C bus, W5500 is connected with SPI. All these modules were bought on the internet.
the 6 buttons are connected directly to IO of the arduino.
The Pilot Wire is driven by a triac (Z0607) triggered by an opto-triac (MOC3063) on a classical diagram. No need for filter or heatsink since there is no power consumed on the bus.
In my implementation, I grouped the low voltage electronic in one box, and the 240V electronic in a deported box because it is connected to 240V network and to the Pilot Wires of the heater. I also protected this box with a 100mA fast fuse.

Each of the 8 calendars is driving its Pilot Wire in On/Off mode (that corresponds to normal heating and reduced heating). 
A calendar is made of 53 weeks. One week is made of 7 days * 24 hours * 2 1/2 hours, coded in bytes (one byte = 8 half hours = 2 hours ;)
Date and time are maintained by the DS3231 clock. Week number is calculated from the date and the CED is taking into account (or not according configuration) daylight saving time and date, calculating the right date.
The configuration of the calendars is done by a software running on Windows 10 using the network interface.
Network connection also permits setting date and time, activating daylight saving management, bypassing Pilot wires On or Off and the display of the current status of the CED.
LCD displays various information about current CED state and permits user to navigate through the screens and bypass Pilot Wires.
Bypassing a PW is valid until next change to avoid bad surprise in case of forgetting.

