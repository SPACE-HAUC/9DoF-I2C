#include <iostream>
#include "spacehauc-i2c-dev.hpp"
#include "spacehauc-i2c-dev.hpp"
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
int main(int argc, char* argv[]) {
  TemperatureSensor tmp102(1, 0x48, 0x00, 0x00, 0x00);
  if (tmp102.initDevice() == false) {
    cerr << "Error: I2C bus failed to open." << endl;
    return false;
  }
  cout << "Opened I2C bus" << endl;
  if (tmp102.initTempSensor() == false) {
    cerr << "Error: Temperature Sensor failed to initalize." << endl;
    return false;
  }
  cout << "Initialized Temperature Sensor" << endl;
  cout << "Reading Temperature data..." << endl;
  for (int i = 0; i < 5; i++) {
    cout << "Temperature = " << (int) tempSensor.readTemp() << endl;
    usleep(500000);
  }
  return true;
  return 0;
}
