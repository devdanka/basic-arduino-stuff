# Temperature and humidity display

Reads a DHT11 sensor and shows temperature in Celsius and relative humidity on a 16x2 I2C LCD. Readings refresh every two seconds. If the sensor cannot provide a valid reading, the LCD shows "Sensor error" until the next attempt.
