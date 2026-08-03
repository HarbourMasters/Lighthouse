# Lifts the flag enums out of include/enums.h into a lookup table for the Flag
# Tracer. Generated into the build tree so it can't drift from enums.h.

function(_flagnames_parse_enum ENUM_NAME OUT_VAR)
    set(_entries "")
    set(_in_enum FALSE)
    set(_next 0)

    foreach(_line IN LISTS FLAGNAMES_LINES)
        if(NOT _in_enum)
            if(_line MATCHES "^[ \t]*enum[ \t]+${ENUM_NAME}[ \t]*(\\{)?[ \t]*$")
                set(_in_enum TRUE)
            endif()
            continue()
        endif()

        if(_line MATCHES "^[ \t]*\\}")
            break()
        endif()

        string(REGEX REPLACE "//.*$" "" _clean "${_line}")
        string(REGEX REPLACE "/\\*.*\\*/" "" _clean "${_clean}")

        if(_clean MATCHES "^[ \t]*([A-Za-z_][A-Za-z0-9_]*)[ \t]*(=[ \t]*(0[xX][0-9a-fA-F]+|[0-9]+))?[ \t]*,?[ \t]*$")
            set(_ident "${CMAKE_MATCH_1}")
            set(_explicit "${CMAKE_MATCH_3}")
            if(NOT _explicit STREQUAL "")
                math(EXPR _next "${_explicit}")
            endif()
            list(APPEND _entries "    { ${_next}, \"${_ident}\" },")
            math(EXPR _next "${_next} + 1")
        endif()
    endforeach()

    if(_entries STREQUAL "")
        message(FATAL_ERROR "GenerateFlagNames: no entries parsed for 'enum ${ENUM_NAME}'! Has enums.h changed shape?")
    endif()

    string(REPLACE ";" "\n" _joined "${_entries}")
    set(${OUT_VAR} "${_joined}" PARENT_SCOPE)
endfunction()

function(_flagnames_emit_table ENUM_NAME TABLE_NAME OUT_VAR)
    _flagnames_parse_enum("${ENUM_NAME}" _rows)
    set(${OUT_VAR}
        "// ${ENUM_NAME}\nstatic const FlagName ${TABLE_NAME}[] = {\n${_rows}\n};\n"
        PARENT_SCOPE)
endfunction()

set(FLAGNAMES_HEADER "${CMAKE_BINARY_DIR}/generated/FlagNames.generated.h")
set(FLAGNAMES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/include/enums.h")

file(STRINGS "${FLAGNAMES_SOURCE}" FLAGNAMES_LINES)

set(FLAGNAMES_BODY
"// GENERATED at configure time by cmake/GenerateFlagNames.cmake from include/enums.h.
#pragma once

struct FlagName {
    int index;
    const char* name;
};
")

foreach(_pair "file_progress_e:kFileProgressNames" "volatile_flags_e:kVolatileNames" "level_flags_e:kLevelNames")
    string(REPLACE ":" ";" _pair "${_pair}")
    list(GET _pair 0 _enum)
    list(GET _pair 1 _table)
    _flagnames_emit_table("${_enum}" "${_table}" _table_text)
    string(APPEND FLAGNAMES_BODY "\n${_table_text}")
endforeach()

set(FLAGNAMES_MAP_GROUPS
    "sm_specific_flags:kSmNames:0xB"
    "mm_specific_flags:kMmNames:0x1"
    "ttc_specific_flags:kTtcNames:0x2"
    "bgs_specific_flags:kBgsNames:0x4"
    "fp_specific_flags:kFpNames:0x5"
    "mmm_specific_flags:kMmmNames:0xA"
    "rbb_main_specific_flags:kRbbMainNames:0x9"
    "rbb_boat_room_specific_flags:kRbbBoatNames:0x9"
    "ccw_zubba_specific_flags:kCcwZubbaNames:0x8"
    "ccw_winter_specific_flags:kCcwWinterNames:0x8"
    "lair_specific_flags:kLairNames:0x6"
)

set(FLAGNAMES_GROUP_ROWS "")
foreach(_entry IN LISTS FLAGNAMES_MAP_GROUPS)
    string(REPLACE ":" ";" _parts "${_entry}")
    list(GET _parts 0 _enum)
    list(GET _parts 1 _table)
    list(GET _parts 2 _level)
    _flagnames_emit_table("${_enum}" "${_table}" _table_text)
    string(APPEND FLAGNAMES_BODY "\n${_table_text}")
    list(APPEND FLAGNAMES_GROUP_ROWS
         "    { ${_level}, ${_table}, (int)(sizeof(${_table}) / sizeof(${_table}[0])) },")
endforeach()

string(REPLACE ";" "\n" FLAGNAMES_GROUP_ROWS "${FLAGNAMES_GROUP_ROWS}")
string(APPEND FLAGNAMES_BODY
"
// Map-specific lookup needs the loaded level to disambiguate.
struct MapFlagGroup {
    int level;
    const FlagName* names;
    int count;
};

static const MapFlagGroup kMapFlagGroups[] = {
${FLAGNAMES_GROUP_ROWS}
};
")

set(_existing "")
if(EXISTS "${FLAGNAMES_HEADER}")
    file(READ "${FLAGNAMES_HEADER}" _existing)
endif()
if(NOT _existing STREQUAL "${FLAGNAMES_BODY}")
    file(WRITE "${FLAGNAMES_HEADER}" "${FLAGNAMES_BODY}")
    message(STATUS "Generated ${FLAGNAMES_HEADER}")
endif()

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FLAGNAMES_SOURCE}")
