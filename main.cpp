#include <iostream>
#include <vector>
#include "psi_protocols.h"
#include "benchmarking.h"
// #include "NTL/BasicThreadPool.h"


// TODO: Allow variable set sizes?
int main() {
    BitSet bit_set({1, 3, 5, 7}, 10);

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
