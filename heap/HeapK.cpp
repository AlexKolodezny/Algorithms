#include <algorithm>
#include <vector>
#include <functional>

template <typename T, int k, typename Compare = std::less<T>>
class HeapKMax {
    std::vector<T> heap;
    Compare cmp;
public:
    HeapKMax(): heap(), cmp() {}

    void insert(const T& x) {
        heap.push_back(x);
        size_t i = heap.size() - 1;
        while (i != 0 && cmp(heap[(i - 1) / k], heap[i])) {
            std::swap(heap[(i - 1) / k], heap[i]);
            i = (i - 1) / k;
        }
    }

    T get_max() const {
        return heap[0];
    }

    void extract_max() {
        std::swap(heap[0], heap.back());
        heap.pop_back();
        size_t i = 0;
        while (true) {
            auto it = std::max_element(
                    heap.begin() + std::min(i * k + 1, heap.size()),
                    heap.begin() + std::min(i * k + k + 1, heap.size()),
                    cmp);
            if (it != heap.end() && cmp(heap[i], *it)) {
                std::swap(heap[i], *it);
                i = it - heap.begin();
            } else {
                break;
            }
        }
    }

    bool empty() const {
        return heap.empty();
    }

    size_t size() const {
        return heap.size();
    }
};
