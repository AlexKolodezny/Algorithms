#include <algorithm>
#include <forward_list>
#include <memory>
#include <functional>

template <typename T, typename Compare = std::less<T>>
class BinomialHeapMax {
    class Node {
    public:
        size_t rk;
        T data;
        std::unique_ptr<Node> child;
        std::unique_ptr<Node> brother;
        Node* parent;
    public:
        Node(const T& _data)
            : rk(0)
            , data(_data)
            , child(nullptr)
            , brother(nullptr)
        {}

        size_t rank() const {
            return rk;
        }

        static std::unique_ptr<Node> merge(std::unique_ptr<Node>&& first, std::unique_ptr<Node>&& second) {
            if (Compare()(first->data, second->data)) {
                swap(first, second);
            }
            second->brother = std::move(first->child);
            ++first->rk;
            first->child = std::move(second);
            return std::move(first);
        }

        T get_data() const {
            return data;
        }
    };

    std::vector<std::unique_ptr<Node>> roots;
    size_t sz;
    size_t cur_max;
    
    friend std::pair<std::unique_ptr<Node>, std::unique_ptr<Node>> full_sumator(
            std::unique_ptr<Node>&& first,
            std::unique_ptr<Node>&& second,
            std::unique_ptr<Node>&& third) {
        if (!first) {
            swap(first, third);
            if (!first) {
                swap(first, second);
            }
        } else {
            if (!second) {
                swap(second, third);
            }
        }
        if (!first) {
            return {nullptr, nullptr};
        }
        if (!second) {
            return {nullptr, std::move(first)};
        }
        if (!third) {
            return {Node::merge(std::move(first), std::move(second)), nullptr};
        }
        return {Node::merge(std::move(first), std::move(second)), std::move(third)};
    }

    BinomialHeapMax(const T& data): roots(), sz(1) {
        roots.emplace_back(new Node(data));
        cur_max = 0;
    }

    void find_max() {
        if (empty()) {
            cur_max = 0;
        }
        cur_max = roots.size() - 1;
        for (size_t i = 0; i < roots.size(); ++i) {
            if (roots[i]) {
                if (Compare()(roots[cur_max]->get_data(), roots[i]->get_data())) {
                    cur_max = i;
                }
            }
        }
    }

public:
    BinomialHeapMax(): roots(), sz(0), cur_max(0) {}

    bool empty() const {
        return roots.empty();
    }
    
    friend BinomialHeapMax merge(BinomialHeapMax& first, BinomialHeapMax& second) {
        BinomialHeapMax ans;
        std::unique_ptr<Node> r;
        size_t i = 0;
        for (; i < first.roots.size() && i < second.roots.size(); ++i) {
            auto [next, prev] = full_sumator(std::move(first.roots[i]), std::move(second.roots[i]), std::move(r));
            ans.roots.push_back(std::move(prev));
            r = std::move(next);
        }
        for (; i < first.roots.size(); ++i) {
            std::unique_ptr<Node> tmp;
            auto [next, prev] = full_sumator(std::move(first.roots[i]), std::move(tmp), std::move(r));
            ans.roots.push_back(std::move(prev));
            r = std::move(next);
        }
        for (; i < second.roots.size(); ++i) {
            std::unique_ptr<Node> tmp;
            auto [next, prev] = full_sumator(std::move(tmp), std::move(second.roots[i]), std::move(r));
            ans.roots.push_back(std::move(prev));
            r = std::move(next);
        }
        if (r) {
            ans.roots.push_back(std::move(r));
        }
        ans.sz = first.sz + second.sz;
        first.sz = 0;
        second.sz = 0;
        first.roots.clear();
        second.roots.clear();
        first.find_max();
        second.find_max();
        ans.find_max();
        return std::move(ans);
    }

    void insert(const T& data) {
        BinomialHeapMax other(data);
        *this = merge(*this, other);
    }

    T get_max() const {
        return roots[cur_max]->get_data();
    }

    void extract_max() {
        if (roots.size() == 1) {
            *this = BinomialHeapMax();
            return;
        }
        auto tmp = std::move(roots[cur_max]);
        while (!roots.empty() && !roots.back()) {
            roots.pop_back();
        }
        sz -= 1 << tmp->rank();
        if (tmp->rank() == 0) {
            find_max();
            return;
        }
        BinomialHeapMax other;
        other.sz = (1 << tmp->rank()) - 1;
        other.roots.resize(tmp->rank());
        other.roots.back() = std::move(tmp->child);
        for (size_t i = 1; i < other.roots.size(); ++i) {
            other.roots[other.roots.size() - i - 1] = std::move(other.roots[other.roots.size() - i]->brother);
        }
        *this = merge(*this, other);
    }
};
