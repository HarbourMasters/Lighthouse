// BanjoDecomp: lair/jigsawpicture.h
// [port] jigsawpicture.c's typedef, moved here for port code, and a declaration of PICTURE_INFO.
typedef struct {
    u8 cost;
    u8 size_bits;
    u16 progress_flag; // enum file_progress_e
}Struct_lair_86F0_0;

extern Struct_lair_86F0_0 PICTURE_INFO[0xb];

typedef enum {
    JIGSAW_PICTURE_LEAVE_PODIUM = 1,
    JIGSAW_PICTURE_ENTER_PODIUM = 2,
    JIGSAW_PICTURE_INSTRUCTIONS = 3,
    JIGSAW_PICTURE_WAITING = 4,
    JIGSAW_PICTURE_ADD_PIECE = 5,
    JIGSAW_PICTURE_ADD_ALL = 6,
    JIGSAW_PICTURE_COMPLETE = 7,
    JIGSAW_PICTURE_REMOVE_PIECE = 8
} JigsawPictureState;
