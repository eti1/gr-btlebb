#include <stdio.h>
#include "btlebb.h"

static uint8_t whitentbl[256*2];
static uint32_t crctbl[256];

static unsigned cnt_bits64(uint64_t i)
{
    i = i - ((i >> 1) & 0x5555555555555555);
    i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
    i = ((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F);
    return (unsigned)((i*(0x0101010101010101))>>56);
}

static void whiten_init(void)
{
	unsigned i, bv;
	uint8_t cl, cx;

	for (i=0; i<256; i++)
	{
		cx = 0;
		cl = i;
		for(bv=1;bv<0x100;bv<<=1)
		{
			if (cl & 1)
			{
				cx ^= bv;
				cl ^= 0x88;
			}
			cl >>= 1;
		}
		whitentbl[i*2] = cx;
		whitentbl[i*2+1] = cl;
	}
}

static void crc_init(void)
{
	uint32_t x, b, c;

	for(x=0;x<256;x++)
	{
		c = 0;
		for(b=1;b<0x100;b<<=1)
		{
			c>>=1;
			if (x&b)
			{
				c ^= 0xda6000;
			}
		}
		crctbl[x] = c;
	}
}


static int btlebb_init_done = 0;
void btlebb_init(void)
{
	if (!btlebb_init_done)
	{
		btlebb_init_done = 1;
		crc_init();
		whiten_init();
	}
}

uint8_t btlebb_whiten(uint8_t seed, uint8_t *buf, unsigned len)
{
	unsigned i;
	uint8_t lfsr;

	lfsr = seed | 0x40;

	for(i=0;i<len;i++)
	{
		buf[i] ^= whitentbl[lfsr*2];
		lfsr = whitentbl[lfsr*2+1];
	}
	return lfsr;
}

/* FIXME: Parametrable init crc */
uint32_t btlebb_calccrc(const uint8_t *buf, unsigned len)
{
	uint32_t i, crc;

	crc = 0xAAAAAA;
	for(i=0;i<len;i++)
	{
		crc = ((crc>>8)^crctbl[buf[i]^(crc&0xff)]);
	}
	return crc;
}

void btlebb_unpack(uint8_t *dst_bytes, const uint8_t *src_syms, uint8_t byte_count)
{
	unsigned i,j, b;

	for(i=0;i<byte_count;i++)
	{
		for(b=0, j=0; j<8; j++)
		{
			b |= *src_syms++<<j;
		}
		dst_bytes[i] = b;
	}
}

int btlebb_detect_adv_preamble(uint32_t channel_num, const uint8_t *syms, unsigned syms_count, phdr_t *phdr, unsigned *err)
{
	unsigned i, sub;
	uint64_t shift_reg, det40;
	uint16_t dw2;
	unsigned errcnt;

	if (syms_count < BTLEBB_DETECT_WINDOW)
		return -1;
	
	/* 40 bits of preamble to look for */
	det40 = ((uint64_t)ADV_ACCESS_ADDR<<8)|0xAA;

	/* Precompute whiten values for the pdu header*/
	dw2 = 0;
	btlebb_whiten(channel_num, (uint8_t*)&dw2, 2);

	for(shift_reg = 0, i=0; i<syms_count; i++, shift_reg>>=1)
	{
		shift_reg |= (uint64_t)syms[i]<<(BTLEBB_DETECT_WINDOW-1);

		/* Check hamming distance of preamble  */	
		if(i >= (BTLEBB_DETECT_WINDOW-1) && (errcnt=cnt_bits64((shift_reg&((1LL<<40)-1))^det40)) <= BTLEBB_DETECT_MAX_ERR)
		{
			/* Get dewhitened pdu hdr */
			phdr->u16 = (0xffff&(shift_reg>>40)) ^ dw2;

			/* More sanity checks */
			if (phdr->rfu != 0 || phdr->type > 8)
			{
				continue;
			}
			*err = errcnt;

			/* Return position of detected preamble */
			return i + 1 - BTLEBB_DETECT_WINDOW;
		}
	}
	return -1;
}
