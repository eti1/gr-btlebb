/* -*- c++ -*- */

#define BTLEBB_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "btlebb_swig_doc.i"

%{
#include "btlebb/decoder.h"
%}


%include "btlebb/decoder.h"
GR_SWIG_BLOCK_MAGIC2(btlebb, decoder);
