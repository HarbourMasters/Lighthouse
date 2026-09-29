// BanjoDecomp: core2/code_5FD90.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "bk_math.h"
#include "model.h"

#include "port/ShipUtils.h"

extern f32  vtxList_getGlobalNorm(BKVertexList *);

#define COLLISION_GEO_SCRATCH_MAX 100

// [port] A collision triangle indexing outside its own model's vertex pool means the
// two lists disagree.
static s32 port_collisionTriValid(BKCollisionTriangle *tri, BKVertexList *vtx_list) {
    static s32 reported = 0;
    u32 count = (u32)(u16)vtx_list->count;
    if ((u16)tri->unk0[0] < count && (u16)tri->unk0[1] < count && (u16)tri->unk0[2] < count) {
        return TRUE;
    }
    if (reported < 8) {
        reported++;
        BK_LOG_DEBUG("[collision] map 0x%02X: triangle indexes %d/%d/%d outside vertex pool of %d - skipped",
                     gsworld_getMap(), tri->unk0[0], tri->unk0[1], tri->unk0[2], (s32)count);
    }
    return FALSE;
}

// [port] Fires per collision query when the geometry scratch list fills, so it is
// rate-limited even at debug level.
static void port_reportCollisionScratchFull(void) {
    static s32 reported = 0;
    if (reported < 8) {
        reported++;
        BK_LOG_DEBUG("[collision] geometry scratch list full (%d cells) - query truncated",
                     COLLISION_GEO_SCRATCH_MAX);
    }
}

#define ABS_F(s) (((s) >= 0.0f) ? (s) : -(s))

 // This is filled but never used
struct { 
//  BKCollisionGeometry *list[100];
    BKCollisionGeometry *list[COLLISION_GEO_SCRATCH_MAX];
    BKCollisionGeometry **current;
} sActiveCollisionLists;

f32 sStoredTriangle[3][3]; // This is calculated but never used
BKProcessedCollisionTriangle sActiveCollTris[100];

void collisionList_setStoredTriangle(BKCollisionTriangle *triangle, BKVertexList *bk_vtx_list) {
    Vtx *vtx_start, *i_vtx;
    int i;

    vtx_start = bk_vtx_list->vertices;
    
    if (!triangle) {
        return;
    }

    if (!port_collisionTriValid(triangle, bk_vtx_list)) {
        return;
    }

    for (i = 0; i < 3; i++) {
        i_vtx = vtx_start + triangle->unk0[i];
        TUPLE_COPY(sStoredTriangle[i], i_vtx->v.ob);
    }
}

void collisionList_getIntersecting_f32(BKCollisionList *this, f32 min[3], f32 max[3], BKCollisionGeometry ***begin_ptr, BKCollisionGeometry ***end_ptr) {
    s32 min_scaled[3], max_scaled[3];
    int i;
    s32 x, y, z;

    if (this->scale == 0) {
        sActiveCollisionLists.current = sActiveCollisionLists.list;
        *sActiveCollisionLists.current++ = (BKCollisionGeometry *) this->data;

        *begin_ptr = sActiveCollisionLists.list;
        *end_ptr = sActiveCollisionLists.current;
    } else {
        for (i = 0 ; i < 3; i++) {
            min_scaled[i] = min[i] >= 0.0f ? (s32) min[i] / this->scale : (s32) min[i] / this->scale - 1;
            max_scaled[i] = max[i] >= 0.0f ? (s32) max[i] / this->scale : (s32) max[i] / this->scale - 1;

            if (min_scaled[i] < this->min[i]) {
                min_scaled[i] = this->min[i];
            }

            if (this->max[i] < min_scaled[i]) {
                min_scaled[i] = this->max[i];
            }

            if (max_scaled[i] < this->min[i]) {
                max_scaled[i] = this->min[i];
            }

            if (this->max[i] < max_scaled[i]) {
                max_scaled[i] = this->max[i];
            }

            min_scaled[i] -= this->min[i];
            max_scaled[i] -= this->min[i];
        }

        sActiveCollisionLists.current = sActiveCollisionLists.list;

        for (z = min_scaled[2]; z <= max_scaled[2]; z++) {
            for (y = min_scaled[1]; y <= max_scaled[1]; y++) {
                for (x = min_scaled[0]; x <= max_scaled[0]; x++) {
                    if (sActiveCollisionLists.current >= &sActiveCollisionLists.list[COLLISION_GEO_SCRATCH_MAX]) {
                        port_reportCollisionScratchFull();
                        goto scratch_full;
                    }
                    *sActiveCollisionLists.current++ = this->z_stride * z + (BKCollisionGeometry *) this->data + x + y * this->y_stride;
                }
            }
        }
    scratch_full:

        *begin_ptr = sActiveCollisionLists.list;
        *end_ptr = sActiveCollisionLists.current;
    }
}

