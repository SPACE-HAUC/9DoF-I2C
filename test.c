/* Lot of the ideas and algos are originally from
 * https://github.com/sparkfun/LSM9DS0_Breakout/
 */

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
#define WHO_AM_I_G         0x0F // r
/* XM_ADDRESS registers */
#define OUT_TEMP_L_XM      0x05 // r
#define WHO_AM_I_XM        0x0F // r
#define CTRL_REG5_XM       0x24 // rw


int write_bytes (int file, uint8_t address, uint8_t *data, uint8_t count);
int write_byte (int file, uint8_t address, uint8_t reg, uint8_t data);
int read_bytes (int file, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count);
int read_byte (int file, uint8_t address, uint8_t reg, uint8_t *dest);
int init_device (const char* device_name);

int main (int argc, char **argv) {
    int file;
    int16_t temp;
    uint8_t data[2] = {0};
    file = init_device (I2C_DEV_NAME);

    // enable temp sensor
    write_byte (file, XM_ADDRESS, CTRL_REG5_XM, 0x98);
    if (file == 0) {
        return 1;
    }
    // temperature is a 12-bit value: cut out 4 highest bits
    read_bytes (file, XM_ADDRESS, OUT_TEMP_L_XM, &data[0], 2);
    printf("data[0] = %d. data[1] = %d", data[0], data[1]);
    temp = (((data[1] & 0x0f) << 8) | data[0]);
    printf ("Temperature: %d\n", temp);
    printf ("Temperature: %d\n", temp);
    return 0;
}

int write_bytes (int file, uint8_t address, uint8_t *data, uint8_t count)
{
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];

  messages[0].addr  = address;
  messages[0].flags = 0;
  messages[0].len   = count;
  messages[0].buf   = data;

  packets.msgs      = messages;
  packets.nmsgs     = 1;

  return ioctl(file, I2C_RDWR, &packets) >= 0 ;
}

int write_byte (int file, uint8_t address, uint8_t reg, uint8_t data)
{
  uint8_t buf[2];
  buf[0] = reg;
  buf[1] = data;
  return write_bytes (file, address, buf, 2);
}

int read_bytes (int file, uint8_t address, uint8_t reg, uint8_t *dest, uint8_t count)
{
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

int read_byte (int file, uint8_t address, uint8_t reg, uint8_t *dest)
{
  return read_bytes (file, address, reg, dest, 1);
}

int init_device (const char* device_name)
{
  int file;
  uint8_t g_id, xm_id;
  if ((file = open(device_name, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open the i2c bus '%s'\n", device_name);
    return 0;
  }
  read_byte (file, G_ADDRESS, WHO_AM_I_G, &g_id);
  read_byte (file, XM_ADDRESS, WHO_AM_I_XM, &xm_id);
  if (g_id != 0xD4 || xm_id != 0x49) {
    fprintf(stderr, "Device id mismatch: Got %02x/%02x, expected %02x/%02x\n",
            g_id, xm_id, 0xD4, 0x49);
    close (file);
    return 0;
  }
  return file;
}
