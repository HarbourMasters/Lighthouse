// BanjoDecomp: core2/code_B8080.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"


void model_getMeshCoordRange(BKModel *model, s32 mesh_id, s16 min[3], s16 max[3]);
s32  model_func_8033F3E8(BKModel *model, f32 position[3], s32 min_id, s32 max_id);
/* .code */
//performs operation "fn" for every vtx in every mesh of a model
void model_transformMeshes(BKModel *model, void (*fn)(s32, BKModelVtxRef *, Vtx *, void *), void *arg3) {
    s32 i;
    BKMesh *iMesh;
    BKModelVtxRef *iVtx;
    BKModelVtxRef *start_vtx_ref;
    BKModelVtxRef *end_vtx_ref;
    Vtx *verts;

    verts = vtxList_getVertices(model->vtx_list);
    iMesh = (BKMesh *)(model + 1);
    for(i = 0; i < model->mesh_list->count; i++){
        start_vtx_ref = (BKModelVtxRef *)(iMesh + 1);
        end_vtx_ref = start_vtx_ref + iMesh->vtx_count;
        for(iVtx = start_vtx_ref; iVtx < end_vtx_ref; iVtx++){
            fn(iMesh->uid, iVtx, &verts[iVtx->vtx_id], arg3);
        }
        iMesh =  (BKMesh*) (((BKModelVtxRef *)(iMesh + 1)) + iMesh->vtx_count);
    };
}

//performs operation "fn" for every vtx in a model's mesh
void model_transformMesh(BKModel *model, s32 mesh_id, void (*fn)(s32, BKModelVtxRef *, Vtx *, void *), void *arg3) {
    s32 i;
    BKMesh *iMesh;
    BKModelVtxRef *iVtx;
    BKModelVtxRef *start_vtx_ref;
    BKModelVtxRef *end_vtx_ref;
    Vtx *verts;

    verts = vtxList_getVertices(model->vtx_list);
    iMesh = (BKMesh *)(model + 1);
    for(i = 0; i < model->mesh_list->count; i++){
        if (mesh_id == iMesh->uid) {
            start_vtx_ref = (BKModelVtxRef *)(iMesh + 1);
            end_vtx_ref = start_vtx_ref + iMesh->vtx_count;
            for(iVtx = start_vtx_ref; iVtx < end_vtx_ref; iVtx++){
                fn(iMesh->uid, iVtx, &verts[iVtx->vtx_id], arg3);
            }
            return;
        }
        iMesh =  (BKMesh*) (((BKModelVtxRef *)(iMesh + 1)) + iMesh->vtx_count);
    };
}

void model_getMeshCenter(BKModel *model, s32 mesh_id, s16 arg2[3]) {
    s16 min[3];
    s16 max[3];

    model_getMeshCoordRange(model, mesh_id, min, max);
    arg2[0] = (min[0] + max[0]) / 2;
    arg2[1] = (min[1] + max[1]) / 2;
    arg2[2] = (min[2] + max[2]) / 2;
}


BKMeshList *model_getMeshList(BKModel *arg0){
    return arg0->mesh_list;
}

void model_getMeshCoordRange(BKModel *model, s32 mesh_id, s16 min[3], s16 max[3]) {
    s32 pad2C;
    s32 pad28;
    BKMesh *mesh;
    Vtx *vtx_pool;
    Vtx *i_vtx;
    s16 *mesh_begin;
    s16 *mesh_end;
    s16 *phi_t4;
    s32 i;

    mesh = meshList_getMesh(model->mesh_list, mesh_id);
    vtx_pool = vtxList_getVertices(model->vtx_list);
    if (mesh == NULL) return;
    
    mesh_begin = (s16*)(mesh + 1);
    mesh_end = mesh_begin + (mesh->vtx_count);
    for(phi_t4 = mesh_begin; phi_t4 < mesh_end; phi_t4++){
        i_vtx = &vtx_pool[*phi_t4];
        for(i = 0; i < 3; i++){
            if (phi_t4 == (s16*)(mesh + 1)) {
                min[i] = max[i] = i_vtx->v.ob[i];
            } else {
                min[i] = MIN(i_vtx->v.ob[i], min[i]);
                max[i] = MAX(i_vtx->v.ob[i], max[i]);
            }
        }
    }
}

//return mesh id "position" is over/under
s32 model_func_8033F3C0(BKModel *model, f32 position[3]){
    return model_func_8033F3E8(model, position, 0, 100000);
}

