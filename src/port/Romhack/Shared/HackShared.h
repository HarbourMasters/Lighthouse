#pragma once

void HackShared_EnableNoteSignSuppression(int signActorId);
void HackShared_EnableDialogSuppression(const int* dialogIds, int count);

template <int N> inline void HackShared_EnableDialogSuppression(const int (&dialogIds)[N]) {
    HackShared_EnableDialogSuppression(dialogIds, N);
}
