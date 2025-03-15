#ifndef _LIB_CPP_SUCCINCT_LOUDS_TRIE_H_
#define _LIB_CPP_SUCCINCT_LOUDS_TRIE_H_

#include "FullyIndexableDictionary.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <utility>


class LoudsTrie {
public:
    LoudsTrie(std::vector<std::string_view> patterns){
        build(std::move(patterns));
    }
    template <class Iterator>
    LoudsTrie(Iterator begin, Iterator end): LoudsTrie(std::vector<std::string_view>(begin, end)) {}
    template <class Str>
    LoudsTrie(const std::vector<Str> &strs): LoudsTrie(strs.begin(), strs.end()) {}

    bool match(std::string_view text) const;
    bool prefixMatch(std::string_view text) const;

private:
    void build(std::vector<std::string_view> patterns);
    std::pair<int, size_t> findNode(int node_id, std::string_view text, size_t i) const;
    std::pair<bool, size_t> findTail(int node_id, std::string_view text, size_t i) const;
    size_t size() const { return match_.size(); }

    std::string label_;
    FullyIndexableDictionary tree_;
    std::vector<bool> match_;

    std::string tail_label_;
    FullyIndexableDictionary tail_bv_;
};

#endif
