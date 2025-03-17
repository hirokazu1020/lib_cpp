#include "DoubleArrayTrie.h"
#include <limits>
#include <tuple>
#include <stack>
#include <algorithm>


bool DoubleArrayTrie::match(std::string_view text) const
{
    int node_id = 0;
    for (size_t i = 0; i < text.size(); i++)
    {
        node_id = findNode(node_id, text[i]);
        if (node_id == -1) return false; 
    }
    return match_[node_id];
}

bool DoubleArrayTrie::prefixMatch(std::string_view text) const
{
    int node_id = 0;
    for (size_t i = 0; i < text.size(); i++)
    {
        node_id = findNode(node_id, text[i]);
        if (node_id == -1) return false;
        if (match_[node_id]) return true;
    }
    return false;
}

void DoubleArrayTrie::build(std::vector<std::string_view> patterns) {
    std::sort(patterns.begin(), patterns.end());

    int base = 1;
    setBase(0, base);
    std::stack<std::tuple<size_t, int, int, int>> st;
    st.emplace(0, 0, patterns.size(), -1);
    while (!st.empty()) {       
        auto [depth, begin, end, par_node_id] = st.top();
        st.pop();
        int node_id = depth == 0? 0 :  base_[par_node_id] + static_cast<unsigned char>(patterns[begin][depth - 1]);
        while (begin < end && depth == patterns[begin].size()){
            match_[node_id] = true;
            begin++;
        }
        if (begin == end) continue;
        std::vector<unsigned char> children;
        unsigned char child_ch = patterns[begin][depth];
        int child_begin = begin;
        for (int i = begin; i < end; i++) {
            unsigned char ch = patterns[i][depth];
            if (ch != child_ch) {
                children.push_back(child_ch);
                child_ch = ch;
                st.emplace(depth + 1, child_begin, i, node_id);
                child_begin = i;
            }
        }
        children.push_back(child_ch);
        st.emplace(depth + 1, child_begin, end, node_id);

        while (1) {
            bool ok = true;
            for (unsigned char ch : children){
                if (base + ch < static_cast<int>(check_.size()) && check_[base + ch] != -1) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                setBase(node_id, base);
                for (unsigned char ch : children) {
                    check_[base + ch] = node_id;
                }
                // 適当に -10 しておく
                base = std::max(1, base - 10);
                break;
            }
            base++;
        }
    }
    base_.shrink_to_fit();
    check_.shrink_to_fit();
    match_.shrink_to_fit();
}

void DoubleArrayTrie::setBase(int node_id, int base) {
    size_t s = base + std::numeric_limits<unsigned char>::max() + 1;
    while (base_.size() < s) {
        base_.push_back(-1);
        check_.push_back(-1);
        match_.emplace_back(false);
    }
    base_[node_id] = base;
}

int DoubleArrayTrie::findNode(int node_id, char c) const {
    int v = base_[node_id] + static_cast<unsigned char>(c);
    return check_[v] == node_id? v: -1;
}