void collisionList_getIntersecting_s32(BKCollisionList *this, s32 min[3], s32 max[3], BKCollisionGeometry ***begin_ptr, BKCollisionGeometry ***end_ptr) {
    s32 min_scaled[3], max_scaled[3];
    int i;
    s32 x, y, z;

    if (this->scale == 0) {
        sActiveCollisionLists.current = sActiveCollisionLists.list;
        *sActiveCollisionLists.current++ = (BKCollisionGeometry *) this->data;

        *begin_ptr = sActiveCollisionLists.list;
        *end_ptr = sActiveCollisionLists.current;
    } else {
        for (i = 0; i < 3; i++) {
            min_scaled[i] = min[i] >= 0 ? min[i] / this->scale : min[i] / this->scale - 1;
            max_scaled[i] = max[i] >= 0 ? max[i] / this->scale : max[i] / this->scale - 1;

            if (min_scaled[i] < this->min[i]) {
                min_scaled[i] = this->min[i];
            }

            if (this->max[i] < min_scaled[i]) {
                min_scaled[i] = this->max[i];
            }

            if (max_scaled[i] < this->min[i]) {
                max_scaled[i] = this->min[i];
            }

            if (this->max[i] < max_scaled[i]) {
                max_scaled[i] = this->max[i];
            }

            min_scaled[i] -= this->min[i];
            max_scaled[i] -= this->min[i];
        }

        sActiveCollisionLists.current = sActiveCollisionLists.list;

        for (z = min_scaled[2]; z <= max_scaled[2]; z++) {
            for (y = min_scaled[1]; y <= max_scaled[1]; y++) {
                for (x = min_scaled[0]; x <= max_scaled[0]; x++) {
                    if (sActiveCollisionLists.current >= &sActiveCollisionLists.list[COLLISION_GEO_SCRATCH_MAX]) {
                        port_reportCollisionScratchFull();
                        goto scratch_full;
                    }
                    *sActiveCollisionLists.current++ = this->z_stride * z + (BKCollisionGeometry *) this->data + x + y * this->y_stride;
                }
            }
        }
    scratch_full:

        *begin_ptr = sActiveCollisionLists.list;
        *end_ptr = sActiveCollisionLists.current;
    }
}

void collisionList_getStoredTriangle(f32 tri[3][3]) {
    int i;

    for (i = 0; i < 3; i++) {
        TUPLE_COPY(tri[i], sStoredTriangle[i]);
    }
}

s32 collisionList_getTriCount_0x1E0000(BKCollisionList *this) {
    BKCollisionTriangle *tri_start, *tri_end, *i_tri;
    s32 count;

    count = 0;
    tri_start = (BKCollisionTriangle *) (this->geo_count + (BKCollisionGeometry *) this->data);
    tri_end = tri_start + this->tri_count;

    for (i_tri = tri_start; i_tri < tri_end; i_tri++) {
        if (i_tri->flags & 0x1E0000) {
            count++;
        }
    }

    return count;
}

s32 collisionList_getTriCount(BKCollisionList *this) {
    return this->tri_count;
}

void collisionList_getTris(BKCollisionList *this, BKCollisionTriangle **begin_ptr, BKCollisionTriangle **end_ptr) {
    *begin_ptr = (BKCollisionTriangle *) (this->geo_count + (BKCollisionGeometry *) this->data);
    *end_ptr = *begin_ptr + this->tri_count;
}

bool collisionList_pointOnLine(f32 point[3], f32 margin, f32 line_start[3], f32 line_end[3]) {
    f32 sp24[3];
    f32 sp20;

    if (margin <= ml_vec3f_distance_to_point(line_start, line_end, point)) {
        return FALSE;
    }

    sp24[0] = (line_start[0] + line_end[0]) / 2;
    sp24[1] = (line_start[1] + line_end[1]) / 2;
    sp24[2] = (line_start[2] + line_end[2]) / 2;

    sp20 = ml_vec3f_distance(sp24, point);

    if ((ml_vec3f_distance(sp24, line_start) + margin) <= sp20) {
        return FALSE;
    }

    return TRUE;
}

bool collisionList_spheresIntersect(f32 pos1[3], f32 radius1, f32 pos2[3], f32 radius2) {
    return ml_vec3f_distance(pos1, pos2) < radius1 + radius2;
}

void collisionList_calculateBoundsAndDirection(f32 start_point[3], f32 end_point[3], s32 min_bounds[3], s32 max_bounds[3], f32 direction_vector[3]) {
    int i;

    for (i = 0; i < 3; i++) {
        if (start_point[i] < end_point[i]) {
            min_bounds[i] = (s32) start_point[i];
            max_bounds[i] = (s32) end_point[i];
        } else {
            min_bounds[i] = (s32) end_point[i];
            max_bounds[i] = (s32) start_point[i];
        }

        min_bounds[i] -= 1;
        max_bounds[i] += 1;
    }

    TUPLE_DIFF_COPY(direction_vector, end_point, start_point);
}

