#include <iostream>
#include "spacehauc-i2c-dev.hpp"

int main(int argc, char* argv[]) {
  TemperatureSensor tmp102(1, 0x48, 0x00, 0x00, 0x00);
  tmp102.initDevice();
  tmp102.initTempSensor();
  std::cout << tmp102.readTemp() << std::endl;
  return 0;
}
