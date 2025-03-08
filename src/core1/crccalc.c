#include <ultra64.h>
#include "core1/core1.h"

// transform seed (in mips3 file)
// handwritten function in assembly. See src/core1/code_72B0.s
/* The function func_8025C29C takes a 64-bit value from memory, performs a
 * series of bitwise operations on it, stores the modified value back to the
 * same memory location, and returns the modified value. */
u32 func_8025C29C(u32 *seed); 

/* The glcrc_calc_checksum function calculates two CRC values for a given
 * range of memory by iterating over the bytes in both forward and backward
 * directions. It uses the func_8025C29C function to transform the seed value
 * during the CRC calculation process. The transformed seed is then used to
 * update the CRC values, which are stored in the checksum array. */
void glcrc_calc_checksum(void *start, void *end, u32 checksum[2]) {
  u8 *p;
  u32 shift = 0;
  u64 seed = 0x8F809F473108B3C1;
  u32 crc1 = 0;
  u32 crc2 = 0;
  u32 tmp;

  // CRC1: Iterate forwards over bytes
  for (p = start; (void *)p < end; p++) {
    seed += *p << (shift & 15);
    tmp = func_8025C29C(&seed);
    shift += 7;
    crc1 ^= tmp;
  }

  // CRC2: Iterate backwards over bytes
  for (p = (u8 *)end - 1; (void *)p >= start; p--) {
    seed += *p << (shift & 15);
    tmp = func_8025C29C(&seed);
    shift += 3;
    crc2 ^= tmp;
  }

  checksum[0] = crc1;
  checksum[1] = crc2;
}
