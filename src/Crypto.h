#ifndef CRYPTO_H__
#define CRYPTO_H__

#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>

#include <openssl/opensslv.h>
#if (OPENSSL_VERSION_NUMBER < 0x010100000) || defined(LIBRESSL_VERSION_NUMBER) // 1.1.0 or LibreSSL
// define getters and setters introduced in 1.1.0

inline int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
	{
		if (sig->r) BN_free (sig->r);
		if (sig->s) BN_free (sig->s);
		sig->r = r; sig->s = s; return 1;
	}
inline void ECDSA_SIG_get0(const ECDSA_SIG *sig, const BIGNUM **pr, const BIGNUM **ps)
	{ *pr = sig->r; *ps = sig->s; }

// EVP
inline int EVP_CIPHER_CTX_reset(EVP_CIPHER_CTX *ctx)
	{
		return EVP_CIPHER_CTX_cleanup (ctx);
	}

// ssl
#define TLS_method TLSv1_method

// ASN1
#define OPENSSL_EC_EXPLICIT_CURVE 0

#endif

#endif

