#include <vector>
#include <iostream>
#include <utility>
#include <iterator>
#include <algorithm>
#include <memory>

template <typename T, unsigned int k = 2, typename Compare = std::less<T>>
class BPlusTree {
    enum TYPE_FIND {FIND, LOWER_BOUND, UPPER_BOUND};
    class Node;
    class Child;
    class Parent;
    class Leaf;
    class Usual;
    class Root;
    template <typename> class Iterator;

public:
    using iterator = Iterator<Leaf*>;
    using const_iterator = Iterator<const Leaf*>;

private:
    class Node {
    public:
        virtual ~Node() {}
        virtual const Leaf* max_leaf() const = 0;
        virtual Leaf* max_leaf() = 0;

        virtual const Leaf* find_next(const T&, const TYPE_FIND type_find) const = 0;
        virtual Leaf* find_next(const T&, const TYPE_FIND type_find) = 0;

        virtual const Leaf* find_deep(const T&, const TYPE_FIND type_find) const = 0;
        virtual Leaf* find_deep(const T&, const TYPE_FIND type_find) = 0;

        const Leaf* find(const T& other_data, const TYPE_FIND type_find) const {
            if (type_find != UPPER_BOUND) {
                if (Compare()(max_leaf()->data, other_data)) {
                    return find_next(other_data, type_find);
                } else {
                    return find_deep(other_data, type_find);
                }
            } else {
                if (!Compare()(other_data, max_leaf()->data)) {
                    return find_next(other_data, type_find);
                } else {
                    return find_deep(other_data, type_find);
                }
            }
        }

        Leaf* find(const T& other_data, const TYPE_FIND type_find) {
            if (type_find != UPPER_BOUND) {
                if (Compare()(max_leaf()->data, other_data)) {
                    return find_next(other_data, type_find);
                } else {
                    return find_deep(other_data, type_find);
                }
            } else {
                if (!Compare()(other_data, max_leaf()->data)) {
                    return find_next(other_data, type_find);
                } else {
                    return find_deep(other_data, type_find);
                }
            }
        }
    };

    class Child : public virtual Node {
    public:
        Parent* parent;
        Child* left;
        std::shared_ptr<Child> right;

        Child(): Node(), parent(nullptr), left(nullptr), right(nullptr) {}

        const Leaf* find_next(const T& other_data, const TYPE_FIND type_find) const override {
            return right->Node::find(other_data, type_find);
        }

        Leaf* find_next(const T& other_data, const TYPE_FIND type_find) override {
            return right->Node::find(other_data, type_find);
        }

        void set_parent(Parent* new_parent) {
            if (parent != nullptr) {
                --parent->cnt_child;
            }
            parent = new_parent;
            ++parent->cnt_child;
        }
        
        void insert_left(std::shared_ptr<Child>&& brother) {
            brother->set_parent(parent);
            if (left == nullptr) {
                brother->right = parent->child;
                parent->child = brother;
                brother->left = left;
                left = brother.get();
            } else {
                brother->right = left->right;
                left->right = brother;
                brother->left = left;
                left = brother.get();
                if (parent->child.get() == this) {
                    parent->child = brother;
                }
            }
            parent->split();
        }

        void insert_right(std::shared_ptr<Child>&& brother) {
            brother->set_parent(parent);
            brother->right = right;
            brother->left = this;
            if (right) {
                right->left = brother.get();
            }
            right = brother;
            if (parent->last_child == this) {
                parent->set_last_child(brother.get());
            }
            parent->split();
        }

        void erase() {
            std::shared_ptr<Child> tmp;
            if (parent->child.get() == this) {
                tmp = parent->child;
                parent->child = right;
            }
            if (parent->last_child == this) {
                parent->set_last_child(left);
            }
            --parent->cnt_child;
            if (right) {
                right->left = left;
            }
            if (left) {
                tmp = left->right;
                left->right = right;
            }
            tmp->parent->check();
        }
    };

    class Leaf : public Child {
    public:
        T data;

        template <typename U>
        Leaf(U&& d): Child(), data(std::forward<U>(d)) {}

