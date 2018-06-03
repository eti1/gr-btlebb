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

#ifndef INCLUDED_BTLEBB_DECODER_IMPL_H
#define INCLUDED_BTLEBB_DECODER_IMPL_H

#include <btlebb/decoder.h>

namespace gr {
	namespace btlebb {

		class decoder_impl : public decoder
		{
		 private:
			unsigned long d_cnt;
			unsigned d_channel;
			unsigned d_debug;
			unsigned d_pcnt;

		 public:
			decoder_impl(unsigned channel_num, unsigned debug);
			int work(int noutput_items,
				 gr_vector_const_void_star &input_items,
				 gr_vector_void_star &output_items);
			void set_channel(unsigned num);
		};

	} // namespace btlebb
} // namespace gr

#endif /* INCLUDED_BTLEBB_DECODER_IMPL_H */