BKCollisionTriangle *collisionList_intersectLine(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 line_start[3], f32 line_end[3], f32 normal[3], u32 flag_filter) {
    int i, j;
    BKCollisionGeometry **start_geo, **i_geo, **end_geo, *temp_v1;
    BKCollisionTriangle *start_tri, *end_tri, *i_tri;
    Vtx *vtx_pool, *tri_vtx[3];
    s32 min_bounds[3], max_bounds[3];
    f32 direction_vector[3];
    Vtx *i_vtx;
    f32 tri_edge_ab[3], tri_edge_ac[3];
    f32 local_line_start[3], local_line_end[3];
    f32 temp_f0_2;
    f32 line_current_pos[3];
    f32 spF0[3];
    f32 spE4[3];

    f32 temp_f12_2;
    f32 temp_f12_3;
    f32 temp_f12_4;
    f32 temp_f14_2;
    f32 vtx_radius;
    f32 temp_f2_2;
    f32 temp_f2_6;
    f32 tri_normal[3];

    s32 primary_axis;
    f32 pad;
    f32 tri[3][3];
    BKCollisionTriangle *result_collision;

    result_collision = NULL;
    vtx_radius = bk_vtx_list->global_norm;
    collisionList_calculateBoundsAndDirection(line_start, line_end, min_bounds, max_bounds, direction_vector);

    // Do bounding sphere of model and bounding box of line intersect?
    for(i = 0; i < 3; i++) {
        if ((max_bounds[i] <= -vtx_radius) || (vtx_radius <= min_bounds[i])) {
            return NULL;
        }
    }

    // Filter collision geometry entities that lies inside the bounding box
    collisionList_getIntersecting_s32(this, min_bounds, max_bounds, &start_geo, &end_geo);

    // Iterate over collision geometry entities
    for (i_geo = start_geo; i_geo < end_geo; i_geo++) {
        start_tri = (BKCollisionTriangle *) ((BKCollisionGeometry *) this->data + this->geo_count) + (*i_geo)->start_tri_index;
        end_tri = start_tri + (*i_geo)->tri_count;

        // Iterate over every triangle
        for (i_tri = start_tri; i_tri < end_tri; i_tri++) {
            vtx_pool = bk_vtx_list->vertices;

            // Is current triangle filtered out?
            if ((i_tri->flags & flag_filter)) {
                continue;
            }

            if (!port_collisionTriValid(i_tri, bk_vtx_list)) {
                continue;
            }

            tri_vtx[0] = &vtx_pool[i_tri->unk0[0]]; 
            tri_vtx[1] = &vtx_pool[i_tri->unk0[1]]; 
            tri_vtx[2] = &vtx_pool[i_tri->unk0[2]];

            // Is current triangle outside bounding box?
            if ((tri_vtx[0]->v.ob[0] < min_bounds[0]) && (tri_vtx[1]->v.ob[0] < min_bounds[0]) && (tri_vtx[2]->v.ob[0] < min_bounds[0])) continue;
            if ((tri_vtx[0]->v.ob[0] > max_bounds[0]) && (tri_vtx[1]->v.ob[0] > max_bounds[0]) && (tri_vtx[2]->v.ob[0] > max_bounds[0])) continue;
            if ((tri_vtx[0]->v.ob[2] < min_bounds[2]) && (tri_vtx[1]->v.ob[2] < min_bounds[2]) && (tri_vtx[2]->v.ob[2] < min_bounds[2])) continue;
            if ((tri_vtx[0]->v.ob[2] > max_bounds[2]) && (tri_vtx[1]->v.ob[2] > max_bounds[2]) && (tri_vtx[2]->v.ob[2] > max_bounds[2])) continue;
            if ((tri_vtx[0]->v.ob[1] < min_bounds[1]) && (tri_vtx[1]->v.ob[1] < min_bounds[1]) && (tri_vtx[2]->v.ob[1] < min_bounds[1])) continue;
            if ((tri_vtx[0]->v.ob[1] > max_bounds[1]) && (tri_vtx[1]->v.ob[1] > max_bounds[1]) && (tri_vtx[2]->v.ob[1] > max_bounds[1])) continue;

            // Copy triangle vertex positions
            for (i = 0; i < 3; i++) {
                i_vtx = &vtx_pool[i_tri->unk0[i]];
                for (j = 0; j <3; j++) {
                    tri[i][j] = i_vtx->v.ob[j];
                }
            }

            // Calculate triangle normal (length is not normalized)
            TUPLE_DIFF_COPY(tri_edge_ab, tri[1], tri[0]);
            TUPLE_DIFF_COPY(tri_edge_ac, tri[2], tri[0]);
            TUPLE_CROSS_PRODUCT(tri_normal, tri_edge_ab, tri_edge_ac);

            // Scale normal if too big
            if ((100000.0f < tri_normal[0]) || (100000.0f < tri_normal[1]) || (100000.0f < tri_normal[2]) ||
                (tri_normal[0] < -100000.0f) || (tri_normal[1] < -100000.0f) || (tri_normal[2] < -100000.0f)
            ) {
                TUPLE_SCALE_INVERSE(tri_normal, 100000.0f);
            }

            // Bring line coordinates in triangle space
            TUPLE_DIFF_COPY(local_line_start, line_start, tri[0]);
            TUPLE_DIFF_COPY(local_line_end, line_end, tri[0]);

            temp_f12_2 = TUPLE_DOT_PRODUCT(local_line_start, tri_normal);
            temp_f2_2 = TUPLE_DOT_PRODUCT(local_line_end, tri_normal);
            pad = temp_f2_2;

            if ((!((temp_f12_2 >= 0.0f) && (pad >= 0.0f))) && !((temp_f12_2 <= 0.0f) && (pad <= 0.0f))) {
                if ((i_tri->flags & 0x10000) && (temp_f12_2 < 0.0f)) {
                    TUPLE_COPY(tri_normal, -tri_normal);
                }

                temp_f12_3 = TUPLE_DOT_PRODUCT(tri_normal, direction_vector);
                pad = TUPLE_DOT_PRODUCT(tri[0], tri_normal);
                if (temp_f12_3 == 0.0f) {
                    continue;
                }

                temp_f0_2 = -(TUPLE_DOT_PRODUCT(tri_normal, line_start) - pad) / temp_f12_3;
                if ((temp_f0_2 <= 0.0f) || (temp_f0_2 >= 1.0f)) {
                    continue;
                }

                TUPLE_ADD_COPY(line_current_pos, line_start, temp_f0_2 * direction_vector);

                primary_axis = (ABS_F(tri_normal[0]) > ABS_F(tri_normal[1])) ? 0 : 1;
                primary_axis = (ABS_F(tri_normal[2]) > ABS_F(tri_normal[primary_axis])) ? 2 : primary_axis;

                spF0[0] = line_current_pos[(primary_axis + 1) % 3] - tri[0][(primary_axis + 1) % 3];
                spF0[1] = tri_edge_ab[(primary_axis + 1) % 3];
                spF0[2] = tri_edge_ac[(primary_axis + 1) % 3];

                spE4[0] = line_current_pos[(primary_axis + 2) % 3] - tri[0][(primary_axis + 2) % 3];
                spE4[1] = tri_edge_ab[(primary_axis + 2) % 3];
                spE4[2] = tri_edge_ac[(primary_axis + 2) % 3];

                temp_f14_2 = ((spF0[1] * spE4[2]) - (spE4[1] * spF0[2]));

                temp_f12_4 = ((spF0[0] * spE4[2]) - (spE4[0] * spF0[2])) / temp_f14_2;
                if(temp_f12_4 < 0.0f || 1.0f < temp_f12_4) 
                    continue;

                temp_f2_6 =  ((spF0[1] * spE4[0]) - (spE4[1] * spF0[0])) / temp_f14_2;

                if ((temp_f2_6 < 0.0f) || (temp_f2_6 > 1.0f)) {
                    continue;
                }

                if (temp_f12_4 + temp_f2_6 > 1.0f) {
                    continue;
                }

                result_collision = i_tri;
                TUPLE_COPY(line_end, line_current_pos);
                ml_vec3f_normalize_copy(normal, tri_normal);
                collisionList_calculateBoundsAndDirection(line_start, line_end, min_bounds, max_bounds, direction_vector);
            }
        }
    }

    collisionList_setStoredTriangle(result_collision, bk_vtx_list);

    return result_collision;
}

