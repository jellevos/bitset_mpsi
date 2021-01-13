//
// Created by jelle on 09-10-20.
//
#include <thread>
#include <future>
#include "psi_protocols.h"

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

std::vector<ZZ> generate_ciphertexts(std::vector<long> &client_set, long domain_size, Keys &keys) {
    /// Step 1
    BitSet client_bitset(client_set, domain_size);
    client_bitset.invert();

    /// Step 2
    std::vector<ZZ> ciphertexts;
    client_bitset.encrypt_all(ciphertexts, keys.public_key);

    return ciphertexts;
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


// TODO: Clean up
// TODO: Fix all file headers
std::vector<long> multiparty_psi(std::vector<std::vector<long>> client_sets,
                                 std::vector<long> leader_set, long domain_size, long threshold_l, Keys &keys) {
    /// Step 1-2. Clients compute their bitset, invert it and encrypt it
    std::vector<std::future<std::vector<ZZ>>> ciphertexts_futures;
    ciphertexts_futures.reserve(client_sets.size());
    for (auto & client_set : client_sets) {
        ciphertexts_futures.push_back(
                std::async(std::launch::async, generate_ciphertexts, std::ref(client_set), domain_size,
                           std::ref(keys)));
    }

    // Wait till the processing is done
    await_futures(ciphertexts_futures);

    // Extract the generated EIBFs from the clients
    std::vector<std::vector<ZZ>> client_ciphertexts;
    client_ciphertexts.reserve(client_sets.size());
    for (std::future<std::vector<ZZ>> &future : ciphertexts_futures) {
        client_ciphertexts.push_back(future.get());
    }

    /// Step 3
    // Clients send their ciphertexts to the server

    /// Step 4
    std::vector<ZZ> aggregated;
    aggregated.reserve(leader_set.size());
    // Instantiate with the first client's elements
    for (long element : leader_set) {
        aggregated.push_back(client_ciphertexts.at(0).at(element));
    }
    // Add other clients' elements
    for (int i = 0; i < leader_set.size(); ++i) {
        for (int j = 1; j < client_ciphertexts.size(); ++j) {
            aggregated.at(i) = add_homomorphically(aggregated.at(i), client_ciphertexts.at(j).at(leader_set.at(i)),
                                                   keys.public_key);
        }
        // TODO: Randomize?
    }

    /// Step 5
    // Server sends ciphertexts to several parties to decrypt-to-zero

    /// Step 6
    std::vector<std::future<std::vector<ZZ>>> randomization_futures;
    randomization_futures.reserve(client_sets.size());
    for (int i = 0; i < client_sets.size(); ++i) {
        randomization_futures.push_back(std::async(std::launch::async, randomize_ciphertexts, aggregated, std::ref(keys)));
    }

    // Wait till the processing is done
    await_futures(randomization_futures);

    // Extract the randomized ciphertexts from the clients
    std::vector<std::vector<ZZ>> client_randomizations;
    client_randomizations.reserve(client_sets.size());
    for (std::future<std::vector<ZZ>> &future : randomization_futures) {
        client_randomizations.push_back(future.get());
    }

    // Sum up all clients' randomized ciphertexts
    std::vector<ZZ> randomized_ciphertexts = client_randomizations.at(0);
    for (int i = 1; i < client_randomizations.size(); ++i) {
        for (int j = 0; j < aggregated.size(); ++j) {
            randomized_ciphertexts.at(j) = add_homomorphically(randomized_ciphertexts.at(j), client_randomizations.at(i).at(j), keys.public_key);
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

    return intersection;
}
