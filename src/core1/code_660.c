#include "core1/core1.h"
#include <ultra64.h>

//rarezip.c

static int rarezip_uncompress_2(u8 **arg0, u8 **arg1, struct huft *arg2);

#define COMP_HEADER_SIZE 6

// border[]= {    /* Order of the bit length code lengths */
u8 bitLengthCodeOrder[] = {16, 17, 18, 0, 8,  7, 9,  6, 10, 5,
                   11, 4,  12, 3, 13, 2, 14, 1, 15};

// static ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
u16 literalCopyLengths[] = {3,  4,   5,   6,   7,   8,   9,   10, 11, 13, 15,
                    17, 19,  23,  27,  31,  35,  43,  51, 59, 67, 83,
                    99, 115, 131, 163, 195, 227, 258, 0,  0};
//         /* note: see note #13 above about the 258 in this list. */

// static uch cplext[] = {         /* Extra bits for literal codes 257..285 */
u8 literalExtraBits[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2,  2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */

// static ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
u16 distanceCopyOffsets[] = {1,    2,    3,    4,     5,     7,    9,    13,
                    17,   25,   33,   49,    65,    97,   129,  193,
                    257,  385,  513,  769,   1025,  1537, 2049, 3073,
                    4097, 6145, 8193, 12289, 16385, 24577};

// static uch cpdext[] = {         /* Extra bits for distance codes */
u8 distanceExtraBits[] = {0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                   6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

// ush mask_bits[] = {
u16 compressionBitMask[] = {0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f,
                    0x003f, 0x007f, 0x00ff, 0x01ff, 0x03ff, 0x07ff,
                    0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff};

s32 literalBits = 9; // lbits
s32 distanceBits = 6; // dbits

/* .data */
extern struct huft defaultHuft;
struct huft *huftPointer;
u8 pad_8027BF08[0x8];
u8 *compressionInputBuffer;          // inbuf
u8 *outputBuffer;          // slide
u32 inputBuffer;          // inptr
u32 writePointer;          // wp
struct huft *currentHuft; // unk
u32 bitBuffer;          // bb
u32 compressionBitCount;          // bk
u32 compressionCRC1;          // crc1
u32 compressionCRC2;          // crc2
u32 huftCount;          // hufts

static int rarezip_inflate_2(u8 *src, u8 *dst, struct huft *arg2);

/* .code */
s32 rarezip_get_uncompressed_size(u8 *arg0) { return *((s32 *)(arg0 + 2)); }

void rarezip_init(void) { huftPointer = &defaultHuft; }

void rarezip_inflate(u8 *src, u8 *dst) {
  rarezip_inflate_2(src, dst, huftPointer);
}

void rarezip_uncompress(u8 **srcPtr, u8 **dstPtr) {
  // updates in and out buffer ptrs,
  rarezip_uncompress_2(srcPtr, dstPtr, huftPointer);
}

void rarezip_noop(void) { return; }

static int rarezip_inflate_2(u8 *src, u8 *dst, struct huft *arg2) {
  compressionInputBuffer = src;
  outputBuffer = dst;
  currentHuft = arg2;
  compressionInputBuffer += COMP_HEADER_SIZE;
  writePointer = 0;
  inputBuffer = 0;
  inflate();
  return writePointer;
}

static int rarezip_uncompress_2(u8 **srcPtr, u8 **dstPtr, struct huft *arg2) {
  int result;
  result = rarezip_inflate_2(*srcPtr, *dstPtr, arg2);
  *dstPtr = *dstPtr + writePointer;
  *dstPtr =
      ((u32)*dstPtr & 0xF) ? (u8 *)((u32)*dstPtr & -0x10) + 0x10 : *dstPtr;
  *srcPtr = *srcPtr + inputBuffer + COMP_HEADER_SIZE;
  return result;
}