BKCollisionTriangle *collisionList_intersectLineGlobal(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 position[3], f32 rotation[3], f32 scale, f32 line_start[3], f32 line_end[3], f32 normal[3], u32 flag_filter) {
    f32 object_line_start[3];
    f32 object_line_end[3];
    BKCollisionTriangle *coll_tri;
    int i;

    // Is object near the given line? If not, return NULL.
    if (!collisionList_pointOnLine(position, bk_vtx_list->global_norm * scale, line_start, line_end)) {
        return NULL;
    }

    // Transform line_start and line_end from world to object space
    mlMtxIdent();
    func_80252CC4(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(object_line_start, line_start);
    mlMtx_apply_vec3f(object_line_end, line_end);

    // Check collision for every triangle
    coll_tri = collisionList_intersectLine(this, bk_vtx_list, object_line_start, object_line_end, normal, flag_filter);
    if (!coll_tri) {
        return NULL;
    }

    // Transform modified line_end from object space to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(line_end, object_line_end);

    // Transform normal from object to world space
    mlMtxIdent();
    func_80252C08(NULL, rotation, 1.0f, NULL);
    mlMtx_apply_vec3f(normal, normal);

    // Transform sStoredTriangle from object to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    for (i = 0; i < 3; i++) {
        mlMtx_apply_vec3f(sStoredTriangle[i], sStoredTriangle[i]);
    }

    return coll_tri;
}

