#include "SparkFunCCS811.hpp"
#include "cox.h"

CCS811Core::CCS811Core(TwoWire &W, uint8_t addr) : Wire(W), I2CAddress(addr) {
}

CCS811Core::status CCS811Core::readRegister(uint8_t offset, uint8_t* outputPointer){
	//Return value
	uint8_t result=0;
	uint8_t numBytes = 1;

	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(offset);
	this->Wire.endTransmission();
	// slave may send less than requeste
	// receive a byte as a proper uint8_t
	this->Wire.requestFrom(I2CAddress, numBytes);
	while(this->Wire.available()){
		result = this->Wire.read();
	}
	*outputPointer = result;

	return SENSOR_SUCCESS;
}


CCS811Core::status CCS811Core::multiReadRegister(uint8_t offset, uint8_t *outputPointer, uint8_t length){
	//define pointer that will point to the external space
	uint8_t i = 0;
	uint8_t c = 0;
	//Set the address
	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(offset);
	this->Wire.endTransmission();
	// request 6 bytes from slave device
	this->Wire.requestFrom(I2CAddress, 4);
	while ((this->Wire.available()) && (i < length)){
	// slave may send less than requested
	// receive a byte as character
		c = this->Wire.read();
		*outputPointer = c;
		outputPointer++;
		i++;
	}
	return SENSOR_SUCCESS;
}


CCS811Core::status CCS811Core::writeRegister(uint8_t offset, uint8_t dataToWrite) {
	//offset -- register to write
	//dataToWrite -- 8 bit data to write to register
	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(offset);
	this->Wire.write(dataToWrite);
	this->Wire.endTransmission();

	return SENSOR_SUCCESS;
}

CCS811Core::status CCS811Core::multiWriteRegister(uint8_t offset, uint8_t *inputPointer, uint8_t length){
	//offset -- register to read
	//*inputPointer -- Pass &variable (base address of) to save read data to
	//length -- number of bytes to read
	//define pointer that will point to the external space
	uint8_t i = 0;
	//Set the address
	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(offset);
	// receive a byte as character
	while ( i < length ){
		this->Wire.write(*inputPointer);
		inputPointer++;
		i++;
	}
	this->Wire.endTransmission();

	return SENSOR_SUCCESS;
}

CCS811::CCS811(TwoWire &W, uint8_t addr) : CCS811Core(W, addr){
	refResistance = 10000;
	resistance = 0;
	temperature = 0;
	tVOC = 0;
	CO2e = 0;
}


CCS811Core::status CCS811::begin( void ){
	//Reset key
	this->Wire.begin();
	uint8_t data[4] = {0x11,0xE5,0x72,0x8A};

	multiWriteRegister(CSS811_SW_RESET, data, 4);

	delay(20);
	//Write 0 bytes to this register to start app
	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(CSS811_APP_START);
	this->Wire.endTransmission();
	//Read every second
	setDriveMode(1);

	return SENSOR_SUCCESS;
}

CCS811Core::status CCS811::readAlgorithmResults( void ){
	uint8_t data[4];
	uint8_t i = 0;
	//Set the address
	this->Wire.beginTransmission(I2CAddress);
	this->Wire.write(CSS811_ALG_RESULT_DATA);
	this->Wire.endTransmission();
	this->Wire.requestFrom(I2CAddress, 4);
	while (  i < 4){
		data[i] = this->Wire.read();
		i++;
	}
	// data : CO2eMSB, CO2eLSB, tvocMSB, tvocLSB
	CO2e  = ((uint16_t)data[0] << 8) | data[1];
	tVOC = ((uint16_t)data[2] << 8) | data[3];

	return SENSOR_SUCCESS;
}

	//Mode 0 = Idle
	//Mode 1 = read every 1s
	//Mode 2 = every 10s
	//Mode 3 = every 60s
	//Mode 4 = RAW mode
CCS811Core::status CCS811::setDriveMode( uint8_t mode ){
	//sanitize input
	if (mode > 4) mode = 4;

	uint8_t value;
	//Read what's currently there
	//Clear DRIVE_MODE bits
	//Mask in mode
	CCS811Core::status returnError = readRegister( CSS811_MEAS_MODE, &value );
	value &= ~(0b00000111 << 4);
	value |= (mode << 4);
	returnError = writeRegister(CSS811_MEAS_MODE, value);

	return returnError;
}

uint16_t CCS811::getTVOC( void ){
	return tVOC;
}

uint16_t CCS811::getCO2e( void ){
	return CO2e;
}
