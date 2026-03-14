#include <Arduino.h>
#include "shared_data.h"
#include "sensors.h"
#include <Adafruit_ADS1X15.h>

// Calibration points (replace with your measured ADCs)
const int32_t ADC_POINTS[] = {239, 16098, 24345};
const int32_t LEVEL_POINTS[] = {0, 50, 100};
const int NUM_POINTS = sizeof(ADC_POINTS)/sizeof(ADC_POINTS[0]);
Adafruit_ADS1115 ads;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
PCF8575 pcf8575(0x20);

void initSensors()
{
    // ---- ADS1115 ----
    if (!ads.begin(0x48)) {
        Serial.println("ADS1115 not found!");
        while (1);
    }
    ads.setGain(GAIN_TWO);
    tempSensor.begin();

    // Set pinMode to OUTPUT
    pcf8575.pinMode(P0, OUTPUT);
    pcf8575.pinMode(P1, OUTPUT);
    pcf8575.pinMode(P2, OUTPUT);
    pcf8575.pinMode(P3, OUTPUT);
    pcf8575.pinMode(P4, OUTPUT);
    pcf8575.pinMode(P5, OUTPUT);
    pcf8575.pinMode(P6, OUTPUT);
    pcf8575.pinMode(P7, OUTPUT);
    pcf8575.pinMode(P8, OUTPUT);
    pcf8575.pinMode(P9, OUTPUT);
    pcf8575.begin();
}

float readWaterLevelMM() {
  
    int adc = ads.readADC_SingleEnded(WATER_LEVEL_PIN);
    //Serial.println(adc);
    // Clamp ADC
    if (adc <= ADC_POINTS[0]) return LEVEL_POINTS[0];
    if (adc >= ADC_POINTS[NUM_POINTS-1]) return LEVEL_POINTS[NUM_POINTS-1];
    float level = 0;
    for (int i = 0; i < NUM_POINTS - 1; i++)
    {
        if (adc >= ADC_POINTS[i] && adc <= ADC_POINTS[i+1])
        {
            // Linear interpolation
            level = LEVEL_POINTS[i] + 
                   (float)(adc - ADC_POINTS[i]) * 
                   (LEVEL_POINTS[i+1] - LEVEL_POINTS[i]) / 
                   (ADC_POINTS[i+1] - ADC_POINTS[i]);
        }
    }
    
    return level;
}

void readPressure(float *V, float *P)
{
    const float LSB = 0.0000625;   // ADS1115 GAIN_TWO
    const float scale_bar = 2.5;   // bar per volt
    const float deadband = 0.01;   // volts
    float pressureOffset = 0.43;

    *V = ads.readADC_SingleEnded(3) * LSB;

    *P = ((*V - pressureOffset) > deadband) ?
         ((*V - pressureOffset) * scale_bar) : 0.0;
}

