#include "IntersectionMatrix.hpp"

uint32_t IntersectionMatrix::get(uint32_t i, uint32_t j) {
    return i < j ? (*this)[i][j] : (*this)[j][i];
}

void IntersectionMatrix::set(uint32_t i, uint32_t j, uint32_t k) {
    (i < j ? (*this)[i][j] : (*this)[j][i]) = k;
}

void IntersectionMatrix::increment(uint32_t i, uint32_t j) {
    i < j ? (*this)[i][j]++ : (*this)[j][i]++;
}

void IntersectionMatrix::merge(const IntersectionMatrix& other) {
    // For every item-row in 'other'
    for(auto i_jk: other) {
        auto i = i_jk.first;
        // For every item-item-pair in that row
        for(auto j_k: i_jk.second) {
            auto j = j_k.first;
            auto k = j_k.second;
            // Add 'k' to the intersection of said items
            (i < j ? (*this)[i][j] : (*this)[j][i]) += k;
        }
    }
}

uint64_t IntersectionMatrix::get_sparsity() const {
    // Count non-zero elements
    uint64_t result = 0;
    for(auto i_jk: *this) {
        result += i_jk.second.size();
    }
    return result;
}
