#include "crypto_hash.h"

#include "api.h"
#include "subterranean-XOF.h"

int crypto_hash(
		unsigned char *out,
		const unsigned char *in,
		unsigned long long inlen)
{
	subterranean_XOF(out, CRYPTO_BYTES, &in, (size_t const *) &inlen, 1);
	return 0;
}
