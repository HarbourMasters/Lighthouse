#ifndef RESOURCE_HELPERS_H
#define RESOURCE_HELPERS_H

#ifdef __cplusplus
#include <string>
#include <unordered_map>
//#include "ResourceManager.h"
#include "ship/Context.h"

std::shared_ptr<Ship::IResource> GetResourceByName(const char* path);

extern "C" {
#endif
#include <libultra/gbi.h>

char* ResourceMgr_LoadByAssetId(uint32_t assetId);
size_t ResourceMgr_GetResourceSize(uint32_t assetId);
int ResourceMgr_IsModelAsset(uint32_t assetId);
int ResourceMgr_GetDialogLanguageCount(void);
int ResourceMgr_IsJapanese(void);
int ResourceMgr_GetDialogLanguage(void);
void ResourceMgr_SetDialogLanguage(int lang);
int ResourceMgr_GetLanguageGeneration(void);
int ResourceMgr_IsAssetRepointed(uint32_t assetId);
const char* ResourceMgr_GetLangString(const char* english);
int ResourceMgr_HasLangStrings(void);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
char* ResourceMgr_LoadTexOrDListByName(const char* filePath);
char* ResourceMgr_LoadIfDListByName(const char* filePath);
Vtx* ResourceMgr_LoadVtxByName(char* path);
Mtx* ResourceMgr_LoadMtxByName(char* path);

#ifdef __cplusplus
}

void ResourceHelpers_ApplyLanguage(std::unordered_map<uint32_t, std::string> dialogOverride, bool isJapanese,
                                   int dialogCount, int dialogIndex);
#endif

#endif
