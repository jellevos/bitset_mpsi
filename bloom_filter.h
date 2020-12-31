//
// Created by jelle on 09-10-20.
//

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <vector>
#include "threshold_paillier.h"

using namespace NTL;

class BitSet {
public:
    BitSet(std::vector<long> set, long domain_size);

    void insert(long element);
    bool contains(long element);
    void invert();
    void encrypt_all(std::vector<ZZ> &ciphertexts, PublicKey &public_key);

private:
    std::vector<bool> storage;
};

#endif //BLOOM_FILTER_H
