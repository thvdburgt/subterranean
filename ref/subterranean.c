#include <assert.h> // assert
#include <string.h> // memcpy

#ifdef DEBUG
#include <stdio.h>
#endif

#include "subterranean.h"


// =====================================================================
//                               Constants
// =====================================================================

// ((12^i) mod 257) - 1 for i in [0, 256]
static uint8_t const EXP_12_X_MINUS_1[256] = {
	  0,  11, 143, 185, 175,  55, 157,  96, 135,  89,  51, 109,  34, 162, 156,  84,
	248, 160, 132,  53, 133,  65,  20, 251, 196,  50,  97, 147, 233, 237,  28,  90,
	 63, 253, 220,  81, 212, 242,  88,  39, 222, 105, 243, 100, 183, 151,  24,  42,
	  1,  23,  30, 114,  94, 111,  58, 193,  14, 179, 103, 219,  69,  68,  56, 169,
	240,  64,   8, 107,  10, 131,  41, 246, 136, 101, 195,  38, 210, 218,  57, 181,
	127, 250, 184, 163, 168, 228, 177,  79, 188, 211, 230, 201, 110,  46,  49,  85,
	  3,  47,  61, 229, 189, 223, 117, 130,  29, 102, 207, 182, 139, 137, 113,  82,
	224, 129,  17, 215,  21,   6,  83, 236,  16, 203, 134,  77, 164, 180, 115, 106,
	255, 244, 112,  70,  80, 200,  98, 159, 120, 166, 204, 146, 221,  93, 99, 171,
	  7,  95, 123, 202, 122, 190, 235,   4,  59, 205, 158, 108,  22,  18, 227, 165,
	192,   2,  35, 174,  43,  13, 167, 216,  33, 150,  12, 155,  72, 104, 231, 213,
	254, 232, 225, 141, 161, 144, 197,  62, 241,  76, 152,  36, 186, 187, 199,  86,
	 15, 191, 247, 148, 245, 124, 214,  9, 119, 154,  60, 217,  45,  37, 198,  74,
	128,   5,  71,  92,  87,  27,  78, 176,  67,  44,  25,  54, 145, 209, 206, 170,
	252, 208, 194,  26,  66,  32, 138, 125, 226, 153,  48,  73, 116, 118, 142, 173,
	 31, 126, 238,  40, 234, 249, 172,  19, 239,  52, 121, 178,  91,  75, 140, 149,
};
#define EXP_12(i) (EXP_12_X_MINUS_1[i] + 1)


// =====================================================================
//                           Private interface
// =====================================================================

// -------------------------- round function ---------------------------

/**
 * Returns a bitmask with the n least significant bits set to 1.
 */
static inline subterranean_rest_unit_t bit_mask_least_sig(uint_fast8_t n)
{
	assert(n <= SUBTERRANEAN_REST_UNIT_LEN);
	if (n == SUBTERRANEAN_REST_UNIT_LEN) {
		return (subterranean_rest_unit_t) ~0;
	} else {
		return ((subterranean_rest_unit_t) 1 << n) - 1;
	}
}

/**
 * Copies the state to B from input state with an offset of o applied to it.
 */
#define A     state->rest
#define A_len SUBTERRANEAN_REST_LEN
#define b     SUBTERRANEAN_REST_UNIT_LEN
static inline void rest_state_offset(
		subterranean_state * const state,
		subterranean_rest_unit_t * const B,
		uint_fast8_t const o_minus_1) // offset in rest
{
	uint_fast16_t o = o_minus_1 + 1;
	uint_fast16_t const neg_o_minus_1 = (257 - o - 1) % 257;
	uint_fast8_t const a_z = neg_o_minus_1 / b;
	uint_fast8_t const j = neg_o_minus_1 % b;
	subterranean_rest_unit_t const M_Z_P = bit_mask_least_sig(j);

	uint_fast8_t o_div_b = o / b;
	uint_fast8_t q = (o % b);
	uint_fast16_t p = b - q; // can be 8 bit type if b < 256
	subterranean_rest_unit_t M_P = bit_mask_least_sig(p);

	subterranean_rest_unit_t P, Q;
	for (uint_fast16_t a = 0; a < A_len; ++a) {
		// get the bytes P and Q needed from the input state
		P = A[(a + o_div_b) % A_len];

		// if necessary recalculate p and q
		if (a == a_z) {
			o_div_b = o_minus_1 / b;
			q = o_minus_1 % b;
			p = b - q;
			M_P = bit_mask_least_sig(p);
		}
		Q = A[(a + o_div_b + 1) % A_len];

		// construct the resulting byte
		if (a == a_z) {
			if (b - j < SUBTERRANEAN_REST_UNIT_LEN) {
				B[a] = (P >> (b - j)) & M_Z_P;
			} else {
				B[a] = 0;
			}
			B[a] |= state->zero & ((subterranean_rest_unit_t) 1 << j);
		} else {
			B[a] = (P >> q) & M_P;
		}
		if (p < SUBTERRANEAN_REST_UNIT_LEN) {
			B[a] |= Q << p;
		}
	}
}
#undef A
#undef A_len
#undef b

