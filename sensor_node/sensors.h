#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4      // DS18B20 on GPIO4

//quality
namespace sensor {
  float ecCalibration = 1.0f;

  // 2-point calibration values
  float tdsRaw1 = 0, tdsRaw2 = 0;
  float tdsRef1 = 0, tdsRef2 = 0;

  float tdsSlope = 0.719642f;
  float tdsOffset = -6.969f;

  float tdsThreshold = 10;
}
float TDS_FACTOR = 0.65f;

//level sensor
#define WATER_LEVEL_PIN 1
#define R_FIXED 150.0f      // Fixed resistor in ohms
#define ADC_MAX 4095.0f
#define VREF 3.3f 
