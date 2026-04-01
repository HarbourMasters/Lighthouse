#pragma once

#include "port/enhancements/events/hooks/EventSystem.h"
#include <stdarg.h>

//#include <libultraship/libultra/gbi.h>
//#include "save.h"

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