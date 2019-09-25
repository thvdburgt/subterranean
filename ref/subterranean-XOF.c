#include "subterranean-XOF.h"

void subterranean_XOF(
		uint8_t *Z, size_t const Z_len,
		uint8_t const * const Ms[], size_t const M_lens[], size_t const len)
{
	subterranean_state state;
	subterranean_init(&state);

	for (size_t i = 0; i < len; i += 1) {
		subterranean_absorb(&state, NULL, Ms[i], M_lens[i], SubterraneanOperationUnkeyed);
	}

	subterranean_blank(&state, 8);
	subterranean_squeeze(&state, Z, Z_len);
}
