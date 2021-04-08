// Created on 21/04/2019
// by Didier Chauveaux

#include "EEPROM_I2C.h"
#include "Arduino.h"
#include "Wire.h"

/*
void EEPROM_I2C::writePage(word addr, byte array[],word size)
{
	int i=0;
  int iSize=size;
	while(iSize-EEPROM_BATCH_WRITE>0)
	{
    //Serial.print("Size "); 
    //Serial.println(iSize);
		Wire.beginTransmission(AT24C256B_ADDR);
		Wire.write(addr>>8); // addr
		Wire.write(addr&0xFF); // addr
		Wire.write(array+i*EEPROM_BATCH_WRITE,EEPROM_BATCH_WRITE);
		Wire.endTransmission();
		delay(EEPROM_DELAY);
		i++;
		iSize-=EEPROM_BATCH_WRITE;
		addr+=EEPROM_BATCH_WRITE;
	}
	if(iSize>0)
	{
    //Serial.print("Size "); 
    //Serial.println(iSize);
		Wire.beginTransmission(AT24C256B_ADDR);
		Wire.write(addr>>8); // addr
		Wire.write(addr&0xFF); // addr
		Wire.write(array+i*EEPROM_BATCH_WRITE,iSize);
		Wire.endTransmission();
		delay(EEPROM_DELAY);
	}
}
*/

void EEPROM_I2C::writePage(word addr, byte array[],word size)
{
  word endAddr;
  word maxSize;
  word begPage;
  word endPage;
  word arWrite=0;
  while (size>0){
    maxSize=min(EEPROM_BATCH_WRITE,size); // determine the max possible size to write
    begPage=addr/EEPROM_PAGE_SIZE; // calculate page where writting starts
    endPage=(addr+maxSize-1)/EEPROM_PAGE_SIZE; // calculate page where writting ends
    if (endPage>begPage) // if the page is different (and therefore higher)
      maxSize=(endPage*EEPROM_PAGE_SIZE)-addr; // the maxSize to write is limited to the end of the page
    Wire.beginTransmission(AT24C256B_ADDR);
    Wire.write(addr>>8); // addr: start  writting
    Wire.write(addr&0xFF); // addr
    Wire.write((byte* )&array[arWrite],maxSize);// from buffer with maxSize
    Wire.endTransmission();
    delay(EEPROM_DELAY); // wait completion of writting within eeprom
    arWrite+=maxSize; // update next address to write
    addr+=maxSize;
    size-=maxSize; // update remaining size to write
  }
}


void EEPROM_I2C::writeAddr(word addr)
{
  Wire.beginTransmission(AT24C256B_ADDR);
  Wire.write(addr>>8);  
  Wire.write(addr&0xFF);
  Wire.endTransmission(); 
  //pos=addr;
}
void EEPROM_I2C::request(word addr,word many)
{
	if(many>EEPROM_BATCH_READ)
	{
		//Serial << "WARNING:\tEEPROM:\trequested more than " << EEPROM_BATCH_READ << "B, giving only " << EEPROM_BATCH_READ << "!!\n";
		many=EEPROM_BATCH_READ;
	}
	writeAddr(addr);
	Wire.requestFrom(AT24C256B_ADDR,many);
}

byte EEPROM_I2C::get(word addr)
{// CHECK: please check this function later
	request(addr,1);
	return Wire.read();
}

void EEPROM_I2C::readPage(word addr, byte array[],word size)
{
  int i=0;
  int iSize=size;
  int arWrite=0;
  while(iSize-EEPROM_BATCH_READ>0)
  {
    //Serial.print("Size "); 
    //Serial.println(iSize);
    Wire.beginTransmission(AT24C256B_ADDR);
    Wire.write(addr>>8); // addr
    Wire.write(addr&0xFF); // addr
    Wire.endTransmission();
    Wire.requestFrom(AT24C256B_ADDR,EEPROM_BATCH_READ);
    for (int b=0;b<EEPROM_BATCH_READ;b++)
    {
      array[arWrite]=Wire.read();
      arWrite++;
    }
    i++;
    iSize-=EEPROM_BATCH_READ;
    addr+=EEPROM_BATCH_READ;
  }
  if(iSize>0)
  {
    //Serial.print("Size "); 
    //Serial.println(iSize);
    Wire.beginTransmission(AT24C256B_ADDR);
    Wire.write(addr>>8); // addr
    Wire.write(addr&0xFF); // addr
    Wire.endTransmission();
    Wire.requestFrom(AT24C256B_ADDR,iSize);
    for (int b=0;b<iSize;b++)
    {
      array[arWrite]=Wire.read();
      arWrite++;
    }
  }
}

//
// END OF FILE
//
