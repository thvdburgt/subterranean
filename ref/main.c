#include <stdio.h>

#include "subterranean.h"
#include "subterranean-SAE.h"

void subterranean_print_state(subterranean_state * const state);
uint32_t subterranean_print_extract(subterranean_state const * const state);
void subterranean_rounds_lazy(subterranean_state * const state, size_t n);

int main() {

	subterranean_state state;
	uint8_t K[16];
	uint8_t N[16];
	uint8_t A[16] = {0};
	uint8_t X[16] = {0};
	uint8_t Y[16] = {0};
	uint8_t T[SUBTERRANEAN_SAE_TAG_LENGTH] = {0};

	printf("K: ");
	for (unsigned i = 0; i < sizeof(K); i++) {
		K[i] = i;
		printf("%02x", K[i]);
	}
	printf("\n");

	printf("N: ");
	for (unsigned i = 0; i < sizeof(N); i++) {
		N[i] = i;
		printf("%02x", N[i]);
	}
	printf("\n\n");

	subterranean_SAE_start(&state, K, sizeof(K), N, sizeof(N));
	subterranean_SAE_wrap(&state, Y, T, A, 16, X, 16, NULL, SubterraneanOperationEncrypt);

	printf("Y: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x", Y[i]);
	}
	printf("\nT: ");
	for (int i = 0; i < SUBTERRANEAN_SAE_TAG_LENGTH; i++) {
		printf("%02x", T[i]);
	}

	/* size_t n = 1000; */
	/* printf("%zu rounds\n", n); */
	/* { */
	/* 	printf("-------------\n"); */
	/* 	subterranean_state s; */
	/* 	subterranean_init(&s); */
	/* 	subterranean_rounds_lazy(&s, n); */
	/* 	printf("CT: "); */
	/* 	subterranean_print_state(&s); */
	/* 	subterranean_print_extract(&s); */
	/* } */

	return 0;
}
