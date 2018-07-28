#include "cox.h"
//Register addresses
#define CSS811_void 0x00
#define CSS811_MEAS_MODE 0x01
#define CSS811_ALG_RESULT_DATA 0x02
#define CSS811_RAW_DATA 0x03
#define CSS811_ENV_DATA 0x05
#define CSS811_NTC 0x06
#define CSS811_THRESHOLDS 0x10
#define CSS811_BASELINE 0x11
#define CSS811_HW_ID 0x20
#define CSS811_HW_VERSION 0x21
#define CSS811_FW_BOOT_VERSION 0x23
#define CSS811_FW_APP_VERSION 0x24
#define CSS811_ERROR_ID 0xE0
#define CSS811_APP_START 0xF4
#define CSS811_SW_RESET 0xFF

class CCS811Core{
public:
	// Return values
	typedef enum{
		SENSOR_SUCCESS,
		SENSOR_ID_ERROR,
		SENSOR_I2C_ERROR,
		SENSOR_INTERNAL_ERROR,
		SENSOR_GENERIC_ERROR
	}status;

	CCS811Core(TwoWire &, uint8_t addr);
	~CCS811Core() = default;
	status beginCore( void );
	status readRegister( uint8_t offset, uint8_t* outputPointer);
	status multiReadRegister(uint8_t offset, uint8_t *outputPointer, uint8_t length);
	status writeRegister(uint8_t offset, uint8_t dataToWrite);
	status multiWriteRegister(uint8_t offset, uint8_t *inputPointer, uint8_t length);

protected:
	TwoWire &Wire;
	uint8_t I2CAddress;
};

class CCS811: public CCS811Core{
public:
	CCS811(TwoWire &, uint8_t addr);

	status begin( void );
	status setDriveMode( uint8_t mode );
	status readAlgorithmResults( void );

	bool dataAvailable( void );

	uint16_t getTVOC( void );
	uint16_t getCO2e( void );

private:
	float refResistance;
	float resistance;
	float temperature;
	uint16_t tVOC;
	uint16_t CO2e;
};
