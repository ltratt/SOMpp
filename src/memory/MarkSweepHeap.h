#pragma once

#include "../misc/defs.h"

#include "Heap.h"

#include <mutex>


class MarkSweepHeap : public Heap<MarkSweepHeap> {
    friend class MarkSweepCollector;
public:
    MarkSweepHeap(long objectSpaceSize = 1048576);
    AbstractVMObject* AllocateObject(size_t size);
private:
    vector<AbstractVMObject*>* allocatedObjects;
    size_t spcAlloc;
    long collectionLimit;

    mutex allocatedObjects_mutex;
};
