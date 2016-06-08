/* Lot of the ideas and algos are originally from
 * https://github.com/sparkfun/LSM9DS0_Breakout/
 */

#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include "edison-9dof-i2c.h"

#define BYTE2BIN(byte) \
    (byte & 0x80 ? 1 : 0), \
    (byte & 0x40 ? 1 : 0), \
    (byte & 0x20 ? 1 : 0), \
    (byte & 0x10 ? 1 : 0), \
    (byte & 0x08 ? 1 : 0), \
    (byte & 0x04 ? 1 : 0), \
    (byte & 0x02 ? 1 : 0), \
    (byte & 0x01 ? 1 : 0)


int main (int argc, char **argv) {
    int file;
    int16_t temp;
    uint8_t data[2] = {0};
    file = init_device (I2C_DEV_NAME);
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