/**
 * Apply chi^{n} to the input state.
 */
static inline void chi(subterranean_state * const state)
{
	subterranean_rest_unit_t new_rest[SUBTERRANEAN_REST_LEN];
	memcpy(new_rest, state->rest, 256 / 8);

	// 12^0 = 1
	// 12^48 = 2
	uint_fast8_t o1_minus_1 = EXP_12_X_MINUS_1[state->n_mod_256];              // (1 * n) % 257)
	uint_fast8_t o2_minus_1 = EXP_12_X_MINUS_1[(state->n_mod_256 + 48) % 256]; // (2 * n) % 257)

#ifdef DEBUG
	size_t n = EXP_12(state->n_mod_256); // 12^r
	assert((1 * n) % 257 - 1 == o1_minus_1);
	assert((2 * n) % 257 - 1 == o2_minus_1);
#endif

	subterranean_rest_unit_t B1[SUBTERRANEAN_REST_LEN];
	rest_state_offset(state, B1, o1_minus_1);

	subterranean_rest_unit_t B2[SUBTERRANEAN_REST_LEN];
	rest_state_offset(state, B2, o2_minus_1);

	for (uint_fast16_t i = 0; i < SUBTERRANEAN_REST_LEN; ++i) {
		new_rest[i] ^= ~B1[i] & B2[i];
	}

	// set bit zero
	state->zero ^=
		~(state->rest[o1_minus_1 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(o1_minus_1 % SUBTERRANEAN_REST_UNIT_LEN))
		&
		(state->rest[o2_minus_1 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(o2_minus_1 % SUBTERRANEAN_REST_UNIT_LEN));
	state->zero &= 1;
	state->zero *= ~0;

	// copy over the new rest
	memcpy(state->rest, new_rest, 256 / 8);
}

/**
 * Apply iota to the input state.
 */
static inline void iota(subterranean_state * const state)
{
	state->zero ^= ~0;
}

/**
 * Apply theta^{n} to the input state.
 */
static inline void theta(subterranean_state * const state)
{
	subterranean_rest_unit_t new_rest[SUBTERRANEAN_REST_LEN];
	memcpy(new_rest, state->rest, 256 / 8);

	// 12^144 = 8
	uint_fast8_t o8_minus_1 = EXP_12_X_MINUS_1[(state->n_mod_256 + 144) % 256]; // (8 * n) % 257)

#ifdef DEBUG
	size_t n = EXP_12(state->n_mod_256); // 12^r
	assert((8 * n) % 257 - 1 == o8_minus_1);
#endif

	subterranean_rest_unit_t B3[SUBTERRANEAN_REST_LEN];
	rest_state_offset(state, B3, state->o3_minus_1);

	subterranean_rest_unit_t B8[SUBTERRANEAN_REST_LEN];
	rest_state_offset(state, B8, o8_minus_1);

	for (uint_fast16_t i = 0; i < SUBTERRANEAN_REST_LEN; ++i) {
		new_rest[i] ^= B3[i] ^ B8[i];
	}

	// set bit zero
	state->zero ^=
		(state->rest[state->o3_minus_1 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(state->o3_minus_1 % SUBTERRANEAN_REST_UNIT_LEN))
		^
		(state->rest[o8_minus_1 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(o8_minus_1 % SUBTERRANEAN_REST_UNIT_LEN));
	state->zero &= 1;
	state->zero *= ~0;

	// copy over the new rest
	memcpy(state->rest, new_rest, 256 / 8);
}

/**
 * Apply the round function to the input state.
 */
static void round_function(subterranean_state * const state)
{
#ifdef DEBUG2
	printf("\ninput       : ");
	print_state(state);

	chi(state);
	assert(state->zero == 0 || state->zero == (subterranean_rest_unit_t) ~0);
	printf("after chi   : ");
	print_state(state);

	iota(state);
	assert(state->zero == 0 || state->zero == (subterranean_rest_unit_t) ~0);
	printf("after iota  : ");
	print_state(state);

	theta(state);
	assert(state->zero == 0 || state->zero == (subterranean_rest_unit_t) ~0);
	printf("after theta : ");
	print_state(state);
	printf("-------------\n");
#else
	/* chi(state); */
	/* iota(state); */
	/* theta(state); */
#endif

	state->n_mod_256 += 1;
	state->exp_12_n = EXP_12(state->n_mod_256);
	state->o3_minus_1 = (3 * state->exp_12_n) % 257 - 1;
}

// ------------------------------ extract ------------------------------

/**
 * XOR j bytes in X with the extract from the input state.
 */
static inline void xor_with_extract(
		subterranean_state const * const state,
		uint8_t * const X,
		uint_fast8_t const X_len)
{
	assert(X_len <= 4);

	uint8_t i1, i2;
	subterranean_rest_unit_t s1, s2;
	for (uint_fast8_t k = 0; k < 8 * X_len; k += 1) {
		// index in evolving state
		i1 = (state->exp_12_n * (uint32_t) EXP_12(k << 2)) % 257 - 1;
		i2 = (state->exp_12_n * (uint32_t) (257 - EXP_12(k << 2))) % 257 - 1;

		// get s_{12^{4j}} and s_{-12^{4j}} from the rest state
		s1 = state->rest[i1 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(i1 % SUBTERRANEAN_REST_UNIT_LEN);
		s2 = state->rest[i2 / SUBTERRANEAN_REST_UNIT_LEN] >>
			(i2 % SUBTERRANEAN_REST_UNIT_LEN);

		// XOR bit in X with s_{12^{4j}} ^ s_{-12^{4j}}
		X[k/8] ^= ((s1 ^ s2) & 1) << (k % 8);
	}
}

// ------------------------------ duplex -------------------------------

/**
 * Add a bit to state at index j of G_{64}.
 */
static inline void add_bit_to_state(
		subterranean_state * const state,
		uint_fast8_t const j,
		uint32_t const src,
		uint_fast8_t const src_bit_index)
{
	assert(j <= 32);

	// index in evolving state
	uint8_t i = (state->exp_12_n * (uint32_t) EXP_12(j << 2)) % 257 - 1;

	// add bit to the state
	state->rest[i / SUBTERRANEAN_REST_UNIT_LEN] ^=
			(subterranean_rest_unit_t) ((src >> src_bit_index) & 1)
		<<
			(i % SUBTERRANEAN_REST_UNIT_LEN);
}

/**
 * Perform duplex on input state.
 */
static void duplex(
		subterranean_state * const state,
		uint8_t const * const sigma,
		uint_fast8_t sigma_len)
{
	assert(sigma != NULL || sigma_len == 0);
	assert(sigma_len <= 4);

	// s <- R(s)
	round_function(state);

	// add sigma to state bit for bit
	for (uint_fast8_t j = 0; j < sigma_len * 8; ++j) {
		add_bit_to_state(state, j, sigma[j / 8], j % 8);
	}

	// add the 1 bit of the padding
	add_bit_to_state(state, sigma_len * 8, 1, 0);
}

// ---------------------------- for absorb -----------------------------

/**
 * Absorb the block in the input state using operation op.
 * Adds temp to Y if operation is en- or decryption
 */
static void absorb_block(
		subterranean_state * const state,
		uint8_t * const Y,
		uint8_t const * const block, size_t const block_len,
		subterranean_operation_t const op)
{
	assert(block != NULL || block_len == 0);
	assert(block_len <= 4);

	uint8_t temp[4];
	if (op == SubterraneanOperationEncrypt || op == SubterraneanOperationDecrypt) {
		// temp <- x[i] + (extract truncated to length of x[i])
		memcpy(temp, block, block_len);
		xor_with_extract(state, temp, block_len);

		// Y <- Y || temp
		memcpy(Y, temp, block_len);
	}

	// if op = decrypt then duplex(temp) else duplex(x[i])
	duplex(state, op == SubterraneanOperationDecrypt ? temp : block, block_len);
	if (op == SubterraneanOperationUnkeyed) {
		duplex(state, NULL, 0);
	}
}


// =====================================================================
//                           Public interface
// =====================================================================

void subterranean_init( subterranean_state * const state) {
	assert(SUBTERRANEAN_REST_LEN * SUBTERRANEAN_REST_UNIT_LEN == 256);

	state->n_mod_256 = 0;
	state->o3_minus_1 = 3 - 1;
	state->zero = 0;
	memset(state->rest, 0x00, 32);
}

void subterranean_absorb(
		subterranean_state * const state,
		uint8_t *Y,
		uint8_t const *X,
		size_t X_len,
		subterranean_operation_t const op)
{
	assert(X != NULL || X_len == 0);

	uint_fast8_t w = op == SubterraneanOperationUnkeyed ? 1 : 4;

	// all blocks of w bits
	while (X_len >= w) {
		absorb_block(state, Y, X, w, op);

		if (Y != NULL) {
			Y += w;
		}
		X += w;
		X_len -= w;
	}

	// the last, strictly shorter, block
	absorb_block(state, Y, X, X_len, op);
}

void subterranean_blank(
		subterranean_state * const state,
		size_t r)
{
	for (; r > 0; r -= 1) {
		duplex(state, NULL, 0);
	}
}

void subterranean_squeeze(
		subterranean_state * const state,
		uint8_t *Z,
		size_t l)
{
	while (l != 0) {
		uint_fast8_t block_len = l < 4 ? l : 4;

		memset(Z, 0x00, block_len);
		xor_with_extract(state, Z, block_len);
		duplex(state, NULL, 0);

		l -= block_len;
		Z += block_len;
	}
}


// =====================================================================
//                           Debug functions
// =====================================================================

#ifdef DEBUG
void subterranean_rounds_lazy(
	subterranean_state * const state,
	size_t n)
{
	for (; n != 0; --n) {
		round_function(state);
	}
}

void subterranean_print_xor_with_extract(subterranean_state const * const state)
{
		uint8_t e[4] = {0};
		xor_with_extract(state, e, 4);

		uint32_t e_as_uint32_t = 0;
		for (size_t i = 0; i < 32; i += 1) {
			e_as_uint32_t |= (uint32_t) ((e[i / 8] >> (i % 8)) & 1) << i;
		}
		printf("extract: %08x\n", e_as_uint32_t);
}


// in U shift bit at index J to index I
#define GET_BIT_TO(U, J, I) ((J) > (I) ? (U) >> ((J) - (I)) : U << ((I) - (J)))

static inline void pi(subterranean_state * const state)
{
	subterranean_rest_unit_t new_rest[SUBTERRANEAN_REST_LEN] = {0};
	for (size_t i = 1; i < 257; ++i) {
		// -1 because the bit at index 0 is not included in the byte array
		size_t i_unit = (i-1) / SUBTERRANEAN_REST_UNIT_LEN;
		size_t i_bit  = (i-1) % SUBTERRANEAN_REST_UNIT_LEN;

		size_t j = (i * 12) % 257;
		assert(j != 0);
		size_t j_unit = (j-1) / SUBTERRANEAN_REST_UNIT_LEN;
		size_t j_bit =  (j-1) % SUBTERRANEAN_REST_UNIT_LEN;

		new_rest[i_unit] ^= (1 << i_bit) & GET_BIT_TO(state->rest[j_unit], j_bit, i_bit);
	}

	// copy over the new rest
	memcpy(state->rest, new_rest, 256 / 8);
}

/**
 * Print the state in hexidacimal format.
 */
void subterranean_print_state(subterranean_state const * const state)
{
	subterranean_state new_state;
	memcpy(&new_state, state, sizeof(subterranean_state));
	for (size_t n = 0; n < new_state.n_mod_256; n += 1) {
		pi(&new_state);
	}

	printf("%x ", new_state.zero & 1);
	for (size_t i = 0; i < SUBTERRANEAN_REST_LEN; ++i) {
		printf("%0*x ", (int) (SUBTERRANEAN_REST_UNIT_LEN / 4), new_state.rest[i]);
	}
	printf("\n");
}

#endif
