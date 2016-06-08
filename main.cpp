/*
Figured out how we were actually reading data...
got this from: https://www.kernel.org/doc/Documentation/i2c/dev-interface

ioctl(file, I2C_RDWR, struct i2c_rdwr_ioctl_data *msgset)
  Do combined read/write transaction without stop in between.
  Only valid if the adapter has I2C_FUNC_I2C.  The argument is
  a pointer to a

  struct i2c_rdwr_ioctl_data {
      struct i2c_msg *msgs;  /* ptr to array of simple messages */
  //    int nmsgs;             /* number of messages to exchange */
  /*}

  The msgs[] themselves contain further pointers into data buffers.
  The function will write or read data to or from that buffers depending
  on whether the I2C_M_RD flag is set in a particular message or not.
  The slave address and whether to use ten bit address mode has to be
  set in each message, overriding the values set with the above ioctl's.

*/

/* unexpected output of 0 */



#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <iostream>
#include <inttypes.h>

#define WHO_AM_I_G         0x0F // r
#define G_ADDRESS          0x6b
#define OUT_TEMP_L_XM      0x05 // r; output temperature pin(?)
#define XM_ADDRESS         0x1d // xm address, one of the two addresses (i2cdetect)
#define WHO_AM_I_XM        0x0F // r

int init_device(const char* device_name);
int read_bytes(int file, uint8_t address, uint8_t reg, uint8_t *dest,
  uint8_t count);

using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
  int bus; // i2c bus, also known as file (this name makes more sense)
  uint8_t temperature[2] = {1};
  bus = init_device("/dev/i2c-1");
  read_bytes(bus, XM_ADDRESS, OUT_TEMP_L_XM, &temperature[0], 2);
  //cout << "Current temperature: " << temperature[0] << " F" << endl;
  printf("Current Temperarure: %d F\n", temperature[0]);
  return 0;
}

int read_bytes (int file, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count) {
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

  return ioctl(file, I2C_RDWR, &packets) >= 0;
}

int init_device (const char* device_name) {
  int file;
  uint8_t g_id, xm_id;

  if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open the i2c bus '%s'\n", device_name);
    return 0;
  }

  read_bytes (file, G_ADDRESS, WHO_AM_I_G, &g_id, 1);
  read_bytes(file, XM_ADDRESS, WHO_AM_I_XM, &xm_id, 1);
  if (g_id != 0xD4 || xm_id != 0x49) {
    fprintf(stderr, "Device id mismatch: Got %02x/%02x, expected %02x/%02x\n",
            g_id, xm_id, 0xD4, 0x49);
    close (file);
    return 0;
  }

  return file;
}
