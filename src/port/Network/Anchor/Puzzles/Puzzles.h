#pragma once

#include <cstdint>

// Each level's catch-ups to a teammate's puzzle progress, registered with the Anchor hooks.
void AnchorPuzzles_Register(bool connected);
void AnchorPuzzles_RegisterMumbosMountain(bool connected);
void AnchorPuzzles_RegisterTreasureTroveCove(bool connected);
void AnchorPuzzles_RegisterClankersCavern(bool connected);
void AnchorPuzzles_RegisterBubblegloopSwamp(bool connected);
void AnchorPuzzles_RegisterFreezeezyPeak(bool connected);
void AnchorPuzzles_RegisterGobisValley(bool connected);
void AnchorPuzzles_RegisterMadMonsterMansion(bool connected);
void AnchorPuzzles_RegisterRustyBucketBay(bool connected);
void AnchorPuzzles_RegisterClickClockWood(bool connected);
void AnchorPuzzles_MarkNapperTeammateCollect();
void AnchorPuzzles_ReplayJiggyCrane(int32_t stage, int32_t ticks);
void AnchorPuzzles_ResendJiggyCrane();
