#include <assert.h>
#include <string.h> // memcmp/memset

#include "subterranean-SAE.h"

void subterranean_SAE_start(
		subterranean_state * const state,
		uint8_t const * const K, size_t const K_len,
		uint8_t const * const N, size_t const N_len)
{
	/* S <= subterranean() */
	subterranean_init(state);
	/* S.absorb(K, keyed) */
	subterranean_absorb(state, NULL, K, K_len, SubterraneanOperationKeyed);
	/* S.absorb(N, keyed) */
	subterranean_absorb(state, NULL, N, N_len, SubterraneanOperationKeyed);
	/* S.blank(8) */
	subterranean_blank(state, 8);
}

int subterranean_SAE_wrap(
		subterranean_state * const state,
		uint8_t * const Y, uint8_t *T,
		uint8_t const * const A, size_t const A_len,
		uint8_t const * const X, size_t const X_len,
		uint8_t const * const T_prime, subterranean_operation_t op)
{
	assert(op == SubterraneanOperationEncrypt || op == SubterraneanOperationDecrypt);
	assert(T_prime != NULL || op == SubterraneanOperationEncrypt);
	assert(T != NULL || op == SubterraneanOperationDecrypt);

	/* make sure we have a place to store the new tag in */
	uint8_t new_tag[SUBTERRANEAN_SAE_TAG_LENGTH];
	if (T == NULL) {
		T = new_tag;
	}

	/* S.absorb(A, keyed) */
	subterranean_absorb(state, NULL, A, A_len, SubterraneanOperationKeyed);
	/* Y <= S.absorb(X, op) */
	subterranean_absorb(state, Y, X, X_len, op);
	/* S.blank(8) */
	subterranean_blank(state, 8);
	/* T <= S.squeeze(tau) */
	subterranean_squeeze(state, T, SUBTERRANEAN_SAE_TAG_LENGTH);

	/* if op = decrypt AND (T' != T) then (Y, T) = (epsilon, epsilon) */
	if (op == SubterraneanOperationDecrypt && memcmp(T_prime, T, SUBTERRANEAN_SAE_TAG_LENGTH) != 0) {
		// (Y,T) = (epsilon, epsilon)
		memset(Y, 0x00, X_len);
		if (T != NULL) {
			memset(T, 0x00, SUBTERRANEAN_SAE_TAG_LENGTH);
		}

		return 1;
	} else {
		return 0;
	}
}
