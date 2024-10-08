#pragma once
#ifndef DLIST_HPP
#define DLIST_HPP
#include <unordered_set>

#include <cassert>
#include <iostream>

//TODO test arena (storage+free_idxs), fake ptrs. Arena to decrease alloc overhead

//not-a-ponter to not have 1 more memory indirection. 
template <class StoredClass>
struct alignas(8) ListElem {
    //first for rtti
    StoredClass storage;
    ListElem* next = nullptr;
    ListElem* prev = nullptr;

    template <typename... Args>
    ListElem(Args&&... args) : storage(std::forward<Args>(args)...) {}
    // pointer to itself
    ListElem* self() {return this;}
    // pointer to stored object
    StoredClass* obj() {return &storage;}
};

template <class Type>
class alignas(8) List {
public:
    using ListElemType = ListElem<Type>;

    ListElemType* _start = nullptr;
    ListElemType* _end = nullptr;

    std::unordered_set<ListElem<Type>*> garbage; 
    // List itself isn't malloced
    ~List() {
        removeAll();
    }

    void removeAll() {
        ListElemType* current = _start;
        while (current) {
            ListElemType* nextElem = current->next;
            delete current;
            current = nextElem;
        }
        _start = _end = nullptr;
    }

    void removeElem(ListElemType* elem_ptr) {
        assert(elem_ptr);
        bool found = false;
        ListElemType* current = _start;
        while (current) {
            ListElemType* nextElem = current->next;
            if(current == elem_ptr) found=true;
            current = nextElem;
        } assert (found);
        
        // assuming elem_ptr is perfectly valid
        if (elem_ptr->prev) {
            elem_ptr->prev->next = elem_ptr->next;
        } else {
            _start = elem_ptr->next;
        }

        if (elem_ptr->next) {
            elem_ptr->next->prev = elem_ptr->prev;
        } else {
            _end = elem_ptr->prev;
        }

        delete elem_ptr;
    }

    //GC
    void softRemoveElem(ListElemType* elem_ptr) {
        garbage.insert(elem_ptr);
    }

    void collectGarbage() {
// al()
    //     ListElemType* current = _start;
    // while (current) {
    //     ListElemType* nextElem = current->next;
    //     pl(current);
    //     current = nextElem;
    // }

        for(auto g : garbage){
// al()
            // pl(g);
            removeElem(g);
        }
// al()
        garbage.clear();
// al()
    }


    ListElemType* insertAfter(ListElemType* elem_ptr, const Type& class_obj) {
        ListElemType* newElem = new ListElemType(class_obj);
        newElem->next = elem_ptr->next;
        newElem->prev = elem_ptr;

        if (elem_ptr->next) {
            elem_ptr->next->prev = newElem;
        } else {
            _end = newElem;
        }

        elem_ptr->next = newElem;
        return newElem;
    }

    ListElemType* insertBefore(ListElemType* elem_ptr, const Type& class_obj) {
        ListElemType* newElem = new ListElemType(class_obj);
        newElem->next = elem_ptr;
        newElem->prev = elem_ptr->prev;

        if (elem_ptr->prev) {
            elem_ptr->prev->next = newElem;
        } else {
            _start = newElem;
        }

        elem_ptr->prev = newElem;
        return newElem;
    }

    //TODO?
    ListElemType* insertSomewhere(const Type& class_obj) {
        return appendBack(class_obj);
    }

    ListElemType* appendBack(const Type& class_obj) {
        ListElemType* newElem = new ListElemType(class_obj);
        if (!_end) {
            _start = _end = newElem;
        } else {
            _end->next = newElem;
            newElem->prev = _end;
            _end = newElem;
        }
        return newElem;
    }

    ListElemType* appendFront(const Type& class_obj) {
        ListElemType* newElem = new ListElemType(class_obj);
        if (!_start) {
            _start = _end = newElem;
        } else {
            newElem->next = _start;
            _start->prev = newElem;
            _start = newElem;
        }
        return newElem;
    }

    // i'll surely need to debug it
    void printList() const {
        ListElemType* current = _start;
        while (current) {
            std::cout << &current->storage << " ";
            current = current->next;
        }
        std::cout << std::endl;
    }


    class Iterator {
    public:
        Iterator(ListElemType* ptr) : current(ptr) {}

        Type& operator*() {
            return current->storage;
        }

        Iterator& operator++() {
            current = current->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
        bool operator==(const Iterator& other) const {
            return current == other.current;
        }

        unsigned long long operator-(const Iterator& other) const {
            return (unsigned long long)(void*)current - (unsigned long long)(void*)other.current;
        }

        operator void*(){
            return current;
        }
    private:

        ListElemType* current;
    };

    class ReverseIterator {
    public:
        ReverseIterator(ListElemType* ptr) : current(ptr) {}

        Type& operator*() {
            return current->storage;
        }

        ReverseIterator& operator++() {
            current = current->prev;
            return *this;
        }

        bool operator!=(const ReverseIterator& other) const {
            return current != other.current;
        }

    private:
        ListElemType* current;
    };

    Iterator begin() {
        return Iterator(_start);
    }

    Iterator end() {
        return Iterator(nullptr);
    }

    ReverseIterator rbegin() {
        return ReverseIterator(_end);
    }

    ReverseIterator rend() {
        return ReverseIterator(nullptr);
    }
};


#endif // DLIST_HPP
