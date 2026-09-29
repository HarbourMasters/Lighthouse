// BanjoDecomp: core2/code_B3A80.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "assets.h"
#include "animation.h"
#include "port/ResourceHelpers.h"
#include "bk_math.h"

extern BKSpriteDisplayData * func_80344A1C(BKSprite *arg0);

s32 assetCacheCurrentSize = 0;
 u8 assetCacheLength = 0; //assetCache_size;
 u8 assetCacheCurrentIndex = 0;
 u8 D_80370A1C = FALSE;


/* .bss */
s32 D_80383CB0;
u8 pad_80383CB8[0x8];
AssetROMHead *assetSectionRomHeader;
AssetFileMeta *assetSectionRomMetaList;
u32 D_80383CC8;
s32 D_80383CCC; //asset_data_rom_offset
void** assetCachePtrList; //assetCache_ptrs;
BKSpriteDisplayData **D_80383CD4;
u8* assetCacheDependencyCount; //assetCache_dependencies;
s16 *assetCacheAssetIdList; //assetCache_indexs
bk_vector(struct21s) *D_80383CE0[2];

/* .public */

void func_8033BAB0(enum asset_e asset_id, s32 offset, s32 size, void *dst_ptr);

/* .core2 */
void func_8033B020(void *ptr){
    struct21s *start_ptr;
    struct21s *end_ptr;
    struct21s *iPtr;

    end_ptr = (struct21s *) bk_vector_getEnd(D_80383CE0[0]);
    start_ptr = (struct21s *) bk_vector_getBegin(D_80383CE0[0]);

    for (iPtr = start_ptr; iPtr < end_ptr && ptr != iPtr->unk1; iPtr++);

    if (iPtr < end_ptr) {
        iPtr->unk0++;
    }
    else {
        iPtr = (struct21s *) bk_vector_pushBackNew(&D_80383CE0[0]);
        iPtr->unk0 = 1;
        iPtr->unk1 = ptr;
    }
}

bool func_8033B0D0(void *arg0) {
    struct21s *start_ptr;
    struct21s *end_ptr;
    struct21s *iPtr;
    s32 j;

    for(j = 0; j < 2; j++){
        end_ptr = (struct21s *) bk_vector_getEnd(D_80383CE0[j]);
        start_ptr = (struct21s *) bk_vector_getBegin(D_80383CE0[j]);
        for(iPtr = start_ptr; iPtr < end_ptr && arg0 != iPtr->unk1; iPtr++){
        }
        if (iPtr < end_ptr){
            return true;
        }
    }
    return false;
}

void func_8033B180(void){
    D_80383CE0[0] = bk_vector_new(sizeof(struct21s), 0x10);
    D_80383CE0[1] = bk_vector_new(sizeof(struct21s), 0x10);
}

void func_8033B1BC(void){
    bk_vector(struct21s) *tmp_a0;
    struct21s *iPtr;
    struct21s *start_ptr;
    struct21s *endPtr;
    int i;

    tmp_a0 = D_80383CE0[0];
    D_80383CE0[0] = D_80383CE0[1];
    D_80383CE0[1] = tmp_a0;
    
    endPtr = (struct21s *) bk_vector_getEnd(D_80383CE0[0]);
    start_ptr = (struct21s *) bk_vector_getBegin(D_80383CE0[0]);
    for(iPtr = start_ptr; iPtr < endPtr; iPtr++){
        for(i = 0; i < iPtr->unk0; i++)
            assetcache_release(iPtr->unk1);
    }
    
    bk_vector_clear(D_80383CE0[0]);
}

void func_8033B268(void){
#if VERSION == VERSION_USA_1_0
    D_80383CE0[0] = (bk_vector(struct21s) *)defrag(D_80383CE0[0]);
    D_80383CE0[1] = (bk_vector(struct21s) *)defrag(D_80383CE0[1]);
#else
    D_80383CE0[0] = (bk_vector(struct21s) *)bk_vector_defrag(D_80383CE0[0]);
    D_80383CE0[1] = (bk_vector(struct21s) *)bk_vector_defrag(D_80383CE0[1]);
#endif
}

void func_8033B2A4(s32 arg0) {
    assetCachePtrList[assetCacheLength] = bk_malloc(arg0);
    D_80383CD4[assetCacheLength] = NULL;
    assetCacheDependencyCount[assetCacheLength] = 1;
    assetCacheAssetIdList[assetCacheLength] = -1;
    assetCacheLength += 1;
}

bool codeB3A80_releaseSprite(void **sprite_ptr, BKSpriteDisplayData **arg1)
{
    void *new_var;
    if ((*sprite_ptr) == NULL)
    return 0;

    new_var = *sprite_ptr;
    assetcache_release(new_var);
    *sprite_ptr = 0;
    *arg1 = 0;
    return 1;
    
}

