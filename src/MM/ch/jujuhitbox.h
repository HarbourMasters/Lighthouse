// [port] jujuhitbox.c's typedef, moved here for the Anchor catch-up (Puzzles/MumbosMountain.cpp).
#ifndef BANJO_KAZOOIE_MM_CH_JUJUHITBOX_H
#define BANJO_KAZOOIE_MM_CH_JUJUHITBOX_H

typedef struct juju_hitbox_s {
    u8           pad0[0x4];
    s32          unk4;
    ActorMarker *jujus[4];
    f32          unk18;
} ActorLocal_JujuHitbox;

#endif
