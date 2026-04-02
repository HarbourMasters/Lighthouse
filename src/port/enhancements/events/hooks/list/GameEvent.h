#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

DEFINE_EVENT(OnGameFileSave,
	int32_t fileNum;	
)

DEFINE_EVENT(OnGameFileLoad,
	int32_t fileNum;	
)

DEFINE_EVENT(OnSaveFileLoad,
	int32_t fileNum;
	int32_t* result;
)

DEFINE_EVENT(OnSaveFileSave,
	void* saveBuffer;
	int32_t fileNum;
	int32_t* result;
)

DEFINE_EVENT(OnEepromRead,
	int32_t result;
)

DEFINE_EVENT(OnEepromWrite,
	int32_t file;
	int32_t offset;
	void* buffer;
	int32_t count;
	int32_t result;
)