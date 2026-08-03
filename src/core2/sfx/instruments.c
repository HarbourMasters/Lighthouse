// BanjoDecomp: core2/code_AE290.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include <n_audio/PR/n_libaudio.h>

extern void func_80243070(Struct87s *arg0);

extern u8 *soundfont1ctl_ROM_START;
extern u8 *soundfont1ctl_ROM_END;
extern u8 *soundfont1tbl_ROM_START;

Struct87s D_803835F0;
ALBank *sSfxSoundBank;

void sfxInstruments_init(void) {
    ALBank *bnk;
    s32 size;
    ALInstrument *inst;
    ALBankFile *bnk_f;

    // [port] parse BE ctl binary into native 64-bit structs
    extern ALBankFile *port_alBnkfNew(u8 *ctlData, s32 ctlSize, u8 *tblData);
    size = soundfont1ctl_ROM_END - soundfont1ctl_ROM_START;
    bnk_f = port_alBnkfNew(soundfont1ctl_ROM_START, size, soundfont1tbl_ROM_START);

    bnk = bnk_f->bankArray[0];
    inst = bnk->instArray[0];

    D_803835F0.unk0 = inst->soundCount;
    D_803835F0.unk4 = 0x100;
    D_803835F0.unk10 = 0x40;
    D_803835F0.max_sounds = 24;
    D_803835F0.heap = audioManager_getALHeapInfo();
    func_80243070(&D_803835F0);

    sSfxSoundBank = bnk;
}

intptr_t sfxInstruments_func_8033531C(enum sfx_e sfx_id, struct46s *arg1) {
    return (intptr_t) func_80244608(sSfxSoundBank, (s16) (sfx_id + 1), arg1);
}

intptr_t sfxInstruments_func_80335354(enum sfx_e sfx_id, struct46s *arg1) {
    return (intptr_t) func_80244608(musicInstruments_getSoundBank(), (s16) (sfx_id + 1), arg1);
}

void sfxInstruments_func_80335394(intptr_t arg0, f32 arg1) {
    func_80244978(arg0, AL_SEQP_STOP_EVT, reinterpret_cast(s32, arg1));
}

void sfxInstruments_func_803353BC(intptr_t arg0, u16 arg1) {
    if(arg1 > 0x7fff)
        arg1 = 0x7fff;
    func_80244978(arg0, AL_SEQP_PROG_EVT, arg1);
}

void sfxInstruments_func_803353F4(intptr_t arg0, s32 arg1) {
    func_80244978(arg0, 0x100, arg1);
}

void sfxInstruments_func_80335418(intptr_t arg0, s32 arg1) {
    func_80244978(arg0, AL_SEQ_END_EVT, arg1);
}

void sfxInstruments_func_8033543C(Struct81s *arg0) {
    if(arg0 != NULL && func_802445AC(arg0) != 0){
        func_80244814(arg0);
    }
}

bool sfxInstruments_func_80335470(Struct81s *arg0) {
    return  func_802445AC(arg0) != 0;
}

u32 sfxInstruments_func_80335494(Struct81s *arg0) {
    return func_802445AC(arg0);
}

s32 sfxInstruments_getSoundCount(void) {
    return sSfxSoundBank->instArray[0]->soundCount;
}

s32 sfxInstruments_getMusicSoundBankSoundCount(void) {
    return musicInstruments_getSoundBank()->instArray[0]->soundCount;
}

bool func_803354EC(enum sfx_e sfx_id) {
    return func_802445C4(sSfxSoundBank, (s16) (sfx_id + 1));
}

bool func_80335520(enum sfx_e sfx_id) {
    return func_802445C4(musicInstruments_getSoundBank(), (s16) (sfx_id + 1));
}
