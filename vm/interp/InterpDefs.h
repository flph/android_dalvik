/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Dalvik interpreter definitions.  These are internal to the interpreter.
 *
 * This includes defines, types, function declarations, and inline functions
 * that are common to all interpreter implementations.
 *
 * Functions and globals declared here are defined in Interp.c.
 */
#ifndef _DALVIK_INTERP_DEFS
#define _DALVIK_INTERP_DEFS


/*
 * Specify the starting point when switching between interpreters.
 */
typedef enum InterpEntry {
    kInterpEntryInstr = 0,      // continue to next instruction
    kInterpEntryReturn = 1,     // jump to method return
    kInterpEntryThrow = 2,      // jump to exception throw
#if defined(WITH_JIT)
    kInterpEntryResume = 3,     // Resume after single-step
#endif
} InterpEntry;

#if defined(WITH_JIT)
/*
 * There are six entry points from the compiled code to the interpreter:
 * 1) dvmJitToInterpNormal: find if there is a corresponding compilation for
 *    the new dalvik PC. If so, chain the originating compilation with the
 *    target then jump to it.
 * 2) dvmJitToInterpInvokeNoChain: similar to 1) but don't chain. This is
 *    for handling 1-to-many mappings like virtual method call and
 *    packed switch.
 * 3) dvmJitToInterpPunt: use the fast interpreter to execute the next
 *    instruction(s) and stay there as long as it is appropriate to return
 *    to the compiled land. This is used when the jit'ed code is about to
 *    throw an exception.
 * 4) dvmJitToInterpSingleStep: use the portable interpreter to execute the
 *    next instruction only and return to pre-specified location in the
 *    compiled code to resume execution. This is mainly used as debugging
 *    feature to bypass problematic opcode implementations without
 *    disturbing the trace formation.
 * 5) dvmJitToTraceSelect: if there is a single exit from a translation that
 *    has already gone hot enough to be translated, we should assume that
 *    the exit point should also be translated (this is a common case for
 *    invokes).  This trace exit will first check for a chaining
 *    opportunity, and if none is available will switch to the debug
 *    interpreter immediately for trace selection (as if threshold had
 *    just been reached).
 * 6) dvmJitToPredictedChain: patch the chaining cell for a virtual call site
 *    to a predicted callee.
 */
struct JitToInterpEntries {
    void *dvmJitToInterpNormal;
    void *dvmJitToInterpNoChain;
    void *dvmJitToInterpPunt;
    void *dvmJitToInterpSingleStep;
    void *dvmJitToTraceSelect;
    void *dvmJitToPatchPredictedChain;
};

#define JIT_TRACE_THRESH_FILTER_SIZE  16
#endif

/*
 * Interpreter context, used when switching from one interpreter to
 * another.  We also tuck "mterp" state in here.
 */
typedef struct InterpState {
    /*
     * To make some mterp state updates easier, "pc" and "fp" MUST come
     * first and MUST appear in this order.
     */
    const u2*   pc;                     // program counter
    u4*         fp;                     // frame pointer

    JValue      retval;                 // return value -- "out" only
    const Method* method;               // method being executed


    /* ----------------------------------------------------------------------
     * Mterp-only state
     */
    DvmDex*         methodClassDex;
    Thread*         self;

    /* housekeeping */
    void*           bailPtr;

    /*
     * These are available globally, from gDvm, or from another glue field
     * (self/method).  They're copied in here for speed.
     */
    const u1*       interpStackEnd;
    volatile int*   pSelfSuspendCount;
#if defined(WITH_DEBUGGER)
    volatile u1*    pDebuggerActive;
#endif
#if defined(WITH_PROFILER)
    volatile int*   pActiveProfilers;
#endif
    /* ----------------------------------------------------------------------
     */

    /*
     * Interpreter switching.
     */
    InterpEntry entryPoint;             // what to do when we start
    int         nextMode;               // INTERP_STD, INTERP_DBG

#if defined(WITH_JIT)
    /*
     * Local copies of field from gDvm placed here for fast access
     */
    unsigned char*     pJitProfTable;
    JitState           jitState;
    void*              jitResume;
    u2*                jitResumePC;
#endif

#if defined(WITH_PROFILER) || defined(WITH_DEBUGGER)
    bool        debugIsMethodEntry;     // used for method entry event triggers
#endif
#if defined(WITH_TRACKREF_CHECKS)
    int         debugTrackedRefStart;   // tracked refs from prior invocations
#endif

#if defined(WITH_JIT)
    struct JitToInterpEntries jitToInterpEntries;

    int currTraceRun;
    int totalTraceLen;        // Number of Dalvik insts in trace
    const u2* currTraceHead;        // Start of the trace we're building
    const u2* currRunHead;          // Start of run we're building
    int currRunLen;           // Length of run in 16-bit words
    int lastThreshFilter;
    const u2* threshFilter[JIT_TRACE_THRESH_FILTER_SIZE];
    JitTraceRun trace[MAX_JIT_RUN_LEN];
#endif

} InterpState;

/*
 * These are generated from InterpCore.h.
 */
extern bool dvmInterpretDbg(Thread* self, InterpState* interpState);
extern bool dvmInterpretStd(Thread* self, InterpState* interpState);
#define INTERP_STD 0
#define INTERP_DBG 1

/*
 * "mterp" interpreter.
 */
extern bool dvmMterpStd(Thread* self, InterpState* interpState);

/*
 * Get the "this" pointer from the current frame.
 */
Object* dvmGetThisPtr(const Method* method, const u4* fp);

/*
 * Verify that our tracked local references are valid.
 */
void dvmInterpCheckTrackedRefs(Thread* self, const Method* method,
    int debugTrackedRefStart);

/*
 * Process switch statement.
 */
s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal);
s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal);

/*
 * Process fill-array-data.
 */
bool dvmInterpHandleFillArrayData(ArrayObject* arrayObject,
                                  const u2* arrayData);

/*
 * Find an interface method.
 */
Method* dvmInterpFindInterfaceMethod(ClassObject* thisClass, u4 methodIdx,
    const Method* method, DvmDex* methodClassDex);

/*
 * Determine if the debugger or profiler is currently active.  Used when
 * selecting which interpreter to start or switch to.
 */
static inline bool dvmDebuggerOrProfilerActive(void)
{
    return gDvm.debuggerActive
#if defined(WITH_PROFILER)
        || gDvm.activeProfilers != 0
#endif
        ;
}

#if defined(WITH_JIT)
/*
 * Determine if the jit, debugger or profiler is currently active.  Used when
 * selecting which interpreter to switch to.
 */
static inline bool dvmJitDebuggerOrProfilerActive(int jitState)
{
    return jitState != kJitOff
#if defined(WITH_PROFILER)
        || gDvm.activeProfilers != 0
#endif
        ||gDvm.debuggerActive;
}
#endif

#endif /*_DALVIK_INTERP_DEFS*/
