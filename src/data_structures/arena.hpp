#include <vector>
#include <algorithm>
#include <macros.hpp>

//TODO

template<typename Type>
class Arena {
public:
    //arena with objects
    std::vector<Type> storage;
    //"pointers" to free objects
    // std::vector<int> usedIndices;
    std::vector<int> freeIndices;
    //"pointers" to objects that need to be freed in next garbage collection
    std::vector<int> garbage;

    void setup(int initial_size) {
        storage.resize(initial_size);
        freeIndices.reserve(initial_size);
        //garbage may be also reserved, but im not sure its worth it
        // usedIndices.reserve(initial_size);
        for (int i = 0; i < initial_size; i++) {
            freeIndices.push_back(i);
        }
    }

    void cleanup() {
        storage.clear();
        freeIndices.clear();
        // usedIndices.clear();
        garbage.clear();
    }

    void clear() {
        freeIndices.clear();
        // usedIndices.clear();
        garbage.clear();
        for (int i = 0; i < storage.size(); i++) {
            freeIndices.push_back(i);
        }
    }

    void resize(int new_size) {
        int oldSize = storage.size();
        storage.resize(new_size);
        for (int i = oldSize; i < new_size; i++) {
            freeIndices.push_back(i);
        }
    }

    int allocate(const Type& obj) {
        assert(!freeIndices.empty());
        //damn it, pop_back doesnt return?
        int index = freeIndices.back();
        freeIndices.pop_back();
        // usedIndices.push_back(index);
        storage[index] = obj;
        return index;
    }
    //makes object references invalid **immediately**
    //is really slow because searches for an index
    //you dont have to free() most of the time
    //prefer softFree instead
    void free(int index) {
        validate_bounds(index, storage.size());
        freeIndices.push_back(index);

        // auto it = std::find(usedIndices.begin(), usedIndices.end(), index);
        // if (it != usedIndices.end()) {
        //     usedIndices.erase(it);
        // }
    }

    //makes object references invalid in the next GC
    void softFree(int index) {
        validate_bounds(index, storage.size());
        garbage.push_back(index);
    }

    //GC itself. Yeah, garbage collection in C++
    void collectGarbage() {
        // for (size_t index : garbage) {
        //     freeIndices.push_back(index);
            // auto it = std::find(usedIndices.begin(), usedIndices.end(), index);
            // if (it != usedIndices.end()) {
            //     usedIndices.erase(it);
            // }
        // }
        freeIndices.insert(freeIndices.end(), garbage.begin(), garbage.end());
        garbage.clear();
    }

    //might be worth it to sort for cache coherence
    void sortFreeIndices() {
        std::sort(freeIndices.begin(), freeIndices.end(), std::greater<int>());
    }

    Type& operator[](int index) {
        validate_bounds(index, storage.size());
        return storage[index];
    }
};