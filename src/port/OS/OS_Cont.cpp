// This file should eventually go to LUS as the SI api. The controller init and
// motor pieces are unchanged from libultra/os.cpp and only live here because
// taking osSetTimer port-side meant taking that whole object with it.

#include "OS.h"

#include <libultraship/libultraship.h>

#include <atomic>
#include <cstring>
#include <mutex>

extern "C" {

uint8_t __osMaxControllers = MAXCONTROLLERS;

int32_t osContInit(OSMesgQueue* mq, uint8_t* controllerBits, OSContStatus* status) {
    *controllerBits = 0;
    status->status |= 1;

    // The SDL game-controller subsystem and mappings are brought up earlier, in
    // Context::InitControlDeck, so controllers work in pre-game UI. ControlDeck::Init
    // stays here as it needs the game's controllerBits.
    Ship::Context::GetRawInstance()->GetControlDeck()->Init(controllerBits);

    return 0;
}

// The SI, as a request and a completion rather than an inline poll.
//
// LUS's StartReadData does nothing and GetReadData polls the control deck on
// whichever thread asked, which cannot work once the tick is off the window
// thread: SDL input belongs to the thread pumping its events. A read is
// posted here, completed by that thread in OS_SiService, and answered with
// OS_EVENT_SI, which is what pfsManager_init already registered for and what
// its thread has been waiting on all along.
namespace {
std::atomic<bool> sReadPending{ false };
std::mutex sLatchMutex;
OSContPad sLatch[MAXCONTROLLERS];
std::atomic<bool> sLatchValid{ false };
std::atomic<bool> sPumpLive{ false };
} // namespace

int32_t osContStartReadData(OSMesgQueue* mesg) {
    (void)mesg; // completion goes to whoever registered for OS_EVENT_SI
    sReadPending.store(true, std::memory_order_release);
    return 0;
}

extern "C" int OS_SiPumpLive(void) {
    return sPumpLive.load(std::memory_order_acquire) ? 1 : 0;
}

extern "C" int OS_SiService(void) {
    sPumpLive.store(true, std::memory_order_release);
    if (!sReadPending.exchange(false, std::memory_order_acq_rel)) {
        return 0;
    }
    {
        std::lock_guard<std::mutex> lock(sLatchMutex);
        memset(sLatch, 0, sizeof(sLatch));
        Ship::Context::GetRawInstance()->GetControlDeck()->WriteToPad(sLatch);
    }
    sLatchValid.store(true, std::memory_order_release);
    OS_SendEventMesg(OS_EVENT_SI);
    return 1;
}

void osContGetReadData(OSContPad* pad) {
    if (sLatchValid.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(sLatchMutex);
        memcpy(pad, sLatch, sizeof(sLatch));
        return;
    }
    memset(pad, 0, sizeof(OSContPad) * __osMaxControllers);
}

int32_t __osMotorAccess(OSPfs* pfs, uint32_t vibrate) {
    auto io = Ship::Context::GetRawInstance()->GetControlDeck()->GetControllerByPort(pfs->channel)->GetRumble();
    if (vibrate) {
        io->StartRumble();
    } else {
        io->StopRumble();
    }

    return 0;
}

int32_t osMotorInit(OSMesgQueue* ctrlrqueue, OSPfs* pfs, int32_t channel) {
    pfs->channel = channel;
    return 0;
}

// Controller Pak gets probed first, the return kickstarts rumble
int32_t osPfsInit(OSMesgQueue* mq, OSPfs* pfs, int32_t channel) {
    (void)mq;
    (void)pfs;
    (void)channel;
    return PFS_ERR_DEVICE;
}

} // extern "C"
