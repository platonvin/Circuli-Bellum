#pragma once
#include <strstream>
#ifndef DLIST_HPP
#define DLIST_HPP

#include <cassert>
#include <iostream>

//TODO test arena (storage+free_idxs), fake ptrs  

template <class StoredClass>
struct alignas(8) ListElem {
    ListElem* next = nullptr;
    ListElem* prev = nullptr;
    StoredClass storage;

    template <typename... Args>
    ListElem(Args&&... args) : storage(std::forward<Args>(args)...) {}
    // pointer to itself
    ListElem* self() {return this;}
    // pointer to stored object
    StoredClass* obj() {return &storage;}
};

template <class StoredClass>
class alignas(8) List {
public:
    using ListElemType = ListElem<StoredClass>;

    ListElemType* _start = nullptr;
    ListElemType* _end = nullptr;

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

    ListElemType* insertAfter(ListElemType* elem_ptr, const StoredClass& class_obj) {
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

    ListElemType* insertBefore(ListElemType* elem_ptr, const StoredClass& class_obj) {
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
    ListElemType* insertSomewhere(const StoredClass& class_obj) {
        return appendBack(class_obj);
    }

    ListElemType* appendBack(const StoredClass& class_obj) {
        ListElemType* newElem = new ListElemType(class_obj);
        if (!_end) {
            pl(_end);
            pl(_start);
            _start = _end = newElem;
            pl(_end);
            pl(_start);
        } else {
            pl(_end);
            pl(_start);
            _end->next = newElem;
            newElem->prev = _end;
            _end = newElem;
            pl(_end);
            pl(_start);
        }
        return newElem;
    }

    ListElemType* appendFront(const StoredClass& class_obj) {
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

        StoredClass& operator*() {
            return current->storage;
        }

        Iterator& operator++() {
            current = current->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return current != other.current;
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

        StoredClass& operator*() {
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