bool collisionList_intersectMovingSphere_FastCheck(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 p1[3], f32 p2[3], f32 velocity[3], f32 margin, u32 flag_filter, BKProcessedCollisionTriangle **activeTriStartPtr, BKProcessedCollisionTriangle **activeTriEndPtr) {
    BKCollisionGeometry **start_geo, **i_geo, **end_geo;
    f32 spC0[3];
    BKCollisionTriangle *start_tri;
    BKCollisionTriangle *end_tri;
    f32 min_bounds[3], max_bounds[3];
    BKCollisionTriangle *i_tri;
    Vtx *vertex_pool;
    Vtx *i_vtx;
    int i, j;
    f32 tri_min_coord[3];
    f32 tri_max_coord[3];
    BKProcessedCollisionTriangle *cur_processed_tri;
    f32 vtx_radius;

    core2_B6640_calculateLineBoundingBox(p1, p2, margin, min_bounds, max_bounds);
    vtx_radius = vtxList_getGlobalNorm(bk_vtx_list);

    for (i = 0; i < 3; i++) {
        if ((max_bounds[i] <= -vtx_radius) || (min_bounds[i] >= vtx_radius)) {
//          return NULL;
            return FALSE;
        }
    }

    collisionList_getIntersecting_f32(this, min_bounds, max_bounds, &start_geo, &end_geo);
    vertex_pool = vtxList_getVertices(bk_vtx_list);
    cur_processed_tri = sActiveCollTris;

    for (i_geo = start_geo; i_geo < end_geo; i_geo++) {
        start_tri = (BKCollisionTriangle *) ((BKCollisionGeometry *) this->data + this->geo_count) + (*i_geo)->start_tri_index;
        end_tri = start_tri + (*i_geo)->tri_count;

        for (i_tri = start_tri; i_tri < end_tri; i_tri++) {
            if (!(i_tri->flags & flag_filter)) {
                if (!port_collisionTriValid(i_tri, bk_vtx_list)) {
                    continue;
                }
                for (i = 0; i < 3; i++) {
                    i_vtx = vertex_pool + i_tri->unk0[i];
                    for (j = 0; j < 3; j++) {
                        cur_processed_tri->tri_coord[i][j] = i_vtx->v.ob[j];
                    }
                }

                //get min/max bounds of triangle
                TUPLE_COPY(tri_min_coord, cur_processed_tri->tri_coord[0]);
                TUPLE_COPY(tri_max_coord, cur_processed_tri->tri_coord[0]);

                for (i = 1; i < 3; i++) {
                    for (j = 0; j < 3; j++) {
                        if (tri_min_coord[j] > cur_processed_tri->tri_coord[i][j]) {
                            tri_min_coord[j] = cur_processed_tri->tri_coord[i][j];
                        }

                        if (tri_max_coord[j] < cur_processed_tri->tri_coord[i][j]) {
                            tri_max_coord[j] = cur_processed_tri->tri_coord[i][j];
                        }
                    }
                }

                //check if tri intersects region
                if ((tri_min_coord[0] > max_bounds[0]) || (tri_max_coord[0] < min_bounds[0])) continue;
                if ((tri_min_coord[1] > max_bounds[1]) || (tri_max_coord[1] < min_bounds[1])) continue;
                if ((tri_min_coord[2] > max_bounds[2]) || (tri_max_coord[2] < min_bounds[2])) continue;

                //calculate normal of tri
                TUPLE_DIFF_COPY(cur_processed_tri->edgeAB, cur_processed_tri->tri_coord[1], cur_processed_tri->tri_coord[0]);
                TUPLE_DIFF_COPY(cur_processed_tri->edgeAC, cur_processed_tri->tri_coord[2], cur_processed_tri->tri_coord[0]);
                TUPLE_CROSS_PRODUCT(cur_processed_tri->normal, cur_processed_tri->edgeAB, cur_processed_tri->edgeAC);
                ml_vec3f_normalize(cur_processed_tri->normal);

                if (i_tri->flags & 0x10000) { //invert normal?
                    TUPLE_DIFF_COPY(spC0, p1, cur_processed_tri->tri_coord[0]);
                    if (TUPLE_DOT_PRODUCT(spC0, cur_processed_tri->normal) < 0.0f) {
                        TUPLE_COPY(cur_processed_tri->normal, -cur_processed_tri->normal);
                    }
                } else {
                    if (TUPLE_DOT_PRODUCT(cur_processed_tri->normal, velocity) > 0.0f) {
                        continue; //velocity is same direction as triangle normal
                    }
                }

                //add tri to active tri collisions
                (cur_processed_tri++)->tri_ptr = i_tri;
//              if (cur_processed_tri - sActiveCollTris > 100) {
                if (cur_processed_tri - sActiveCollTris >= 100) {
                    break;      
                } 
            }
        }

//      if (cur_processed_tri - sActiveCollTris > 100) {
        if (cur_processed_tri - sActiveCollTris >= 100) {
            break;
        }
    }

    *activeTriStartPtr = sActiveCollTris;
    *activeTriEndPtr = cur_processed_tri;

    return cur_processed_tri - sActiveCollTris > 0;
}

