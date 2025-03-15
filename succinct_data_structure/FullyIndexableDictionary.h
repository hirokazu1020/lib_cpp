#ifndef _LIB_CPP_SUCCINCT_INDEXABLE_DICTIONALY_H_
#define _LIB_CPP_SUCCINCT_INDEXABLE_DICTIONALY_H_

#include <cstddef>
#include <cstdint>
#include <vector>


// (64/32768 + 16/128 + 1) <= 1.127 bits
class FullyIndexableDictionary {
public:
    FullyIndexableDictionary(){}
    FullyIndexableDictionary(size_t size){
        init(size);
    }
    void init(size_t size);
    void set(size_t k);
    void finalize();
    bool access(size_t k) const {
        return bits[k / 64] >> (k % 64) & 1;
    }
    //[0,k)の1の個数
    int rank(size_t k) const;
    //k番目の１の場所(1<=k)
    int select(size_t k) const;
private:

    static constexpr int LSIZE = 32768;
    static constexpr int SSIZE = 128;
    std::vector<uint64_t> large; // LSIZE bits
    std::vector<uint16_t> small; // SSIZE bits
    std::vector<uint64_t> bits;
};


#endif