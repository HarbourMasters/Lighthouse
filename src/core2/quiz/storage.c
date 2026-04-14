#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/quiz_storage.h"

extern struct FF_StorageStruct *ffStorage;

void code_C9E70_defrag(void){
    quizQuestionAskedBitfield_defrag();
    gcquiz_defrag();
    if(ffStorage == NULL)
        return;
    
    if(ffStorage->unk20){
        gczoombox_defrag(ffStorage->unk20);
        ffStorage->unk20 = (GcZoombox *)defrag(ffStorage->unk20);
    }
    ffStorage->unk48 = (struct FF_StorageStruct_48 *)defrag(ffStorage->unk48);
    ffStorage = (struct FF_StorageStruct *) defrag(ffStorage);
}