BKProcessedCollisionTriangle *collisionList_intersectMovingSphere_Iteration(BKProcessedCollisionTriangle *startTri, BKProcessedCollisionTriangle *endTri, f32 position[3], f32 radius, f32 normal[3]) {
    int i;
    BKProcessedCollisionTriangle *i_tri;
    
    f32 sp144[3][3];
    f32 sp120[3][3];
    
    f32 normal_length;
    f32 temp_f0_3;
    f32 temp_f0_4;

    f32 temp_f12_3;
    f32 theta;
    f32 temp_f22;
    f32 spFC[3];
    f32 spF0[3];
    u32 var_a0;
    s32 var_a1;
    s32 primary_axis;
    f32 projected_position[3];
    f32 temp_f2_4;
    BKProcessedCollisionTriangle *coll_tri;
    f32 temp_f8;
    f32 *var_s0;

    coll_tri = NULL;
    TUPLE_SET(normal, 0.0f);

    for (i_tri = startTri; i_tri < endTri; i_tri++) {
        // Project point onto plane of triangle
        TUPLE_DIFF_COPY(sp120[0], position, i_tri->tri_coord[0]);
        theta = TUPLE_DOT_PRODUCT(sp120[0], i_tri->normal);

        if ((-(radius - 0.5) >= theta) || (theta >= (radius - 0.5))) {
            continue;
        }

        temp_f8 = i_tri->normal[0];
        normal_length = (temp_f8 * i_tri->normal[0]) + (i_tri->normal[1] * i_tri->normal[1]) + (i_tri->normal[2] * i_tri->normal[2]);
        if (normal_length == 0.0f) {
            continue;
        }
        
        normal_length = -theta / normal_length;
        TUPLE_ADD_COPY(projected_position, position, normal_length * i_tri->normal);
        
        // Check if projected point is inside of triangle
        primary_axis = (ABS_F(i_tri->normal[0]) > ABS_F(i_tri->normal[1])) ? 0 : 1;
        primary_axis = (ABS_F(i_tri->normal[2]) > ABS_F(i_tri->normal[primary_axis])) ? 2 : primary_axis;

        spFC[0] = projected_position[(primary_axis + 1) % 3] - i_tri->tri_coord[0][(primary_axis + 1) % 3];
        spFC[1] = i_tri->edgeAB[(primary_axis + 1) % 3];
        spFC[2] = i_tri->edgeAC[(primary_axis + 1) % 3];

        spF0[0] = projected_position[(primary_axis + 2) % 3] - i_tri->tri_coord[0][(primary_axis + 2) % 3];
        spF0[1] = i_tri->edgeAB[(primary_axis + 2) % 3];
        spF0[2] = i_tri->edgeAC[(primary_axis + 2) % 3];

        temp_f2_4 = (spFC[1] * spF0[2]) - (spF0[1] * spFC[2]);
        temp_f12_3 = ((spFC[0] * spF0[2]) - (spF0[0] * spFC[2])) / temp_f2_4;
        temp_f0_4 = ((spFC[1] * spF0[0]) - (spF0[1] * spFC[0])) / temp_f2_4;

        // Projected point lies on triangle?
        if ((0.0f <= temp_f12_3) && (temp_f12_3 <= 1.0f) && (0.0f <= temp_f0_4) && (temp_f0_4 <= 1.0f) && (0 <= 1.0f - (temp_f12_3 + temp_f0_4))) {
            coll_tri = i_tri;
            TUPLE_ADD_COPY(normal, normal, i_tri->normal);
            continue;
        }

        // Projected point lies outside triangle?
        for (i = 0; i < 3; i++) {
            TUPLE_DIFF_COPY(sp120[i], position, i_tri->tri_coord[i]);
            if (LENGTH_SQ_VEC3F(sp120[i]) < SQ(radius)) {
                coll_tri = i_tri;
                TUPLE_ADD_COPY(normal, normal, i_tri->normal);
                break;
            }
        }

        if (i < 3) {
            continue;
        }

        for (i = 0; i < 3; i++) {
            TUPLE_DIFF_COPY(sp144[i], i_tri->tri_coord[(i + 1) % 3], i_tri->tri_coord[i]);
            temp_f22 = LENGTH_SQ_VEC3F(sp144[i]);
            ml_vec3f_normalize(sp144[i]);
            temp_f0_3 = TUPLE_DOT_PRODUCT(sp144[i], sp120[i]);
            if ((0.0f <= temp_f0_3) && (SQ(temp_f0_3) <= temp_f22)) {
                TUPLE_DIFF_COPY(projected_position, sp120[i], temp_f0_3 * sp144[i]);
                if (LENGTH_SQ_VEC3F(projected_position) < SQ(radius)) {
                    coll_tri = i_tri;
                    TUPLE_ADD_COPY(normal, normal, i_tri->normal);
                    break;
                }
            } 
        }
    }

    return coll_tri;
}

BKCollisionTriangle *collisionList_intersectMovingSphere(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 p1[3], f32 p2[3], f32 sphere_radius, f32 normal[3], s32 steps, u32 flag_filter) {
    BKProcessedCollisionTriangle *start_ptri, *end_ptri;
    f32 distance[3];
    f32 current, start, end;
    int i;
    f32 current_pos[3];
    f32 temp_normal[3];
    BKProcessedCollisionTriangle *coll_tri, *cur_coll_tri;
    f32 velocity[3];

    TUPLE_DIFF_COPY(velocity, p2, p1);

    if (!collisionList_intersectMovingSphere_FastCheck(this, bk_vtx_list, p1, p2, velocity, sphere_radius + 0.5, flag_filter, &start_ptri, &end_ptri)) {
        return NULL;
    }

    coll_tri = collisionList_intersectMovingSphere_Iteration(start_ptri, end_ptri, p2, sphere_radius, temp_normal);
    if (!coll_tri) {
        return NULL;
    }

    TUPLE_COPY(normal, temp_normal);
    TUPLE_DIFF_COPY(distance, p2, p1);
    TUPLE_COPY(p2, p1);
    start = 0.0f;
    end = 1.0f;

    for (i = 0; i < steps; i++) {
        current = (start + end) * 0.5;
        TUPLE_ADD_COPY(current_pos, p1, current * distance);
        cur_coll_tri = collisionList_intersectMovingSphere_Iteration(start_ptri, end_ptri, current_pos, sphere_radius, temp_normal);
        if (cur_coll_tri) {
            TUPLE_COPY(normal, temp_normal);
            end = current;
            coll_tri = cur_coll_tri;
        } else {
            TUPLE_COPY(p2, current_pos);
            start = current;
        }
    }

    if (!coll_tri) {
        return NULL;
    }

    ml_vec3f_normalize(normal);
    collisionList_setStoredTriangle(coll_tri->tri_ptr, bk_vtx_list);

    return coll_tri->tri_ptr;
}

