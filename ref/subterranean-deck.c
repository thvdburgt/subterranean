#include "subterranean-deck.h"

void subterranean_Deck(
		uint8_t *Z, size_t Z_len,
		uint8_t const * K, size_t K_len,
		uint8_t const * const Ms[], size_t const M_lens[], size_t len)
{
	subterranean_state state;
	subterranean_init(&state);

	subterranean_absorb(&state, NULL, K, K_len, SubterraneanOperationKeyed);

	for (size_t i = 0; i < len; i += 1) {
		subterranean_absorb(&state, NULL, Ms[i], M_lens[i], SubterraneanOperationKeyed);
	}

	subterranean_blank(&state, 8);
	subterranean_squeeze(&state, Z, Z_len);
}
