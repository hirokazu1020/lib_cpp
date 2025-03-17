#ifndef _LIB_CPP_STRINGL_DOUBLE_ARRAY_TRIE_H_
#define _LIB_CPP_STRINGL_DOUBLE_ARRAY_TRIE_H_

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>


class DoubleArrayTrie {
public:
    DoubleArrayTrie(std::vector<std::string_view> patterns){
        build(std::move(patterns));
    }
    template <class Iterator>
    DoubleArrayTrie(Iterator begin, Iterator end): DoubleArrayTrie(std::vector<std::string_view>(begin, end)) {}
    template <class Str>
    DoubleArrayTrie(const std::vector<Str> &strs): DoubleArrayTrie(strs.begin(), strs.end()) {}

    bool match(std::string_view text) const;

    bool prefixMatch(std::string_view text) const;

    int findNode(int node_id, char c) const;
    size_t size() const {
        return base_.size();
    }

private:
    void build(std::vector<std::string_view> patterns);
    void setBase(int node_id, int base);

    std::vector<int> base_;
    std::vector<int> check_;
    std::vector<bool> match_;
};

#endif