BKCollisionTriangle *collisionList_intersectMovingSphereGlobal(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 position[3], f32 rotation[3], f32 scale, f32 p1[3], f32 p2[3], f32 sphere_radius, f32 normal[3], s32 steps, u32 flag_filter) {
    f32 object_p1[3], object_p2[3];
    BKCollisionTriangle *coll_tri;
    int i;

    // Do bounding spheres intersect?
    if ((bk_vtx_list->global_norm * scale) <= (ml_vec3f_distance(p2, position) - sphere_radius)) {
        return NULL;
    }

    // Transform sphere center from world to object space
    mlMtxIdent();
    func_80252CC4(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(object_p1, p1);
    mlMtx_apply_vec3f(object_p2, p2);

    // Check collision for every triangle
    coll_tri = collisionList_intersectMovingSphere(this, bk_vtx_list, object_p1, object_p2, sphere_radius / scale, normal, steps, flag_filter);
    if (!coll_tri) {
        return NULL;
    }

    // Transform modified sphere center from object space to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(p2, object_p2);

    // Transform normal from object to world space
    mlMtxIdent();
    func_80252C08(NULL, rotation, 1.0f, NULL);
    mlMtx_apply_vec3f(normal, normal);

    // Transform sStoredTriangle from object to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    for (i = 0; i < 3; i++) {
        mlMtx_apply_vec3f(sStoredTriangle[i], sStoredTriangle[i]);
    }

    return coll_tri;
}

BKCollisionTriangle *collisionList_intersectSphere(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 position[3], f32 radius, f32 normal[3], u32 flag_filter) {
    BKCollisionGeometry **start_geo, **i_geo, **end_geo;
    int i, j;
    f32 min[3], max[3];
    BKCollisionTriangle *start_tri, *i_tri, *end_tri;
    f32 tri_vtx_coord[3][3];
    Vtx *vtx_pool, *i_vtx;
    f32 min_coord[3], max_coord[3];
    BKCollisionGeometry *temp_a2;
    f32 tri_edge[3][3];
    f32 sp138[3][3];
    f32 temp_f0_10;
    f32 vtx_radius;
    f32 temp_f0_6;
    f32 tri_normal[3];
    f32 temp_f0_8;
    f32 temp_f12_2;
    f32 temp_f14_2;
    f32 sp108[3];
    f32 spFC[3];
    f32 temp_f2_3;
    f32 temp_f2_6;
    f32 temp_f2_7;
    f32 spE4[3];
    f32 projected_position[3];
    f32 temp_f8;
    BKCollisionTriangle *matched_tri;
    s32 primary_axis;
    s32 var_v1_2;

    // [min, max] = bounding box around position
    for (i = 0; i < 3; i++) {
        min[i] = position[i] - (radius + 0.5);
        max[i] = position[i] + (radius + 0.5);
    }

    // Are vertices within bounding box? If not, return.
    vtx_radius = vtxList_getGlobalNorm(bk_vtx_list);
    for (i = 0; i < 3; i++) {
        if ((max[i] <= -vtx_radius) || (min[i] >= vtx_radius)) {
            return NULL;
        }
    }

    // Iterate over collision geometry intersect range
    matched_tri = NULL;
    TUPLE_SET(spE4, 0.0f);
    collisionList_getIntersecting_f32(this, min, max, &start_geo, &end_geo);
    vtx_pool = vtxList_getVertices(bk_vtx_list);

    for (i_geo = start_geo; i_geo < end_geo; i_geo++) {
        //Iterate over each triangle in collision geometry
        start_tri = (BKCollisionTriangle *) ((BKCollisionGeometry *) this->data + this->geo_count) + (*i_geo)->start_tri_index;
        end_tri = start_tri + (*i_geo)->tri_count;

        for (i_tri = start_tri; i_tri < end_tri; i_tri++) {
            // Filter triangle
            if (!(i_tri->flags & flag_filter)) {
                if (!port_collisionTriValid(i_tri, bk_vtx_list)) {
                    continue;
                }
                // Get Triangle coords
                for (i = 0; i < 3; i++) {
                    i_vtx = &vtx_pool[i_tri->unk0[i]];
                    TUPLE_COPY(tri_vtx_coord[i], i_vtx->v.ob);
                }

                // Calculate bounding box for triangle
                TUPLE_COPY(min_coord, tri_vtx_coord[0]);
                TUPLE_COPY(max_coord, tri_vtx_coord[0]);

                for (i = 1; i < 3; i++) {
                    for (j = 0; j < 3; j++) {
                        if (tri_vtx_coord[i][j] < min_coord[j]) {
                            min_coord[j] = tri_vtx_coord[i][j];
                        }

                        if (max_coord[j] < tri_vtx_coord[i][j]) {
                            max_coord[j] = tri_vtx_coord[i][j];
                        }
                    }
                }

                // Check if triangle is within collision region
                if ((max[0] < min_coord[0]) || (max_coord[0] < min[0])) continue;
                if ((max[1] < min_coord[1]) || (max_coord[1] < min[1])) continue;
                if ((max[2] < min_coord[2]) || (max_coord[2] < min[2])) continue;

                // Caluclate normal
                TUPLE_DIFF_COPY(tri_edge[0], tri_vtx_coord[1], tri_vtx_coord[0]);
                TUPLE_DIFF_COPY(tri_edge[1], tri_vtx_coord[2], tri_vtx_coord[0]);
                TUPLE_CROSS_PRODUCT(tri_normal, tri_edge[0], tri_edge[1]);
                ml_vec3f_normalize(tri_normal);

                if ((tri_normal[0] == 0.0f) && (tri_normal[1] == 0.0f) && (tri_normal[2] == 0.0f)) {
                    continue;
                }

                // Project point onto triangle?
                TUPLE_DIFF_COPY(sp138[0], position, tri_vtx_coord[0]);

                temp_f12_2 = TUPLE_DOT_PRODUCT(sp138[0], tri_normal);
                if((temp_f12_2 <= -(radius - 0.5)) || ((radius - 0.5) <= temp_f12_2))
                    continue;

                temp_f2_3 = LENGTH_SQ_VEC3F(tri_normal);
                if (temp_f2_3 == 0.0f) 
                    continue;

                temp_f0_6 = -temp_f12_2 / temp_f2_3;
                TUPLE_ADD_COPY(projected_position, position, temp_f0_6 * tri_normal);

                // ???
                primary_axis = (ABS_F(tri_normal[0]) > ABS_F(tri_normal[1])) ? 0 : 1;
                primary_axis = (ABS_F(tri_normal[2]) > ABS_F(tri_normal[primary_axis])) ? 2 : primary_axis;

                sp108[0] = projected_position[(primary_axis + 1) % 3] - tri_vtx_coord[0][(primary_axis + 1) % 3];
                sp108[1] = tri_edge[0][(primary_axis + 1) % 3];
                sp108[2] = tri_edge[1][(primary_axis + 1) % 3];

                spFC[0] = projected_position[(primary_axis + 2) % 3] - tri_vtx_coord[0][(primary_axis + 2) % 3];
                spFC[1] = tri_edge[0][(primary_axis + 2) % 3];
                spFC[2] = tri_edge[1][(primary_axis + 2) % 3];

                temp_f14_2 = (sp108[1] * spFC[2]) - (spFC[1] * sp108[2]);
                temp_f2_6 = ((sp108[0] * spFC[2]) - (spFC[0] * sp108[2])) / temp_f14_2;

                if ((0.0f <= temp_f2_6) && (temp_f2_6 <= 1)) {
                    temp_f0_8 = ((sp108[1] * spFC[0]) - (spFC[1] * sp108[0])) / temp_f14_2;

                    if (1) {
                        if ((0.0f <= temp_f0_8) && ((temp_f0_8 <= 1))) {
                            if ((temp_f2_6 + temp_f0_8 <= 1)) {
                                matched_tri = i_tri;
                                TUPLE_ADD_COPY(spE4, spE4, tri_normal);
                                continue;
                            }
                        }
                    }
                }

                for (i = 0; i < 3; i++) {
                    TUPLE_DIFF_COPY(sp138[i], position, tri_vtx_coord[i]);
                    if (LENGTH_SQ_VEC3F(sp138[i]) < SQ(radius)) {
                        matched_tri = i_tri;
                        TUPLE_ADD_COPY(spE4, spE4, tri_normal);
                        break;
                    }
                }

                if (i < 3) {
                    continue;
                }

                for (i = 0; i < 3; i++) {
                    TUPLE_DIFF_COPY(tri_edge[i], tri_vtx_coord[(i + 1) % 3], tri_vtx_coord[i]);
                    temp_f2_7 = LENGTH_SQ_VEC3F(tri_edge[i]);
                    ml_vec3f_normalize(tri_edge[i]);
                    temp_f0_10 = TUPLE_DOT_PRODUCT(tri_edge[i], sp138[i]);

                    if ((temp_f0_10 >= 0.0f) && (SQ(temp_f0_10) <= temp_f2_7)) {
                        TUPLE_DIFF_COPY(projected_position, sp138[i], temp_f0_10 * tri_edge[i]);

                        if (LENGTH_SQ_VEC3F(projected_position) < SQ(radius)) {
                            matched_tri = i_tri;
                            TUPLE_ADD_COPY(spE4, spE4, tri_normal);
                            break;
                        }
                    } 
                }
            }
        }
    }

    if (matched_tri != NULL) {
        ml_vec3f_normalize_copy(normal, spE4);
    }

    collisionList_setStoredTriangle(matched_tri, bk_vtx_list);

    return matched_tri;
}

