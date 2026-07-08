#include "src/port/ShipInit.hpp"
#include "src/port/Rando/Logic/Logic.h"

using namespace Rando::Logic;

// clang-format off
static RegisterShipInitFunc initFunc([]() {
    Regions[RR_SPIRAL_MOUNTAIN_ENTRANCE] = RandoRegion{ .regionName = "Spiral Mountain", .mapId = MAP_1_SM_SPIRAL_MOUNTAIN,
        .checks = {
		    CHECK(RC_SM_EMPTY_HONEYCOMB_COLLIWOBBLE,    CAN_USE_ABILITY(ABILITY_B_RATATAT_RAP)),
            CHECK(RC_SM_EMPTY_HONEYCOMB_QUARRIES, 	    CAN_BREAK_OBJECT(RA_BREAK_OBJECT_BOULDER)),
            CHECK(RC_SM_EMPTY_HONEYCOMB_STUMP, 		    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_SM_EMPTY_HONEYCOMB_TREE, 		    CAN_USE_ABILITY(ABILITY_5_CLIMB) || CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_SM_EMPTY_HONEYCOMB_UNDERWATER,     CAN_USE_ABILITY(ABILITY_F_DIVE)),
            CHECK(RC_SM_EMPTY_HONEYCOMB_WATERFALL, 	    CAN_EXTEND_JUMP_DISTANCE),
            CHECK(RC_SM_EXTRA_LIFE_ABOVE_BANJOS_HOUSE, 	    CAN_USE_ABILITY(ABILITY_8_FLAP_FLIP)),
            CHECK(RC_SM_EXTRA_LIFE_WATERFALL, 	            CAN_EXTEND_JUMP_DISTANCE),
            CHECK(RC_SM_MOLEHILL_ATTACK, 			    true),
            CHECK(RC_SM_MOLEHILL_BEAK_BARGE, 		    true),
            CHECK(RC_SM_MOLEHILL_CAMERA_CONTROL, 	    true),
            CHECK(RC_SM_MOLEHILL_CLIMB, 			    true),
            CHECK(RC_SM_MOLEHILL_DIVE, 				    true),
            CHECK(RC_SM_MOLEHILL_JUMP, 				    true),
		},
        .connections = {
            CONNECTION(RR_GRUNTILDAS_LAIR_LOBBY, true),
        },
    };

}, {});
// clang-format on