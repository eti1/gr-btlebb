/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.	If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdint.h>

#include <gnuradio/io_signature.h>
#include "decoder_impl.h"
extern "C" 
{
#include "btlebb.h"
}

void printhex(uint8_t *b, unsigned len)
{
	unsigned i;

	for(i=0;i<len;i++)
		printf("%02x ", b[i]);
	puts("");
}

namespace gr {
	namespace btlebb {

		decoder::sptr
		decoder::make(unsigned channel_num, unsigned debug)
		{
			return gnuradio::get_initial_sptr
				(new decoder_impl(channel_num, debug));
		}

		decoder_impl::decoder_impl(unsigned channel_num, unsigned debug)
			: gr::sync_block("decoder",
							gr::io_signature::make(1, 1, 1),
							gr::io_signature::make(0, 0, 0))
			, d_cnt(0)
			, d_channel(channel_num)
			, d_debug(debug)
			, d_pcnt(0)
		{
			btlebb_init();
			message_port_register_out(pmt::mp("out"));
		}

		int
		decoder_impl::work(int noutput_items,
				gr_vector_const_void_star &input_items,
				gr_vector_void_star &output_items)
		{
			const uint8_t *syms = (const uint8_t *) input_items[0];
			int sync_offset;
			unsigned datalen, errcnt;
			uint8_t pdu[2+255+3];
			uint32_t crccal, crcval;
			phdr_t phdr;
			int consume = 0;
			pmt::pmt_t blob;
            pmt::pmt_t dict;

			/* Check if enough bits for a preamble+pdu_header+crc */
			if(noutput_items < BTLEBB_DETECT_WINDOW + (2+3)*8)
			{
				consume = 0;
				goto ret;
			}

			/* Run advertisement detection */
			if((sync_offset = btlebb_detect_adv_preamble(d_channel, syms, noutput_items, &phdr, &errcnt)) == -1)
			{
				consume = noutput_items - (BTLEBB_DETECT_WINDOW-1);
				goto ret;
			}

			/* Advertisement found, enough data for pdu header + pdu len + crc ? */
			datalen = 2 + phdr.len + 3;
			if (sync_offset + BTLEBB_PDUHDR_OFFSET + 8*datalen > noutput_items)
			{
				consume = sync_offset;
				goto ret;
			}

			/* Read data, unwhiten, compute crc */
			btlebb_unpack(pdu, syms + sync_offset + BTLEBB_PDUHDR_OFFSET, datalen);
			btlebb_whiten(d_channel, pdu, datalen);
			crccal = btlebb_calccrc(pdu, 2+phdr.len);
			crcval = 0xffffff & *(uint32_t*)(pdu+2+phdr.len);

			if (crccal != crcval)
			{
				printf("%d: PDU (BAD): ", d_cnt+sync_offset);
				printhex(pdu, datalen);

				/* Invalid packet, consume up to next detection window */
				consume = sync_offset+1;
				goto ret;
			}

			/* Packet detected */
			if (d_debug)
			{
				printf("%d: PDU (GOOD=%4d): ", d_cnt+sync_offset,++d_pcnt);
				printhex(pdu, datalen);
			}

			/* Publish message */
			blob = pmt::make_blob(pdu, datalen);
			dict = pmt::make_dict();
			dict = pmt::dict_add(dict, pmt::mp("channel"), pmt::from_uint64(d_channel));
			dict = pmt::dict_add(dict, pmt::mp("addr"), pmt::from_uint64(ADV_ACCESS_ADDR));
			dict = pmt::dict_add(dict, pmt::mp("addr_errors"), pmt::from_uint64(errcnt));
			message_port_pub(pmt::mp("out"), pmt::cons(dict, blob));

			/* Consume up to packet end */
			consume = sync_offset+8*datalen;
		ret:
			if(consume)
			{
				d_cnt += consume;
				consume_each(consume);
			}
			return 0;
		}

	} /* namespace btlebb */
} /* namespace gr */

