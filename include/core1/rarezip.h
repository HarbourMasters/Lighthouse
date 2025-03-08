#ifndef BANJO_KAZOOIE_CORE1_RAREZIP_H
#define BANJO_KAZOOIE_CORE1_RAREZIP_H

#ifdef __cplusplus
extern "C" {
#endif

extern u8  bitLengthCodeOrder[]; 
//border[]= {    /* Order of the bit length code lengths */
    //16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
extern u16 literalCopyLengths[]; 
// static ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
//         3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
//         35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
//         /* note: see note #13 above about the 258 in this list. */

extern u8 literalExtraBits[];
// static ush cplext[] = {         /* Extra bits for literal codes 257..285 */
//         0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
//         3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */

extern u16 distanceCopyOffsets[];
// static ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
//         1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
//         257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
//         8193, 12289, 16385, 24577};

extern u8 distanceExtraBits[];
// static ush cpdext[] = {         /* Extra bits for distance codes */
//         0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
//         7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
//         12, 12, 13, 13};

extern u16 compressionBitMask[];
/*ush mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};*/

extern s32 literalBits; //lbits
extern s32 distanceBits; //dbits

extern struct huft *huftPointer;
extern u8 *compressionInputBuffer; //inbuf
extern u8 *outputBuffer; //slide
extern u32 inputBuffer; //inptr
extern u32 writePointer; //wp
extern struct huft *currentHuft; //unk
extern u32 bitBuffer; //bb
extern u32 compressionBitCount; //bk
extern u32 compressionCRC1; //crc1
extern u32 compressionCRC2; //crc2
extern u32 crc1; //crc1
extern u32 crc2; //crc2
extern u32 huftCount; //hufts


#ifndef WSIZE
#  define WSIZE 0x8000     /* window size--must be a power of two, and */
#endif                     /*  at least 32K for zip's deflate method */

//#define get_byte()  (inputBuffer < insize ? inbuf[inputBuffer++] : fill_inbuf(0))
#define get_byte()  (compressionInputBuffer[inputBuffer++])

#ifdef CRYPT
  uch cc;
#  define NEXTBYTE() \
     (decrypt ? (cc = get_byte(), zdecode(cc), cc) : get_byte())
#else
#  define NEXTBYTE()  (u8)get_byte()
#endif
#define NEEDBITS(n) {while(k<(n)){b|=((u32)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

struct huft {
  u8 e;                /* number of extra bits or operation */
  u8 b;                /* number of bits in this code or subcode */
  union {
    u16 n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */

int bkboot_inflate(void);


#ifdef __cplusplus
}
#endif


#endif
