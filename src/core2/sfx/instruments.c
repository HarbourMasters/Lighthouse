#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include <n_audio/PR/n_libaudio.h>

extern ALBank *music_get_sound_bank(void);
extern OSIoMesg *func_802405D0(void);
extern OSMesgQueue *func_802405C4(void);
extern ALHeap *func_802405B8(void);
extern void func_80243070(void *arg0);

extern u8 *soundfont1ctl_ROM_START;
extern u8 *soundfont1ctl_ROM_END;
extern u8 *soundfont1tbl_ROM_START;

struct {
    s32 unk0; //sound state cnt
    s32 unk4;
    s32 unk8; //maxSounds
    ALHeap *unkC; //heap
    s16 unk10;
} D_803835F0;
ALBank * sfx_sound_bank;

/* .code */
void sfxInstruments_init(void){
    ALBank *bnk;
    s32 size;
    ALInstrument *inst;
    ALBankFile * bnkf;


    // [port] parse BE ctl binary into native 64-bit structs
    extern ALBankFile *port_alBnkfNew(u8 *ctlData, s32 ctlSize, u8 *tblData);
    size = soundfont1ctl_ROM_END - soundfont1ctl_ROM_START;
    bnkf = port_alBnkfNew(soundfont1ctl_ROM_START, size, soundfont1tbl_ROM_START);
    bnk = bnkf->bankArray[0];
    inst = bnk->instArray[0];
    D_803835F0.unk0 = inst->soundCount;
    D_803835F0.unk4 = 0x100;
    D_803835F0.unk10 = 0x40;
    D_803835F0.unk8 = 0x18;
    D_803835F0.unkC = func_802405B8();
    func_80243070(&D_803835F0);
    sfx_sound_bank = bnk;
}

intptr_t func_8033531C(enum sfx_e uid, struct46s *arg1)
{
    intptr_t result = (intptr_t)func_80244608(sfx_sound_bank, (s16) (uid + 1), arg1);
    return result;
}

intptr_t func_80335354(int uid, struct46s *arg1)
{
    ALBank *bank = music_get_sound_bank();
    return (intptr_t)func_80244608(bank, (s16) (uid + 1), arg1);
}

void func_80335394(intptr_t arg0, f32 arg1)
{
    func_80244978(arg0, AL_SEQP_STOP_EVT, reinterpret_cast(s32, arg1));
}

void func_803353BC(intptr_t arg0, u16 arg1)
{
    if(arg1 > 0x7fff)
        arg1 = 0x7fff;
    func_80244978(arg0, AL_SEQP_PROG_EVT, arg1);
}

void func_803353F4(intptr_t arg0, s32 arg1)
{
    func_80244978(arg0, 0x100, arg1);
}

void func_80335418(intptr_t arg0, s32 arg1)
{
    func_80244978(arg0, AL_SEQ_END_EVT, arg1);
}

void func_8033543C(Struct81s *arg0){
    if(arg0 != NULL && func_802445AC(arg0) != 0){
        func_80244814(arg0);
    }
}

bool func_80335470(Struct81s *arg0)
{
    return  func_802445AC(arg0) != 0;
}

u32 func_80335494(Struct81s *arg0)
{
    return func_802445AC(arg0);
}

s32 func_803354B4(void)
{
    return sfx_sound_bank->instArray[0]->soundCount;
}

s32 func_803354C8(void)
{
    ALBank *bank = music_get_sound_bank();
    return bank->instArray[0]->soundCount;
}

bool func_803354EC(enum sfx_e sfx_id)
{
    return func_802445C4(sfx_sound_bank, (s16)(sfx_id + 1));
}

bool func_80335520(s32 arg0)
{
    ALBank *bank = music_get_sound_bank();
    return func_802445C4(bank, (s16)(arg0 + 1));
}
