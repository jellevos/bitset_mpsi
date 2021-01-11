#include <iostream>
#include <vector>
#include <future>
#include "psi_protocols.h"
#include "benchmarking.h"
// #include "NTL/BasicThreadPool.h"

template <class T>
void await_futures(std::vector<std::future<T>> &futures) {
    bool processing = true;
    while (processing) {
        processing = false;

        for (auto &future : futures) {
            if (not future.valid()) {
                processing = true;
                break;
            }
        }
    }
}

std::vector<ZZ> randomize_ciphertexts(std::vector<ZZ> ciphertexts, Keys &keys) {
    std::vector<ZZ> randomized_ciphertexts;
    randomized_ciphertexts.reserve(ciphertexts.size());

    for (const ZZ& ciphertext : ciphertexts) {
        ZZ random = Gen_Coprime(keys.public_key.n);
        randomized_ciphertexts.push_back(multiply_homomorphically(ciphertext, random, keys.public_key));
    }

    return randomized_ciphertexts;
}

std::vector<std::pair<long, ZZ>> compute_decryption_shares(std::vector<ZZ> ciphertexts, long client_id, Keys &keys) {
    std::vector<std::pair<long, ZZ>> decryption_shares;
    decryption_shares.reserve(ciphertexts.size());

    for (auto ciphertext : ciphertexts) {
        decryption_shares.emplace_back(client_id + 1, partial_decrypt(ciphertext, keys.public_key,
                                                                      keys.private_keys.at(client_id)));
    }

    return decryption_shares;
}

// TODO: Allow variable set sizes?
int main() {
    long domain_size = 10;
    long threshold_l = 2;

    /// Setup
    Keys keys;
    key_gen(&keys, 1024, threshold_l, 3);

    /// Step 1
    std::vector<BitSet> client_bitsets = {BitSet({1, 3, 5, 7}, domain_size), BitSet({1, 4, 8, 5}, domain_size)};
    std::vector<long> leader_set = {5, 1, 2, 4};
    BitSet leader_bitset(leader_set, domain_size);

    for (BitSet &client_bitset : client_bitsets) {
        client_bitset.invert();
    }
    leader_bitset.invert();

    /// Step 2
    std::vector<std::vector<ZZ>> element_ciphertexts;
    element_ciphertexts.reserve(domain_size);
    for (int i = 0; i < domain_size; ++i) {
        element_ciphertexts.emplace_back();
    }

    for (BitSet client_set : client_bitsets) {
        std::vector<ZZ> ciphertexts;
        client_set.encrypt_all(ciphertexts, keys.public_key);

        for (int i = 0; i < ciphertexts.size(); ++i) {
            element_ciphertexts.at(i).push_back(ciphertexts.at(i));
        }
    }

    std::vector<ZZ> leader_ciphertext;
    leader_bitset.encrypt_all(leader_ciphertext, keys.public_key);

    /// Step 3
    // Clients send their ciphertexts to the server

    /// Step 4
    std::vector<ZZ> aggregated;
    aggregated.reserve(leader_set.size());
    for (long element : leader_set) {
        ZZ sum = sum_homomorphically(element_ciphertexts.at(element), keys.public_key);
        sum = add_homomorphically(sum, leader_ciphertext.at(element), keys.public_key);
        aggregated.push_back(sum);
        // TODO: Randomize?
    }

    /// Step 5
    // Server sends ciphertexts to several parties to decrypt-to-zero

    /// Step 6
    std::vector<std::future<std::vector<ZZ>>> randomization_futures;
    randomization_futures.reserve(client_bitsets.size());
    for (int i = 0; i < client_bitsets.size(); ++i) {
        randomization_futures.push_back(std::async(std::launch::async, randomize_ciphertexts, aggregated, std::ref(keys)));
    }

    // Wait till the processing is done
    await_futures(randomization_futures);

    // Extract the randomized ciphertexts from the clients
    std::vector<std::vector<ZZ>> client_ciphertexts;
    client_ciphertexts.reserve(client_bitsets.size());
    for (std::future<std::vector<ZZ>> &future : randomization_futures) {
        client_ciphertexts.push_back(future.get());
    }

    // Sum up all clients' randomized ciphertexts
    std::vector<ZZ> randomized_ciphertexts = client_ciphertexts.at(0);
    for (int i = 1; i < client_ciphertexts.size(); ++i) {
        for (int j = 0; j < aggregated.size(); ++j) {
            randomized_ciphertexts.at(j) = add_homomorphically(randomized_ciphertexts.at(j), client_ciphertexts.at(i).at(j), keys.public_key);
        }
    }

    // Partial decryption (let threshold + 1 parties decrypt)
    std::vector<std::future<std::vector<std::pair<long, ZZ>>>> decryption_share_futures;
    decryption_share_futures.reserve(threshold_l + 1);
    for (int i = 0; i < (threshold_l + 1); ++i) {
        decryption_share_futures.push_back(std::async(std::launch::async, compute_decryption_shares, randomized_ciphertexts, i, std::ref(keys)));
    }

    // Wait till the processing is done
    await_futures(decryption_share_futures);

    // Extract the decryption shares from the clients
    std::vector<std::vector<std::pair<long, ZZ>>> client_decryption_shares;
    client_decryption_shares.reserve(threshold_l + 1);
    for (std::future<std::vector<std::pair<long, ZZ>>> &future : decryption_share_futures) {
        client_decryption_shares.push_back(future.get());
    }

    /// Step 7
    std::vector<ZZ> decryptions;
    decryptions.reserve(aggregated.size());

    for (int i = 0; i < aggregated.size(); ++i) {
        std::vector<std::pair<long, ZZ>> ciphertext_decryption_shares;
        ciphertext_decryption_shares.reserve(client_decryption_shares.size());

        for (auto & client_decryption_share : client_decryption_shares) {
            ciphertext_decryption_shares.push_back(client_decryption_share.at(i));
        }

        decryptions.push_back(combine_partial_decrypt(ciphertext_decryption_shares, keys.public_key));
    }

    /// Step 8
    std::vector<long> intersection;
    for (int i = 0; i < aggregated.size(); ++i) {
        if (decryptions.at(i) == 0) {
            intersection.push_back(leader_set.at(i));
        }
    }

    std::cout << "ITEMS: " << std::endl;
    for (long element : intersection) {
        std::cout << element << std::endl;
    }

//    Keys keys;
//    key_gen(&keys, 1024, 2, 3);
//
//    std::vector<long> client1_set({1, 3, 5, 7});
//    std::vector<long> client2_set({2, 3, 4, 5});
//    std::vector<long> server_set({6, 5, 2, 1});
//
//    std::cout << "Computing the set intersection between multiple parties using a (2, 3)-encryption of 1024 bits."
//              << std::endl;
//
//    auto start = std::chrono::high_resolution_clock::now();
//    std::vector<long> result = multiparty_psi(std::vector({client1_set, client2_set}), server_set,
//                                              2,
//                                              16, 4,
//                                              keys);
//    auto stop = std::chrono::high_resolution_clock::now();
//
//    std::cout << "The resulting set intersection was: { ";
//    for (long element : result) {
//        std::cout << element << " ";
//    }
//    std::cout << "}." << std::endl;
//    std::cout << "Took: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms."
//        << std::endl << std::endl;

    return 0;
}
