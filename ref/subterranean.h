#ifndef _subterranean_h_
#define _subterranean_h_

#include <stdint.h> // uint8_t etc.
#include <stddef.h> // size_t
#include <limits.h> // CHAR_BIT

#define SUBTERRANEAN_SIZE 257

typedef uint32_t subterranean_rest_unit_t;

#define SUBTERRANEAN_REST_UNIT_LEN (sizeof(subterranean_rest_unit_t) * CHAR_BIT)
#define SUBTERRANEAN_REST_LEN      (256 / SUBTERRANEAN_REST_UNIT_LEN)

typedef enum {
	SubterraneanOperationUnkeyed,
	SubterraneanOperationKeyed,
	SubterraneanOperationEncrypt,
	SubterraneanOperationDecrypt,
} subterranean_operation_t;

typedef struct {
	// current round modulo 256
	uint_fast8_t n_mod_256;
	// 12^n
	uint_fast16_t exp_12_n;
	// offset in state of s_3
	uint_fast8_t o3_minus_1;
	// filled with bits equal to s_0, so either 0 or ~0
	subterranean_rest_unit_t zero;
	// s_i for i in [1,257), grouped in units of size SUBTERRANEAN_REST_UNIT_LEN
	subterranean_rest_unit_t rest[SUBTERRANEAN_REST_LEN];
} subterranean_state;

void subterranean_init(subterranean_state *state);
void subterranean_absorb(
		subterranean_state *state,
		uint8_t *Y,
		uint8_t const *X, size_t X_len,
		subterranean_operation_t op);
void subterranean_blank(subterranean_state *state, size_t r);
void subterranean_squeeze(subterranean_state *state, uint8_t *Z, size_t l);

#endif
