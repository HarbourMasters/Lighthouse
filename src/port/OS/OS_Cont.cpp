// This file should eventually go back to LUS; it is unchanged from its
// libultra/os.cpp original, and only lives here because taking osSetTimer
// port-side means taking that whole object with it.

#include <libultraship/libultraship.h>

#include <cstring>

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

int32_t osContStartReadData(OSMesgQueue* mesg) {
    return 0;
}

void osContGetReadData(OSContPad* pad) {
    memset(pad, 0, sizeof(OSContPad) * __osMaxControllers);

    Ship::Context::GetRawInstance()->GetControlDeck()->WriteToPad(pad);
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

} // extern "C"
