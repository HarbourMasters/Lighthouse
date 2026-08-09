#include "MiscBehavior.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "save.h"

extern "C" {
#include "core2/gc/zoombox.h"

extern s8 gameFile_GameIdToFileIdMap[4];
void func_803152C4(GcZoombox* self);
}

static void FinishPortraitCrossfade(GcZoombox* zoombox) {
    func_803152C4(zoombox);
    zoombox->unk1A4_14 = 0;
    zoombox->unk1A4_13 = 0;
    zoombox->unk17C = 0.0f;
}

void Rando::MiscBehavior::InitFileSelectBehavior() {
    REGISTER_LISTENER(OnFileSelectPortrait, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnFileSelectPortrait* ev = (OnFileSelectPortrait*)event;

        if (ev->zoombox == nullptr || ev->gamenum < 0 || ev->gamenum >= 3) {
            return;
        }

        int32_t fileNum = gameFile_GameIdToFileIdMap[ev->gamenum];
        bool isRando = fileNum >= 0 && fileNum < 4 && gameFile_saveData[fileNum].magic != 0 &&
                       gameFile_saveData[fileNum].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO;

        // Cheato zoombox sprite for Rando files, otherwise Vanilla
        GcZoomboxSprite wanted = isRando ? ZOOMBOX_SPRITE_5B_CHEATO : ZOOMBOX_SPRITE_C_BANJO_2;
        if (gczoombox_loadSprite((GcZoombox*)ev->zoombox, wanted)) {
            FinishPortraitCrossfade((GcZoombox*)ev->zoombox);
        }
    })
}
