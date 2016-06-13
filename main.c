#include <unistd.h>
#include <getopt.h>

#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>

#include <linux/i2c-dev.h>
#include <fcntl.h>


/* I2C device */
#define I2C_DEV_NAME       "/dev/i2c-1"
/* I2C device adresses */
#define XM_ADDRESS         0x1d 
#define G_ADDRESS          0x6b
/* G_ADDRESS registers */
#define WHO_AM_I_G         0x0F // read only
/* XM_ADDRESS registers */
#define OUT_TEMP_L_XM      0x05 // read only
#define WHO_AM_I_XM        0x0F // read only
#define CTRL_REG5_XM       0x24 // read/write

// function declarations
int write_bytes (int bus, uint8_t address, uint8_t *data, uint8_t count);
int write_byte (int bus, uint8_t address, uint8_t reg, uint8_t data);
int read_bytes (int bus, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count);
int read_byte (int bus, uint8_t address, uint8_t reg, uint8_t *dest);
int init_device (const char* device_name);

int main (int argc, char **argv) {
    //set up variables
    int bus; // which i2c bus is being used
    int16_t temp; // temperature variable
    uint8_t data[2] = {0}; // array with two 8-bit unsigned ints

    // check to make sure the I2C bus is working properly and set bus
    bus = init_device (I2C_DEV_NAME); //see init_device
    if (bus == 0) { //if init_device returns 0 then stop because something is broken
        return 1;
    }

    // enable temp sensor
    write_byte (bus, XM_ADDRESS, CTRL_REG5_XM, 0x98);

    // read data from the I2C sensor
    while(1) { //infinite loop
        read_bytes (bus, XM_ADDRESS, OUT_TEMP_L_XM, &data[0], 2);
        temp = data[0];
        printf("Temperature = %d\n", temp); //prints the first item in the array, which is the temperature data
        usleep(250000); //this just delays the program so that it doesnt output temperature as fast as it can
    }
    return(0);
}

/*
 * Writes multiple bytes to the i2c device.
 *
 * @param bus This is the i2c bus number.
 * @param address This is the i2c sensor address.
 * @param data This is a pointer to the data being written
 * @param count This is the number of bytes being written
 */
int write_bytes (int bus, uint8_t address, uint8_t *data, uint8_t count) {
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];

  messages[0].addr  = address;
  messages[0].flags = 0;
  messages[0].len   = count;
  messages[0].buf   = data;

  packets.msgs      = messages;
  packets.nmsgs     = 1;

  return ioctl(bus, I2C_RDWR, &packets) >= 0 ;
}

/*
 * Writes one byte to the i2c sensor
 *
 * @param bus This is the i2c bus number.
 * @param address This is the i2c sensor address.
 * @param reg This is one of the sensor's data registers.
 * @param data This is a pointer to the data being written.
 */
int write_byte (int bus, uint8_t address, uint8_t reg, uint8_t data) {
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = data;
  return write_bytes (bus, address, buf, 2);
}

/*
 * Reads mulitple bytes from the i2c sensor
 *
 * @param bus This is the i2c bus number.
 * @param address This is the i2c sensor address.
 * @param reg This is one of the sensor's data registers.
 * @param dest The place for the read data to be stored
 * @param count The number of bytes being read
 */
int read_bytes (int bus, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count) {
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  /* secret handshake for multibyte read */
  reg = reg | 0x80;

  /* write the register we want to read from */
  messages[0].addr  = address;
  messages[0].flags = 0;
  messages[0].len   = 1;
  messages[0].buf   = &reg;

  /* read */
  messages[1].addr  = address;
  messages[1].flags = I2C_M_RD;
  messages[1].len   = count;
  messages[1].buf   = dest;

  packets.msgs      = messages;
  packets.nmsgs     = 2;

  return ioctl(bus, I2C_RDWR, &packets) >= 0;
}

/*
 * Reads one byte from the i2c sensor
 *
 * @param bus This is the i2c bus number.
 * @param address This is the i2c sensor address.
 * @param reg This is one of the sensor's data registers.
 * @param dest The place for the read data to be stored
 */
int read_byte (int bus, uint8_t address, uint8_t reg, uint8_t *dest)
{
  return read_bytes (bus, address, reg, dest, 1);
}

/*
 * Initializes i2c and the 9DoF sensor, does not turn any sensors on
 *
 * @param device_name The file name of the device to be opened
 */
int init_device (const char* device_name)
{
  int bus;
  uint8_t g_id, xm_id;

  // Try to open the device bus
  if ((bus = open(device_name, O_RDWR)) < 0) {
    // stop on failure
    fprintf(stderr, "Failed to open the i2c bus '%s'\n", device_name);
    return 0;
  }

  // Check to make sure the connected device is what we expect it to be (9DoF)

  // read at the identidication registers
  read_byte (bus, G_ADDRESS, WHO_AM_I_G, &g_id);
  read_byte (bus, XM_ADDRESS, WHO_AM_I_XM, &xm_id);
  // if the numbers there don't match, we connected the wrong sensor. stop.
  if (g_id != 0xD4 || xm_id != 0x49) {
    fprintf(stderr, "Device id mismatch: Got %02x/%02x, expected %02x/%02x\n",
            g_id, xm_id, 0xD4, 0x49);
    close (bus);
    return 0;
  }
  return bus;
}
