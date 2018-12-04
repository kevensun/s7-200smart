/*
 * rWebCfgCrc32.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */




/*
 * No copyright.
 */
#include "rWebCfgCrc32.h"

/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by make_crc_table)
 */
static const uint32 crc_table[256] = {
cpu_to_le32(0x00000000L), cpu_to_le32(0x77073096L), cpu_to_le32(0xee0e612cL), cpu_to_le32(0x990951baL),
cpu_to_le32(0x076dc419L), cpu_to_le32(0x706af48fL), cpu_to_le32(0xe963a535L), cpu_to_le32(0x9e6495a3L),
cpu_to_le32(0x0edb8832L), cpu_to_le32(0x79dcb8a4L), cpu_to_le32(0xe0d5e91eL), cpu_to_le32(0x97d2d988L),
cpu_to_le32(0x09b64c2bL), cpu_to_le32(0x7eb17cbdL), cpu_to_le32(0xe7b82d07L), cpu_to_le32(0x90bf1d91L),
cpu_to_le32(0x1db71064L), cpu_to_le32(0x6ab020f2L), cpu_to_le32(0xf3b97148L), cpu_to_le32(0x84be41deL),
cpu_to_le32(0x1adad47dL), cpu_to_le32(0x6ddde4ebL), cpu_to_le32(0xf4d4b551L), cpu_to_le32(0x83d385c7L),
cpu_to_le32(0x136c9856L), cpu_to_le32(0x646ba8c0L), cpu_to_le32(0xfd62f97aL), cpu_to_le32(0x8a65c9ecL),
cpu_to_le32(0x14015c4fL), cpu_to_le32(0x63066cd9L), cpu_to_le32(0xfa0f3d63L), cpu_to_le32(0x8d080df5L),
cpu_to_le32(0x3b6e20c8L), cpu_to_le32(0x4c69105eL), cpu_to_le32(0xd56041e4L), cpu_to_le32(0xa2677172L),
cpu_to_le32(0x3c03e4d1L), cpu_to_le32(0x4b04d447L), cpu_to_le32(0xd20d85fdL), cpu_to_le32(0xa50ab56bL),
cpu_to_le32(0x35b5a8faL), cpu_to_le32(0x42b2986cL), cpu_to_le32(0xdbbbc9d6L), cpu_to_le32(0xacbcf940L),
cpu_to_le32(0x32d86ce3L), cpu_to_le32(0x45df5c75L), cpu_to_le32(0xdcd60dcfL), cpu_to_le32(0xabd13d59L),
cpu_to_le32(0x26d930acL), cpu_to_le32(0x51de003aL), cpu_to_le32(0xc8d75180L), cpu_to_le32(0xbfd06116L),
cpu_to_le32(0x21b4f4b5L), cpu_to_le32(0x56b3c423L), cpu_to_le32(0xcfba9599L), cpu_to_le32(0xb8bda50fL),
cpu_to_le32(0x2802b89eL), cpu_to_le32(0x5f058808L), cpu_to_le32(0xc60cd9b2L), cpu_to_le32(0xb10be924L),
cpu_to_le32(0x2f6f7c87L), cpu_to_le32(0x58684c11L), cpu_to_le32(0xc1611dabL), cpu_to_le32(0xb6662d3dL),
cpu_to_le32(0x76dc4190L), cpu_to_le32(0x01db7106L), cpu_to_le32(0x98d220bcL), cpu_to_le32(0xefd5102aL),
cpu_to_le32(0x71b18589L), cpu_to_le32(0x06b6b51fL), cpu_to_le32(0x9fbfe4a5L), cpu_to_le32(0xe8b8d433L),
cpu_to_le32(0x7807c9a2L), cpu_to_le32(0x0f00f934L), cpu_to_le32(0x9609a88eL), cpu_to_le32(0xe10e9818L),
cpu_to_le32(0x7f6a0dbbL), cpu_to_le32(0x086d3d2dL), cpu_to_le32(0x91646c97L), cpu_to_le32(0xe6635c01L),
cpu_to_le32(0x6b6b51f4L), cpu_to_le32(0x1c6c6162L), cpu_to_le32(0x856530d8L), cpu_to_le32(0xf262004eL),
cpu_to_le32(0x6c0695edL), cpu_to_le32(0x1b01a57bL), cpu_to_le32(0x8208f4c1L), cpu_to_le32(0xf50fc457L),
cpu_to_le32(0x65b0d9c6L), cpu_to_le32(0x12b7e950L), cpu_to_le32(0x8bbeb8eaL), cpu_to_le32(0xfcb9887cL),
cpu_to_le32(0x62dd1ddfL), cpu_to_le32(0x15da2d49L), cpu_to_le32(0x8cd37cf3L), cpu_to_le32(0xfbd44c65L),
cpu_to_le32(0x4db26158L), cpu_to_le32(0x3ab551ceL), cpu_to_le32(0xa3bc0074L), cpu_to_le32(0xd4bb30e2L),
cpu_to_le32(0x4adfa541L), cpu_to_le32(0x3dd895d7L), cpu_to_le32(0xa4d1c46dL), cpu_to_le32(0xd3d6f4fbL),
cpu_to_le32(0x4369e96aL), cpu_to_le32(0x346ed9fcL), cpu_to_le32(0xad678846L), cpu_to_le32(0xda60b8d0L),
cpu_to_le32(0x44042d73L), cpu_to_le32(0x33031de5L), cpu_to_le32(0xaa0a4c5fL), cpu_to_le32(0xdd0d7cc9L),
cpu_to_le32(0x5005713cL), cpu_to_le32(0x270241aaL), cpu_to_le32(0xbe0b1010L), cpu_to_le32(0xc90c2086L),
cpu_to_le32(0x5768b525L), cpu_to_le32(0x206f85b3L), cpu_to_le32(0xb966d409L), cpu_to_le32(0xce61e49fL),
cpu_to_le32(0x5edef90eL), cpu_to_le32(0x29d9c998L), cpu_to_le32(0xb0d09822L), cpu_to_le32(0xc7d7a8b4L),
cpu_to_le32(0x59b33d17L), cpu_to_le32(0x2eb40d81L), cpu_to_le32(0xb7bd5c3bL), cpu_to_le32(0xc0ba6cadL),
cpu_to_le32(0xedb88320L), cpu_to_le32(0x9abfb3b6L), cpu_to_le32(0x03b6e20cL), cpu_to_le32(0x74b1d29aL),
cpu_to_le32(0xead54739L), cpu_to_le32(0x9dd277afL), cpu_to_le32(0x04db2615L), cpu_to_le32(0x73dc1683L),
cpu_to_le32(0xe3630b12L), cpu_to_le32(0x94643b84L), cpu_to_le32(0x0d6d6a3eL), cpu_to_le32(0x7a6a5aa8L),
cpu_to_le32(0xe40ecf0bL), cpu_to_le32(0x9309ff9dL), cpu_to_le32(0x0a00ae27L), cpu_to_le32(0x7d079eb1L),
cpu_to_le32(0xf00f9344L), cpu_to_le32(0x8708a3d2L), cpu_to_le32(0x1e01f268L), cpu_to_le32(0x6906c2feL),
cpu_to_le32(0xf762575dL), cpu_to_le32(0x806567cbL), cpu_to_le32(0x196c3671L), cpu_to_le32(0x6e6b06e7L),
cpu_to_le32(0xfed41b76L), cpu_to_le32(0x89d32be0L), cpu_to_le32(0x10da7a5aL), cpu_to_le32(0x67dd4accL),
cpu_to_le32(0xf9b9df6fL), cpu_to_le32(0x8ebeeff9L), cpu_to_le32(0x17b7be43L), cpu_to_le32(0x60b08ed5L),
cpu_to_le32(0xd6d6a3e8L), cpu_to_le32(0xa1d1937eL), cpu_to_le32(0x38d8c2c4L), cpu_to_le32(0x4fdff252L),
cpu_to_le32(0xd1bb67f1L), cpu_to_le32(0xa6bc5767L), cpu_to_le32(0x3fb506ddL), cpu_to_le32(0x48b2364bL),
cpu_to_le32(0xd80d2bdaL), cpu_to_le32(0xaf0a1b4cL), cpu_to_le32(0x36034af6L), cpu_to_le32(0x41047a60L),
cpu_to_le32(0xdf60efc3L), cpu_to_le32(0xa867df55L), cpu_to_le32(0x316e8eefL), cpu_to_le32(0x4669be79L),
cpu_to_le32(0xcb61b38cL), cpu_to_le32(0xbc66831aL), cpu_to_le32(0x256fd2a0L), cpu_to_le32(0x5268e236L),
cpu_to_le32(0xcc0c7795L), cpu_to_le32(0xbb0b4703L), cpu_to_le32(0x220216b9L), cpu_to_le32(0x5505262fL),
cpu_to_le32(0xc5ba3bbeL), cpu_to_le32(0xb2bd0b28L), cpu_to_le32(0x2bb45a92L), cpu_to_le32(0x5cb36a04L),
cpu_to_le32(0xc2d7ffa7L), cpu_to_le32(0xb5d0cf31L), cpu_to_le32(0x2cd99e8bL), cpu_to_le32(0x5bdeae1dL),
cpu_to_le32(0x9b64c2b0L), cpu_to_le32(0xec63f226L), cpu_to_le32(0x756aa39cL), cpu_to_le32(0x026d930aL),
cpu_to_le32(0x9c0906a9L), cpu_to_le32(0xeb0e363fL), cpu_to_le32(0x72076785L), cpu_to_le32(0x05005713L),
cpu_to_le32(0x95bf4a82L), cpu_to_le32(0xe2b87a14L), cpu_to_le32(0x7bb12baeL), cpu_to_le32(0x0cb61b38L),
cpu_to_le32(0x92d28e9bL), cpu_to_le32(0xe5d5be0dL), cpu_to_le32(0x7cdcefb7L), cpu_to_le32(0x0bdbdf21L),
cpu_to_le32(0x86d3d2d4L), cpu_to_le32(0xf1d4e242L), cpu_to_le32(0x68ddb3f8L), cpu_to_le32(0x1fda836eL),
cpu_to_le32(0x81be16cdL), cpu_to_le32(0xf6b9265bL), cpu_to_le32(0x6fb077e1L), cpu_to_le32(0x18b74777L),
cpu_to_le32(0x88085ae6L), cpu_to_le32(0xff0f6a70L), cpu_to_le32(0x66063bcaL), cpu_to_le32(0x11010b5cL),
cpu_to_le32(0x8f659effL), cpu_to_le32(0xf862ae69L), cpu_to_le32(0x616bffd3L), cpu_to_le32(0x166ccf45L),
cpu_to_le32(0xa00ae278L), cpu_to_le32(0xd70dd2eeL), cpu_to_le32(0x4e048354L), cpu_to_le32(0x3903b3c2L),
cpu_to_le32(0xa7672661L), cpu_to_le32(0xd06016f7L), cpu_to_le32(0x4969474dL), cpu_to_le32(0x3e6e77dbL),
cpu_to_le32(0xaed16a4aL), cpu_to_le32(0xd9d65adcL), cpu_to_le32(0x40df0b66L), cpu_to_le32(0x37d83bf0L),
cpu_to_le32(0xa9bcae53L), cpu_to_le32(0xdebb9ec5L), cpu_to_le32(0x47b2cf7fL), cpu_to_le32(0x30b5ffe9L),
cpu_to_le32(0xbdbdf21cL), cpu_to_le32(0xcabac28aL), cpu_to_le32(0x53b39330L), cpu_to_le32(0x24b4a3a6L),
cpu_to_le32(0xbad03605L), cpu_to_le32(0xcdd70693L), cpu_to_le32(0x54de5729L), cpu_to_le32(0x23d967bfL),
cpu_to_le32(0xb3667a2eL), cpu_to_le32(0xc4614ab8L), cpu_to_le32(0x5d681b02L), cpu_to_le32(0x2a6f2b94L),
cpu_to_le32(0xb40bbe37L), cpu_to_le32(0xc30c8ea1L), cpu_to_le32(0x5a05df1bL), cpu_to_le32(0x2d02ef8dL)
};

