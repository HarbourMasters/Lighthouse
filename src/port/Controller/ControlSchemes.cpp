// Control Schemes
//
// Provides three selectable gamepad presets and the per-frame input shaping two
// of them depend on:
//
//   Retro  - Stock N64 layout
//   Modern - Xbox Live Arcade layout
//   Pocket - Super Pocket layout

#include <memory>
#include <SDL2/SDL.h>

#include <ship/Context.h>
#include <ship/controller/controldeck/ControlDeck.h>
#include <ship/controller/controldevice/controller/Controller.h>
#include <ship/controller/controldevice/controller/mapping/ControllerAxisDirectionMapping.h>
#include <ship/controller/controldevice/controller/mapping/sdl/SDLButtonToButtonMapping.h>
#include <ship/controller/controldevice/controller/mapping/sdl/SDLAxisDirectionToButtonMapping.h>
#include <ship/controller/controldevice/controller/mapping/sdl/SDLButtonToAxisDirectionMapping.h>
#include <ship/controller/physicaldevice/PhysicalDeviceType.h>
#include <libultraship/libultra/controller.h>
#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Camera/FreeLookCamera.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "ControlSchemes.h"

extern "C" {
#include "enums.h"
int bs_getState(void);
int bsbtrot_inSet(int state); // nonzero if `state` is one of the Talon Trot states
s32 getGameMode(void);
}

using namespace Ship;

namespace {

std::shared_ptr<Controller> GetPort0Controller() {
    auto ctx = Ship::Context::GetRawInstance();
    if (ctx == nullptr) {
        return nullptr;
    }
    auto controlDeck = ctx->GetControlDeck();
    if (controlDeck == nullptr) {
        return nullptr;
    }
    return controlDeck->GetControllerByPort(0);
}

// Replace a single N64 button's SDL mappings (keyboard mappings are untouched).
void ClearButtonSDL(const std::shared_ptr<Controller>& controller, CONTROLLERBUTTONS_T bitmask) {
    auto button = controller->GetButton(bitmask);
    if (button != nullptr) {
        button->ClearAllButtonMappingsForDeviceType(Ship::SDLGamepad);
    }
}

void BindButton(const std::shared_ptr<Controller>& controller, CONTROLLERBUTTONS_T bitmask, int32_t sdlButton) {
    auto button = controller->GetButton(bitmask);
    if (button == nullptr) {
        return;
    }
    auto mapping = std::make_shared<SDLButtonToButtonMapping>(0, bitmask, sdlButton);
    button->AddButtonMapping(mapping);
    mapping->SaveToConfig();
    button->SaveButtonMappingIdsToConfig();
}

void BindAxisToButton(const std::shared_ptr<Controller>& controller, CONTROLLERBUTTONS_T bitmask, int32_t sdlAxis,
                      int32_t axisDirection) {
    auto button = controller->GetButton(bitmask);
    if (button == nullptr) {
        return;
    }
    auto mapping = std::make_shared<SDLAxisDirectionToButtonMapping>(0, bitmask, sdlAxis, axisDirection);
    button->AddButtonMapping(mapping);
    mapping->SaveToConfig();
    button->SaveButtonMappingIdsToConfig();
}

void BindButtonToLeftStick(const std::shared_ptr<Controller>& controller, Direction direction, int32_t sdlButton) {
    auto stick = controller->GetLeftStick();
    if (stick == nullptr) {
        return;
    }
    auto mapping = std::make_shared<SDLButtonToAxisDirectionMapping>(0, LEFT_STICK, direction, sdlButton);
    stick->AddAxisDirectionMapping(direction, mapping);
    mapping->SaveToConfig();
    stick->SaveAxisDirectionMappingIdsToConfig();
}

// How many of a button's mappings are physically active right now. Lets a
// scheme tell one binding for a button apart from another without reaching past
// the mapper to poll hardware that the player may have unbound.
int CountActiveMappings(const std::shared_ptr<Controller>& controller, CONTROLLERBUTTONS_T bitmask) {
    auto button = controller->GetButton(bitmask);
    if (button == nullptr) {
        return 0;
    }
    int active = 0;
    for (auto& [id, mapping] : button->GetAllButtonMappings()) {
        uint16_t held = 0;
        mapping->UpdatePad(held);
        if (held != 0) {
            active++;
        }
    }
    return active;
}

// Splits a C-button's mappings into the two kinds the shaping pass cares about.
void CollectCButtonSources(const std::shared_ptr<Controller>& controller, CONTROLLERBUTTONS_T bitmask,
                           uint16_t& axisMask, uint16_t& otherHeld) {
    auto button = controller->GetButton(bitmask);
    if (button == nullptr) {
        return;
    }
    for (auto& [id, mapping] : button->GetAllButtonMappings()) {
        auto axis = dynamic_cast<SDLAxisDirectionToButtonMapping*>(mapping.get());
        if (axis != nullptr && axis->AxisIsStick()) {
            axisMask |= bitmask;
        } else {
            mapping->UpdatePad(otherHeld);
        }
    }
}

} // namespace

