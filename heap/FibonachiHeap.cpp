#include <functional>
#include <utility>
#include <vector>

template <typename T, typename Compare = std::less<T>>
class FibonachiHeap {
    struct Node {
        Node *left;
        Node *right;
        Node *parent;
        Node *child;
        size_t number_child;
        T data;
        bool mark;
    };

    class Iterator {
        FibonachiHeap& cont;
        Node* node;
    public:
        Iterator(FibonachiHeap& heap, Node* n): cont(heap), node(n) {}
        const T& operator*() const {
            return node->data;
        }

        const T* operator->() const {
            return &node->data;
        }

        template <typename U>
        void increase_key(U&& data) {
            node->data = std::forward<U>(data);
            Node* cur = node;
            if (cur->parent == nullptr) {
                return;
            }
            if (Compare()(cur->data, cur->parent->data)) {
                return;
            }
            while (true) {
                Node* p = cur->parent;
                Node* extracted = extract(cur, p->number_child);
                p->child = cur;
                cur = extracted;
                become_child(cont.root, cont.number_child, nullptr, cur);
                cur->mark = false;
                cur = p;
                if (p->parent == nullptr) {
                    break;
                }
                if (!p->mark) {
                    p->mark = true;
                    break;
                } else {
                    cur = p;
                }
            } 
        }
    };

    Node* root;
    Node* cur_max;
    size_t number_child;
    size_t _size;

    static void del_child(Node*& child, size_t& number_child) {
        while (child->child != nullptr) {
            del_child(child->child, child->number_child);
        }
        Node* died = extract(child, number_child);
        delete died;
    }


    static void become_child(Node*& child, size_t& number_child, Node* parent, Node* other) {
        ++number_child;
        if (child == nullptr) {
            child = other;
            other->left = other;
            other->right = other;
            other->parent = parent;
            return;
        }
        other->parent = parent;
        other->left = child;
        other->right = child->right;
        child->right->left = other;
        child->right = other;
        return;
    }

    static Node* extract(Node*& child, size_t& number_child) {
        --number_child;
        Node* extracted = child;
        if (child->left == child) {
            child = nullptr;
        } else {
            child = child->left;
            extracted->left->right = extracted->right;
            extracted->right->left = extracted->left;
        }
        extracted->parent = nullptr;
        extracted->left = nullptr;
        extracted->right = nullptr;
        return extracted;
    }

    static Node* merge(Node* first, Node* second) {
        if (!Compare()(first->data, second->data)) {
            std::swap(first, second);
        }
        become_child(second->child, second->number_child, second, first);
        return second;
    }

    void compact() {
        std::vector<std::vector<Node*>> rank;
        while (root != nullptr) {
            Node* extracted = extract(root, number_child);
            if (rank.size() < extracted->number_child + 1) {
                rank.resize(extracted->number_child + 1);
            }
            rank[extracted->number_child].push_back(extracted);
        }
        cur_max = nullptr;
        for (size_t i = 0; i < rank.size(); ++i) {
            while (!rank[i].empty()) {
                if (rank[i].size() == 1) {
                    become_child(root, number_child, nullptr, rank[i].back());
                    if (cur_max == nullptr || Compare()(cur_max->data, rank[i].back()->data)) {
                        cur_max = rank[i].back();
                    }
                    rank[i].pop_back();
                } else {
                    if (rank.size() <= i + 1) {
                        rank.emplace_back();
                    }
                    rank[i + 1].push_back(merge(rank[i][rank[i].size() - 1], rank[i][rank[i].size() - 2]));
                    rank[i].pop_back();
                    rank[i].pop_back();
                }
            }
        }
    }

public:
    using iterator = Iterator;
    FibonachiHeap(): root(nullptr), cur_max(nullptr), number_child(0), _size(0) {}

    FibonachiHeap(const T&) = delete;
    
    FibonachiHeap(T&& other): root(nullptr), cur_max(nullptr), number_child(0) {
        std::swap(root, other.root);
        std::swap(cur_max, other.cur_max);
        std::swap(number_child, other.number_child);
        std::swap(_size, other._size);
    }

    FibonachiHeap& operator=(const FibonachiHeap&) = delete;

    FibonachiHeap& operator=(FibonachiHeap&& other) {
        std::swap(root, other.root);
        std::swap(cur_max, other.cur_max);
        std::swap(number_child, other.number_child);
        std::swap(_size, other._size);
        return *this;
    }

    bool empty() const {
        return root == nullptr;
    }

    template <typename U>
    iterator insert(U&& data) {
        Node * other = new Node{nullptr, nullptr, nullptr, nullptr, 0, std::forward<U>(data), false};
        become_child(root, number_child, nullptr, other);
        if (cur_max == nullptr) {
            cur_max = other;
        } else if (Compare()(cur_max->data, other->data)) {
            cur_max = other;
        }
        ++_size;
        return iterator(*this, other);
    }

    T get_max() const {
        return cur_max->data;
    }

    void extract_max() {
        Node* extracted = extract(cur_max, number_child);
        root = cur_max;
        while (extracted->child != nullptr) {
            Node* node = extract(extracted->child, extracted->number_child);
            become_child(root, number_child, nullptr, node);
            node->mark = false;
        }
        cur_max = nullptr;
        delete extracted;
        --_size;
        compact();
    }

    ~FibonachiHeap() {
        while (root != nullptr) {
            del_child(root, number_child);
        }
    }
    void merge(FibonachiHeap&& other) {
        while (other.root != nullptr) {
            auto extracted = extract(other.root, other.number_child);
            become_child(root, number_child, nullptr, extracted);
            if (cur_max == nullptr || Compare()(cur_max->data, extracted->data)) {
                cur_max = extracted;
            }
        }
        other.cur_max = nullptr;
        _size += other._size;
        other._size = 0;
    }

    size_t size() const {
        return _size;
    }
};
