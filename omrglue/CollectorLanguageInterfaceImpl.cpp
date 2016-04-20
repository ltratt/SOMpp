/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 1991, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/
#include "modronbase.h"

#include "CollectorLanguageInterfaceImpl.hpp"
#if defined(OMR_GC_MODRON_CONCURRENT_MARK)
#include "ConcurrentSafepointCallback.hpp"
#endif /* OMR_GC_MODRON_CONCURRENT_MARK */
#if defined(OMR_GC_MODRON_COMPACTION)
#include "CompactScheme.hpp"
#endif /* OMR_GC_MODRON_COMPACTION */
#include "EnvironmentStandard.hpp"
#include "GCExtensionsBase.hpp"
#include "MarkingScheme.hpp"
#include "mminitcore.h"
#include "objectdescription.h"
#include "ObjectIterator.hpp"
#include "omr.h"
#include "omrvm.h"
#include "OMRVMInterface.hpp"
#include "ParallelTask.hpp"
#include "ScanClassesMode.hpp"

#include "Heap.hpp"
#include "HeapRegionIterator.hpp"
#include "ObjectHeapIteratorAddressOrderedList.hpp"

#include "vm/Universe.h"

/**
 * Initialization
 */
MM_CollectorLanguageInterfaceImpl *
MM_CollectorLanguageInterfaceImpl::newInstance(MM_EnvironmentBase *env)
{
	MM_CollectorLanguageInterfaceImpl *cli = NULL;
	OMR_VM *omrVM = env->getOmrVM();
	MM_GCExtensionsBase *extensions = MM_GCExtensionsBase::getExtensions(omrVM);

	cli = (MM_CollectorLanguageInterfaceImpl *)extensions->getForge()->allocate(sizeof(MM_CollectorLanguageInterfaceImpl), MM_AllocationCategory::FIXED, OMR_GET_CALLSITE());
	if (NULL != cli) {
		new(cli) MM_CollectorLanguageInterfaceImpl(omrVM);
		if (!cli->initialize(omrVM)) {
			cli->kill(env);
			cli = NULL;
		}
	}

	return cli;
}

void
MM_CollectorLanguageInterfaceImpl::kill(MM_EnvironmentBase *env)
{
	OMR_VM *omrVM = env->getOmrVM();
	tearDown(omrVM);
	MM_GCExtensionsBase::getExtensions(omrVM)->getForge()->free(this);
}

void
MM_CollectorLanguageInterfaceImpl::tearDown(OMR_VM *omrVM)
{

}

bool
MM_CollectorLanguageInterfaceImpl::initialize(OMR_VM *omrVM)
{
	return true;
}

void
MM_CollectorLanguageInterfaceImpl::flushNonAllocationCaches(MM_EnvironmentBase *env)
{
}

OMR_VMThread *
MM_CollectorLanguageInterfaceImpl::attachVMThread(OMR_VM *omrVM, const char *threadName, uintptr_t reason)
{
	OMR_VMThread *omrVMThread = NULL;
	omr_error_t rc = OMR_ERROR_NONE;

	rc = OMR_Glue_BindCurrentThread(omrVM, threadName, &omrVMThread);
	if (OMR_ERROR_NONE != rc) {
		return NULL;
	}
	return omrVMThread;
}

