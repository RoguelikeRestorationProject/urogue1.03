/*
    magicitm.c  -
   
    Last Modified: Jan 5, 1991

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990, 1991 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    Based on "Super-Rogue"
    Copyright (C) 1982, 1983 Robert D. Kindelberger
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

struct magic_item   things[] = {
    {"potion", "POTION", 250, 5},   /* potion */
    {"scroll", "SCROLL", 260, 30},  /* scroll */
    {"ring", "RING", 70, 5},    /* ring */
    {"stick", "STICK", 60, 0},  /* stick */
    {"food", "FOOD", 210, 7},   /* food */
    {"weapon", "WEAPON", 60, 0},    /* weapon */
    {"armor", "ARMOR", 90, 0},  /* armor */
    {"artifact", "ARTIFACT", 0, 0}  /* special artifacts */
};
int numthings = NUMTHINGS;

struct magic_item   s_magic[] = {
    {"monster confusion", "CON", 50, 125, 0, 0},
    {"magic mapping", "MAP", 45, 150, 20, 10},
    {"light", "WATT", 0, 0, 0, 0},
    {"hold monster", "HOLD", 25, 200, 33, 10},
    {"sleep", "SNOOZE", 23, 50, 20, 10},
    {"enchantment", "ENCHANT", 110, 400, 15, 10},
    {"identify", "ID", 150, 50, 0, 15},
    {"scare monster", "SCARE", 35, 250, 27, 21},
    {"detect gold", "GOLD", 0, 0, 0, 0},
    {"teleportation", "TELEP", 50, 165, 10, 20},
    {"create monster", "CREATE", 25, 75, 30, 0},
    {"remove curse", "REM", 75, 220, 10, 15},
    {"petrification", "PET", 25, 285, 0, 0},
    {"genocide", "GEN", 10, 1200, 0, 0},
    {"cure disease", "CURE", 70, 80, 0, 0},
    {"acquirement", "MAKE", 5, 2500, 50, 15},
    {"protection", "PROT", 50, 1150, 0, 0},
    {"nothing", "NOTHING", 75, 50, 50, 50},
    {"magic hitting", "SILVER", 25, 1875, 45, 10},
    {"ownership", "OWN", 15, 1550, 45, 10},
    {"detect food", "FOOD", 0, 0, 0, 0},
    {"electrification", "ELECTRIFY", 20, 1450, 0, 0},
    {"charm monster", "CHARM", 26, 1500, 25, 15},
    {"summon monster", "SUMMON", 26, 1500, 25, 15},
    {"gaze reflection", "REFLECT", 25, 400, 25, 15},
    {"summon familiar", "SUMFAM", 0, 0, 0, 0},
    {"fear", "FEAR", 20, 200, 20, 10},
    {"missile protection", "MSHIELD", 20, 300, 20, 10}
};
int maxscrolls = MAXSCROLLS;

struct magic_item   p_magic[] = {
    {"clear thought", "CLEAR", 90, 380, 27, 15},
    {"gain ability", "GAINABIL", 40, 1250, 15, 15},
    {"see invisible", "SEE", 0, 0, 0, 0},
    {"healing", "HEAL", 120, 330, 27, 27},
    {"detect monster", "MON", 0, 0, 0, 0},
    {"detect magic", "MAG", 0, 0, 0, 0},
    {"raise level", "RAISE", 1, 1900, 11, 10},
    {"haste self", "HASTE", 140, 300, 30, 5},
    {"restore abilities", "RESTORE", 130, 120, 0, 0},
    {"phasing", "PHASE", 45, 340, 21, 20},
    {"invisibility", "INVIS", 30, 300, 0, 15},
    {"acute scent", "SMELL", 30, 100, 20, 15},
    {"acute hearing", "HEAR", 30, 100, 20, 15},
    {"super heroism", "SUPER", 10, 800, 20, 15},
    {"disguise", "DISGUISE", 30, 500, 0, 15},
    {"fire resistance", "NOFIRE", 40, 350, 20, 15},
    {"cold resistance", "NOCOLD", 40, 300, 20, 15},
    {"continuous breathing", "BREATHE", 10, 200, 20, 15},
    {"flying", "FLY", 30, 300, 20, 15},
    {"regeneration", "REGEN", 20, 500, 20, 15},
    {"shield", "SHIELD", 100, 200, 20, 10},
    {"true sight", "TRUESEE", 64, 570, 25, 15}
};
int maxpotions = MAXPOTIONS;

