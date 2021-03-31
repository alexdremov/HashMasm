//
// Created by Александр Дремов on 31.03.2021.
//

#ifndef HashMasm_GUARD
#define HashMasm_GUARD
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "FastList.h"
#include "hashes.h"

template<typename T>
class HashMasm {
    struct HashCell {
        char *key;
        T value;
        size_t hash;
        bool dublicateKey;

        void dest(){
            if (dublicateKey)
                free(key);
        }

        void init(char *keyDubNew, const T& valueNew, size_t hashedInitial, bool dublicateKeyNew) {
            key = keyDubNew;
            value = valueNew;
            hash = hashedInitial;
            dublicateKey = dublicateKeyNew;
        }

        static HashCell* New(char *keyDubNew, const T& valueNew, size_t hashedInitial, bool dublicateKeyNew){
            HashCell *thou = static_cast<HashCell *>(calloc(1, sizeof(HashCell)));
            thou->init(keyDubNew, valueNew, hashedInitial, dublicateKeyNew);
            return thou;
        }

        static void Delete(HashCell* thou) {
            thou->dest();
            free(thou);
        }
    };
    typedef HashCell* listCell;
    FastList<listCell>* storage;
    bool         isRehash;
    size_t       capacity;
    unsigned     loadRate;
    size_t       size;
    size_t       threshold;

    size_t hashString(const char* key) {
//        return key[0] + key[1];
//        return CRC::hash64(-1, reinterpret_cast<const unsigned char *>(key));
        return FNV::fnv64(key);
    }

    constexpr static int listInitSize = 8;
    constexpr static int minCapacity = 32;

    int rehash() {
        FastList<listCell>* previousStorage = storage;
        size_t newCapacity = capacity * 2;
        storage = (FastList<listCell>*)calloc(newCapacity, sizeof(FastList<listCell>));
        if (!storage)
            return EXIT_FAILURE;

        initStorage(storage, newCapacity);

        for (size_t i = 0; i < capacity; i++) {
            for (size_t iter = previousStorage[i].begin(); iter != 0; previousStorage[i].nextIterator(&iter)) {
                listCell* value = nullptr;
                previousStorage[i].get(iter, &value);
                storage[(*value)->hash % newCapacity].pushBack(*value);
            }
        }

        capacity = newCapacity;
        threshold = loadRate * capacity / 100;
        return EXIT_SUCCESS;
    }

    int tryRehash() {
        if (isRehash && size > threshold)
            return rehash();
        return EXIT_SUCCESS;
    }

    void freeStorage(FastList<HashCell*>* storageTest) {
        listCell* tmp = nullptr;
        for (int i = 0; i < capacity; i++) {
            for (auto it = storageTest[i].begin(); it != 0; storageTest[i].nextIterator(&it)){
                storageTest[i].get(it, &tmp);
                HashCell::Delete(*tmp);
            }
        }
        free(storageTest);
    }

    void initStorage(FastList<listCell>* storageTest, size_t len){
        for (size_t i = 0; i < len; i++) {
            storageTest[i].init(listInitSize);
        }
    }

public:

    int init(size_t newCapacity, bool rehash=true, unsigned newLoadrate = 75) {
        if (newCapacity < minCapacity)
            newCapacity = minCapacity;
        capacity = newCapacity;
        loadRate = newLoadrate;
        isRehash = rehash;
        threshold = loadRate * capacity / 100;
        size = 0;
        storage = (FastList<listCell>*)calloc(capacity, sizeof(FastList<listCell>));
        if (!storage) {
            return EXIT_FAILURE;
        }
        initStorage(storage, capacity);
        return EXIT_SUCCESS;
    }

    int init() {
        return init(minCapacity);
    }

    void dest() {
        freeStorage(storage);
    }

    void set(const char* key, const T& value, bool dublicateKey=true) {
        tryRehash();
        size_t hashedInitial = hashString(key);
        size_t hashed = hashedInitial % capacity;
        char* keyDub = const_cast<char *>(key);
        if (dublicateKey)
            keyDub = strdup(key);


        listCell* node = nullptr;
        for (size_t i = storage[hashed].begin(); i != 0; storage[hashed].nextIterator(&i)) {
            listCell* tmpNode = nullptr;
            storage[hashed].get(i, &tmpNode);
            if (strcmp((*tmpNode)->key, key) == 0){
                node = tmpNode;
                break;
            }
        }
        if (node) {
            (*node)->value = value;
        } else {
            listCell newCell = HashCell::New(keyDub, value, hashedInitial, dublicateKey);
            storage[hashed].pushBack(newCell);
            size++;
        }
    }

    T* get(const char* key) {
        size_t hashed = hashString(key) % capacity;

        listCell *node = nullptr;
        for (size_t i = storage[hashed].begin(); i != 0; storage[hashed].nextIterator(&i)) {
            listCell *tmpNode = nullptr;
            storage[hashed].get(i, &tmpNode);
            if (strcmp((*tmpNode)->key, key) == 0) {
                node = tmpNode;
                break;
            }
        }
        if (node)
            return &((*node)->value);
        else
            return nullptr;
    }

    static HashMasm *New() {
        HashMasm *thou = static_cast<HashMasm *>(calloc(1, sizeof(HashMasm)));
        thou->init();
        return thou;
    }

    void Delete() {
        dest();
        free(this);
    }

    bool getIsRehash() const {
        return isRehash;
    }

    size_t getCapacity() const {
        return capacity;
    }

    unsigned int getLoadRate() const {
        return loadRate;
    }

    size_t getSize() const {
        return size;
    }

    size_t getThreshold() const {
        return threshold;
    }

    static int getListInitSize() {
        return listInitSize;
    }

    static int getMinCapacity() {
        return minCapacity;
    }
};

#endif //HashMasm_GUARD
