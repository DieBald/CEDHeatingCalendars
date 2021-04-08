#ifndef	_EEPROM_I2C_H
#define	_EEPROM_I2C_H
#include <Arduino.h>


#define	AT24C256B_ADDR 0x50
#define EEPROM_BATCH_WRITE  16
#define EEPROM_BATCH_READ 32
#define EEPROM_DELAY  2
#define EEPROM_PAGE_SIZE 64

class EEPROM_I2C 
{
  public:
    void writePage(word addr, byte array[],word size);
    void writeAddr(word addr);
		void request(word addr,word many);
    void  readPage(word addr, byte array[],word size);
		byte get(word addr);

};

#endif

//
// End of File
//