struct magic_item   r_magic[] = {
    {"protection", "", 70, 500, 33, 25},
    {"add strength", "", 65, 300, 33, 25},
    {"sustain ability", "", 40, 380, 10, 0},
    {"searching", "", 65, 250, 10, 0},
    {"see invisible", "", 30, 175, 10, 0},
    {"alertness", "", 40, 190, 10, 0},
    {"aggravate monster", "", 35, 100, 100, 0},
    {"dexterity", "", 65, 220, 33, 25},
    {"increase damage", "", 65, 320, 33, 25},
    {"regeneration", "", 35, 860, 10, 0},
    {"slow digestion", "", 40, 340, 15, 10},
    {"teleportation", "", 35, 100, 100, 0},
    {"stealth", "", 50, 700, 10, 0},
    {"add intelligence", "", 60, 540, 33, 25},
    {"increase wisdom", "", 60, 540, 33, 25},
    {"sustain health", "", 60, 250, 10, 0},
    {"vampiric regeneration", "", 20, 900, 25, 10},
    {"illumination", "", 20, 300, 10, 0},
    {"delusion", "", 20, 100, 75, 0},
    {"carrying", "", 20, 400, 30, 30},
    {"adornment", "", 15, 10000, 10, 0},
    {"levitation", "", 20, 450, 30, 0},
    {"fire resistance", "", 10, 750, 10, 0},
    {"cold resistance", "", 10, 650, 10, 0},
    {"lightning resistance", "", 10, 750, 10, 0},
    {"resurrection", "", 1, 8000, 10, 0},
    {"breathing", "", 10, 250, 10, 0},
    {"free action", "", 10, 225, 10, 0},
    {"wizardry", "", 2, 1950, 10, 0},
    {"piety", "", 2, 1950, 10, 0},
    {"teleport control", "", 5, 450, 10, 0},
    {"true sight", "", 10, 775, 10, 0}
};
int maxrings = MAXRINGS;

struct magic_item   ws_magic[] = {
    {"light", "LIGHT", 90, 150, 20, 20},
    {"striking", "HIT", 58, 400, 0, 0},
    {"lightning", "BOLT", 25, 800, 0, 0},
    {"fire", "FIRE", 25, 600, 0, 0},
    {"cold", "COLD", 30, 600, 0, 0},
    {"polymorph", "POLY", 90, 210, 0, 0},
    {"magic missile", "MLE", 90, 500, 0, 0},
    {"slow monster", "SLOW", 76, 320, 25, 20},
    {"drain life", "DRAIN", 90, 310, 20, 0},
    {"charging", "CHARGE", 70, 1100, 0, 0},
    {"teleport monster", "RANDOM", 90, 240, 25, 20},
    {"cancellation", "CANCEL", 38, 230, 0, 0},
    {"confuse monster", "CONFMON", 50, 200, 0, 0},
    {"disintegration", "KILL-O-ZAP", 10, 1550, 33, 0},
    {"anti-matter", "BLACKHOLE", 30, 980, 0, 0},
    {"paralyze monster", "PARAL", 38, 200, 0, 0},
    {"heal monster", "XENOHEAL", 30, 200, 40, 10},
    {"nothing", "NOTHING", 30, 100, 0, 0},
    {"invisibility", "WS_INVIS", 30, 150, 30, 5},
    {"blasting", "BLAST", 10, 220, 0, 0},
    {"webbing", "WEB", 0, 0, 0, 0},
    {"door opening", "KNOCK", 0, 0, 0, 0},
    {"hold portal", "CLOSE", 0, 0, 0, 0}
};
int maxsticks = MAXSTICKS;

struct magic_item   fd_data[] = {
    {"food ration", "RATION", 400, 20, 20, 20},
    {"fruit-with-very-long-name-that-i-am-padding-out-to-be-greater-than-eighty-characters-so-that-a-test-in-options.c-will-work", "FRUIT", 300, 10, 0, 0},
    {"cram", "CRAM", 120, 30, 0, 0},
    {"honey cake", "CAKES", 80, 10, 0, 0},
    {"lemba", "LEMBA", 50, 80, 0, 0},
    {"miruvor", "MIRUVOR", 50, 200, 0, 0}
};
int maxfoods = MAXFOODS;

/*
 * weapons and their attributes
 * Average Damage = (min_damage + max_damage) / 2)
 * AD of 2D5+3 = (5 + 13) / 2 = 9
 * AD of 3D6   = (3 + 18) / 2 = 10.5
 */
