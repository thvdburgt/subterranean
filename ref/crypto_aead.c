#include "crypto_aead.h"

#include "api.h"
#include "subterranean-SAE.h"

int crypto_aead_encrypt(
        unsigned char *c, unsigned long long *clen,
        const unsigned char *m, unsigned long long mlen,
        const unsigned char *ad, unsigned long long adlen,
        const unsigned char *nsec,
        const unsigned char *npub,
        const unsigned char *k
        )
{
    subterranean_state state;
    subterranean_SAE_start(&state, k, CRYPTO_KEYBYTES, npub, CRYPTO_NPUBBYTES);
    subterranean_SAE_wrap(&state,
            c, c + mlen,
            ad, adlen,
            m, mlen,
            NULL, SubterraneanOperationEncrypt);

    *clen = mlen + SUBTERRANEAN_SAE_TAG_LENGTH;
    return 0;
}

int crypto_aead_decrypt(
        unsigned char *m, unsigned long long *mlen,
        unsigned char *nsec,
        const unsigned char *c, unsigned long long clen,
        const unsigned char *ad, unsigned long long adlen,
        const unsigned char *npub,
        const unsigned char *k
        )
{
    *mlen = clen - SUBTERRANEAN_SAE_TAG_LENGTH;

    subterranean_state state;
    subterranean_SAE_start(&state, k, CRYPTO_KEYBYTES, npub, CRYPTO_NPUBBYTES);

    return subterranean_SAE_wrap(&state,
            m, NULL,
            ad, adlen,
            c, clen - SUBTERRANEAN_SAE_TAG_LENGTH,
            c + *mlen, SubterraneanOperationDecrypt);
}
