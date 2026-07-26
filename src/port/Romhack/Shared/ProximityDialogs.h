#pragma once

struct ProximityDialogPage {
    float anchor[3];
    float radius;
    int textId;
    int dialogFlags;
    unsigned char word;     // which sDialogShown word tracks this page
    unsigned short doneBit; // bit within that word; 0 means "guarded by token instead"
    int token;              // mumbotoken to guard on / bank (-1 = none)
    const float* dialogPos;
    int skipIfJiggy;   // skip if this jiggy is collected (>0)
    int needFlag;      // skip unless this FILEPROG flag is set (>0)
    int needFlagClear; // skip if this FILEPROG flag is set (>0)
};

struct ProximityDialogMap {
    int map;
    const ProximityDialogPage* pages;
    int pageCount;
    // Optional per-map hook run before the page loop. Return false to skip this
    // map's pages this frame; may also perform side effects. nullptr = always run.
    bool (*gate)();
};

// Run one frame of checks against a caller-owned table (used by the storybook).
void ProximityDialogs_Run(const ProximityDialogMap* maps, int count);

// Register a GameFrameUpdate that runs the given world table every frame.
void ProximityDialogs_Enable(const ProximityDialogMap* maps, int count);

// Deduce the count from a fixed table so callers pass just the table.
template <int N> inline void ProximityDialogs_Enable(const ProximityDialogMap (&maps)[N]) {
    ProximityDialogs_Enable(maps, N);
}

// Shown-bit access for gate() callbacks that re-arm a page.
bool ProximityDialogs_IsShown(int word, unsigned bits);
void ProximityDialogs_ClearShown(int word, unsigned bits);
