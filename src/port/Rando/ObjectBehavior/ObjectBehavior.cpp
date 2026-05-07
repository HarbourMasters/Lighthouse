#include "ObjectBehavior.h"
#include "port/ui/UIWidgets.hpp"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ui/Notification.h"
#include "port/Rando/Logic/Logic.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include "spdlog/spdlog.h"

#define WIDGET_TEXT_COLOR(id) UIWidgets::ColorValues.at(id)

extern "C" {
int __baMarker_8028BC60(void);
void __baMarker_resolveMusicNoteCollision(Prop* arg0);
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);

Actor* marker_getActor(ActorMarker* thisx);
bool func_802C9C14(Actor* actor);
}

// clang-format off
std::vector<int32_t> actorSpawnWhitelist = {
    ACTOR_2D_MUMBO_TOKEN,
    ACTOR_46_JIGGY,
    ACTOR_47_EMPTY_HONEYCOMB,
    ACTOR_51_MUSIC_NOTE,
    ACTOR_5E_JINJO_YELLOW,
    ACTOR_5F_JINJO_ORANGE,
    ACTOR_60_JINJO_BLUE,
    ACTOR_61_JINJO_PINK,
    ACTOR_62_JINJO_GREEN,
    ACTOR_12C_MOLEHILL,
};

std::map<int32_t, UIWidgets::Colors> randoItemColors = {
    { RI_EMPTY_HONEYCOMB,   UIWidgets::Colors::Yellow },
    { RI_JIGGY,             UIWidgets::Colors::Yellow },
    { RI_JINJO_BLUE,        UIWidgets::Colors::SkyBlue },
    { RI_JINJO_GREEN,       UIWidgets::Colors::Green },
    { RI_JINJO_ORANGE,      UIWidgets::Colors::Orange },
    { RI_JINJO_PINK,        UIWidgets::Colors::Pink },
    { RI_JINJO_YELLOW,      UIWidgets::Colors::Yellow },
    { RI_MUMBO_TOKEN,       UIWidgets::Colors::Gray },
    { RI_MUSIC_NOTE,        UIWidgets::Colors::Yellow },
};

std::map<int32_t, actor_e> jinjoMarkerMap = {
    { MARKER_5A_JINJO_BLUE, 	ACTOR_60_JINJO_BLUE },
    { MARKER_5B_JINJO_GREEN, 	ACTOR_62_JINJO_GREEN },
    { MARKER_5C_JINJO_ORANGE, 	ACTOR_5F_JINJO_ORANGE },
    { MARKER_5D_JINJO_PINK, 	ACTOR_61_JINJO_PINK },
    { MARKER_5E_JINJO_YELLOW, 	ACTOR_5E_JINJO_YELLOW },
};
// clang-format on

bool nextActorSaveState = false;

void LogOutSpawns(int32_t actorId, int16_t posX, int16_t posY, int16_t posZ) {
    std::string locationStr = std::to_string(posX) + ", " + std::to_string(posY) + ", " + std::to_string(posZ);
    SPDLOG_INFO("Actor ID: {} | Position: {}", actorId, locationStr);
}

void LogOutCollision(int32_t actorId, int16_t posX, int16_t posY, int16_t posZ) {
    std::string locationStr = std::to_string(posX) + ", " + std::to_string(posY) + ", " + std::to_string(posZ);
    SPDLOG_INFO("Collect ID: {} | Position: {}", actorId, locationStr);
}

bool IsActorWhitelisted(int32_t actorId) {
    for (auto& entry : actorSpawnWhitelist) {
        if (entry == actorId) {
            return true;
        }
    }
    return false;
}

int32_t GetJinjoActorMarkerId(actor_e actorId) {
    for (auto& [marker, actor] : jinjoMarkerMap) {
        if (actor == actorId) {
            return marker;
        }
    }

    return NULL;
}

void SendCollisionNotification(RandoItemId randoItemId) {
    std::string prefix = "You collected ";
    prefix += Rando::StaticData::Items[randoItemId].article;
    std::string message = Rando::StaticData::Items[randoItemId].name;
    
    Notification::Emit({ .prefix = prefix,
                         .prefixColor = WIDGET_TEXT_COLOR(UIWidgets::Colors::White),
                         .message = message,
                         .messageColor = WIDGET_TEXT_COLOR(randoItemColors.at(randoItemId)) });
};

bool ShouldOverrideSpawn(RandoCheckId randoCheckId) {
    if (randoCheckId == RC_UNKNOWN) {
        return false;
    }

    if (CustomObject::CheckSpawnQueue(randoCheckId)) {
        return false;
    }

    if (Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return true;
    }

    return false;
}