BKCollisionTriangle *collisionList_intersectSphereGlobal(BKCollisionList *this, BKVertexList *bk_vtx_list, f32 position[3], f32 rotation[3], f32 scale, f32 sphere_center[3], f32 sphere_radius, f32 normal[3], u32 flag_filter) {
    f32 object_sphere_center[3];
    BKCollisionTriangle *coll_tri;
    int i;

    // Do bounding spheres intersect?
    if ((bk_vtx_list->global_norm * scale) <= (ml_vec3f_distance(sphere_center, position) - sphere_radius)) {
        return NULL;
    }

    // Transform sphere center from world to object space
    mlMtxIdent();
    func_80252CC4(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(object_sphere_center, sphere_center);

    // Check collision for every triangle
    coll_tri = collisionList_intersectSphere(this, bk_vtx_list, object_sphere_center, sphere_radius / scale, normal, flag_filter);
    if (!coll_tri) {
        return NULL;
    }

    // Transform modified sphere center from object space to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    mlMtx_apply_vec3f(sphere_center, object_sphere_center);

    // Transform normal from object to world space
    mlMtxIdent();
    func_80252C08(NULL, rotation, 1.0f, NULL);
    mlMtx_apply_vec3f(normal, normal);

    // Transform sStoredTriangle from object to world space
    mlMtxIdent();
    func_80252C08(position, rotation, scale, NULL);
    for (i = 0; i < 3; i++) {
        mlMtx_apply_vec3f(sStoredTriangle[i], sStoredTriangle[i]);
    }

    return coll_tri;
}
