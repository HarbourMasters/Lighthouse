#include "ObjectBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
// #include "port/Rando/Logic/Logic.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include "spdlog/spdlog.h"

extern "C" {
int __baMarker_8028BC60(void);
void __baMarker_resolveMusicNoteCollision(Prop* arg0);
}

void LogOutSpawns(int32_t actorId, int16_t posX, int16_t posY, int16_t posZ) {
    std::string locationStr = std::to_string(posX) + ", " + std::to_string(posY) + ", " + std::to_string(posZ);
    SPDLOG_INFO("Actor ID: {} | Position: {}", actorId, locationStr);
}

void LogOutCollision(int32_t actorId, int16_t posX, int16_t posY, int16_t posZ) {
    std::string locationStr = std::to_string(posX) + ", " + std::to_string(posY) + ", " + std::to_string(posZ);
    SPDLOG_INFO("Collect ID: {} | Position: {}", actorId, locationStr);
}

// Entry point for the module, run once on game boot
void Rando::ObjectBehavior::Init() {
    REGISTER_LISTENER(OnActorCollision, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorCollision* ev = (OnActorCollision*)event;

        if (!IS_RANDO) {
            return;
        }

        switch (ev->propId->spriteProp.unk0_31) {
            case RP_MUSIC_NOTE:
                if (!__baMarker_8028BC60()) {
                    ev->propId->spriteProp.unk8_4 = 0;
                    __baMarker_resolveMusicNoteCollision(ev->propId);
                    event->cancelled = true;
                    LogOutCollision(ev->propId->spriteProp.unk0_31, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                }
                break;
            default:
                break;
        }

    })

    REGISTER_LISTENER(OnActorSpawn, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        if (!IS_RANDO) {
            return;
        }

        //LogOutSpawns(ev->actorId, ev->posX, ev->posY, ev->posZ);

        event->cancelled = true;
        ev->result = CustomObject::SpawnRandoObject(ev->actorId, ev->posX, ev->posY, ev->posZ, ev->rot);
    })
}
