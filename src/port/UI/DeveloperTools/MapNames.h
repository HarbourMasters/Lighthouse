#pragma once

#include "enums.h"

struct MapNameEntry {
    const char* displayName;
    const char* shortName;
};

// clang-format off
inline constexpr MapNameEntry mapNames[MAP_NUM_MAPS] = {
    { "Unknown",                          "Unknown"                      }, // 0x00
    { "SM Spiral Mountain",               "Spiral Mountain"              }, // 0x01
    { "MM Mumbo's Mountain",              "Mumbo's Mountain"             }, // 0x02
    { "Unused",                           "Unused"                       }, // 0x03
    { "Unused",                           "Unused"                       }, // 0x04
    { "TTC Blubber's Ship",               "Blubber's Ship"               }, // 0x05
    { "TTC Nipper's Shell",               "Nipper's Shell"               }, // 0x06
    { "TTC Treasure Trove Cove",          "Treasure Trove Cove"          }, // 0x07
    { "Unused",                           "Unused"                       }, // 0x08
    { "Unused",                           "Unused"                       }, // 0x09
    { "TTC Sandcastle",                   "Sandcastle"                   }, // 0x0A
    { "CC Clanker's Cavern",              "Clanker's Cavern"             }, // 0x0B
    { "MM Ticker's Tower",                "Ticker's Tower"               }, // 0x0C
    { "BGS Bubblegloop Swamp",            "Bubblegloop Swamp"            }, // 0x0D
    { "MM Mumbo's Skull",                 "Mumbo's Skull"                }, // 0x0E
    { "Unused",                           "Unused"                       }, // 0x0F
    { "BGS Mr Vile",                      "Mr Vile"                      }, // 0x10
    { "BGS Tiptup",                       "Tiptup"                       }, // 0x11
    { "GV Gobi's Valley",                 "Gobi's Valley"                }, // 0x12
    { "GV Memory Game",                   "Memory Game"                  }, // 0x13
    { "GV Sandybutt's Maze",              "Sandybutt's Maze"             }, // 0x14
    { "GV Water Pyramid",                 "Water Pyramid"                }, // 0x15
    { "GV Rubee's Chamber",               "Rubee's Chamber"              }, // 0x16
    { "Unused",                           "Unused"                       }, // 0x17
    { "Unused",                           "Unused"                       }, // 0x18
    { "Unused",                           "Unused"                       }, // 0x19
    { "GV Inside Jinxy",                  "Inside Jinxy"                 }, // 0x1A
    { "MMM Mad Monster Mansion",          "Mad Monster Mansion"          }, // 0x1B
    { "MMM Church",                       "Church"                       }, // 0x1C
    { "MMM Cellar",                       "Cellar"                       }, // 0x1D
    { "CS Start Nintendo",                "Start Nintendo"               }, // 0x1E
    { "CS Start Rareware",                "Start Rareware"               }, // 0x1F
    { "CS End Not 100",                   "End Not 100"                  }, // 0x20
    { "CC Witch Switch Room",             "Witch Switch Room"            }, // 0x21
    { "CC Inside Clanker",                "Inside Clanker"               }, // 0x22
    { "CC Goldfeather Room",              "Goldfeather Room"             }, // 0x23
    { "MMM Tumblar's Shed",               "Tumblar's Shed"               }, // 0x24
    { "MMM Well",                         "Well"                         }, // 0x25
    { "MMM Napper's Room",                "Napper's Room"                }, // 0x26
    { "FP Freezeezy Peak",                "Freezeezy Peak"               }, // 0x27
    { "MMM Egg Room",                     "Egg Room"                     }, // 0x28
    { "MMM Note Room",                    "Note Room"                    }, // 0x29
    { "MMM Feather Room",                 "Feather Room"                 }, // 0x2A
    { "MMM Secret Church Room",           "Secret Church Room"           }, // 0x2B
    { "MMM Bathroom",                     "Bathroom"                     }, // 0x2C
    { "MMM Bedroom",                      "Bedroom"                      }, // 0x2D
    { "MMM Honeycomb Room",               "Honeycomb Room"               }, // 0x2E
    { "MMM Waterdrain Barrel",            "Waterdrain Barrel"            }, // 0x2F
    { "MMM Mumbo's Skull",                "Mumbo's Skull"                }, // 0x30
    { "RBB Rusty Bucket Bay",             "Rusty Bucket Bay"             }, // 0x31
    { "Unused",                           "Unused"                       }, // 0x32
    { "Unused",                           "Unused"                       }, // 0x33
    { "RBB Engine Room",                  "Engine Room"                  }, // 0x34
    { "RBB Warehouse",                    "Warehouse"                    }, // 0x35
    { "RBB Boathouse",                    "Boathouse"                    }, // 0x36
    { "RBB Container 1",                  "Container 1"                  }, // 0x37
    { "RBB Container 3",                  "Container 3"                  }, // 0x38
    { "RBB Crew Cabin",                   "Crew Cabin"                   }, // 0x39
    { "RBB Boss Boom Box",                "Boss Boom Box"                }, // 0x3A
    { "RBB Storage Room",                 "Storage Room"                 }, // 0x3B
    { "RBB Kitchen",                      "Kitchen"                      }, // 0x3C
    { "RBB Navigation Room",              "Navigation Room"              }, // 0x3D
    { "RBB Container 2",                  "Container 2"                  }, // 0x3E
    { "RBB Captain's Cabin",              "Captain's Cabin"              }, // 0x3F
    { "CCW Hub",                          "Hub"                          }, // 0x40
    { "FP Boggy's Igloo",                 "Boggy's Igloo"                }, // 0x41
    { "Unused",                           "Unused"                       }, // 0x42
    { "CCW Spring",                       "Spring"                       }, // 0x43
    { "CCW Summer",                       "Summer"                       }, // 0x44
    { "CCW Autumn",                       "Autumn"                       }, // 0x45
    { "CCW Winter",                       "Winter"                       }, // 0x46
    { "BGS Mumbo's Skull",                "Mumbo's Skull"                }, // 0x47
    { "FP Mumbo's Skull",                 "Mumbo's Skull"                }, // 0x48
    { "Unused",                           "Unused"                       }, // 0x49
    { "CCW Spring Mumbo's Skull",         "Spring Mumbo's Skull"         }, // 0x4A
    { "CCW Summer Mumbo's Skull",         "Summer Mumbo's Skull"         }, // 0x4B
    { "CCW Autumn Mumbo's Skull",         "Autumn Mumbo's Skull"         }, // 0x4C
    { "CCW Winter Mumbo's Skull",         "Winter Mumbo's Skull"         }, // 0x4D
    { "Unused",                           "Unused"                       }, // 0x4E
    { "Unused",                           "Unused"                       }, // 0x4F
    { "Unused",                           "Unused"                       }, // 0x50
    { "Unused",                           "Unused"                       }, // 0x51
    { "Unused",                           "Unused"                       }, // 0x52
    { "FP Christmas Tree",                "Christmas Tree"               }, // 0x53
    { "Unused",                           "Unused"                       }, // 0x54
    { "Unused",                           "Unused"                       }, // 0x55
    { "Unused",                           "Unused"                       }, // 0x56
    { "Unused",                           "Unused"                       }, // 0x57
    { "Unused",                           "Unused"                       }, // 0x58
    { "Unused",                           "Unused"                       }, // 0x59
    { "CCW Summer Zubba Hive",            "Summer Zubba Hive"            }, // 0x5A
    { "CCW Spring Zubba Hive",            "Spring Zubba Hive"            }, // 0x5B
    { "CCW Autumn Zubba Hive",            "Autumn Zubba Hive"            }, // 0x5C
    { "Unused",                           "Unused"                       }, // 0x5D
    { "CCW Spring Nabnut's House",        "Spring Nabnut's House"        }, // 0x5E
    { "CCW Summer Nabnut's House",        "Summer Nabnut's House"        }, // 0x5F
    { "CCW Autumn Nabnut's House",        "Autumn Nabnut's House"        }, // 0x60
    { "CCW Winter Nabnut's House",        "Winter Nabnut's House"        }, // 0x61
    { "CCW Winter Honeycomb Room",        "Winter Honeycomb Room"        }, // 0x62
    { "CCW Autumn Nabnut's Water Supply", "Autumn Nabnut's Water Supply" }, // 0x63
    { "CCW Winter Nabnut's Water Supply", "Winter Nabnut's Water Supply" }, // 0x64
    { "CCW Spring Whipcrack Room",        "Spring Whipcrack Room"        }, // 0x65
    { "CCW Summer Whipcrack Room",        "Summer Whipcrack Room"        }, // 0x66
    { "CCW Autumn Whipcrack Room",        "Autumn Whipcrack Room"        }, // 0x67
    { "CCW Winter Whipcrack Room",        "Winter Whipcrack Room"        }, // 0x68
    { "GL MM Lobby",                      "MM Lobby"                     }, // 0x69
    { "GL TTC and CC Puzzle",             "TTC and CC Puzzle"            }, // 0x6A
    { "GL 180 Note Door",                 "180 Note Door"                }, // 0x6B
    { "GL Red Cauldron Room",             "Red Cauldron Room"            }, // 0x6C
    { "GL TTC Lobby",                     "TTC Lobby"                    }, // 0x6D
    { "GL GV Lobby",                      "GV Lobby"                     }, // 0x6E
    { "GL FP Lobby",                      "FP Lobby"                     }, // 0x6F
    { "GL CC Lobby",                      "CC Lobby"                     }, // 0x70
    { "GL Statue Room",                   "Statue Room"                  }, // 0x71
    { "GL BGS Lobby",                     "BGS Lobby"                    }, // 0x72
    { "Unused",                           "Unused"                       }, // 0x73
    { "GL GV Puzzle",                     "GV Puzzle"                    }, // 0x74
    { "GL MMM Lobby",                     "MMM Lobby"                    }, // 0x75
    { "GL 640 Note Door",                 "640 Note Door"                }, // 0x76
    { "GL RBB Lobby",                     "RBB Lobby"                    }, // 0x77
    { "GL RBB and MMM Puzzle",            "RBB and MMM Puzzle"           }, // 0x78
    { "GL CCW Lobby",                     "CCW Lobby"                    }, // 0x79
    { "GL Crypt",                         "Crypt"                        }, // 0x7A
    { "CS Intro GL Dingpot 1",            "Intro GL Dingpot 1"           }, // 0x7B
    { "CS Intro Banjo's House 1",         "Intro Banjo's House 1"        }, // 0x7C
    { "CS Spiral Mountain 1",             "Spiral Mountain 1"            }, // 0x7D
    { "CS Spiral Mountain 2",             "Spiral Mountain 2"            }, // 0x7E
    { "FP Wozza's Cave",                  "Wozza's Cave"                 }, // 0x7F
    { "GL FF Entrance",                   "FF Entrance"                  }, // 0x80
    { "CS Intro GL Dingpot 2",            "Intro GL Dingpot 2"           }, // 0x81
    { "CS Entering GL Machine Room",      "Entering GL Machine Room"     }, // 0x82
    { "CS Game Over Machine Room",        "Game Over Machine Room"       }, // 0x83
    { "CS Unused Machine Room",           "Unused Machine Room"          }, // 0x84
    { "CS Spiral Mountain 3",             "Spiral Mountain 3"            }, // 0x85
    { "CS Spiral Mountain 4",             "Spiral Mountain 4"            }, // 0x86
    { "CS Spiral Mountain 5",             "Spiral Mountain 5"            }, // 0x87
    { "CS Spiral Mountain 6",             "Spiral Mountain 6"            }, // 0x88
    { "CS Intro Banjo's House 2",         "Intro Banjo's House 2"        }, // 0x89
    { "CS Intro Banjo's House 3",         "Intro Banjo's House 3"        }, // 0x8A
    { "RBB Anchor Room",                  "Anchor Room"                  }, // 0x8B
    { "SM Banjo's House",                 "Banjo's House"                }, // 0x8C
    { "MMM Inside Loggo",                 "Inside Loggo"                 }, // 0x8D
    { "GL Furnace Fun",                   "Furnace Fun"                  }, // 0x8E
    { "TTC Sharkfood Island",             "Sharkfood Island"             }, // 0x8F
    { "GL Battlements",                   "Battlements"                  }, // 0x90
    { "File Select",                      "File Select"                  }, // 0x91
    { "GV SNS Chamber",                   "SNS Chamber"                  }, // 0x92
    { "GL Dingpot",                       "Dingpot"                      }, // 0x93
    { "CS Intro Spiral 7",                "Intro Spiral 7"               }, // 0x94
    { "CS End All 100",                   "End All 100"                  }, // 0x95
    { "CS End Beach 1",                   "End Beach 1"                  }, // 0x96
    { "CS End Beach 2",                   "End Beach 2"                  }, // 0x97
    { "CS End Spiral Mountain 1",         "End Spiral Mountain 1"        }, // 0x98
    { "CS End Spiral Mountain 2",         "End Spiral Mountain 2"        }, // 0x99
};
// clang-format on