bool func_8033B388(BKSprite **sprite_ptr, BKSpriteDisplayData **arg1){
    if(*sprite_ptr == NULL)
        return false;
    
    func_8033B020(*sprite_ptr);
    *sprite_ptr = NULL;
    *arg1 = NULL;

    if(sprite_ptr);
    
    return true;
}

void assetcache_release(void * arg0){
    // Lighthouse [port] Stubbing for now, not sure if we want this stuff happening...
    return;
#if 0
    s32 i;
    if(arg0){
        for(i = 0; i < assetCacheLength  && arg0 != assetCachePtrList[i]; i++);
        
        if(i == assetCacheLength)
            return 2;

        assetCacheCurrentIndex = i;
        if(assetCacheDependencyCount[i] == 1){
            if(D_80383CD4[i])
                func_803449DC(D_80383CD4[i]);
            bk_free(arg0);
            assetCacheLength--;
            assetCacheDependencyCount[i] = assetCacheDependencyCount[assetCacheLength];
            assetCachePtrList[i] = assetCachePtrList[assetCacheLength];
            D_80383CD4[i] = D_80383CD4[assetCacheLength];
            assetCacheAssetIdList[i] = assetCacheAssetIdList[assetCacheLength];
            return 0;
        }
        else{
            assetCacheDependencyCount[i]--;
            return 1;
        }
    } else{
        return 3;
    }
#endif
}

void assetcache_update_ptr(void * arg0, void* arg1){
    s32 i;

    if((arg0 == NULL) || (arg1 == NULL) || (arg0 == arg1))
        return;

    for(i = 0; i < assetCacheLength  && arg0 != assetCachePtrList[i]; i++);

    if(i != assetCacheLength && arg1 != assetCachePtrList[i])
        assetCachePtrList[i] = arg1;
}

void func_8033B5FC(void){
    func_8033B268();
}

void func_8033B61C(void){
    core1_15B30_sendMesg3ToRenderThread();
    func_8033B1BC();
    func_8033B1BC();
}

s32 asset_getFlag(enum asset_e arg0){
    // [port] assetSectionRomMetaList is not initialized (ROM init path is #if 0'd).
    // Return 0 to avoid null dereference. Callers use this for render hints.
    if (assetSectionRomMetaList == NULL) {
        return 0;
    }
    return assetSectionRomMetaList[arg0].unk6;
}

s32 assetSection_getCount(void){
    // [port] assetSectionRomHeader not initialized on port
    if (assetSectionRomHeader == NULL) {
        return 0;
    }
    return assetSectionRomHeader->count-1;
}

s32 func_8033B678(void ){
    return assetCacheCurrentSize;
}

s32 asset_getSize(s32 arg0){
    // [port] assetSectionRomMetaList not initialized on port
    if (assetSectionRomMetaList == NULL) {
        return 0;
    }
    return assetSectionRomMetaList[arg0+1].offset - assetSectionRomMetaList[arg0].offset;
}

bool asset_isCompressed(enum asset_e arg0){ //asset_compressed?
    // [port] assetSectionRomMetaList not initialized on port
    if (assetSectionRomMetaList == NULL) {
        return false;
    }
    return (assetSectionRomMetaList[arg0].compFlag & 1) !=0;
}

// [port] Sprite display data cache lives in SpritePatches.cpp
extern BKSpriteDisplayData *port_getOrCreateDisplayData(BKSprite *sprite);

//returns raw sprite(as saved in ROM) and points arg1 to a parsed sprite(?)
BKSprite *codeB3A80_getSprite(enum asset_e sprite_id, BKSpriteDisplayData **arg1){
    BKSprite *s0;
    s0 = assetcache_get(sprite_id);
#if 0 // [port] original decomp — assetCacheCurrentIndex is never updated in the port
    if(D_80383CD4[assetCacheCurrentIndex] == NULL){
        codeAEDA0_setSpriteDrawMode(-1);
        func_80338308(sprite_getUnk8(s0), sprite_getUnkA(s0));
        D_80383CD4[assetCacheCurrentIndex] = func_80344A1C(s0);
    }
    *arg1 = D_80383CD4[assetCacheCurrentIndex];
#endif
    if (s0 == NULL) {
        *arg1 = NULL;
        return NULL;
    }
    *arg1 = port_getOrCreateDisplayData(s0);
    return s0;
}

void assetcache_func_8033B788(void) {
    D_80370A1C = TRUE;
}

