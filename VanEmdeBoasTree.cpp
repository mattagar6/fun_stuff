#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

#define NDEBUG

// Builtin GNU binary search tree (Red-Black Tree), used for performance benchmark
#include <ext/pb_ds/assoc_container.hpp> // Common file
#include <ext/pb_ds/tree_policy.hpp> // Including tree_order_statistics_node_update

typedef __gnu_pbds::tree<
        int,
        __gnu_pbds::null_type,
        std::less<>,
        __gnu_pbds::rb_tree_tag,
        __gnu_pbds::tree_order_statistics_node_update>
        ordered_set;

////////////////////////////////////////////////////////////////////

// My homemade Van Emde Boas tree
// Handles insertion, deletion, successor of integer keys in O(log log U) time,
// where all integer keys lie in { 0, 1, 2, ..., U-1 }. O(U) space required.
// Status: Stress Tested

#define SMALL 32

struct V {
    int U, B; // universe size, block size
    int min, max;
    int small; // if U is small ( < 32) use bitmasks

    V* summary;
    std::vector<V*> block;

    explicit V(int size);
    ~V();

    // helper functions
    int index(int i, int j) const;
    int high(int x) const ;
    int low(int x) const;

    void insert(int x); // assumes x not in VEB
    void erase(int x);
    int successor(int x); // returns -1 if x has no successor

};

V::V(int size) : U(size), B(sqrt(U)), min(-1), max(-1), small(0), summary(nullptr) {
    if (U >= SMALL) {
        int blocks = B;

        while (blocks * B < U) ++blocks;

        block.resize(blocks, nullptr);
        for (int i = 0; i < blocks; i++) block[i] = new V(B);

        summary = new V(blocks);
    }
}

V::~V() {
    delete summary;
    for (V* v : block) delete v;
}

int V::index(int i, int j) const { return i * B + j; }
int V::high(int x) const { return x / B; }
int V::low(int x) const { return x % B; }

void V::insert(int x) {
    assert(0 <= x && x < U);

    if (min == -1) {
        min = max = x;
        return;
    }

    if (x < min) std::swap(x, min);
    if (x > max) max = x;

    if (U < SMALL) {
        small |= 1<<x;
        return;
    }

    int i = high(x), j = low(x);

    if (block[i]->min == -1)
        summary->insert(i);

    block[i]->insert(j);
}

void V::erase(int x) {
    assert(0 <= x && x < U);

    if (U < SMALL) {
        if (x == min) min = -1;
        if (x == max) max = min;

        small &= ~(1 << x);

        for (int i = 0; i < U; i++) {
            if (small >> i & 1) {
                if (min == -1) {
                    min = i;
                    small &= ~(1 << i);
                }
                max = i;
            }
        }
        return;
    }

    if (x == min) {
        int i = summary->min;
        if (i == -1) { // deleting last element
            min = max = -1;
            return;
        }
        x = min = index(i, block[i]->min); // next smallest element
    }

    block[high(x)]->erase(low(x));
    if (block[high(x)]->min == -1)
        summary->erase(high(x));

    if (x == max) {
        int i = summary->max;
        if (i == -1)
            max = min;
        else
            max = index(i, block[i]->max);
    }
}

int V::successor(int x) {
    assert(0 <= x && x < U);

    if (x < min) return min;

    if (U < SMALL) {
        for (int i = x+1; i < U; i++)
            if (small >> i & 1) return i;
        return -1;
    }

    int i = high(x), j = low(x);

    if (j < block[i]->max) {
        j = block[i]->successor(j);
    } else {
        i = summary->successor(i);
        if (i != -1) {
            j = block[i]->min;
            assert(j != -1);
        } else return -1;
    }

    return index(i, j);
}

////////////////////////////////////////////////////////////////////

int getSuccessor(const std::vector<int>& a, int x) {
    for (int i = x+1; i < (int) a.size(); i++) {
        if (a[i] != 0) {
            return i;
        }
    }
    return -1;
}

// uses a direct access table to check the correctness of the VEB
// direct access table uses a linear scan to find successor
bool check_correctness(int U, int numInserted) {

    V* VEB = new V(U);
    std::vector<int> table(U);

    std::vector<int> inserted(numInserted);
    for (int i = 0; i < numInserted; i++) {
        inserted[i] = rand() % U;
        if (table[inserted[i]] == 0)
            VEB->insert(inserted[i]);
        table[inserted[i]] = 1;
    }

    for (int round = 0; round < 10; round++) {
        for (int x = 0; x < U; x++) {
            // check successor of each element is correct
            if (VEB->successor(x) != getSuccessor(table, x)) {
                return false;
            }
        }

        // remove some random elements
        for (int i = 0; i < 20 && !inserted.empty(); i++) {
            int n = inserted.size();

            std::swap(inserted[rand() % n], inserted.back());
            int x = inserted.back();
            inserted.pop_back();

            table[x] = 0;
            VEB->erase(x);
        }
    }

    std::cout << "All tests passed!" << std::endl;
    delete VEB;
    return true;
}


long long check_performance_BST(int U, int insertions, int erases, int successors) {
    long long ans = 0;

    ordered_set s;

    for (int i = 0; i < insertions; i++) {
        int x = rand() % (U - 1) + 1;
        s.insert(x);
    }

    for (int i = 0; i < erases; i++) {
        s.erase(rand() % U);
    }

    for (int i = 0; i < successors; i++) {
        int ord = s.order_of_key(rand() % U);
        if (ord < s.size() - 1) {
            ans += *s.find_by_order(ord + 1); // next biggest element
        } else {
            ans += -1;
        }
    }

    std::cout << "All tests passed!" << std::endl;
    return ans;
}

long long check_performance_VEB(int U, int insertions, int erases, int successors) {
    long long ans = 0;

    V* VEB = new V(U);

    for (int i = 0; i < insertions; i++) {
        int x = rand() % (U - 1) + 1;
        if (VEB->successor(x-1) == x) continue; // already present in VEB
        VEB->insert(x);
    }

    for (int i = 0; i < erases; i++) {
        VEB->erase(rand() % U);
    }

    for (int i = 0; i < successors; i++) {
        ans += VEB->successor(rand() % U);
    }

    std::cout << "All tests passed!" << std::endl;
    delete VEB;
    return ans;
}

int main(int argc, char* argv[]) {
    srand(time(nullptr));

//    for (int i = 0; i < 100; i++) {
//        std::cout << "Test #" << i << std::endl;
//        if (!check_correctness(5000, rand() % 1000)) {
//            std::cout << "Failed Test :(" << std::endl;
//            return 1;
//        }
//    }

//    std::cout << check_performance_BST(1e8, 1e7, 1e7, 1e7) << std::endl; // approx. 60 seconds on my laptop
    std::cout << check_performance_VEB(1e8, 1e7, 1e7, 1e7) << std::endl; // approx. 15 seconds on my laptop
    return 0;
}