struct init_weps    weaps[] = {
    /* Missile weapons */
    {"sling", "0d0", "0d0", NONE, ISLAUNCHER | ISLITTLE, 5, 1},
    {"rock", "1d2", "1d4", SLING, ISMANY | ISMISL | ISLITTLE, 5, 1},
    {"sling bullet", "1d1", "1d8", SLING, ISSHARP | ISMANY | ISMISL | ISMETAL | ISLITTLE, 3, 1},
    {"short bow", "1d1", "1d1", NONE, ISLAUNCHER, 40, 75},
    {"arrow", "1d1", "1d6", BOW, ISSHARP | ISMANY | ISMISL | ISLITTLE, 5, 1},
    {"arrow", "1d2", "2d8", BOW, ISSHARP | ISSILVER | ISMANY | ISMISL | ISLITTLE, 10, 5},
    {"fire arrow", "1d2", "2d8", BOW, ISSHARP | CANBURN | ISMANY | ISMISL | ISLITTLE, 10, 3},
    {"footbow", "1d1", "1d1", NONE, ISLAUNCHER, 90, 125},
    {"footbow bolt", "1d2", "1d10", FOOTBOW, ISSHARP | ISMANY | ISMISL | ISLITTLE, 5, 2},
    {"crossbow", "1d1", "1d1", NONE, ISLAUNCHER, 100, 175},
    {"crossbow bolt", "1d2", "1d12", CROSSBOW, ISSHARP | ISMANY | ISMISL | ISLITTLE, 7, 3},

    /* Useful throwing weapons */
    {"dart", "1d1", "1d3", NONE, ISSHARP | ISMANY | ISMISL | ISLITTLE, 5, 1},
    {"dagger", "1d6", "1d4", NONE, ISSHARP | ISMETAL | ISMANY | ISMISL | ISLITTLE, 10, 2},
    {"hammer", "1d3", "1d5", NONE, ISMETAL | ISMISL, 50, 3},
    {"leuku", "1d6", "1d5", NONE, ISSHARP | ISMETAL | ISTWOH, 40, 4},
    {"javelin", "1d4", "1d6", NONE, ISSHARP | ISMISL | ISTWOH, 10, 5},
    {"tomahawk", "1d6", "1d6", NONE, ISSHARP | ISMISL, 45, 7},
    {"machete", "1d7", "1d6", NONE, ISSHARP | ISMETAL | ISMISL, 45, 4},
    {"throwing axe", "1d3", "1d6+2", NONE, ISSHARP | ISMETAL | ISMISL, 50, 8},
    {"short spear", "1d6", "1d8", NONE, ISSHARP | ISMETAL | ISMISL, 50, 2},
    {"boomerang", "1d1", "1d8", NONE, CANRETURN | ISMANY | ISMISL | ISLITTLE, 10, 13},
    {"long spear", "1d8", "1d10", NONE, ISSHARP | ISMETAL | ISMISL | ISTWOH, 50, 20},
    {"shuriken", "1d1", "2d5", NONE, ISSHARP | ISMETAL | ISMANY | ISMISL | ISLITTLE, 4, 20},
    {"burning oil", "0d0", "2d10+5", NONE, CANBURN | ISMANY | ISMISL | ISLITTLE, 20, 30},
    {"grenade", "1d1", "1d2/4d8", NONE, ISMANY | ISSMALL, 10, 50},