        const Leaf* find_deep(const T& other_data, const TYPE_FIND type_find) const override {
            if (type_find == FIND && Compare()(other_data, data)) {
                return (nullptr);
            }
            return (this);
        }

        Leaf* find_deep(const T& other_data, const TYPE_FIND type_find) override {
            if (type_find == FIND && Compare()(other_data, data)) {
                return (nullptr);
            }
            return (this);
        }

        const Leaf* max_leaf() const override {
            return this;
        }

        Leaf* max_leaf() override {
            return this;
        }
    };

    class Parent : public virtual Node {
    public:
        Leaf* _max_leaf;
        size_t cnt_child;
        std::shared_ptr<Child> child;
        Child* last_child;

        Parent(): Node(), _max_leaf(nullptr), cnt_child(0), child(), last_child(nullptr) {}

        void set_children(std::shared_ptr<Child> first, Child* last) {
            child = first;
            set_last_child(last);
            for (Child* v = first.get(); v != last; v = v->right.get()) {
                v->set_parent(this);
            }
            last->set_parent(this);
            return;
        }
            

        const Leaf* find_deep(const T& other_data, const TYPE_FIND type_find) const override {
            return child->find(other_data, type_find);
        }

        Leaf* find_deep(const T& other_data, const TYPE_FIND type_find) override {
            return child->find(other_data, type_find);
        }

        const Leaf* max_leaf() const override {
            return _max_leaf;
        }

        Leaf* max_leaf() override {
            return _max_leaf;
        }

        void set_last_child(Child* child) {
            last_child = child;
            _max_leaf = last_child->max_leaf();
        }

        virtual void split() = 0;
        virtual void update() {
            _max_leaf = last_child->max_leaf();
        }
        virtual void check() = 0;
    };

    class Usual : public Child, public Parent {
    public:
        Usual(): Node(), Child(), Parent() {}

        void split() override {
            if (Parent::cnt_child == 2 * k) { std::shared_ptr<Usual> new_parent(new Usual());
                Child* mid = Parent::child.get();
                for (size_t i = 0; i < k; ++i) {
                    mid = mid->right.get();
                }
                new_parent->set_children(mid->left->right, Parent::last_child);
                Parent::set_last_child(mid->left);
                Child::insert_right(new_parent);
            }
        }

        void update() override {
            Parent::update();
            if (this == Child::parent->last_child) {
                Child::parent->update();
            }
        }

        void check() override {
            if (Parent::cnt_child < k) {
                if (Child::right) {
                    auto v = static_cast<Usual*>(Child::right.get());
                    if (v->cnt_child == k) {
                        v->set_children(Parent::child, v->last_child);
                        Child::erase();
                    } else {
                        Parent::set_last_child(v->child.get());
                        v->child->set_parent(this);
                        v->child = v->child->right;
                    }
                } else if (Child::left != nullptr) {
                    auto v = static_cast<Usual*>(Child::left);
                    if (v->cnt_child == k) {
                        v->set_children(v->child, Parent::last_child);
                        Child::erase();
                    } else {
                        v->last_child->set_parent(this);
                        v->set_last_child(Child::left);
                        Parent::child = v->last_child->right;
                    }
                } else {
                    if (Parent::cnt_child == 1) {
                        Child::parent->check();
                    }
                }
            }
        }
    };

    class Root : public Parent {
    public:
        Root(): Parent() {}

        const Leaf* find_next(const T&, const TYPE_FIND) const {
            return (nullptr);
        }

        Leaf* find_next(const T&, const TYPE_FIND) {
            return (nullptr);
        }

        void split() override {
            if (Parent::cnt_child == 2) {
                std::shared_ptr<Usual> new_parent(new Usual());
                new_parent->set_children(Parent::child, Parent::last_child);
                Parent::set_children(new_parent, new_parent.get());
            }
        }

        void check() override {
            std::shared_ptr<Child> died = Parent::child;
            Parent::set_children(static_cast<Usual*>(died.get())->child, static_cast<Usual*>(died.get())->last_child);
        }
    };
            


