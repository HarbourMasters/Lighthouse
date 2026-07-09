#pragma once

#include <map>
extern std::map<int32_t, std::pair<std::string, const char*>> shufflableEnemyMap;

typedef enum {
    BOOTSEQUENCE_DEFAULT,
    BOOTSEQUENCE_AUTHENTIC,
    BOOTSEQUENCE_FILESELECT,
} BootSequenceType;

typedef enum {
    LANGUAGE_ENG,
    LANGUAGE_FRE,
    LANGUAGE_GER,
} LanguageType;

typedef enum {
    TIMER_DISPLAY_NONE,
    TIMER_DISPLAY_RTA,
    TIMER_DISPLAY_IGT,
} TimerDisplayOptions;
