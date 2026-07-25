#pragma once

#include "ProximityDialogs.h"

struct StorybookConfig {
    const ProximityDialogMap* pages; // book pages, one entry per book map
    int pageCount;                   // number of book maps
    int resumeDest;                  // MAP<<8 | ENTRY warp used when resuming a save
    int newGameSeenToken;            // mumbotoken the first book banks (-1 = none)
};

void Storybook_Enable(const StorybookConfig& cfg);
