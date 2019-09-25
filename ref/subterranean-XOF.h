#ifndef _subterranean_XOF_h_
#define _subterranean_XOF_h_

#include "subterranean.h"

void subterranean_XOF(
		uint8_t *Z, size_t Z_len,
		uint8_t const * const Ms[], size_t const M_lens[], size_t len);

#endif