void readWaterQuality(int* tdsInt, int* tempInt) {

  /* ---------- SERIAL CALIBRATION ---------- */
  /*
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("CAL1=")) {
      sensor::tdsRef1 = cmd.substring(5).toFloat();
      sensor::tdsRaw1 = 0;  // will be captured live
      Serial.println("CAL1 reference set. Place probe in LOW ppm water.");
    }

    else if (cmd.startsWith("CAL2=")) {
      sensor::tdsRef2 = cmd.substring(5).toFloat();
      sensor::tdsRaw2 = 0;
      Serial.println("CAL2 reference set. Place probe in HIGH ppm water.");
    }

    else if (cmd == "SAVE") {
      if (sensor::tdsRaw1 > 0 && sensor::tdsRaw2 > 0 &&
          sensor::tdsRef1 > 0 && sensor::tdsRef2 > 0) {

        sensor::tdsSlope =
          (sensor::tdsRef2 - sensor::tdsRef1) /
          (sensor::tdsRaw2 - sensor::tdsRaw1);

        sensor::tdsOffset =
          sensor::tdsRef1 - sensor::tdsSlope * sensor::tdsRaw1;

        Serial.println("2-Point Calibration Saved!");
        Serial.print("Slope: ");  Serial.println(sensor::tdsSlope, 6);
        Serial.print("Offset: "); Serial.println(sensor::tdsOffset, 3);
      } else {
        Serial.println("Calibration failed: missing points");
      }
    }
  } */

  /* ---------- TEMPERATURE ---------- */
  tempSensor.requestTemperatures();
  float temperature = tempSensor.getTempCByIndex(0);
  if (temperature == DEVICE_DISCONNECTED_C) return;

  /* ---------- ADS1115 READ ---------- */
  int16_t adc = ads.readADC_SingleEnded(0);

  // GAIN_TWO → 0.0625 mV per bit
  float voltage = adc * 0.0625f / 1000.0f;

  /* ---------- TEMP COMPENSATION ---------- */
  float compensationCoefficient = 1.0f + 0.02f * (temperature - 25.0f);
  float compensatedVoltage = voltage / compensationCoefficient;

  /* ---------- VOLTAGE → EC ---------- */
  float ec_us =
    (133.42f * compensatedVoltage * compensatedVoltage * compensatedVoltage
   - 255.86f * compensatedVoltage * compensatedVoltage
   + 857.39f * compensatedVoltage)
   * sensor::ecCalibration;

  if (ec_us < 0) ec_us = 0;

  /* ---------- EC → RAW TDS ---------- */
  float tds_raw = ec_us * TDS_FACTOR;

  /* ---------- CAPTURE CAL POINTS ---------- */
  if (sensor::tdsRef1 > 0 && sensor::tdsRaw1 == 0)
    sensor::tdsRaw1 = tds_raw;

  if (sensor::tdsRef2 > 0 && sensor::tdsRaw2 == 0)
    sensor::tdsRaw2 = tds_raw;

  /* ---------- APPLY 2-POINT CAL ---------- */
  float tds = sensor::tdsSlope * tds_raw + sensor::tdsOffset;
  if (tds < 0) tds = 0;

  *tdsInt  = (int)tds;
  *tempInt = (int)temperature;

 //Serial.print("Temp: "); Serial.print(temperature, 1);
 //Serial.print(" C | EC: "); Serial.print(ec_us, 1);
// Serial.print(" uS/cm | TDS: ");
// Serial.print(tds);
// Serial.println(" ppm");

  /* ---------- OUTPUT ---------- */
/*  Serial.print("Temp: "); Serial.print(temperature, 1);
  Serial.print(" C | V: "); Serial.print(voltage, 4);
  Serial.print(" | EC: "); Serial.print(ec_us, 1);
  Serial.print(" uS/cm | TDS: ");
  Serial.print(tds, 1);
  Serial.println(" ppm"); 

  if (tds > sensor::tdsThreshold)
    Serial.println("Water quality is POOR");
  else
    Serial.println("Water quality is GOOD"); */
}

void sensorTask(void *pvParameters)
{
    while (true)
    {
        int quality;
        float voltage, pressure;

        // Water quality + temp
        readWaterQuality(&quality, &temperatureValue);
        quality = constrain(quality, 0, 400);
        qualityByte = (uint8_t)(quality * 255.0f / 400.0f);

        // Water level
        waterLevel = readWaterLevelMM();

        // Flow rate
        flowRate = constrain(flowRate, 0.0f, 30.0f);
        flowByte = (uint8_t)(flowRate * 255.0f / 30.0f);

        // Pressure
        readPressure(&voltage, &pressure);
        pressure = constrain(pressure, 0.0f, 10.0f);
        pressureByte = (uint8_t)(pressure * 255.0f / 10.0f);

        // Optional debug
      //  Serial.printf(
      //      "WL=%.1f%% Q=%d F=%.1f P=%.2f\n",
      //      waterLevel, quality, flowRate, pressure
      //  );

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void startSensorTask()
{
    xTaskCreatePinnedToCore(sensorTask,"SensorTask",4096,NULL,2,NULL,1);
}