void
MM_CollectorLanguageInterfaceImpl::detachVMThread(OMR_VM *omrVM, OMR_VMThread *omrVMThread, uintptr_t reason)
{
	if (NULL != omrVMThread) {
		OMR_Glue_UnbindCurrentThread(omrVMThread);
	}
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_masterSetupForGC(MM_EnvironmentBase *env)
{
}

static gc_oop_t
mark_object(gc_oop_t oop)
{
	if (IS_TAGGED(oop)) {
		return oop;
	}

	MM_EnvironmentBase *env = MM_EnvironmentBase::getEnvironment(omr_vmthread_getCurrent(GetHeap<OMRHeap>()->getOMRVM()));
	MM_CollectorLanguageInterfaceImpl *cli = (MM_CollectorLanguageInterfaceImpl*)env->getExtensions()->collectorLanguageInterface;

	cli->markObject(env, (omrobjectptr_t)oop);

	return oop;
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_scanRoots(MM_EnvironmentBase *env)
{
	if (env->_currentTask->synchronizeGCThreadsAndReleaseSingleThread(env, UNIQUE_ID)) {
		// This walks the globals of the universe, and the interpreter
		GetUniverse()->WalkGlobals(mark_object);
		env->_currentTask->releaseSynchronizedGCThreads(env);
	}
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_completeMarking(MM_EnvironmentBase *env)
{
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_markLiveObjectsComplete(MM_EnvironmentBase *env)
{
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_masterSetupForWalk(MM_EnvironmentBase *env)
{
}

void
MM_CollectorLanguageInterfaceImpl::markingScheme_masterCleanupAfterGC(MM_EnvironmentBase *env)
{
#if 0
	MM_HeapRegionManager *regionManager = _extensions->getHeap()->getHeapRegionManager();
	GC_HeapRegionIterator regionIterator(regionManager);
	MM_HeapRegionDescriptor *hrd = NULL;
	while (NULL != (hrd = regionIterator.nextRegion())) {
		GC_ObjectHeapIteratorAddressOrderedList objectIterator(_extensions, hrd, false);
		omrobjectptr_t omrobjptr = NULL;
		while (NULL != (omrobjptr = objectIterator.nextObject())) {
			if (!_markingScheme->isMarked(omrobjptr)) {
				/* object will be collected. We write the full contents of the object with a known value  */
				uintptr_t objsize = _extensions->objectModel.getSizeInBytesWithHeader(omrobjptr);
				memset(omrobjptr,0x5E,(size_t) objsize);
				MM_HeapLinkedFreeHeader::fillWithHoles(omrobjptr, objsize);
			}
		}
	}
#endif
}

uintptr_t
MM_CollectorLanguageInterfaceImpl::markingScheme_scanObject(MM_EnvironmentBase *env, omrobjectptr_t objectPtr, MarkingSchemeScanReason reason)
{
    AbstractVMObject* curObject = (AbstractVMObject*)objectPtr;
    curObject->WalkObjects(mark_object);

    return _extensions->objectModel.getConsumedSizeInBytesWithHeader(objectPtr);
}

#if defined(OMR_GC_MODRON_CONCURRENT_MARK)
uintptr_t
MM_CollectorLanguageInterfaceImpl::markingScheme_scanObjectWithSize(MM_EnvironmentBase *env, omrobjectptr_t objectPtr, MarkingSchemeScanReason reason, uintptr_t sizeToDo)
{
#error implement an object scanner which scans up to sizeToDo bytes
}
#endif /* OMR_GC_MODRON_CONCURRENT_MARK */

void
MM_CollectorLanguageInterfaceImpl::parallelDispatcher_handleMasterThread(OMR_VMThread *omrVMThread)
{
	/* Do nothing for now.  only required for SRT */
}

#if defined(OMR_GC_MODRON_SCAVENGER)
void
MM_CollectorLanguageInterfaceImpl::scavenger_reportObjectEvents(MM_EnvironmentBase *env)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_masterSetupForGC_clearLangStats(MM_EnvironmentBase *env)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_workerSetupForGC_clearEnvironmentLangStats(MM_EnvironmentBase *env)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_reportScavengeEnd(MM_EnvironmentBase * envBase, bool scavengeSuccessful)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_mergeGCStats_mergeLangStats(MM_EnvironmentBase *envBase)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_masterThreadGarbageCollect_scavengeComplete(MM_EnvironmentBase *envBase)
{
	/* Do nothing for now */
}

void
MM_CollectorLanguageInterfaceImpl::scavenger_masterThreadGarbageCollect_scavengeSuccess(MM_EnvironmentBase *envBase)
{
	/* Do nothing for now */
}

bool
MM_CollectorLanguageInterfaceImpl::scavenger_internalGarbageCollect_shouldPercolateGarbageCollect(MM_EnvironmentBase *envBase, PercolateReason *reason, uint32_t *gcCode)
{
	/* Do nothing for now */
	return false;
}

GC_ObjectScanner *
MM_CollectorLanguageInterfaceImpl::parallelScavenger_getObjectScanner(MM_EnvironmentStandard *env, omrobjectptr_t objectPtr, void *allocSpace, uintptr_t flags)
{
#error provide an object scanner
}
#endif /* OMR_GC_MODRON_SCAVENGER */

#if defined(OMR_GC_MODRON_COMPACTION)
void
MM_CollectorLanguageInterfaceImpl::compactScheme_verifyHeap(MM_EnvironmentBase *env, MM_MarkMap *markMap)
{
	Assert_MM_unimplemented();
}

void
MM_CollectorLanguageInterfaceImpl::compactScheme_fixupRoots(MM_EnvironmentBase *env, MM_CompactScheme *compactScheme)
{
	Assert_MM_unimplemented();
}

void
MM_CollectorLanguageInterfaceImpl::compactScheme_workerCleanupAfterGC(MM_EnvironmentBase *env)
{
	Assert_MM_unimplemented();
}

void
MM_CollectorLanguageInterfaceImpl::compactScheme_languageMasterSetupForGC(MM_EnvironmentBase *env)
{
	Assert_MM_unimplemented();
}
#endif /* OMR_GC_MODRON_COMPACTION */

#if defined(OMR_GC_MODRON_CONCURRENT_MARK)
MM_ConcurrentSafepointCallback*
MM_CollectorLanguageInterfaceImpl::concurrentGC_createSafepointCallback(MM_EnvironmentBase *env)
{
	MM_EnvironmentStandard *envStd = MM_EnvironmentStandard::getEnvironment(env);
	return MM_ConcurrentSafepointCallback::newInstance(envStd);
}

uintptr_t
MM_CollectorLanguageInterfaceImpl::concurrentGC_collectRoots(MM_EnvironmentStandard *env, ConcurrentStatus concurrentStatus, MM_ScanClassesMode *scanClassesMode, bool &collectedRoots, bool &paidTax)
{
	uintptr_t bytesScanned = 0;
	collectedRoots = true;
	paidTax = true;

	switch (concurrentStatus) {
	case CONCURRENT_ROOT_TRACING1:
		break;
	case CONCURRENT_ROOT_TRACING2:
		markingScheme_scanRoots(env);
		break;
	case CONCURRENT_ROOT_TRACING3:
		break;
	case CONCURRENT_ROOT_TRACING4:
		break;
	case CONCURRENT_ROOT_TRACING5:
		break;
#if defined(OMR_GC_DYNAMIC_CLASS_UNLOADING)
	case CONCURRENT_TRACE_ONLY:
		break;
#endif /* OMR_GC_DYNAMIC_CLASS_UNLOADING */
	default:
		Assert_MM_unreachable();
	}

	return bytesScanned;
}
#endif /* OMR_GC_MODRON_CONCURRENT_MARK */

omrobjectptr_t
MM_CollectorLanguageInterfaceImpl::heapWalker_heapWalkerObjectSlotDo(omrobjectptr_t object)
{
	return NULL;
}
