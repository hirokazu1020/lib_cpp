#include "LoudsTrie.h"
#include <cstdint>
#include <tuple>
#include <algorithm>


namespace {
class CircularQueue{
public:
    using T = std::tuple<uint32_t, uint32_t, uint32_t>;
    CircularQueue(int capacity): head_(0), tail_(0){
        q_.resize(capacity + 1);
    }
    bool empty() const {
        return head_ == tail_;
    }
    void push(uint32_t depth, uint32_t begin ,uint32_t end) {
        q_[tail_] = std::make_tuple(depth, begin, end);
        if (++tail_ == q_.size()) tail_ = 0;
    }
    T pop() {
        int h = head_;
        if (++head_ == q_.size()) head_ = 0;
        return q_[h];
    }

private:
    size_t head_, tail_;
    std::vector<T> q_;
};
}

bool LoudsTrie::match(std::string_view text) const
{
    int node_id = 0;
    auto [ok, text_idx] = findTail(node_id, text, 0);
    if (!ok) return false;
    if (text_idx == text.size()) return true;

    while (text_idx < text.size()) {
        std::tie(node_id, text_idx) = findNode(node_id, text, text_idx);
        if (node_id == -1) return false;
    }
    return match_[node_id];
}

bool LoudsTrie::prefixMatch(std::string_view text) const
{
    int node_id = 0;
    auto [ok, text_idx] = findTail(node_id, text, 0);
    if (!ok) return false;
    if (match_[node_id]) return true;

    while (text_idx < text.size()) {
        std::tie(node_id, text_idx) = findNode(node_id, text, text_idx);
        if (node_id == -1) return false;
        if (match_[node_id]) return true;
    }
    return false;
}

void LoudsTrie::build(std::vector<std::string_view> patterns) {
    std::sort(patterns.begin(), patterns.end());

    CircularQueue que(patterns.size());
    que.push(0, 0, patterns.size());

    std::vector<bool> tree;
    std::vector<bool> tail_bv;
    tree.push_back(false);
    tail_bv.push_back(true);

    while (!que.empty()) {
        auto [depth, begin, end] = que.pop();
        tree.push_back(true);
        bool f = false;

        while (1) {
            uint32_t child_begin = begin;
            if (depth == patterns[child_begin].size()) {
                f = true;
                while (child_begin < end && depth == patterns[child_begin].size()){
                    child_begin++;
                }
                if (child_begin == end) break;
            }
            char child_ch = patterns[child_begin][depth];

            for (uint32_t i = child_begin; i < end; i++) {
                char ch = patterns[i][depth];
                if (ch != child_ch) {
                    label_.push_back(child_ch);          
                    que.push(depth + 1, child_begin, i);        
                    tree.push_back(false);
                    child_ch = ch;
                    child_begin = i;
                }
            }
            if (begin != child_begin) {
                label_.push_back(child_ch);
                que.push(depth + 1, child_begin, end);
                tree.push_back(false);
                break;
            }
            tail_label_ += child_ch;
            tail_bv.push_back(false);
            depth++;
        }
        match_.push_back(f);
        tail_bv.push_back(true);
    }
    tree.push_back(true);

    label_.shrink_to_fit();
    tail_label_.shrink_to_fit();
    match_.shrink_to_fit();

    this->tree_.init(tree.size());
    for (size_t i = 0; i < tree.size(); i++) 
        if (tree[i]) this->tree_.set(i);
    this->tree_.finalize();
    
    this->tail_bv_.init(tail_bv.size());
    for (size_t i = 0; i < tail_bv.size(); i++) 
        if (tail_bv[i]) this->tail_bv_.set(i);
    this->tail_bv_.finalize();
}

std::pair<int, size_t> LoudsTrie::findNode(int node_id, std::string_view text, size_t text_idx) const {
    int child_start = tree_.select(node_id + 1) + 1;
    int child_id = child_start - tree_.rank(child_start);

    for (int i = child_start; !tree_.access(i); i++){
        if (text[text_idx] == label_[child_id - 1]){
            auto [ok, idx] = findTail(child_id, text, text_idx + 1);
            return std::make_pair(ok? child_id: -1, idx);
        }
        child_id++;
    }
    return std::make_pair(-1, text_idx);
}

std::pair<bool, size_t> LoudsTrie::findTail(int node_id, std::string_view text, size_t text_idx) const {
    int bv_idx = tail_bv_.select(node_id + 1) + 1;
    auto ch = tail_label_.begin() + (bv_idx - tail_bv_.rank(bv_idx));
    while (!tail_bv_.access(bv_idx)) {
        if (text_idx == text.size()) return std::make_pair(false, text_idx);
        if (*ch != text[text_idx]) return std::make_pair(false, text_idx);
        ++ch;
        ++bv_idx;
        ++text_idx;
    }
    return std::make_pair(true, text_idx);
}