void ControlSchemes_Apply(int scheme) {
    auto controller = GetPort0Controller();
    if (controller == nullptr) {
        return;
    }

    // Every scheme starts from the stock defaults (the Retro layout), then
    // repurposes only the buttons it changes. This keeps the left-stick and
    // start/D-pad mappings correct without rebuilding them by hand.
    controller->ClearAllMappingsForDeviceType(Ship::SDLGamepad);
    controller->AddDefaultMappings(Ship::SDLGamepad);

    switch (scheme) {
        case CONTROL_SCHEME_MODERN: {
            // Yaw-only free look + Wonderwing when crouched
            ClearButtonSDL(controller, BTN_CLEFT);
            ClearButtonSDL(controller, BTN_CRIGHT);
            ClearButtonSDL(controller, BTN_CDOWN);
            ClearButtonSDL(controller, BTN_CUP);
            BindButton(controller, BTN_CUP, SDL_CONTROLLER_BUTTON_Y);
            BindButton(controller, BTN_CDOWN, SDL_CONTROLLER_BUTTON_B);
            BindAxisToButton(controller, BTN_CLEFT, SDL_CONTROLLER_AXIS_RIGHTX, -1);
            BindAxisToButton(controller, BTN_CRIGHT, SDL_CONTROLLER_AXIS_RIGHTX, 1);

            // Recenter camera
            ClearButtonSDL(controller, BTN_R);
            BindButton(controller, BTN_R, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

            // Crouch / Talon Trot
            ClearButtonSDL(controller, BTN_Z);
            BindAxisToButton(controller, BTN_Z, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1);
            BindAxisToButton(controller, BTN_Z, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1);

            // Attacks
            ClearButtonSDL(controller, BTN_B);
            BindButton(controller, BTN_B, SDL_CONTROLLER_BUTTON_X);

            // L remains L to preserve skip dialog combo
            ClearButtonSDL(controller, BTN_L);
            BindButton(controller, BTN_L, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            break;
        }
        case CONTROL_SCHEME_POCKET: {
            // Movement: D-pad
            ClearButtonSDL(controller, BTN_DUP);
            ClearButtonSDL(controller, BTN_DDOWN);
            ClearButtonSDL(controller, BTN_DLEFT);
            ClearButtonSDL(controller, BTN_DRIGHT);
            controller->GetLeftStick()->ClearAllMappingsForDeviceType(Ship::SDLGamepad);
            BindButtonToLeftStick(controller, UP, SDL_CONTROLLER_BUTTON_DPAD_UP);
            BindButtonToLeftStick(controller, DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            BindButtonToLeftStick(controller, LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            BindButtonToLeftStick(controller, RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

            // First person
            ClearButtonSDL(controller, BTN_CUP);
            BindButton(controller, BTN_CUP, SDL_CONTROLLER_BUTTON_Y);

            // Camera left/right
            ClearButtonSDL(controller, BTN_CLEFT);
            BindButton(controller, BTN_CLEFT, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            ClearButtonSDL(controller, BTN_CRIGHT);
            BindButton(controller, BTN_CRIGHT, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

            // Zoom out / poop egg
            // Context-aware, imgui menu takes up BACK/SEL
            // so we can't be faithful to Pocket here
            ClearButtonSDL(controller, BTN_CDOWN);
            BindButton(controller, BTN_CDOWN, SDL_CONTROLLER_BUTTON_B);

            // Tip-toe / Talon Trot trigger. Recentering moves to a B hold, which
            // frees R to carry Pocket's extra gesture as a real, rebindable
            // binding instead of the shaping pass polling the trigger itself.
            ClearButtonSDL(controller, BTN_R);
            BindAxisToButton(controller, BTN_R, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1);

            // Crouch / Talon Trot
            ClearButtonSDL(controller, BTN_Z);
            BindAxisToButton(controller, BTN_Z, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1);

            // Attacks
            ClearButtonSDL(controller, BTN_B);
            BindButton(controller, BTN_B, SDL_CONTROLLER_BUTTON_X);
            break;
        }
        case CONTROL_SCHEME_RETRO:
        default:
            break;
    }

    auto ctx = Ship::Context::GetRawInstance();
    if (ctx != nullptr && ctx->GetConsoleVariables() != nullptr) {
        ctx->GetConsoleVariables()->Save();
    }
}

// Frames of C-Down passthrough left for a jigsaw podium.
static int32_t sJigsawPodiumTTL = 0;

void RegisterControlSchemes_Init() {
    COND_HOOK(OnJigsawPodiumInput, EVENT_PRIORITY_NORMAL, true, [](IEvent*) { sJigsawPodiumTTL = 4; });
}

static RegisterShipInitFunc initControlSchemes(RegisterControlSchemes_Init);

extern "C" void port_shapeControllerInput(void* contPad) {
    auto* pad = static_cast<OSContPad*>(contPad);
    if (pad == nullptr) {
        return;
    }
    // Demo modes feed their own recorded pad; don't reshape live input over it.
    if (IsDemoMode()) {
        return;
    }
    auto controller = GetPort0Controller();
    if (controller == nullptr) {
        return;
    }

    const int scheme = CVarGetInteger(CVAR_SETTING("Controls.Scheme"), CONTROL_SCHEME_RETRO);
    const bool modern = (scheme == CONTROL_SCHEME_MODERN);
    const bool pocket = (scheme == CONTROL_SCHEME_POCKET);
    const bool crouched = (bs_getState() == BS_7_CROUCH);
    const bool eggPooping = (bs_getState() == BS_A_EGG_ASS);

    // Pocket takes its tip-toe / Talon Trot gesture from R and its crouch from
    // Z, so both follow whatever the player has bound. Sample them here, before
    // the blocks below start injecting synthetic R and Z bits, and consume R -
    // in Pocket that button is the gesture channel, not the camera recenter.
    bool pocketGesture = false;
    bool pocketCrouch = false;
    if (pocket) {
        pocketGesture = (pad->button & BTN_R) != 0;
        pocketCrouch = (pad->button & BTN_Z) != 0;
        pad->button &= ~BTN_R;
    }

    int32_t rx = pad->right_stick_x;
    int32_t ry = pad->right_stick_y;
    int32_t arx = (rx < 0) ? -rx : rx;
    int32_t ary = (ry < 0) ? -ry : ry;

    const bool onJigsawPodium = (sJigsawPodiumTTL > 0);
    if (sJigsawPodiumTTL > 0) {
        sJigsawPodiumTTL--;
    }

    if (modern) {
        if (!crouched && !eggPooping && !onJigsawPodium) {
            pad->button &= ~BTN_CDOWN;
        }

        const s32 mode = getGameMode();
        const bool picturePuzzle = (mode == GAME_MODE_8_BOTTLES_BONUS || mode == GAME_MODE_A_SNS_PICTURE);
        if (!picturePuzzle) {
            const uint16_t stickC = pad->button & (BTN_CLEFT | BTN_CRIGHT);
            uint16_t allow = 0;

            // One action per push, so holding the stick cannot repeat Wonderwing.
            static bool sStickArmed = true;
            if (stickC == 0) {
                sStickArmed = true;
            } else if (crouched && (stickC & BTN_CRIGHT) && arx >= ary && sStickArmed) {
                allow = BTN_CRIGHT;
                sStickArmed = false;
            }
            pad->button = (pad->button & ~(BTN_CLEFT | BTN_CRIGHT)) | allow;
        }
    } else {
        uint16_t axisMask = 0;
        uint16_t otherHeld = 0;
        static const CONTROLLERBUTTONS_T kCButtons[] = { BTN_CRIGHT, BTN_CLEFT, BTN_CDOWN, BTN_CUP };
        for (CONTROLLERBUTTONS_T cButton : kCButtons) {
            CollectCButtonSources(controller, cButton, axisMask, otherHeld);
        }
        // Directions the stick owns outright; anything a second binding is
        // holding stays put instead of being cleared and regenerated.
        const uint16_t clearMask = axisMask & ~otherHeld;

        static uint16_t sLatchDir = 0;
        static int32_t sLatchTTL = 0;

        if (port_freeLook_isEnabled() && !onJigsawPodium) {
            uint16_t stickBits = 0;
            if (arx * arx + ary * ary > 24 * 24) {
                if (arx >= ary) {
                    stickBits = (rx > 0) ? BTN_CRIGHT : BTN_CLEFT;
                } else {
                    stickBits = (ry > 0) ? BTN_CUP : BTN_CDOWN;
                }
            }
            pad->button &= ~(stickBits & clearMask);
            sLatchDir = 0;
            sLatchTTL = 0;
        } else if (axisMask != 0) {
            pad->button &= ~clearMask;

            if (arx * arx + ary * ary > 24 * 24) {
                uint16_t dir;
                if (arx >= ary) {
                    dir = (rx > 0) ? BTN_CRIGHT : BTN_CLEFT;
                } else {
                    dir = (ry > 0) ? BTN_CUP : BTN_CDOWN;
                }

                if (dir & axisMask) {
                    if (dir != sLatchDir) {
                        if (sLatchTTL <= 0) {
                            sLatchDir = dir;
                            sLatchTTL = 6;
                        }
                    } else {
                        sLatchTTL = 6;
                    }
                    if (sLatchDir & axisMask) {
                        pad->button |= sLatchDir;
                    }
                } else {
                    if (sLatchTTL > 0) {
                        sLatchTTL--;
                    } else {
                        sLatchDir = 0;
                    }
                }
            } else {
                if (sLatchTTL > 0) {
                    sLatchTTL--;
                } else {
                    sLatchDir = 0;
                }
            }
        }
    }

    // Modern Talon Trot
    //
    // Crouch gets bound to both triggers, and pressing the second one while the
    // first is held starts the trot. Counting how many crouch bindings are live
    // expresses that without naming a trigger: whatever the player has on Z is
    // what works, and a trigger they have unbound does nothing.
    static int sPrevCrouchBindings = 0;
    if (modern) {
        const int crouchBindings = CountActiveMappings(controller, BTN_Z);
        if (crouched && crouchBindings >= 2 && crouchBindings > sPrevCrouchBindings) {
            pad->button |= BTN_CLEFT;
        }
        sPrevCrouchBindings = crouchBindings;
    } else {
        sPrevCrouchBindings = 0;
    }

    // Toggle Trot
    static bool sTrotLatched = false;
    static bool sTrotToggledOff = false;
    static bool sPrevZ = false;
    if (CVarGetInteger(CVAR_SETTING("Controls.TalonTrotToggle"), 0) != 0 && bsbtrot_inSet(bs_getState())) {
        bool zNow = (pad->button & BTN_Z) != 0;
        if (!sTrotToggledOff) {
            if (!sTrotLatched) {
                if (!zNow) {
                    sTrotLatched = true; // crouch released after entering trot -> latch it on
                }
            } else if (zNow && !sPrevZ) {
                sTrotLatched = false; // fresh Z tap while latched -> toggle off
                sTrotToggledOff = true;
            }
            if (sTrotLatched) {
                pad->button |= BTN_Z; // keep Z held so the trot doesn't exit
            }
        }
        sPrevZ = zNow;
    } else {
        // Not trotting (or feature off): reset for the next trot.
        sTrotLatched = false;
        sTrotToggledOff = false;
        sPrevZ = (pad->button & BTN_Z) != 0;
    }

    // Pocket: Context-aware B
    // While crouched, poop an egg
    // While standing, quick tap cycles zooms and hold recenters the camera
    static int32_t sBHeldFrames = 0;
    static bool sBHoldFired = false;
    static bool sBWasHeld = false;
    const int32_t kBHoldFrames = 12; // ~0.2s before a standing press counts as a hold
    bool bHeld = (pad->button & BTN_CDOWN) != 0;
    if (pocket && !crouched && !eggPooping) {
        pad->button &= ~BTN_CDOWN;
        if (bHeld) {
            if (sBHeldFrames < kBHoldFrames) {
                sBHeldFrames++;
            }
            if (sBHeldFrames >= kBHoldFrames) {
                pad->button |= BTN_R;
                sBHoldFired = true;
            }
        } else {
            if (sBWasHeld && !sBHoldFired && sBHeldFrames > 0) {
                pad->button |= BTN_CDOWN;
            }
            sBHeldFrames = 0;
            sBHoldFired = false;
        }
    } else {
        sBHeldFrames = 0;
        sBHoldFired = false;
    }
    sBWasHeld = bHeld;

    // Pocket: Talon Trot (Z + R) and tip-toe (R)
    //  Z crouches. While crouched, an R press injects C-Left to start the Talon Trot.
    //  R on its own (no crouch) lightly touches the analog stick for a tip-toe.
    //  Both come off the mapped pad, so rebinding either button moves the gesture.
    static bool sTipToe = false;
    static bool sPrevGesture = false;
    if (pocket) {
        if (pocketCrouch) {
            // Crouching: R is the Talon Trot trigger, not tip-toe.
            if (crouched && pocketGesture && !sPrevGesture) {
                pad->button |= BTN_CLEFT; // C-Left while crouched starts the Talon Trot
            }
            sTipToe = false;
        } else if (CVarGetInteger(CVAR_SETTING("Controls.PocketTipToeHold"), 0) != 0) {
            sTipToe = pocketGesture; // hold mode
        } else if (pocketGesture && !sPrevGesture) {
            sTipToe = !sTipToe; // tap toggles
        }
        sPrevGesture = pocketGesture;

        if (sTipToe) {
            const float kTipToeScale = 0.25f;
            pad->stick_x = static_cast<int8_t>(pad->stick_x * kTipToeScale);
            pad->stick_y = static_cast<int8_t>(pad->stick_y * kTipToeScale);
        }
    } else {
        sTipToe = false;
        sPrevGesture = false;
    }
}