s32 model_func_8033F3E8(BKModel *arg0, f32 position[3], s32 min_id, s32 max_id) {
    int i;
    int j;
    int k;
    s16 min[3];
    s16 max[3];
    s16 position_s16[3];
    s32 temp_v1_3;
    Vtx *vertex_pool;
    BKMesh *current_mesh;
    Vtx *current_vertex;
    s16 *vertex_index_list;

    vertex_pool = vtxList_getVertices(arg0->vtx_list);
    position_s16[0] = (s16) position[0];
    position_s16[1] = (s16) position[1];
    position_s16[2] = (s16) position[2];
    current_mesh = (BKMesh *)(arg0->mesh_list + 1);
    for(k = 0; k < arg0->mesh_list->count; k++, current_mesh = (BKMesh *)(((s16 *)(current_mesh + 1)) + current_mesh->vtx_count)){
        if ((min_id > current_mesh->uid || current_mesh->uid >= max_id))
            continue;

        vertex_index_list = ((s16*)(current_mesh + 1));
        current_vertex = vertex_pool + vertex_index_list[0];
        for(j = 0; j < 3; j++){
            min[j] = max[j] = current_vertex->v.ob[j];
        };

        
        for(j = 1; j < current_mesh->vtx_count; j++){
            current_vertex = vertex_pool + vertex_index_list[j];
            for(i = 0; i < 3; i++){\
                temp_v1_3 = current_vertex->v.ob[i];
                min[i] = MIN(temp_v1_3, min[i]);
                max[i] = MAX(temp_v1_3, max[i]);
            };
        }
        if( (min[0] < position_s16[0] && position_s16[0] < max[0])
            && (min[2] < position_s16[2] && position_s16[2] < max[2])
        ){
            return current_mesh->uid;
        }
         
    }
    return 0;
}

void model_free(BKModel *model){
    bk_free(model);
}

BKModel *meshList_createModel(BKMeshList *meshList, BKVertexList *vertexList) {
    s32 temp_s1;
    BKModel *sp40;
    void *temp_v0;
    BKMesh *phi_s3;
    BKMesh *phi_s5;
    BKModelVtxRef *phi_s0;
    Vtx *new_var;
    s32 phi_s1;
    s32 phi_s6;

    sp40 = (BKModel *)bk_malloc((meshList_getVtxCount(meshList) * sizeof(BKModelVtxRef)) + (meshList->count * sizeof(BKMesh)) + sizeof(BKModel));
    sp40->mesh_list = meshList;
    sp40->vtx_list = vertexList;
    phi_s3 = (BKMesh *)(meshList + 1);
    phi_s5 = (BKMesh *)(sp40 + 1);
    for(phi_s6 = 0; phi_s6 < meshList->count; phi_s6++){
            phi_s5->uid = (s16) phi_s3->uid;
            phi_s5->vtx_count = (s16) phi_s3->vtx_count;
            phi_s0 = ((BKModelVtxRef *)(phi_s5 + 1));
            for(phi_s1 = 0; phi_s1 < phi_s3->vtx_count; phi_s1++){
                phi_s0->vtx_id = ((s16 *)(phi_s3 + 1))[phi_s1];
                memcpy(phi_s0, ((Vtx *)(vertexList + 1)) + phi_s0->vtx_id, sizeof(Vtx));
                phi_s0++;
            }
            phi_s3 = (BKMesh *)((s16 *)(phi_s3 + 1) + phi_s3->vtx_count);
            phi_s5 = (BKMesh *)((BKModelVtxRef *)(phi_s5 + 1) + phi_s5->vtx_count);
    }
    return sp40;
}

void func_8033F738(ActorMarker *arg0) {
    BKModelBin *sp1C;
    BKMeshList *sp18;

    sp1C = marker_loadModelBin(arg0);
    sp18 = (BKMeshList *)modelbin_getMeshList(sp1C);
    arg0->unk48 = meshList_createModel(sp18, modelbin_getVtxList(sp1C));
}


void func_8033F784(ActorMarker *arg0){
    model_free(arg0->unk48);
}

void func_8033F7A4(ActorMarker *arg0, BKVertexList *arg1) {
    arg0->unk48->mesh_list = (BKMeshList *)modelbin_getMeshList(func_80330DE4(arg0));
    arg0->unk48->vtx_list  = arg1;
}

void func_8033F7E8(s32 arg0){}
