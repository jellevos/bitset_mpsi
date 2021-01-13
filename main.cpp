#include <iostream>
#include <vector>
#include <future>
#include <unistd.h>
#include <random>
#include <climits>
#include "psi_protocols.h"
#include "benchmarking.h"
// #include "NTL/BasicThreadPool.h"


std::vector<long> sample_set(long set_size, long domain_size) {
    std::default_random_engine generator(rand());
    std::uniform_int_distribution<long> distribution(0, domain_size);

    std::vector<long> set;
    set.reserve(set_size);

    while (set.size() < set_size) {
        long element = distribution(generator);
        bool duplicate = false;

        for (long other_element : set) {
            if (element == other_element) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            set.push_back(element);
        }
    }

    return set;
}

double sample_mean(const std::vector<long>& measurements) {
    // Computes the sample mean
    double sum = 0;

    for (long measurement : measurements) {
        sum += measurement;
    }

    return sum / measurements.size();
}


double sample_std(const std::vector<long>& measurements, double mean) {
    // Computes the corrected sample standard deviation
    double sum = 0;

    for (long measurement : measurements) {
        sum += pow(measurement - mean, 2);
    }

    return sqrt(sum / (measurements.size() - 1.0));
}

// TODO: Allow variable set sizes?
int main(int argc, char *argv[]) {
    // Runs MPSI protocol a set number of times and reports the mean and std
    // -n <set size> = 16
    // -t <party count> = 5
    // -l <threshold> = 3
    // -d <domain size> = 256
    // -r <repetitions> = 10
    long set_size = 16;
    long party_count = 5;
    long threshold = 3;
    long domain_size = 256;
    long repetitions = 10;

    int option;
    while ((option = getopt(argc, argv, "n:t:l:d:r:")) != -1) { //get option from the getopt() method
        switch (option) {
            //For option i, r, l, print that these are options
            case 'n':
                set_size = std::stol(optarg);
                break;
            case 't':
                party_count = std::stol(optarg);
                break;
            case 'l':
                threshold = std::stol(optarg);
                break;
            case 'd':
                domain_size = std::stol(optarg);
                break;
            case 'r':
                repetitions = std::stol(optarg);
                break;
            case '?': //used for some unknown options
                printf("unknown option: %c\n", optopt);
                break;
        }
    }

    // Generate client sets
    std::vector<std::vector<long>> client_sets;
    client_sets.reserve(party_count - 1);
    for (int i = 0; i < party_count; ++i) {
        client_sets.push_back(sample_set(set_size, domain_size));
    }

    // Generate leader set
    std::vector<long> leader_set = sample_set(set_size, domain_size);

    /// Setup
    Keys keys;
    key_gen(&keys, 1024, threshold, party_count);

    /// Execute protocol
    std::vector<long> times;
    for (int i = 0; i < repetitions; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        multiparty_psi(client_sets, leader_set, domain_size, threshold, keys);

        auto stop = std::chrono::high_resolution_clock::now();
        times.push_back((stop-start).count());
    }

    double mean = sample_mean(times);
    double std = sample_std(times, mean);

    std::cout << mean << std::endl;
    std::cout << std << std::endl;

    return 0;
}
