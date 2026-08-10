#include "src/port/ShipInit.hpp"
#include "src/port/Rando/Logic/Logic.h"

using namespace Rando::Logic;

// clang-format off
static RegisterShipInitFunc initFunc([]() {
    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN] = RandoRegion{ .regionName = "Autumn Season", .mapId = MAP_45_CCW_AUTUMN,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_GNAWTYS_INTERIOR_1,                    CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_GNAWTYS_INTERIOR_2,                    CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_GNAWTYS_INTERIOR_3,                    CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_LEAF_NEAR_ENTRANCE_1,                  true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_LEAF_NEAR_ENTRANCE_2,                  true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_MUMBOS_SKULL_1,                   true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_MUMBOS_SKULL_2,                   true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_MUMBOS_SKULL_3,                   true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_TREEHOUSE_1,                      true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_TREEHOUSE_2,                      true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_1,                       true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_2,                       true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_3,                       true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_4,                       true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_5,                       true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_TOP_OF_BRANCH_6,                       true),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_ABOVE_THE_LAKE_1,             CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_ABOVE_THE_LAKE_2,             CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_ABOVE_THE_LAKE_3,             CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_GRASS_NEAR_THE_ENTRANCE_1,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_GRASS_NEAR_THE_ENTRANCE_2,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_GRASS_NEAR_THE_ENTRANCE_3,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_IN_THE_TREEHOUSE_1,           CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_IN_THE_TREEHOUSE_2,           CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_IN_THE_TREEHOUSE_3,           CAN_ATTACK),
            CHECK(RC_CCW_JIGGY_FLOWER,                                          true),
            CHECK(RC_CCW_JIGGY_GNAWTY,                                          CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_EXTRA_LIFE_AUTUMN_BEHIND_GNAWTYS_FIRE,                 CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_JINJO_ORANGE,                                          true),
            CHECK(RC_CCW_MUMBO_TOKEN_AUTUMN_FLOATING_ABOVE_BIG_CLUCKER,         CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_MUMBO_TOKEN_AUTUMN_LEAF_NEAR_TREEHOUSE,                true),
            CHECK(RC_CCW_MUMBO_TOKEN_AUTUMN_SNAREBEAR_NEAR_ENTRANCE,            CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_MUMBO_TOKEN_AUTUMN_TOP_OF_BRANCH,                      true),
            CHECK(RC_CCW_NOTE_AUTUMN_GNAWTYS_INTERIOR_1,                        CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_GNAWTYS_INTERIOR_2,                        CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_1,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_2,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_3,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_4,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_5,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_6,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_7,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_8,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_9,                        true),
            CHECK(RC_CCW_NOTE_AUTUMN_LOWER_TREE_LEDGE_10,                       true),
            CHECK(RC_CCW_NOTE_AUTUMN_NEAR_BIG_FLOWER_1,                         true),
            CHECK(RC_CCW_NOTE_AUTUMN_NEAR_BIG_FLOWER_2,                         true),
            CHECK(RC_CCW_NOTE_AUTUMN_NEAR_BIG_FLOWER_3,                         true),
            CHECK(RC_CCW_NOTE_AUTUMN_NEAR_BIG_FLOWER_4,                         true),
            CHECK(RC_CCW_NOTE_AUTUMN_NEAR_BIG_FLOWER_5,                         true),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_BIG_FLOWER_1,               CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_BIG_FLOWER_2,               CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_BIG_FLOWER_3,               CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_ENTRANCE_1,                 CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_ENTRANCE_2,                 CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_SNAREBEAR_NEAR_ENTRANCE_3,                 CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_BEEHIVE,         CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_MUMBOS_SKULL,    CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS) || CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_UPPER_TREE,               true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_BEEHIVE] = RandoRegion{ .regionName = "Autumn - Inside the Beehive", .mapId = MAP_5C_CCW_AUTUMN_ZUBBA_HIVE,
        .checks = {
            CHECK(RC_CCW_NOTE_AUTUMN_BEEHIVE_INTERIOR_1, true),
            CHECK(RC_CCW_NOTE_AUTUMN_BEEHIVE_INTERIOR_2, true),
            CHECK(RC_CCW_NOTE_AUTUMN_BEEHIVE_INTERIOR_3, true),
            CHECK(RC_CCW_NOTE_AUTUMN_BEEHIVE_INTERIOR_4, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_MUMBOS_SKULL] = RandoRegion{ .regionName = "Autumn - Inside Mumbo's Skull", .mapId = MAP_4C_CCW_AUTUMN_MUMBOS_SKULL,
        .checks = {
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_MUMBOS_SKULL_1, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_MUMBOS_SKULL_2, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_MUMBOS_SKULL_3, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_MUMBOS_SKULL_4, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN, CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_NABNUTS_HOUSE] = RandoRegion{ .regionName = "Autumn - Inside Nabnut's House", .mapId = MAP_60_CCW_AUTUMN_NABNUTS_HOUSE,
        .checks = {
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_NABNUTS_HOUSE_1, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_NABNUTS_HOUSE_2, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_AUTUMN_INSIDE_NABNUTS_HOUSE_3, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_NABNUTS_WINDOW] = RandoRegion{ .regionName = "Autumn - Inside the Window near Nabnut's House", .mapId = MAP_63_CCW_AUTUMN_NABNUTS_WATER_SUPPLY,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NABNUTS_WINDOW_INTERIOR_1, true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NABNUTS_WINDOW_INTERIOR_2, true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NABNUTS_WINDOW_INTERIOR_3, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_TOP_ROOM] = RandoRegion{ .regionName = "Autumn - Top Room of the Tree", .mapId = MAP_67_CCW_AUTUMN_WHIPCRACK_ROOM,
        .checks = {
            CHECK(RC_CCW_EXTRA_LIFE_AUTUMN_TOP_ROOM,            true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_AUTUMN_UPPER_TREE] = RandoRegion{ .regionName = "Autumn - Upper Portion of the Tree", .mapId = MAP_45_CCW_AUTUMN,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_EYRIES_NEST_1,            true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_EYRIES_NEST_2,            true),
            CHECK(RC_CCW_BLUE_EGG_AUTUMN_NEAR_EYRIES_NEST_3,            true),
            CHECK(RC_CCW_EXTRA_LIFE_AUTUMN_SNAREBEAR_NEAR_TREEHOUSE,    true),
            CHECK(RC_CCW_JIGGY_NABNUT,                                  CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WINDOWS) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_MUMBO_TOKEN_AUTUMN_SNAREBEAR_TOP_OF_TREE,      CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_1,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_2,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_3,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_4,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_5,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_6,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_7,                     true),
            CHECK(RC_CCW_NOTE_AUTUMN_EYRIES_NEST_8,                     true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_NABNUTS_HOUSE, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_TOP_ROOM, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WOODEN_DOOR) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN_INTERIOR_NABNUTS_WINDOW, true),
        },
        .events = {
            EVENT(RA_TRIGGER_SWITCH_CCW_WINTER, CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_ENTRANCE] = RandoRegion{ .regionName = "Click Clock Wood Entrance", .mapId = MAP_40_CCW_HUB,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_ENTRANCE_HUB_1,                               true),
            CHECK(RC_CCW_BLUE_EGG_ENTRANCE_HUB_2,                               true),
            CHECK(RC_CCW_BLUE_EGG_ENTRANCE_HUB_3,                               true),
            CHECK(RC_CCW_BLUE_EGG_ENTRANCE_HUB_4,                               true),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_LEFT_OF_SUMMER_DOOR_1,      CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_LEFT_OF_SUMMER_DOOR_2,      CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_LEFT_OF_SUMMER_DOOR_3,      CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_RIGHT_OF_SUMMER_DOOR_1,     CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_RIGHT_OF_SUMMER_DOOR_2,     CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_RIGHT_OF_SUMMER_DOOR_3,     CAN_ATTACK),
            CHECK(RC_CCW_JIGGY_JINJO,                                           CAN_COLLECT_JINJOS(LEVEL_8_CLICK_CLOCK_WOOD)),
            CHECK(RC_CCW_NOTE_ENTRANCE_HUB_1,                                   true),
            CHECK(RC_CCW_NOTE_ENTRANCE_HUB_2,                                   true),
            CHECK(RC_CCW_NOTE_ENTRANCE_HUB_3,                                   true),
            CHECK(RC_CCW_NOTE_ENTRANCE_HUB_4,                                   true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING,                          CAN_ACCESS(RA_TRIGGER_SWITCH_CCW_SPRING)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER,                          CAN_ACCESS(RA_TRIGGER_SWITCH_CCW_SUMMER)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_AUTUMN,                          CAN_ACCESS(RA_TRIGGER_SWITCH_CCW_AUTUMN)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER,                          CAN_ACCESS(RA_TRIGGER_SWITCH_CCW_WINTER)),
            CONNECTION(RR_GRUNTILDAS_LAIR_NEAR_ENTRANCE_CLICK_CLOCK_WOOD,   true),
        },
        .events = {
            EVENT(RA_TRIGGER_SWITCH_CCW_SPRING, CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING] = RandoRegion{ .regionName = "Spring Season", .mapId = MAP_43_CCW_SPRING,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_1,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_2,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_3,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_4,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_5,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_6,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_7,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_8,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_9,                    true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_LOWER_TREE_LEDGE_10,                   true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_1,                        true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_2,                        true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_3,                        true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_4,                        CAN_EXTEND_JUMP_DISTANCE),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_5,                        CAN_EXTEND_JUMP_DISTANCE),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_6,                        CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_7,                        CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_BEEHIVE_8,                        CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_GNAWTYS_HOUSE_1,                  CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_GNAWTYS_HOUSE_2,                  CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_GNAWTYS_HOUSE_3,                  CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_NEAR_GNAWTYS_HOUSE_4,                  CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_EXTRA_LIFE_SPRING_BRANCHES_ABOVE_MUMBOS_SKULL,         CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_EXTRA_LIFE_SPRING_SNAREBEAR_IN_THE_LAKE,               CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_BY_THE_BIG_FLOWER_1,          CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_BY_THE_BIG_FLOWER_2,          CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_BY_THE_BIG_FLOWER_3,          CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_GRASS_NEAR_THE_ENTRANCE_1,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_GRASS_NEAR_THE_ENTRANCE_2,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_GRASS_NEAR_THE_ENTRANCE_3,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_UNDER_THE_TREEHOUSE_1,        CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_UNDER_THE_TREEHOUSE_2,        CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SPRING_UNDER_THE_TREEHOUSE_3,        CAN_ATTACK),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_SNAREBEAR_NEAR_BIG_FLOWER,          CAN_USE_ABILITY(ABILITY_12_WONDERWING) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_SNAREBEAR_NEAR_ENTRANCE,            CAN_USE_ABILITY(ABILITY_12_WONDERWING) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_THORN_FIELD_FRONT_OF_MUMBOS_SKULL,  CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_TOP_OF_BRANCH,                      CAN_USE_ABILITY(ABILITY_10_TALON_TROT) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_TOP_OF_BEEHIVE,                     CAN_USE_ABILITY(ABILITY_10_TALON_TROT) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_BIG_FLOWER_1,                   CAN_EXTEND_JUMP_DISTANCE || CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_BIG_FLOWER_2,                   CAN_EXTEND_JUMP_DISTANCE || CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_BIG_FLOWER_3,                   CAN_EXTEND_JUMP_DISTANCE || CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_MUMBOS_SKULL_1,                 true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_MUMBOS_SKULL_2,                 true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_NEAR_MUMBOS_SKULL_3,                 true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_1,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_2,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_3,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_4,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_5,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LEDGE_OVER_LAKE_6,                         true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_1,                        true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_2,                        true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_3,                        true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_4,                        true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_5,                        true),
            CHECK(RC_CCW_NOTE_SPRING_LOWER_TREE_LEDGE_6,                        true),
            CHECK(RC_CCW_NOTE_SPRING_NEAR_BIG_FLOWER_1,                         true),
            CHECK(RC_CCW_NOTE_SPRING_NEAR_BIG_FLOWER_2,                         true),
            CHECK(RC_CCW_NOTE_SPRING_NEAR_BIG_FLOWER_3,                         true),
            CHECK(RC_CCW_NOTE_SPRING_NEAR_BIG_FLOWER_4,                         true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_ENTRANCE,                        true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_BEEHIVE,         CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_MUMBOS_SKULL,    CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_UPPER_TREE,               (CAN_USE_ABILITY(ABILITY_10_TALON_TROT) && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_BEEHIVE] = RandoRegion{ .regionName = "Spring - Inside the Beehive", .mapId = MAP_5B_CCW_SPRING_ZUBBA_HIVE,
        .checks = {
            CHECK(RC_CCW_JINJO_PINK, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_MUMBOS_SKULL] = RandoRegion{ .regionName = "Spring - Inside Mumbo's Skull", .mapId = MAP_4A_CCW_SPRING_MUMBOS_SKULL,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_MUMBOS_SKULL_1, true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_MUMBOS_SKULL_2, true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_MUMBOS_SKULL_3, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING, CAN_USE_ABILITY(ABILITY_E_WADING_BOOTS) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_NABNUTS_HOUSE] = RandoRegion{ .regionName = "Spring - Inside Nabnut's House", .mapId = MAP_5E_CCW_SPRING_NABNUTS_HOUSE,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_NABNUTS_HOUSE_1,    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_NABNUTS_HOUSE_2,    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_BLUE_EGG_SPRING_INSIDE_NABNUTS_HOUSE_3,    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_NABNUTS_HOUSE,          true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING_TOP_ROOM] = RandoRegion{ .regionName = "Spring - Top Room of the Tree", .mapId = MAP_65_CCW_SPRING_WHIPCRACK_ROOM,
        .checks = {
            CHECK(RC_CCW_EXTRA_LIFE_SPRING_TOP_ROOM_BY_THE_JIGGY,       true),
            CHECK(RC_CCW_EXTRA_LIFE_SPRING_TOP_ROOM_IN_THE_BRANCHES,    true),
            CHECK(RC_CCW_JIGGY_TOP_ROOM,                                true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SPRING_UPPER_TREE] = RandoRegion{ .regionName = "Spring - Upper Portion of the Tree", .mapId = MAP_43_CCW_SPRING,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_1,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_2,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_3,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_4,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_5,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_6,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_7,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_EYRIES_NEST_8,                 true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_UNFINISHED_BRIDGE_1,           true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_UNFINISHED_BRIDGE_2,           true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_UNFINISHED_BRIDGE_3,           true),
            CHECK(RC_CCW_BLUE_EGG_SPRING_UNFINISHED_BRIDGE_4,           true),
            CHECK(RC_CCW_JIGGY_TREE_TOP,                                CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE) || (CAN_ACCESS(RA_TRIGGER_SWITCH_CCW_WINTER) && CAN_USE_ABILITY(ABILITY_9_FLIGHT))),
            CHECK(RC_CCW_JINJO_GREEN,                                   (CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) && CAN_USE_ABILITY(ABILITY_12_WONDERWING)) || CAN_USE_TRANSFORMATION(TRANSFORM_6_BEE)),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_FRONT_OF_EYRIE,             true),
            CHECK(RC_CCW_MUMBO_TOKEN_SPRING_UNFINISHED_WOOD_TREEHOUSE,  true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_INTERIOR_NABNUTS_HOUSE, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SPRING_TOP_ROOM, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WOODEN_DOOR) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
        .events = {
            EVENT(RA_TRIGGER_SWITCH_CCW_SUMMER, CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER] = RandoRegion{ .regionName = "Summer Season", .mapId = MAP_44_CCW_SUMMER,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_1,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_2,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_3,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_4,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_5,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_6,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_7,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_BIG_FLOWER_8,                     true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_1,                   true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_2,                   true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_3,                   true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_4,                   true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_5,                   true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_MUMBOS_SKULL_6,                   true),
            CHECK(RC_CCW_EXTRA_LIFE_SUMMER_NEAR_THE_BIG_FLOWER,                 true),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_DRIED_UP_LAKE_1,              CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_DRIED_UP_LAKE_2,              CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_DRIED_UP_LAKE_3,              CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_GRASS_NEAR_THE_ENTRANCE_1,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_GRASS_NEAR_THE_ENTRANCE_2,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_GRASS_NEAR_THE_ENTRANCE_3,    CAN_ATTACK),
            CHECK(RC_CCW_JIGGY_SUMMER_LEAF_JUMPS,                               CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
            CHECK(RC_CCW_JINJO_YELLOW,                                          true),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_BIG_FLOWER_FIELD,                   true),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_ENTRANCE_GNAWTYS_HOUSE,             CAN_ACCESS(RA_GNAWTYS_BOULDER)),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_FLOATING_ABOVE_BIG_CLUCKER,         CAN_ATTACK && CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_SNAREBEAR_NEAR_THORN_FIELD,         CAN_USE_ABILITY(ABILITY_12_WONDERWING)),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_TOP_OF_BRANCH,                      CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CHECK(RC_CCW_NOTE_SUMMER_GNAWTYS_ENTRANCE_1,                        CAN_ACCESS(RA_GNAWTYS_BOULDER)),
            CHECK(RC_CCW_NOTE_SUMMER_GNAWTYS_ENTRANCE_2,                        CAN_ACCESS(RA_GNAWTYS_BOULDER)),
            CHECK(RC_CCW_NOTE_SUMMER_LEAF_NEAR_ENTRANCE_1,                      true),
            CHECK(RC_CCW_NOTE_SUMMER_LEAF_NEAR_ENTRANCE_2,                      true),
            CHECK(RC_CCW_NOTE_SUMMER_NEAR_BEEHIVE_1,                            CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CHECK(RC_CCW_NOTE_SUMMER_NEAR_BEEHIVE_2,                            CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CHECK(RC_CCW_NOTE_SUMMER_NEAR_BEEHIVE_3,                            CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_MUMBOS_SKULL, CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_BEEHIVE, CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER) && CAN_USE_ABILITY(ABILITY_10_TALON_TROT)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_UPPER_TREE, (CAN_USE_ABILITY(ABILITY_10_TALON_TROT) || (CAN_ATTACK && CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP))) && CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
        },
        .events = {
            EVENT(RA_TRIGGER_SWITCH_CCW_AUTUMN, CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER)),
            EVENT(RA_GNAWTYS_BOULDER, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_GNAWTYS_BOULDER)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_BEEHIVE] = RandoRegion{ .regionName = "Summer - Inside the Beehive", .mapId = MAP_5A_CCW_SUMMER_ZUBBA_HIVE,
        .checks = {
            CHECK(RC_CCW_JIGGY_ZUBBAS, CAN_KILL_ENEMY(ACTOR_29B_ZUBBA)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_MUMBOS_SKULL] = RandoRegion{ .regionName = "Summer - Inside Mumbo's Skull", .mapId = MAP_4B_CCW_SUMMER_MUMBOS_SKULL,
        .checks = {
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_INSIDE_MUMBOS_SKULL, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER, CAN_USE_ABILITY(ABILITY_D_SHOCK_JUMP)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_NABNUTS_HOUSE] = RandoRegion{ .regionName = "Summer - Inside Nabnut's House", .mapId = MAP_5F_CCW_SUMMER_NABNUTS_HOUSE,
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER_TOP_ROOM] = RandoRegion{ .regionName = "Summer - Top Room of the Tree", .mapId = MAP_66_CCW_SUMMER_WHIPCRACK_ROOM,
        .checks = {
            CHECK(RC_CCW_EXTRA_LIFE_SUMMER_TOP_ROOM, true)
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_SUMMER_UPPER_TREE] = RandoRegion{ .regionName = "Summer - Upper Portion of the Tree", .mapId = MAP_44_CCW_SUMMER,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_1,              true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_2,              true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_3,              true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_4,              true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_5,              true),
            CHECK(RC_CCW_BLUE_EGG_SUMMER_NEAR_NABNUTS_HOUSE_6,              true),
            CHECK(RC_CCW_EXTRA_LIFE_SUMMER_TREEHOUSE,                       true),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_OUTSIDE_NABNUTS_1,        CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_OUTSIDE_NABNUTS_2,        CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_OUTSIDE_NABNUTS_3,        CAN_ATTACK),
            CHECK(RC_CCW_JIGGY_HOUSE,                                       true),
            CHECK(RC_CCW_MUMBO_TOKEN_SUMMER_PLATFORMS_BEFORE_EYRIES_NEST,   true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_NABNUTS_HOUSE_1,               true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_NABNUTS_HOUSE_2,               true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_NABNUTS_HOUSE_3,               true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_NABNUTS_HOUSE_4,               true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_NABNUTS_HOUSE_5,               true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_TREEHOUSE_1,                   true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_TREEHOUSE_2,                   true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_TREEHOUSE_3,                   true),
            CHECK(RC_CCW_NOTE_SUMMER_OUTSIDE_TREEHOUSE_4,                   true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_INTERIOR_NABNUTS_HOUSE, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_SUMMER_TOP_ROOM, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WOODEN_DOOR) && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER] = RandoRegion{ .regionName = "Winter Season", .mapId = MAP_46_CCW_WINTER,
        .checks = {
            CHECK(RC_CCW_EXTRA_LIFE_WINTER_BENEATH_SIR_SLUSH,                               CAN_USE_ABILITY(ABILITY_9_FLIGHT) && CAN_USE_ABILITY(ABILITY_1_BEAK_BOMB)),
            CHECK(RC_CCW_EXTRA_LIFE_WINTER_UNDER_THE_ICY_LAKE,                              CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_EMPTY_HONEYCOMB_GNAWTYS,                                           CAN_ACCESS(RA_GNAWTYS_BOULDER) && CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_AROUND_THE_TREE_BASE_1,                   CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_AROUND_THE_TREE_BASE_2,                   CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_AROUND_THE_TREE_BASE_3,                   CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_OUTSIDE_THE_TREEHOUSE_1,                  CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_OUTSIDE_THE_TREEHOUSE_2,                  CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_OUTSIDE_THE_TREEHOUSE_3,                  CAN_ATTACK),
            CHECK(RC_CCW_JINJO_BLUE,                                                        CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CHECK(RC_CCW_MUMBO_TOKEN_WINTER_BIG_FLOWER,                                     true),
            CHECK(RC_CCW_MUMBO_TOKEN_WINTER_BROKEN_BEEHIVE,                                 true),
            CHECK(RC_CCW_MUMBO_TOKEN_WINTER_FROZEN_RIVER_NEAR_FLIGHT_PAD,                   true),
            CHECK(RC_CCW_MUMBO_TOKEN_WINTER_SIR_SLUSH_BETWEEN_BIG_FLOWER_AND_MUMBOS_SKULL,  CAN_KILL_ENEMY(ACTOR_124_SIR_SLUSH)),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_BRANCH_1,                                       true),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_BRANCH_2,                                       true),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_BRANCH_3,                                       true),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_BRANCH_4,                                       true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_MUMBOS_SKULL, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_MUMBOS_SKULL] = RandoRegion{ .regionName = "Winter - Inside Mumbo's Skull", .mapId = MAP_4D_CCW_WINTER_MUMBOS_SKULL,
        .checks = {
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_INSIDE_MUMBOS_SKULL_1,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_INSIDE_MUMBOS_SKULL_2,    CAN_ATTACK),
            CHECK(RC_CCW_HONEYCOMB_BEEHIVE_WINTER_INSIDE_MUMBOS_SKULL_3,    CAN_ATTACK),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_NABNUTS_HOUSE] = RandoRegion{ .regionName = "Winter - Inside Nabnut's House", .mapId = MAP_61_CCW_WINTER_NABNUTS_HOUSE,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_WINTER_INSIDE_NABNUTS_HOUSE_1, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_INSIDE_NABNUTS_HOUSE_2, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_INSIDE_NABNUTS_HOUSE_3, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_INSIDE_NABNUTS_HOUSE_4, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_INSIDE_NABNUTS_HOUSE_5, true),
            CHECK(RC_CCW_SNS_YELLOW_EGG, true),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_NABNUTS_WINDOW] = RandoRegion{ .regionName = "Winter - Inside the Window near Nabnut's House", .mapId = MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_1, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_2, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_3, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_4, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_5, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_6, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_7, CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_CCW_BLUE_EGG_WINTER_NABNUTS_WINDOW_INTERIOR_8, CAN_USE_ABILITY(ABILITY_F_DIVE)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_WINDOW_ABOVE_NABNUT] = RandoRegion{ .regionName = "Winter - Inside the Window Above Nabnut's House", .mapId = MAP_62_CCW_WINTER_HONEYCOMB_ROOM,
        .checks = {
            CHECK(RC_CCW_BLUE_EGG_WINTER_WINDOW_ABOVE_NABNUT_1, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_WINDOW_ABOVE_NABNUT_2, true),
            CHECK(RC_CCW_BLUE_EGG_WINTER_WINDOW_ABOVE_NABNUT_3, true),
            CHECK(RC_CCW_EMPTY_HONEYCOMB_NABNUTS,               CAN_EXTEND_JUMP_DISTANCE && CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_TOP_ROOM] = RandoRegion{ .regionName = "Winter - Top Room of the Tree", .mapId = MAP_68_CCW_WINTER_WHIPCRACK_ROOM,
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE, true),
        },
    };

    Regions[RR_CLICK_CLOCK_WOOD_WINTER_UPPER_TREE] = RandoRegion{ .regionName = "Winter - Upper Portion of the Tree", .mapId = MAP_46_CCW_WINTER,
        .checks = {
            CHECK(RC_CCW_JIGGY_EYRIE,                                       true),
            CHECK(RC_CCW_MUMBO_TOKEN_WINTER_WALKWAY_FRONT_OF_NABNUTS_HOUSE, true),
            CHECK(RC_CCW_NOTE_WINTER_NEAR_NABNUTS_HOUSE_1,                  true),
            CHECK(RC_CCW_NOTE_WINTER_NEAR_NABNUTS_HOUSE_2,                  true),
            CHECK(RC_CCW_NOTE_WINTER_NEAR_NABNUTS_HOUSE_3,                  true),
            CHECK(RC_CCW_NOTE_WINTER_NEAR_NABNUTS_HOUSE_4,                  true),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_TREEHOUSE_1,                    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_TREEHOUSE_2,                    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_TREEHOUSE_3,                    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CHECK(RC_CCW_NOTE_WINTER_TOP_OF_TREEHOUSE_4,                    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP) || CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CHECK(RC_CCW_NOTE_WINTER_TREETOP_PLATFORMS_1,                   true),
            CHECK(RC_CCW_NOTE_WINTER_TREETOP_PLATFORMS_2,                   CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_WINTER_TREETOP_PLATFORMS_3,                   CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_CCW_NOTE_WINTER_TREETOP_PLATFORMS_4,                   CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
        },
        .connections = {
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER, true),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_NABNUTS_HOUSE, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WINDOWS)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_NABNUTS_WINDOW, CAN_BREAK_OBJECT(RA_BREAK_OBJECT_WINDOWS)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_INTERIOR_WINDOW_ABOVE_NABNUT, CAN_USE_ABILITY(ABILITY_1_BEAK_BOMB) && CAN_USE_ABILITY(ABILITY_9_FLIGHT)),
            CONNECTION(RR_CLICK_CLOCK_WOOD_WINTER_TOP_ROOM, CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP))
        },
    };

}, {});
// clang-format on