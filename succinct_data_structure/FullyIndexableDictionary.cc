#include "FullyIndexableDictionary.h"
#include <cassert>
#include <bit>
#include <nmmintrin.h>
//#include <immintrin.h>

namespace {
inline int popCount(uint64_t x) {
    return std::popcount(x);
}

inline int popSelect(uint64_t block1, uint32_t k){
    // -march=native
    /*uint64_t z = 1ULL << (k - 1);
    uint64_t y = _pdep_u64(z, block1);
    return _tzcnt_u64(y);*/
    uint64_t block2 = (block1>>1  & 0x5555555555555555)+(block1  & 0x5555555555555555);
    uint64_t block4 = (block2>>2  & 0x3333333333333333)+(block2  & 0x3333333333333333);
    uint64_t block8 = (block4>>4  & 0x0f0f0f0f0f0f0f0f)+(block4  & 0x0f0f0f0f0f0f0f0f);
    uint64_t block16= (block8>>8  & 0x00ff00ff00ff00ff)+(block8  & 0x00ff00ff00ff00ff);
    uint64_t block32= (block16>>16& 0x0000ffff0000ffff)+(block16 & 0x0000ffff0000ffff);
    int pos = 0;
    if((block32 & 0xffffffff) < k){
        k -= block32 & 0xffffffff;
        pos |= 32;
    }
    if((block16 >> pos & 0x0000ffff) < k){
        k -= block16 >> pos & 0x0000ffff;
        pos |= 16;
    }
    if((block8 >> pos & 0x000000ff) < k){
        k -= block8 >> pos & 0x000000ff;
        pos |= 8;
    }
    if((block4 >> pos & 0x0000000f) < k){
        k -= block4 >> pos & 0x0000000f;
        pos |= 4;
    }
    if((block2 >> pos & 0x00000003) < k){
        k -= block2 >> pos & 0x00000003;
        pos |= 2;
    }
    if((block1 >> pos & 0x00000001) < k)pos |= 1;
    return pos;
}

template<class Vec>
size_t search(const Vec &vec, size_t begin, size_t end, size_t k) {
    while(end - begin > 1) {
        size_t m = (begin + end) / 2;
        if (vec[m] < k) begin = m;
        else end = m;
    }
    return begin;
}
}


void FullyIndexableDictionary::init(size_t size){
    large.assign(size / LSIZE + 1, 0);
    small.assign(large.size() * (LSIZE / SSIZE), 0);
    bits.assign(small.size() * (SSIZE / 64), 0);
}

void FullyIndexableDictionary::set(size_t k){
    bits[k / 64] |= 1ull << (k % 64);
}

void FullyIndexableDictionary::finalize(){
    for (size_t i = 0; i < bits.size(); i++) {
        int pop = popCount(bits[i]);
        size_t pos_small = i / (SSIZE / 64);
        if (pos_small + 1 < small.size()) small[pos_small + 1] += pop;
        size_t pos_large = pos_small / (LSIZE / SSIZE);
        if (pos_large + 1 < large.size()) large[pos_large + 1] += pop;
    }
    for (size_t i = 1; i < large.size(); i++) {
        large[i] += large[i - 1];
    }
    for (size_t i = 0; i < small.size(); i++) {
        if (i % (LSIZE / SSIZE) == 0) small[i] = 0;
        else small[i] += small[i - 1];
    }
}

int FullyIndexableDictionary::rank(size_t k) const {
    size_t block = large[k / LSIZE] + small[k / SSIZE];
    size_t pos_bits = k / 64;
    uint64_t mask = (1ull <<(k % 64)) - 1;
    if (pos_bits & 1) {
        return block + popCount(bits[pos_bits - 1]) + popCount(bits[pos_bits] & mask);
    } else {
        return block + popCount(bits[pos_bits] & mask);
    }
}

int FullyIndexableDictionary::select(size_t k) const {
    assert(k > 0);
    size_t pos_large = search(large, 0, large.size(), k);
    k -= large[pos_large];
    size_t small_begin = pos_large * (LSIZE / SSIZE);
    size_t small_end = (pos_large + 1) * (LSIZE / SSIZE);
    size_t pos_small = search(small, small_begin, small_end, k);
    k -= small[pos_small];
    size_t pos_bits = pos_small * (SSIZE / 64);
    size_t pop = popCount(bits[pos_bits]);
    if (pop < k) {
        k -= pop;
        pos_bits++;
    }
    return pos_bits * 64 +  popSelect(bits[pos_bits], k);
}
