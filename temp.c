#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>

#define XM_ADDRESS     0x1d
#define G_ADDRESS      0x6b
#define OUT_TEMP_L_XM  0x05
#define WHO_AM_I_G     0x0F
#define WHO_AM_I_XM    0x0F
#define I2C_DEV_NAME   "/dev/i2c-1"

int read_bytes (int file, uint8_t xmaddress, uint8_t reg, uint8_t *dest, uint8_t count);
int init_device (const char* device_name);
int read_byte (int file, uint8_t address, uint8_t reg, uint8_t *dest);

int main (int argc, char* argv[]) {
    int file;
    uint8_t data[2] = {0};

    printf("fizz\n");

    file = init_device (I2C_DEV_NAME); //see function "init device"
    if (file == 0) { //if init device returns 1, function errors
      printf ("\nError - Device could not be opened\n\n");
      return 1;
    }
    printf ("spaghet");
    while (1) {
	printf("spaghet2");
        read_bytes (file, XM_ADDRESS, OUT_TEMP_L_XM, &data[0], 2);
        printf("Temperature = %d",data[0]); //prints the first item in the array "data"
        usleep (500000); //time that the program delays, in microseconds (10^-6)
    }
}

int read_bytes (int file, uint8_t xmaddress, uint8_t reg, uint8_t *dest, uint8_t count) {
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  /* secret handshake for multibyte read */
  reg = reg | 0x80; //still not sure what this does...

  /* write the register we want to read from */
  messages[0].addr  = xmaddress;
  messages[0].flags = 0;
  messages[0].len   = 1;
  messages[0].buf   = &reg;

  /* read */
  messages[1].addr  = xmaddress;
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
  printf("buzz");
  // open i2c bus (most likely 1 in this case (only bus 1 is active on Edison))
  if ((file = open(device_name, O_RDWR)) < 0) {
    fprintf(stderr, "Failed to open the i2c bus '%s'\n", device_name);
    return 0;
  }
  // make sure that the devide is the 9DoF board by checking id pins
  read_byte (file, G_ADDRESS, WHO_AM_I_G, &g_id);
  read_byte (file, XM_ADDRESS, WHO_AM_I_XM, &xm_id);
  if (g_id != 0xD4 || xm_id != 0x49) {
    fprintf(stderr, "Device id mismatch: Got %02x/%02x, expected %02x/%02x\n",
            g_id, xm_id, 0xD4, 0x49);
    close (file);
    return 0;
  }
 // return 1 asssuming stuff didnt break
  return file;
}

int read_byte (int file, uint8_t address, uint8_t reg, uint8_t *dest) {
  return read_bytes (file, address, reg, dest, 1);
}

