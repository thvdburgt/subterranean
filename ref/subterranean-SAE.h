#ifndef _subterranean_SAE_h_
#define _subterranean_SAE_h_

#include "subterranean.h"

#define SUBTERRANEAN_SAE_TAG_LENGTH (128/8) // in bytes

void subterranean_SAE_start(
		subterranean_state *state,
		uint8_t const *K, size_t K_len,
		uint8_t const *N, size_t N_len);

int subterranean_SAE_wrap(
		subterranean_state *state,
		uint8_t *Y, uint8_t *T,
		uint8_t const *A, size_t A_len,
		uint8_t const *X, size_t X_len,
		uint8_t const *T_prime,
		subterranean_operation_t op);

#endif