void *assetcache_get(enum asset_e assetId) {
    // Lighthouse [port] Using Resource Manager for asset loading.
    // Must also set assetCacheCurrentSize so callers of func_8033B678()
    // (e.g. demo_load) get the correct asset data size.
    void *result = ResourceMgr_LoadByAssetId(assetId);
    if (result) {
        assetCacheCurrentSize = (s32)ResourceMgr_GetResourceSize(assetId);
    }
    return result;
}

// [port] Reload an asset from disk, bypassing the cache.
// On N64, assets were always loaded fresh from ROM. In the port, the resource
// cache preserves modified data (e.g. displaced vertices in map models).
// Use this for assets whose data gets mutated at runtime.
void *assetcache_reload(enum asset_e assetId) {
    extern char* ResourceMgr_ReloadByAssetId(uint32_t);
    void *result = ResourceMgr_ReloadByAssetId(assetId);
    if (result) {
        assetCacheCurrentSize = (s32)ResourceMgr_GetResourceSize(assetId);
    }
    return result;
#if 0
    s32 comp_size;//sp_44
    s32 i;
    volatile s32 sp3C; //sp3C
    s32 uncomp_size; //sp38
    void *uncompressed_file;//sp34
    u8 sp33; //sp33
    void *compressed_file;//sp2C
    bool sp28;
    
    sp28 = D_80370A1C;
    D_80370A1C = FALSE;
    for(i = 0; i < assetCacheLength && assetId != assetCacheAssetIdList[i]; i++);
    assetCacheCurrentIndex = i;
    if(i == 0x96)
        return NULL;
    
    if(i < assetCacheLength){ //asset exists in array;
        assetCacheDependencyCount[i]++;
        return assetCachePtrList[i];
    }
    comp_size = assetSectionRomMetaList[assetId+1].offset - assetSectionRomMetaList[assetId].offset;
    if(comp_size & 1) 
        comp_size++;
    sp3C = comp_size;

    if(assetSectionRomMetaList[assetId].compFlag & 0x0001){//compressed
        func_8033BAB0(assetId, 0, 0x10, &D_80383CB0);
        assetCacheCurrentSize = rarezip_get_uncompressed_size(&D_80383CB0);
        uncomp_size = assetCacheCurrentSize;
        if(uncomp_size & 0xF){
            uncomp_size -= uncomp_size & 0xF;
            uncomp_size += 0x10;
        }
        
        if (func_8025498C((u32)comp_size + uncomp_size) && !sp28) {
            sp33 = 1;
            uncompressed_file = bk_malloc((u32)comp_size + uncomp_size);
            compressed_file = (void *)((s32) uncompressed_file + uncomp_size);
        } else {
            sp33 = 2;
            if (sp28) {
                func_80254C98();
            }
            uncompressed_file = bk_malloc(uncomp_size);
            compressed_file = bk_malloc(comp_size);
        }
    } else { //uncompressed
        uncompressed_file = bk_malloc(comp_size);
        compressed_file = uncompressed_file;
    }
    parallel_readDMA(compressed_file, assetSectionRomMetaList[assetId].offset + D_80383CCC, sp3C);
    if(assetSectionRomMetaList[assetId].compFlag & 0x0001){//decompress
        rarezip_inflate(compressed_file, uncompressed_file);
        bk_realloc(uncompressed_file, assetCacheCurrentSize);
        osWritebackDCache(uncompressed_file, assetCacheCurrentSize);
        if (sp33 == 2) {
            bk_free(compressed_file);
        }
    }
    assetCacheCurrentIndex = assetCacheLength;
    assetCacheDependencyCount[assetCacheLength] = 1;
    assetCachePtrList[assetCacheLength] = uncompressed_file;
    D_80383CD4[assetCacheLength] = 0;
    assetCacheAssetIdList[assetCacheLength] = assetId;
    assetCacheLength++;
    return uncompressed_file;
#endif
}

void func_8033BAB0(enum asset_e asset_id, s32 offset, s32 size, void *dst_ptr) {
    parallel_readDMA(dst_ptr, assetSectionRomMetaList[asset_id].offset + D_80383CCC + offset, size);
}

void assetCache_resizeAsset(void *assetPtr, s32 size){
    s32 tmp;
    s32 i;

    for(i = 0; i < assetCacheLength  && assetPtr != assetCachePtrList[i]; i++);
    assetCachePtrList[i] = bk_realloc(assetPtr, size);
}

