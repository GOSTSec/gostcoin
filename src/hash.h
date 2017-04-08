// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_HASH_H
#define BITCOIN_HASH_H

#include "uint256.h"
#include "serialize.h"
#include "util.h"
#include "Gost.h"

#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <vector>
#include <sstream>

template<typename T1>
inline uint256 Hash(const T1 pbegin, const T1 pend)
{
	// GOST 34.11-256 (GOST 34.11-512 (...))
    static unsigned char pblank[1];
    uint8_t hash1[64];
    i2p::crypto::GOSTR3411_2012_512 ((pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), hash1);	
	uint32_t digest[8];
    i2p::crypto::GOSTR3411_2012_256 (hash1, 64, (uint8_t *)digest);
	// to little endian
	uint256 hash2;	
	for (int i = 0; i < 8; i++)
		hash2.pn[i] = ByteReverse (digest[7-i]);
    return hash2;
}

class CHashWriter
{
private:
    std::stringstream ctx;

public:
    int nType;
    int nVersion;

    void Init() {
        ctx.str("");
    }

    CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {
        Init();
    }

    CHashWriter& write(const char *pch, size_t size) {
		ctx.write (pch, size);
        return (*this);
    }

    // invalidates the object
    uint256 GetHash() {
		uint8_t hash1[64];
    	i2p::crypto::GOSTR3411_2012_512 ((uint8_t *)ctx.str ().c_str (), ctx.str ().length (), hash1);
        uint256 hash2;
        i2p::crypto::GOSTR3411_2012_256 (hash1, 64, (unsigned char*)&hash2);
        return hash2;
    }

    template<typename T>
    CHashWriter& operator<<(const T& obj) {
        // Serialize to this stream
        ::Serialize(*this, obj, nType, nVersion);
        return (*this);
    }
};


template<typename T1, typename T2>
inline uint256 Hash(const T1 p1begin, const T1 p1end,
                    const T2 p2begin, const T2 p2end)
{
	static unsigned char pblank[1];
	size_t s1 = (p1end - p1begin) * sizeof(p1begin[0]),
		   s2 = (p2end - p2begin) * sizeof(p2begin[0]);	

	uint8_t * buf = new uint8_t[s1+s2];
	memcpy (buf, (unsigned char*)&p1begin[0], s1);
	memcpy (buf + s1, (unsigned char*)&p1begin[0], s2);
	uint8_t hash1[64];
	i2p::crypto::GOSTR3411_2012_512 ((s1 + s2) ? buf : pblank, s1 + s2, hash1); 
	delete[] buf;	
	uint256 hash2;
	i2p::crypto::GOSTR3411_2012_256 (hash1, 64, (unsigned char*)&hash2);
    return hash2;
}


template<typename T>
uint256 SerializeHash(const T& obj, int nType=SER_GETHASH, int nVersion=PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

template<typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1];
    uint8_t hash1[64];
    i2p::crypto::GOSTR3411_2012_512((pbegin == pend ? pblank : (unsigned char*)&pbegin[0]), (pend - pbegin) * sizeof(pbegin[0]), hash1);
    uint160 hash2;
    RIPEMD160(hash1, 64, (unsigned char*)&hash2);
    return hash2;
}

inline uint160 Hash160(const std::vector<unsigned char>& vch)
{
    return Hash160(vch.begin(), vch.end());
}

unsigned int MurmurHash3(unsigned int nHashSeed, const std::vector<unsigned char>& vDataToHash);

#endif
