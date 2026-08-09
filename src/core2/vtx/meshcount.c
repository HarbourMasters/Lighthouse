// BanjoDecomp: core2/code_62FD0.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

s32 meshList_getVtxCount(BKMeshList *meshList){
    s32 i;
    s32 v1 = 0;
    BKMesh *v0 = (BKMesh *)(meshList + 1);

    for(i = 0; i < meshList->count; ++i){
        v1 += v0->vtx_count;
        v0 = (BKMesh *)&((s16*)(v0 +1))[v0->vtx_count];
        
    }
    return v1;
}

BKMesh * meshList_getMesh(BKMeshList *meshList, s32 mesh_id){
    s32 i;
    BKMesh *v1 = (BKMesh *)(meshList + 1);

    for(i=0; i < meshList->count; i++){
        if(v1->uid == mesh_id){
            return v1;
        }
        v1 = (BKMesh *)&((s16*)(v1 +1 ))[v1->vtx_count];
    }
    return NULL;
}

bool meshList_meshContainsVtx(BKMeshList * meshList, s32 mesh_id, void *vtx_id){
    s32 i;
    BKMesh *v0 = meshList_getMesh(meshList, mesh_id);

    if(v0){
        for(i = 0; i < v0->vtx_count; i++){
            if(((s16*)(v0 + 1))[i] == *(s16 *)vtx_id){
                return true;
            }
        }
    }
    return false;
}