/* ========================================================================= */
#define  DO_CRC(x)      crc = tab[(crc ^ (x)) & 255] ^ (crc >> 8)

/* ========================================================================= */

uint32 crc32_no_comp(uint32 crc, const uint8 *buf, uint32 len)
{
    const uint32 *tab = crc_table;
    const uint32 *b =(const uint32 *)buf;
    uint32 rem_len;

    crc = cpu_to_le32(crc);
    /* Align it */
    if ((((long)b) & 3) && len) {
	 uint8 *p = (uint8 *)b;
	 do {
	      DO_CRC(*p++);
	 } while ((--len) && (((long)p)&3));
	 b = (uint32 *)p;
    }

    rem_len = len & 3;
    len = len >> 2;
    for (--b; len; --len) {
	 /* load data 32 bits wide, xor data 32 bits wide. */
	 crc ^= *++b;
	 DO_CRC(0);
	 DO_CRC(0);
	 DO_CRC(0);
	 DO_CRC(0);
    }
    len = rem_len;
    /* Handle last few bytes */
    if (len) {
	 uint8 *p = (uint8 *)(b + 1) - 1;
	 do {
	      DO_CRC(*++p);
	 } while (--len);
    }

    return le32_to_cpu(crc);
}
#undef DO_CRC

uint32 crc32(uint32 crc, const uint8 *p, uint32 len)
{
     return crc32_no_comp(crc ^ 0xffffffffL, p, len) ^ 0xffffffffL;
}