// Entry point for the module, run once on game boot
void Rando::ObjectBehavior::Init() {
    InitBundleBehavior();
    InitJiggyBehavior();
    InitJinjoBehavior();
    InitMolehillBehavior();
    InitMusicNoteBehavior();
    InitPropBehavior();

    UpdateJunkList();

    REGISTER_LISTENER(OnActorSpawn, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        if (IsActorWhitelisted(ev->actorId)) {
            LogOutSpawns(ev->actorId, ev->posX, ev->posY, ev->posZ);
        }

        if (!IsActorWhitelisted(ev->actorId)) {
            CustomObject::InitializeSpawnQueue();
            return;
        }

        RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(ev->posX, ev->posY, ev->posZ);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }
        
        if (!ShouldOverrideSpawn(randoCheckId)) {
            CustomObject::InitializeSpawnQueue();
            return;
        }

        int32_t position[3];
        position[0] = ev->posX;
        position[1] = ev->posY;
        position[2] = ev->posZ;

        CustomObject::AddToSpawnQueue(randoCheckId, position);
        CustomObject::InitializeSpawnQueue();

        if (nextActorSaveState) {
            event->cancelled = true;
            ev->result = CustomObject::GetCustomActor(randoCheckId);
            nextActorSaveState = false;
            return;
        }


        switch (ev->actorId) {
            case ACTOR_12C_MOLEHILL:
                event->cancelled = true;
                ev->result = CustomObject::GetCustomActor(randoCheckId);
                break;
            default:
                event->cancelled = true;
                ev->result = NULL;
                break;
        }
    })

    REGISTER_LISTENER(OnActorSaveState, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorSaveState* ev = (OnActorSaveState*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        nextActorSaveState = true;
    })

    REGISTER_LISTENER(OnActorCollision, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnActorCollision* ev = (OnActorCollision*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        if (!ev->propId->markerFlag) {
            switch (ev->propId->spriteProp.spriteId) {
                case RP_MUSIC_NOTE:
                    LogOutCollision(ACTOR_51_MUSIC_NOTE, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    break;
            }
        }

        RandoItemId randoItemId = RI_UNKNOWN;

        if (ev->propId->markerFlag) {
            Actor* markerActor = marker_getActor(ev->propId->actorProp.marker);
            
            if (markerActor->is_bundle && func_802C9C14(markerActor)) {
                return;
            }
            
            switch (ev->propId->actorProp.marker->id) {
                case MARKER_39_MUMBO_TOKEN:
                    LogOutCollision(ACTOR_2D_MUMBO_TOKEN, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    randoItemId = RI_MUMBO_TOKEN;
                    break;
                case MARKER_52_JIGGY:
                    LogOutCollision(ACTOR_46_JIGGY, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    randoItemId = RI_JIGGY;
                    break;
                case MARKER_53_EMPTY_HONEYCOMB:
                    LogOutCollision(ACTOR_47_EMPTY_HONEYCOMB, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    randoItemId = RI_EMPTY_HONEYCOMB;
                    break;
                case MARKER_5A_JINJO_BLUE:
                case MARKER_5B_JINJO_GREEN:
                case MARKER_5C_JINJO_ORANGE:
                case MARKER_5D_JINJO_PINK:
                case MARKER_5E_JINJO_YELLOW:
                    LogOutCollision(jinjoMarkerMap.at(ev->propId->actorProp.marker->id), ev->propId->actorProp.x,
                                    ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    randoItemId = Rando::StaticData::GetRandoItemByActorId(jinjoMarkerMap.at(ev->propId->actorProp.marker->id));
                    break;
                case MARKER_5F_MUSIC_NOTE:
                    LogOutCollision(ACTOR_51_MUSIC_NOTE, ev->propId->actorProp.x, ev->propId->actorProp.y,
                                    ev->propId->actorProp.z);
                    randoItemId = RI_MUSIC_NOTE;
                    break;
                default:
                    break;
            }
        }

        if (randoItemId != RI_UNKNOWN) {
            if (randoItemId == RI_MUSIC_NOTE) {
                event->cancelled = true;
            }
            CustomObject::ObjectCollected(ev->propId);
            SendCollisionNotification(randoItemId);
        }
    })

    REGISTER_LISTENER(OnWarpDispatch, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnWarpDispatch* ev = (OnWarpDispatch*)event;

        // if (!IS_RANDO) {
        //     return;
        // }

        CustomObject::InitializeSpawnQueue();
    })
}
