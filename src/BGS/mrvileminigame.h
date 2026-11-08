// [port] mrvileminigame.c's typedefs, moved here for the Anchor Mr. Vile sync (VileSync.c).
#ifndef BANJO_KAZOOIE_BGS_MRVILEMINIGAME_H
#define BANJO_KAZOOIE_BGS_MRVILEMINIGAME_H

struct vilegame_piece{
    enum chvilegame_piece_type_e type;
    f32 position[3];
    ActorMarker *marker; //yumblie ptr;
};

typedef struct {
    u8 current_type;
    // u8 pad1[3];
    bk_vector(struct vilegame_piece) *game_pieces;
    BKModelBin *grumblie_model_bin;
    u8 dialogIndex;
    u8 mode;
    u8 player_score;
    u8 vile_score;
    f32 type_change_timer;
    ActorMarker *vile_marker;
}chMrVileMinigameActor;

#endif
