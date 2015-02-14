#include "BaseThread.h"

#if GC_TYPE==PAUSELESS
    #include "PauselessHeap.h"
#elif GC_TYPE==GENERATIONAL
    #include "GenerationalHeap.h"
#endif

//#include "GenerationalHeap.h"


#if GC_TYPE==PAUSELESS
BaseThread::BaseThread(Page* page) : page(page), expectedNMT(false) {
    if (page)
        page->SetNonRelocatablePage(GetHeap<HEAP_CLS>()->pagedHeap.GetNextPage());

}

BaseThread::BaseThread(Page* page, bool expectedNMT) : page(page), expectedNMT(expectedNMT) {
    if (page)
        page->SetNonRelocatablePage(GetHeap<HEAP_CLS>()->pagedHeap.GetNextPage());
}
#else
BaseThread::BaseThread(Page* page) : page(page) {}
#endif

BaseThread::~BaseThread() {
#warning TODO: is the page properly reglingushed even if not done here?
//   GetHeap<HEAP_CLS>()->RelinquishPage(page);
}

Page* BaseThread::GetPage() {
    return page;
}

void BaseThread::SetPage(Page* page) {
    this->page = page;
}

#if GC_TYPE==PAUSELESS
bool BaseThread::GetExpectedNMT() {
    return expectedNMT;
}
#endif