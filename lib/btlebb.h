#ifndef DEF_BTLEBB_H
#define DEF_BTLEBB_H
#include <stdint.h>

#define ADV_ACCESS_ADDR 0x8e89bed6u

/* Preamble window size: sync:8, addr:32, pdu_hdr:16 */
#define BTLEBB_DETECT_WINDOW (8+32+16)

/* Position of pdu header in detection window  */
#define BTLEBB_PDUHDR_OFFSET (8+32)

/* Maximum hamming distance in 40 bits preamble */
#define BTLEBB_DETECT_MAX_ERR 5u

typedef union phdr_u {
	struct {
		uint8_t type:4;
		uint8_t rfu:1;
		uint8_t chsel:1;
		uint8_t txadd:1;
		uint8_t rxadd:1;
		uint8_t len;
	};
	uint16_t u16;
} phdr_t ;

/* Required init for crc and whitening */
void btlebb_init(void);

/* (Un)whiten for given seed (ie: channel number)
   Returns next seed */
uint8_t btlebb_whiten(uint8_t seed, uint8_t *buf, unsigned len);

uint32_t btlebb_calccrc(const uint8_t *buf, unsigned len);

/* Detect preamble in symbol (binary) stream, returns offset or -1, fills phdr and err */
int btlebb_detect_adv_preamble(uint32_t channel_num, const uint8_t *syms, unsigned syms_count, phdr_t *phdr, unsigned *err);

/* Unpack bytes */
void btlebb_unpack(uint8_t *dst_bytes, const uint8_t *src_syms, uint8_t byte_count);

#endif
