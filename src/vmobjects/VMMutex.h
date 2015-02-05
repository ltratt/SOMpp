#pragma once

#include <mutex>

#include "VMObject.h"

class VMMutex : public VMObject {
public:
    typedef GCMutex Stored;
    
    VMMutex() : VMObject(VMMutexNumberOfFields),
                lock(new unique_lock<recursive_mutex>(
                                *new recursive_mutex(), std::defer_lock)) {};
    
    void Lock();
    void Unlock();
    bool IsLocked() const;

    VMCondition* NewCondition(Page* page);
    
    virtual StdString AsDebugString();
    virtual VMMutex* Clone(Page*);
    virtual void MarkObjectAsInvalid();

#if GC_TYPE==PAUSELESS
    virtual void MarkReferences();
    virtual void CheckMarking(void (vm_oop_t));
#endif

    std::unique_lock<recursive_mutex>* GetLock() const { return lock; }
    
private:
    VMMutex(std::unique_lock<recursive_mutex>* const lock)
        : VMObject(VMMutexNumberOfFields), lock(lock) {};
    std::unique_lock<recursive_mutex>* const lock;
    
    static const long VMMutexNumberOfFields;
    
};
