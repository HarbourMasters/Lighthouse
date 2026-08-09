// BanjoDecomp: BGS/bgsspawnqueue.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "actor.h"

extern ActorInfo gChCroctus;
extern ActorInfo gChFlibbit;
extern ActorInfo chPinkEggLargest;
extern ActorInfo chPinkEggLarge;
extern ActorInfo chPinkEggMedium;
extern ActorInfo chPinkEggSmall;
extern ActorInfo chPinkEggSmallest;
extern ActorInfo chMudHut;
extern ActorInfo gChTanktup;
extern ActorInfo gChTanktupLegFrontLeft;
extern ActorInfo gChTanktupLegBackLeft;
extern ActorInfo gChTanktupLegFrontRight;
extern ActorInfo gChTanktupLegBackRight;
extern ActorInfo chFrogMinigame;
extern ActorInfo gChYellowFlibbit;
extern ActorInfo D_80390960;
extern ActorInfo gChYumblie;
extern ActorInfo gChVile;
extern ActorInfo gChTiptup;
extern ActorInfo chChoirTurtleYellow;
extern ActorInfo chChoirTurtleCyan;
extern ActorInfo chChoirTurtleBlue;
extern ActorInfo chChoirTurtleRed;
extern ActorInfo chChoirTurtlePink;
extern ActorInfo chChoirTurtlePurple;
extern ActorInfo chLeafBoat;
extern ActorInfo chLargeCrocodile;
extern ActorInfo chLongSwampSwitch;
extern ActorInfo chShortSwampSwitch;

void bgs_updateSpawnableActors(void){//bgs_updateSpawnableActors
    spawnableActorList_add(&gChCroctus, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_3);//croctus
    spawnableActorList_add(&gChFlibbit, actor_new, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0); //flibbit
    spawnableActorList_add(&chPinkEggLargest, actor_new, ACTOR_FLAG_UNKNOWN_10); //pink_egg_largest
    spawnableActorList_add(&chPinkEggLarge, actor_new, ACTOR_FLAG_UNKNOWN_10); //pink_egg_large
    spawnableActorList_add(&chPinkEggMedium, actor_new, ACTOR_FLAG_UNKNOWN_10); //pink_egg_medium
    spawnableActorList_add(&chPinkEggSmall, actor_new, ACTOR_FLAG_UNKNOWN_10); //pink_egg_small
    spawnableActorList_add(&chPinkEggSmallest, actor_new, ACTOR_FLAG_UNKNOWN_10); //pink_egg_smallest
    spawnableActorList_add(&chMudHut, actor_new, ACTOR_FLAG_NONE); //mudhut_top
    spawnableActorList_add(&gChTanktup, actor_new, ACTOR_FLAG_UNKNOWN_26 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_3);//tanktup
    spawnableActorList_add(&gChTanktupLegFrontLeft, actor_new, ACTOR_FLAG_UNKNOWN_26 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_2);//tanktup_leg
    spawnableActorList_add(&gChTanktupLegBackLeft, actor_new, ACTOR_FLAG_UNKNOWN_26 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_2);//tanktup_leg
    spawnableActorList_add(&gChTanktupLegFrontRight, actor_new, ACTOR_FLAG_UNKNOWN_26 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_2);//tanktup_leg
    spawnableActorList_add(&gChTanktupLegBackRight, actor_new, ACTOR_FLAG_UNKNOWN_26 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_2);//tanktup_leg
    spawnableActorList_add(&chFrogMinigame, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_0);
    spawnableActorList_add(&gChYellowFlibbit, actor_new, ACTOR_FLAG_UNKNOWN_25 | ACTOR_FLAG_UNKNOWN_16 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_0); //yellow_flibbit
    spawnableActorList_add(&D_80390960, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&gChYumblie, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7); //yumblie
    spawnableActorList_add(&gChVile, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_1); //mr. vile
    spawnableActorList_add(&gChTiptup, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup
    spawnableActorList_add(&chChoirTurtleYellow, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chChoirTurtleCyan, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chChoirTurtleBlue, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chChoirTurtleRed, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chChoirTurtlePink, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chChoirTurtlePurple, actor_new, ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_3 ); //tiptup_chiorMember
    spawnableActorList_add(&chLeafBoat, actor_new, ACTOR_FLAG_UNKNOWN_14); //leafboat
    spawnableActorList_add(&chLargeCrocodile, actor_new, ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_7); //bigAlligator
    spawnableActorList_add(&chLongSwampSwitch, actor_new, ACTOR_FLAG_UNKNOWN_3); //green_jiggy_switch
    spawnableActorList_add(&chShortSwampSwitch, actor_new, ACTOR_FLAG_UNKNOWN_3); //green_jiggy_switch
}


