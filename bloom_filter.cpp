//
// Created by Jelle Vos on 09-10-20.
//

#include "bloom_filter.h"

BitSet::BitSet(std::vector<long> set, long domain_size): storage(domain_size) {
    for (long element : set) {
        storage.at(element) = true;
    }
}

/// Inverts the Bloom filter in place so that all 0s become 1s and all 1s become 0s
void BitSet::invert() {
    for (long i = 0; i < this->storage.size(); ++i) {
        this->storage.at(i) = !this->storage.at(i);
    }
}

/// Returns the bit-by-bit ciphertexts of the encrypted Bloom filter
void BitSet::encrypt_all(std::vector<ZZ> &ciphertexts, PublicKey &public_key) {
    ciphertexts.reserve(this->storage.size());
    for (bool element : this->storage) {
        ciphertexts.push_back(encrypt(ZZ(element), public_key));
    }
}
