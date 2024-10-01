#pragma once
#ifndef __UMAP_GC_HPP__
#define __UMAP_GC_HPP__

#include "al.hpp"
#include <unordered_map>

/*
wrapper around std umap that does not invalidate element immediately
this is required for phyics processing, as some events that destroy objects 
might occure before other events that need to reference that objects
*/

template<typename Type> 
class gcmap{
private: 
    std::unordered_map<int, Type> storage;
    //TODO test pod array
    vector<int> garbage;
    int counter = 0;
public:
    int allocate(Type elem){
        storage[counter] = elem;
        return counter++;
    }
    void free(int index){
        storage.erase(index);
    }
    //exists to make references valid for a single frame. Also might havehigher perfomance on big structs
    void softFree(int index){
        garbage.push_back(index);
    }
    void collectGarbage(){
        storage.erase(garbage.begin(), garbage.end());
        garbage.clear();
    }
    void clear(){
        storage.clear();
    }
    bool contains(int index){
        return storage.contains(index);
    }
    Type& operator [](int index){
        return storage[index];
    }

    auto begin(){
        return storage.begin();
    }
    auto end(){
        return storage.end();
    }
};
#endif // __UMAP_GC_HPP__