//
// Created by jelle on 09-10-20.
//

#ifndef PSI_PROTOCOLS_H
#define PSI_PROTOCOLS_H

#include <vector>
#include "threshold_paillier.h"
#include "bloom_filter.h"

std::vector<long> multiparty_psi(std::vector<std::vector<long>> client_sets, std::vector<long> leader_set,
                                 long domain_size, long threshold_l, Keys &keys);

#endif //PSI_PROTOCOLS_H