    template <typename U>
    class Iterator {
    public:
        U node;
        const Root* root;
        Iterator(U leaf, const Root* tree): node(leaf), root(tree) {}

        const T& operator*() {
            return node->data;
        }

        const T* operator->() {
            return &node->data;
        }

        friend bool operator==(const Iterator& first, const Iterator& second) {
            return first.node == second.node;
        }

        friend bool operator!=(const Iterator& first, const Iterator& second) {
            return !(first == second);
        }

        Iterator& operator++() {
            node = static_cast<U>(node->right.get());
            return *this;
        }

        Iterator operator++(int) {
            auto tmp = *this;
            node = static_cast<U>(node->right.get());
            return tmp;
        }

    };

    std::shared_ptr<Root> root;
    Leaf* first;
    size_t _size;

public:
    BPlusTree(): root(new Root()), first(nullptr), _size(0) {}

    BPlusTree(const BPlusTree&) = delete;

    BPlusTree(BPlusTree&& other): root(nullptr), first(nullptr), _size(0) {
        std::swap(root, other.root);
        std::swap(_size, other._size);
        std::swap(first, other.first);
    }

    BPlusTree& operator=(const BPlusTree&) = delete;

    BPlusTree& operator=(BPlusTree&& other) {
        std::swap(root, other.root);
        std::swap(_size, other._size);
        std::swap(first, other.first);
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return !root->child;
    }

    const_iterator end() const {
        return const_iterator(nullptr, root.get());
    }

    iterator end() {
        return iterator(nullptr, root.get());
    }

    const_iterator begin() const {
        return const_iterator(first, root.get());
    }

    iterator begin() {
        return iterator(first, root.get());
    }

    const_iterator find(const T& data) const {
        if (empty()) {
            return const_iterator(nullptr, root.get());
        }
        return const_iterator(root->find(data, FIND), root.get());
    }

    const_iterator lower_bound(const T& data) const {
        if (empty()) {
            return const_iterator(nullptr, root.get());
        }
        return const_iterator(root->find(data, LOWER_BOUND), root.get());
    }

    const_iterator upper_bound(const T& data) const {
        if (empty()) {
            return const_iterator(nullptr, root.get());
        }
        return const_iterator(root->find(data, UPPER_BOUND), root.get());
    }

    iterator find(const T& data) {
        if (empty()) {
            return iterator(nullptr, root.get());
        }
        return iterator(root->find(data, FIND), root.get());
    }

    iterator lower_bound(const T& data) {
        if (empty()) {
            return iterator(nullptr, root.get());
        }
        return iterator(root->find(data, LOWER_BOUND), root.get());
    }

    iterator upper_bound(const T& data) {
        if (empty()) {
            return iterator(nullptr, root.get());
        }
        return iterator(root->find(data, UPPER_BOUND), root.get());
    }

    template <typename U>
    void insert(U&& data) {
        ++_size;
        if (empty()) {
            std::shared_ptr<Leaf> leaf(new Leaf(std::forward<U>(data)));
            root->set_children(leaf, leaf.get());
            first = leaf.get();
            return;
        }
        auto it = root->find(data, LOWER_BOUND);
        std::shared_ptr<Leaf> leaf(new Leaf(std::forward<U>(data)));
        if (it == nullptr) {
            root->max_leaf()->insert_right(leaf);
        } else {
            if (it == first) {
                first = leaf.get();
            }
            it->insert_left(leaf);
        }
        leaf->parent->update();
        return;
    }

    void erase(iterator it) {
        if (it.node == nullptr) {
            return;
        }
        --_size;
        if (size() == 0) {
            root = std::shared_ptr<Root>(new Root());
            first = nullptr;
            return;
        }
        if (it.node == first) {
            first = static_cast<Leaf*>(it.node->right.get());
            it.node->erase();
        } else {
            auto tmp = it.node->left;
            it.node->erase();
            tmp->parent->update();
        }
        return;
    }

    void erase(const T& data) {
        auto it = find(data);
        erase(it);
        return;
    }
};  
