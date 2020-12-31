//
// Created by jelle on 09-10-20.
//
#include <thread>
#include <future>
#include "psi_protocols.h"

//template <class T>
//void await_futures(std::vector<std::future<T>> &futures) {
//    bool processing = true;
//    while (processing) {
//        processing = false;
//
//        for (auto &future : futures) {
//            if (not future.valid()) {
//                processing = true;
//                break;
//            }
//        }
//    }
//}

//std::vector<long> multiparty_psi(std::vector<std::vector<long>> sets,
//                                 long threshold_l,
//                                 long m_bits, long k_hashes,
//                                 Keys &keys) {
//    std::vector<std::vector<long>> client_sets;
//    client_sets.reserve(sets.size() - 1);
//    for (int i = 0; i < sets.size() - 1; ++i) {
//        client_sets.push_back(sets.at(i));
//    }
//
//    std::vector<long> server_set = sets.at(sets.size() - 1);
//
//    return multiparty_psi(client_sets, server_set, threshold_l, m_bits, k_hashes, keys);
//}

// TODO: Clean up
// TODO: Fix all file headers
std::vector<long> multiparty_psi(std::vector<std::vector<long>> client_sets,
                                 std::vector<long> server_set, long domain_size, Keys &keys) {

}
