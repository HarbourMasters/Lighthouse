// BanjoDecomp: code_0.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"


extern void anSeq_func_80288F78(bk_vector(AnSeqElement) **, f32, s32);

/* .code */

void SM_func_803863F0(bk_vector(AnSeqElement) **anseq, ActorMarker *marker) {
    anSeq_func_80288F78(anseq, 0.20f, 0xF280FA);
}


void SM_func_80386420(bk_vector(AnSeqElement) **anseq, ActorMarker *marker) {
    anSeq_func_80288F78(anseq, 0.20f, 0x3ED803E);
    anSeq_func_80288F78(anseq, 0.35f, 0x3ED8C3E);
    anSeq_func_80288F78(anseq, 0.71f, 0x21F336);
    anSeq_func_80288F78(anseq, 0.79f, 0x21F336);
    anSeq_func_80288F78(anseq, 0.80f, 0x3ED8C3E);
    anSeq_func_80288F78(anseq, 0.87f, 0x21F336);
}

/* .data */
Struct63s D_8038AAC0[] = {
    {0x20A, SM_func_803863F0},
    0
};

Struct63s D_8038AAD0[] = {
    {0x208, SM_func_80386420},
    0
};