void assetCache_init(void){
    // Lighthouse TODO assets
    D_80370A1C = FALSE;
    func_8033B180();
    assetCachePtrList = (void **)bk_malloc(150*sizeof(void*));
    // Lighthouse [port] Changed from 600 to sizeof
    D_80383CD4 = bk_malloc(150*sizeof(BKSpriteDisplayData*));
#if 0
    assetCacheDependencyCount = (u8*)bk_malloc(150*sizeof(u8));
    assetCacheAssetIdList = (s16 *)bk_malloc(150*sizeof(s16));
    assetCacheLength = 0;
    assetSectionRomHeader = (AssetROMHead *)bk_malloc(sizeof(AssetROMHead));
    D_80383CC8 = (u32)assets_ROM_START;
    parallel_readDMA(assetSectionRomHeader, D_80383CC8, sizeof(AssetROMHead));
    assetSectionRomMetaList = (AssetFileMeta *)bk_malloc(assetSectionRomHeader->count*sizeof(AssetFileMeta));
    parallel_readDMA(assetSectionRomMetaList, D_80383CC8 + sizeof(AssetROMHead),assetSectionRomHeader->count*sizeof(AssetFileMeta));
    D_80383CCC = D_80383CC8 + sizeof(AssetROMHead) + assetSectionRomHeader->count*sizeof(AssetFileMeta);
#endif
}

s32 asset_getCompressedSize(enum asset_e arg0){
    // [port] assetSectionRomMetaList not initialized on port
    if (assetSectionRomMetaList == NULL) {
        return 0;
    }
    return assetSectionRomMetaList[arg0+1].offset - assetSectionRomMetaList[arg0].offset;
}

s32 assetCache_getDependencyCount(enum asset_e arg0){
    s32 i;

    for(i = 0; i < assetCacheLength  && arg0 != assetCacheAssetIdList[i]; i++);
    if(i < assetCacheLength){
        return assetCacheDependencyCount[i];
    }
    return 0;
}

void func_8033BD20(void **arg0){
    func_8033B020(*arg0);
    *arg0 = NULL;
}

void assetCache_free(void *arg0){
    func_8033B020(arg0);
}

void func_8033BD6C(void){
    func_8033B1BC();
}

bool func_8033BD8C(void* arg0){
    return func_8033B0D0(arg0);
}

s32 code_B3A80_func_8033BDAC(enum asset_e id, void *dst, s32 size) {
    // Lighthouse [port] assetSectionRomMetaList is not initialized (ROM-direct read path).
    // Callers fall back to assetcache_get() when this returns 0, which is the port-compatible path.
    return 0;
#if 0
    s32 comp_size;
    s32 var_s0;
    s32 sp34;
    s32 phi_v0;
    uintptr_t comp_ptr;
    u8 sp2B;
    s32 sp20;

    //find asset in cache
    for(phi_v0 = 0; phi_v0 < assetCacheLength && id != assetCacheAssetIdList[phi_v0]; phi_v0++);
    assetCacheCurrentIndex = phi_v0;
    if (phi_v0 == 150) { //asset not in cache
        return 0;
    }
    comp_ptr = assetSectionRomMetaList[id + 1].offset - assetSectionRomMetaList[id].offset;
    if (comp_ptr & 1) {
        comp_ptr++;
    }
    sp34 = comp_ptr;
        
    if (assetSectionRomMetaList[id].compFlag & 1) {
        func_8033BAB0(id, 0, 0x10, &D_80383CB0);
        assetCacheCurrentSize = rarezip_get_uncompressed_size(&D_80383CB0);

        // get aligned uncompressed size
        var_s0 = assetCacheCurrentSize;
        if (var_s0 & 0xF) {
            var_s0 = (var_s0 - (var_s0 & 0xF)) + 0x10;
        }

        if (size >= (comp_ptr + var_s0)) {
            sp2B = 1;
            comp_ptr = (uintptr_t)dst + var_s0;
        }
        else if(size >= var_s0) {
            sp2B = 2;
            comp_ptr = (uintptr_t)bk_malloc(comp_ptr);
        }
        else{
            return 0;
        }
    }
    else{
        var_s0 = comp_ptr;
        if(comp_ptr & (0x10 -1)) 
           var_s0 = (comp_ptr - (comp_ptr & (0x10 -1))) + 0x10;
        
        if(size >= comp_ptr){
            comp_ptr = (uintptr_t)dst;
        }
        else{
            return 0;
        }
    }
    comp_size = assetSectionRomMetaList[id].offset + D_80383CCC;
    parallel_readDMA((void *)comp_ptr, comp_size, sp34);
    if (assetSectionRomMetaList[id].compFlag & 1) {
        rarezip_inflate((void *)comp_ptr, dst);
        osWritebackDCache(dst, assetCacheCurrentSize);
        if (sp2B == 2) {
            bk_free((void *)comp_ptr);
        }
    }
    return var_s0;
#endif
}