    /* other weapons */
    {"club", "1d4", "1d2", NONE, 0, 30, 2},
    {"pitchfork", "1d5", "2d2", NONE, ISSHARP | ISMETAL, 15, 5},
    {"short sword", "1d6", "1d2", NONE, ISSHARP | ISMETAL, 50, 10},
    {"hand axe", "1d6", "1d2", NONE, ISSHARP | ISMETAL, 40, 15},
    {"partisan", "1d6", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 75, 4},
    {"grain flail", "1d6", "1d4", NONE, ISSHARP | ISMETAL, 100, 2},
    {"singlestick", "1d4+2", "1d2", NONE, 0, 30, 20},
    {"rapier", "1d6+1", "1d2", NONE, ISSHARP | ISMETAL, 7, 75},
    {"sickle", "1d6+1", "1d2", NONE, ISSHARP | ISMETAL, 30, 15},
    {"hatchet", "1d6+1", "1d4", NONE, ISSHARP | ISMETAL, 50, 10},
    {"scimitar", "1d8", "1d2", NONE, ISSHARP | ISMETAL, 40, 10},
    {"light mace", "2d4", "1d3", NONE, 0, 100, 40},
    {"morning star", "2d4", "1d3", NONE, ISMETAL, 125, 35},
    {"broad sword", "2d4", "1d3", NONE, ISSHARP | ISMETAL, 75, 50},
    {"miner's pick", "2d4", "1d2", NONE, ISSHARP | ISMETAL, 85, 40},
    {"guisarme", "2d4", "1d3", NONE, ISSHARP | ISMETAL | ISTWOH, 100, 25},
    {"war flail", "1d6+2", "1d4", NONE, ISSHARP | ISMETAL | ISTWOH, 150, 50},
    {"crysknife", "3d3", "1d3", NONE, ISSHARP | ISPOISON | ISMANY | ISLITTLE, 12, 100},
    {"battle axe", "1d8+2", "1d3", NONE, ISSHARP | ISMETAL, 80, 100},
    {"cutlass", "1d10", "1d2", NONE, ISSHARP | ISMETAL, 55, 120},
    {"glaive", "1d10", "1d3", NONE, ISSHARP | ISMETAL | ISTWOH, 80, 80},
    {"pertuska", "2d5", "1d3", NONE, ISSHARP | ISMETAL | ISTWOH, 130, 100},
    {"long sword", "1d12", "1d2", NONE, ISSHARP | ISMETAL, 100, 150},
    {"lance", "1d12", "1d8", NONE, ISSHARP | ISTWOH, 80, 140},
    {"ranseur", "1d12", "1d8", NONE, ISSHARP | ISMETAL | ISTWOH, 100, 130},
    {"sabre", "2d6", "1d3", NONE, ISSHARP | ISMETAL, 50, 200},
    {"spetum", "2d6", "1d3", NONE, ISSHARP | ISMETAL | ISTWOH, 50, 180},
    {"halberd", "2d6", "1d3", NONE, ISSHARP | ISMETAL | ISTWOH, 150, 125},
    {"trident", "3d4", "1d4", NONE, ISSHARP | ISMETAL | ISTWOH, 50, 200},
    {"war pick", "3d4", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 75, 175},
    {"bardiche", "3d4", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 125, 125},
    {"heavy mace", "3d4", "1d3", NONE, ISTWOH, 200, 50},
    {"great scythe", "2d6+2", "1d2", NONE, ISSHARP | ISTWOH, 100, 200},
    {"quarter staff", "3d5", "1d2", NONE, 0, 70, 250},
    {"bastard sword", "2d8", "1d2", NONE, ISSHARP | ISMETAL, 150, 300},
    {"pike", "2d8", "2d6", NONE, ISSHARP | ISMETAL | ISTWOH, 200, 275},
    {"great flail", "2d6+2", "1d4", NONE, ISSHARP | ISMETAL | ISTWOH, 200, 300},
    {"great maul", "4d4", "1d3", NONE, ISTWOH, 400, 250},
    {"great pick", "2d9", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 175, 330},
    {"great sword", "3d6", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 250, 300},
    {"claymore", "3d7", "1d2", NONE, ISSHARP | ISMETAL | ISTWOH, 200, 500}
};
int maxweapons = MAXWEAPONS;

struct init_armor   armors[] = {
    {"soft leather", 75, 20, 9, 50},
    {"cuirboilli", 150, 30, 8, 130},
    {"heavy leather", 175, 40, 8, 100},
    {"ring mail", 350, 49, 7, 250},
    {"studded leather", 400, 58, 7, 200},
    {"scale mail", 500, 66, 6, 250},
    {"padded armor", 550, 72, 6, 150},
    {"chain mail", 750, 78, 5, 300},
    {"brigandine", 800, 84, 5, 280},
    {"splint mail", 1000, 88, 4, 350},
    {"banded mail", 1250, 90, 4, 300},
    {"superior chain", 1500, 93, 3, 350},
    {"plate mail", 1400, 96, 3, 400},
    {"plate armor", 1650, 98, 2, 450},
    {"mithril", 30000, 99, 2, 200},
    {"crystalline armor", 15000, 100, 0, 300}
};
int     maxarmors = MAXARMORS;

struct init_artifact    arts[] = {
    {"Magic Purse of Yendor", 25, 1, 1, 1, 1, 460000L, 50},
    {"Phial of Galadriel", 35, 2, 2, 2, 1, 1250000L, 10},
    {"Amulet of Yendor", 45, 4, 1, 1, 2, 1600000L, 10},
    {"Palantir of Might", 60, 1, 4, 1, 2, 1850000L, 70},
    {"Crown of Might", 75, 6, 2, 1, 1, 2350000L, 50},
    {"Sceptre of Might", 80, 2, 2, 1, 6, 3800000L, 50},
    {"Silmaril of Ea", 90, 4, 2, 5, 1, 5000000L, 50},
    {"Wand of Lucifer", 100, 4, 2, 3, 10, 8000000L, 50}
};
int     maxartifact = MAXARTIFACT;

/*
 * In this module so the sizeof()s will work
 */
char        *s_names[MAXSCROLLS];   /* Names of the scrolls */
char        *p_colors[MAXPOTIONS];  /* Colors of the potions */
char        *r_stones[MAXRINGS];    /* Stone settings of the rings */
char        *ws_made[MAXSTICKS];    /* What sticks are made of */
char        *ws_type[MAXSTICKS];    /* Is it a wand or a staff */

char        *guess_items[MAXMAGICTYPES][MAXMAGICITEMS]; /* Players guess at what
                                 * magic is */
bool        know_items[MAXMAGICTYPES][MAXMAGICITEMS];   /* Does he know what a
                                 * magic item does */
