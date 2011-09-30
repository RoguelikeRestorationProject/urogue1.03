/*
    rogue.h  -  Rogue definitions and variable declarations
   
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

#include <curses.h>


/* Movement penalties */
#define SHOTPENALTY 2       /* In line of sight of missile */
#define DOORPENALTY 1       /* Moving out of current room */



/*
 * Maximum number of different things
 */
#define MAXROOMS    9   /* max rooms per normal level */
#define MAXDOORS    4   /* max doors to a room */
#define MAXOBJ      8   /* max number of items to find on a level */
#define MAXPACK     23  /* max number of seperate items to carry */
#define MAXTREAS    30  /* max number monsters/treasure in treasure
                 * room */
#define MAXTRAPS    50  /* max traps per level */
#define MAXTRPTRY   16  /* max attempts/level allowed for setting
                 * traps */
#define MAXPURCH    8   /* max purchases per trading post visit */

#define NUMMONST    (sizeof(monsters) / sizeof(struct monster) - 2)
#define NUMSUMMON   48  /* number of creatures that can summon hero */
#define NLEVMONS    4   /* number of new monsters per level */

#define LINELEN     80  /* characters in a buffer */

/*
 * The character types
 */
#define C_FIGHTER    0
#define C_PALADIN    1
#define C_RANGER     2
#define C_CLERIC     3
#define C_DRUID      4
#define C_MAGICIAN   5
#define C_ILLUSION   6
#define C_THIEF      7
#define C_ASSASIN    8
#define C_NINJA      9
#define C_MONSTER   10
#define C_NOTSET    11  /* Must not be a value from above */

/*
 * used for ring stuff
 */
#define LEFT_1  0
#define LEFT_2  1
#define LEFT_3  2
#define LEFT_4  3
#define LEFT_5  4
#define RIGHT_1 5
#define RIGHT_2 6
#define RIGHT_3 7
#define RIGHT_4 8
#define RIGHT_5 9

/*
 * All the fun defines
 */
#define next(ptr) (ptr->l_next)
#define prev(ptr) (ptr->l_prev)
#define identifier(ptr) (ptr->o_ident)
#define winat(y, x) (mvwinch(mw, y, x)==' '?mvwinch(stdscr, y, x):winch(mw))
#define debug if (wizard && wiz_verbose) msg
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)    (struct object *)((*what).l_data)
#define THINGPTR(what)  (struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#ifndef CTRL
#define CTRL(ch) ('ch' & 037)
#endif
#define GOLDCALC (rnd(50 + 30 * level) + 2)
#define o_charges o_ac
#define mi_wght mi_worth

/*
 * Things that appear on the screens
 */
#define WALL        ' '
#define PASSAGE     '#'
#define DOOR        '+'
#define FLOOR       '.'
#define VPLAYER     '@'
#define IPLAYER     '_'
#define POST        '^'
#define LAIR        '('
#define RUSTTRAP    ';'
#define TRAPDOOR    '>'
#define ARROWTRAP   '{'
#define SLEEPTRAP   '$'
#define BEARTRAP    '}'
#define TELTRAP     '~'
#define DARTTRAP    '`'
#define POOL        '"'
#define MAZETRAP    '\\'
#define FIRETRAP    '<'
#define POISONTRAP  '['
#define ARTIFACT    ','
#define SECRETDOOR  '&'
#define STAIRS      '%'
#define GOLD        '*'
#define POTION      '!'
#define SCROLL      '?'
#define MAGIC       '$'
#define BMAGIC      '>' /* Blessed magic */
#define CMAGIC      '<' /* Cursed  magic */
#define FOOD        ':'
#define WEAPON      ')'
#define ARMOR       ']'
#define RING        '='
#define STICK       '/'

/*
 * Various constants
 */
#define HOLDTIME      2
#define BEARTIME      3
#define SLEEPTIME     4
#define FREEZETIME    6
#define STINKTIME     6
#define CHILLTIME     (roll(2, 4))
#define STONETIME     8
#define SICKTIME     10
#define CLRDURATION  15
#define HUHDURATION  20
#define SMELLTIME    20
#define HEROTIME     20
#define HEALTIME     30
#define WANDERTIME   80
#define GONETIME    200
#define PHASEDURATION   300
#define SEEDURATION 850

#define STPOS         0
#define BEFORE        1
#define AFTER         2

#define MORETIME     150
#define HUNGERTIME  1300
#define STOMACHSIZE 2000

#define BOLT_LENGTH 10
#define MARKLEN     20

#define LINEFEED    10
#define CARRIAGE_RETURN 13
#define ESCAPE      27

/*
 * Adjustments for save against things
 */
#define VS_POISON       0
#define VS_PARALYZATION     0
#define VS_DEATH        0
#define VS_PETRIFICATION    1
#define VS_WAND         2
#define VS_BREATH       3
#define VS_MAGIC        4

/*
 * attributes for treasures in dungeon
 */
#define ISNORMAL    0x00000000L /* Neither blessed nor cursed */
#define ISCURSED    0x00000001L /* cursed */
#define ISKNOW      0x00000002L /* has been identified */
#define ISPOST      0x00000004L /* object is in a trading post */
#define ISMETAL     0x00000008L /* is metallic */
#define ISPROT      0x00000010L /* object is protected */
#define ISBLESSED   0x00000020L /* blessed */
#define ISZAPPED    0x00000040L /* weapon has been charged by dragon */
#define ISVORPED    0x00000080L /* vorpalized weapon */
#define ISSILVER    0x00000100L /* silver weapon */
#define ISPOISON    0x00000200L /* poisoned weapon */
#define CANRETURN   0x00000400L /* weapon returns if misses */
#define ISOWNED     0x00000800L /* weapon returns always */
#define ISLOST      0x00001000L /* weapon always disappears */
#define ISMISL      0x00002000L /* missile weapon */
#define ISMANY      0x00004000L /* show up in a group */
#define CANBURN     0x00008000L /* burns monsters */
#define ISSHARP     0x00010000L /* cutting edge */
#define ISTWOH      0x00020000L /* needs two hands to wield */
#define ISLITTLE    0x00040000L /* small weapon */
#define ISLAUNCHER  0x00080000L /* used to throw other weapons */
#define TYP_MAGIC_MASK  0x0f000000L
#define POT_MAGIC   0x01000000L
#define SCR_MAGIC   0x02000000L
#define ZAP_MAGIC   0x04000000L
#define SP_WIZARD   0x10000000L /* only wizards */
#define SP_ILLUSION 0x20000000L /* only illusionists */
#define SP_CLERIC   0x40000000L /* only clerics/paladins */
#define SP_DRUID    0x80000000L /* only druids/rangers */
#define SP_MAGIC    0x30000000L /* wizard or illusionist */
#define SP_PRAYER   0xc0000000L /* cleric or druid */
#define SP_ALL      0xf0000000L /* all special classes */
#define _TWO_       ISBLESSED   /* more powerful spell */

/*
 * Various flag bits
 */
#define ISDARK      0x00000001L
#define ISGONE      0x00000002L
#define ISTREAS     0x00000004L
#define ISFOUND     0x00000008L
#define ISTHIEFSET  0x00000010L
#define WASDARK     0x00000020L

/*
 * struct thing t_flags (might include player) for monster attributes
 */
#define ISBLIND     0x00000001L
#define ISINWALL    0x00000002L
#define ISRUN       0x00000004L
#define ISFLEE      0x00000008L
#define ISINVIS     0x00000010L
#define ISMEAN      0x00000020L
#define ISGREED     0x00000040L
#define CANSHOOT    0x00000080L
#define ISHELD      0x00000100L
#define ISHUH       0x00000200L
#define ISREGEN     0x00000400L
#define CANHUH      0x00000800L
#define CANSEE      0x00001000L
#define HASFIRE     0x00002000L
#define ISSLOW      0x00004000L
#define ISHASTE     0x00008000L
#define ISCLEAR     0x00010000L
#define CANINWALL   0x00020000L
#define ISDISGUISE  0x00040000L
#define CANBLINK    0x00080000L
#define CANSNORE    0x00100000L
#define HALFDAMAGE  0x00200000L
#define CANSUCK     0x00400000L
#define CANRUST     0x00800000L
#define CANPOISON   0x01000000L
#define CANDRAIN    0x02000000L
#define ISUNIQUE    0x04000000L
#define STEALGOLD   0x08000000L
#define STEALMAGIC  0x10000001L
#define CANDISEASE  0x10000002L
#define HASDISEASE  0x10000004L
#define CANSUFFOCATE    0x10000008L
#define DIDSUFFOCATE    0x10000010L
#define BOLTDIVIDE  0x10000020L
#define BLOWDIVIDE  0x10000040L
#define NOCOLD      0x10000080L
#define TOUCHFEAR   0x10000100L
#define BMAGICHIT   0x10000200L
#define NOFIRE      0x10000400L
#define NOBOLT      0x10000800L
#define CARRYGOLD   0x10001000L
#define CANITCH     0x10002000L
#define HASITCH     0x10004000L
#define DIDDRAIN    0x10008000L
#define WASTURNED   0x10010000L
#define CANSELL     0x10020000L
#define CANBLIND    0x10040000L
#define CANBBURN    0x10080000L
#define ISCHARMED   0x10100000L
#define CANSPEAK    0x10200000L
#define CANFLY      0x10400000L
#define ISFRIENDLY  0x10800000L
#define CANHEAR     0x11000000L
#define ISDEAF      0x12000000L
#define CANSCENT    0x14000000L
#define ISUNSMELL   0x18000000L
#define WILLRUST    0x20000001L
#define WILLROT     0x20000002L
#define SUPEREAT    0x20000004L
#define PERMBLIND   0x20000008L
#define MAGICHIT    0x20000010L
#define CANINFEST   0x20000020L
#define HASINFEST   0x20000040L
#define NOMOVE      0x20000080L
#define CANSHRIEK   0x20000100L
#define CANDRAW     0x20000200L
#define CANSMELL    0x20000400L
#define CANPARALYZE 0x20000800L
#define CANROT      0x20001000L
#define ISSCAVENGE  0x20002000L
#define DOROT       0x20004000L
#define CANSTINK    0x20008000L
#define HASSTINK    0x20010000L
#define ISSHADOW    0x20020000L
#define CANCHILL    0x20040000L
#define CANHUG      0x20080000L
#define CANSURPRISE 0x20100000L
#define CANFRIGHTEN 0x20200000L
#define CANSUMMON   0x20400000L
#define TOUCHSTONE  0x20800000L
#define LOOKSTONE   0x21000000L
#define CANHOLD     0x22000000L
#define DIDHOLD     0x24000000L
#define DOUBLEDRAIN 0x28000000L
#define ISUNDEAD    0x30000001L
#define BLESSMAP    0x30000002L
#define BLESSGOLD   0x30000004L
#define BLESSMONS   0x30000008L
#define BLESSMAGIC  0x30000010L
#define BLESSFOOD   0x30000020L
#define CANBRANDOM  0x30000040L /* Types of breath */
#define CANBACID    0x30000080L
#define CANBFIRE    0x30000100L
#define CANBBOLT    0x30000200L
#define CANBGAS     0x30000400L
#define CANBICE     0x30000800L
#define CANBPGAS    0x30001000L /* Paralyze gas */
#define CANBSGAS    0x30002000L /* Sleeping gas */
#define CANBSLGAS   0x30004000L /* Slow gas */
#define CANBFGAS    0x30008000L /* Fear gas */
#define CANBREATHE  0x3000ffc0L /* Can it breathe at all? */
#define STUMBLER    0x30010000L
#define POWEREAT    0x30020000L
#define ISELECTRIC  0x30040000L
#define HASOXYGEN   0x30080000L /* Doesn't need to breath air */
#define POWERDEXT   0x30100000L
#define POWERSTR    0x30200000L
#define POWERWISDOM 0x30400000L
#define POWERINTEL  0x30800000L
#define POWERCONST  0x31000000L
#define SUPERHERO   0x32000000L
#define ISUNHERO    0x34000000L
#define CANCAST     0x38000000L
#define CANTRAMPLE  0x40000001L
#define CANSWIM     0x40000002L
#define LOOKSLOW    0x40000004L
#define CANWIELD    0x40000008L
#define CANDARKEN   0x40000010L
#define ISFAST      0x40000020L
#define CANBARGAIN  0x40000040L
#define NOMETAL     0x40000080L
#define CANSPORE    0x40000100L
#define NOSHARP     0x40000200L
#define DRAINWISDOM 0x40000400L
#define DRAINBRAIN  0x40000800L
#define ISLARGE     0x40001000L
#define ISSMALL     0x40002000L
#define CANSTAB     0x40004000L
#define ISFLOCK     0x40008000L
#define ISSWARM     0x40010000L
#define CANSTICK    0x40020000L
#define CANTANGLE   0x40040000L
#define DRAINMAGIC  0x40080000L
#define SHOOTNEEDLE 0x40100000L
#define CANZAP      0x40200000L
#define HASARMOR    0x40400000L
#define CANTELEPORT 0x40800000L
#define ISBERSERK   0x41000000L
#define ISFAMILIAR  0x42000000L
#define HASFAMILIAR 0x44000000L
#define SUMMONING   0x48000000L
#define CANREFLECT  0x50000001L
#define LOWFRIENDLY 0x50000002L
#define MEDFRIENDLY 0x50000004L
#define HIGHFRIENDLY    0x50000008L
#define MAGICATTRACT    0x50000010L
#define ISGOD       0x50000020L
#define CANLIGHT    0x50000040L
#define HASSHIELD   0x50000080L
#define HASMSHIELD  0x50000100L
#define LOWCAST     0x50000200L
#define MEDCAST     0x50000400L
#define HIGHCAST    0x50000800L
#define WASSUMMONED 0x50001000L
#define HASSUMMONED 0x50002000L
#define CANTRUESEE  0x50004000L


#define FLAGSHIFT       28L
#define FLAGINDEX       0x0000000fL
#define FLAGMASK        0x0fffffffL
/*
 * on - check if a monster flag is on
 */
#define on(th, flag) \
        ((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag << 4))

/*
 * off - check if a monster flag is off
 */
#define off(th, flag) \
        (!((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag << 4)))

/*
 * turn_on - turn on a monster flag
 */
#define turn_on(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag << 4))

/*
 * turn_off - turn off a monster flag
 */
#define turn_off(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~(flag << 4))



#define SAME_POS(c1,c2) ( (c1.x == c2.x) && (c1.y == c2.y) )

/* types of things */
/* All magic spells duplicate a potion, scroll, or stick effect */
#define TYP_POTION  0
#define TYP_SCROLL  1
#define TYP_RING    2
#define TYP_STICK   3
#define MAXMAGICTYPES   4   /* max number of items in magic class */
#define MAXMAGICITEMS   50  /* max number of items in magic class */
#define TYP_FOOD    4
#define TYP_WEAPON  5
#define TYP_ARMOR   6
#define TYP_ARTIFACT    7
#define NUMTHINGS   (sizeof(things) / sizeof(struct magic_item))

/*
 * Artifact types
 */
#define TR_PURSE    0
#define TR_PHIAL    1
#define TR_AMULET   2
#define TR_PALANTIR 3
#define TR_CROWN    4
#define TR_SCEPTRE  5
#define TR_SILMARIL 6
#define TR_WAND     7
#define MAXARTIFACT (sizeof(arts) / sizeof(struct init_artifact))

/*
 * Artifact flags
 */
#define ISUSED      01
#define ISACTIVE    02

/*
 * Potion types - add also to magic_item.c and potions.c
 */
#define P_CLEAR     0
#define P_GAINABIL  1
#define P_SEEINVIS  2
#define P_HEALING   3
#define P_MONSTDET  4
#define P_TREASDET  5
#define P_RAISELEVEL    6
#define P_HASTE     7
#define P_RESTORE   8
#define P_PHASE     9
#define P_INVIS     10
#define P_SMELL     11
#define P_HEAR      12
#define P_SHERO     13
#define P_DISGUISE  14
#define P_FIRERESIST    15
#define P_COLDRESIST    16
#define P_HASOXYGEN 17
#define P_LEVITATION    18
#define P_REGENERATE    19
#define P_SHIELD    20
#define P_TRUESEE   21
#define MAXPOTIONS  (sizeof(p_magic) / sizeof(struct magic_item))

/*
 * Scroll types - add also to magic_item.c and scrolls.c
 */
#define S_CONFUSE   0
#define S_MAP       1
#define S_LIGHT     2
#define S_HOLD      3
#define S_SLEEP     4
#define S_ENCHANT   5
#define S_IDENTIFY  6
#define S_SCARE     7
#define S_GFIND     8
#define S_SELFTELEP 9
#define S_CREATE    10
#define S_REMOVECURSE   11
#define S_PETRIFY   12
#define S_GENOCIDE  13
#define S_CURING    14
#define S_MAKEITEMEM    15
#define S_PROTECT   16
#define S_NOTHING   17
#define S_SILVER    18
#define S_OWNERSHIP 19
#define S_FOODDET   20
#define S_ELECTRIFY 21
#define S_CHARM     22
#define S_SUMMON    23
#define S_REFLECT   24
#define S_SUMFAMILIAR   25
#define S_FEAR      26
#define S_MSHIELD   27
#define MAXSCROLLS  (sizeof(s_magic) / sizeof(struct magic_item))

/*
 * Rod/Wand/Staff types - add also to magic_item.c and sticks.c
 */

#define WS_LIGHT    0
#define WS_HIT      1
#define WS_ELECT    2
#define WS_FIRE     3
#define WS_COLD     4
#define WS_POLYMORPH    5
#define WS_MISSILE  6
#define WS_SLOW_M   7
#define WS_DRAIN    8
#define WS_CHARGE   9
#define WS_MONSTELEP    10
#define WS_CANCEL   11
#define WS_CONFMON  12
#define WS_DISINTEGRATE 13
#define WS_ANTIMATTER   14
#define WS_PARALYZE 15
#define WS_XENOHEALING  16
#define WS_NOTHING  17
#define WS_INVIS    18
#define WS_BLAST    19
#define WS_WEB      20
#define WS_KNOCK    21
#define WS_CLOSE    22
#define MAXSTICKS   (sizeof(ws_magic) / sizeof(struct magic_item))

/*
 * Ring types
 */
#define R_PROTECT   0
#define R_ADDSTR    1
#define R_SUSABILITY    2
#define R_SEARCH    3
#define R_SEEINVIS  4
#define R_ALERT     5
#define R_AGGR      6
#define R_ADDHIT    7
#define R_ADDDAM    8
#define R_REGEN     9
#define R_DIGEST    10
#define R_TELEPORT  11
#define R_STEALTH   12
#define R_ADDINTEL  13
#define R_ADDWISDOM 14
#define R_HEALTH    15
#define R_VREGEN    16
#define R_LIGHT     17
#define R_DELUSION  18
#define R_CARRYING  19
#define R_ADORNMENT 20
#define R_LEVITATION    21
#define R_FIRERESIST    22
#define R_COLDRESIST    23
#define R_ELECTRESIST   24
#define R_RESURRECT 25
#define R_BREATHE   26
#define R_FREEDOM   27
#define R_WIZARD    28
#define R_PIETY     29
#define R_TELCONTROL    30
#define R_TRUESEE   31
#define MAXRINGS    (sizeof(r_magic) / sizeof(struct magic_item))

/*
 * Weapon types
 */
#define SLING       0   /* sling */
#define ROCK        1   /* rocks */
#define BULLET      2   /* sling bullet */
#define BOW     3   /* short bow */
#define ARROW       4   /* arrow */
#define SILVERARROW 5   /* silver arrows */
#define FLAMEARROW  6   /* flaming arrows */
#define FOOTBOW     7   /* footbow */
#define FBBOLT      8   /* footbow bolt */
#define CROSSBOW    9   /* crossbow */
#define BOLT        10  /* crossbow bolt */

#define DART        11  /* darts */
#define DAGGER      12  /* dagger */
#define HAMMER      13  /* hammer */
#define LEUKU       14  /* leuku */
#define JAVELIN     15  /* javelin */
#define TOMAHAWK    16  /* tomahawk */
#define MACHETE     17  /* machete */
#define THROW_AXE   18  /* throwing axe */
#define SHORT_SPEAR 19  /* spear */
#define BOOMERANG   20  /* boomerangs */
#define LONG_SPEAR  21  /* spear */
#define SHIRIKEN    22  /* shurikens */
#define MOLOTOV     23  /* molotov cocktails */
#define GRENADE     24  /* grenade for explosions */
#define CLUB        25  /* club */
#define PITCHFORK   26  /* pitchfork */
#define SHORT_SWORD 27  /* short sword */
#define HAND_AXE    28  /* hand axe */
#define PARTISAN    29  /* partisan */
#define GRAIN_FLAIL 30  /* grain flail */
#define SINGLESTICK 31  /* singlestick */
#define RAPIER      32  /* rapier */
#define SICKLE      33  /* sickle */
#define HATCHET     34  /* hatchet */
#define SCIMITAR    35  /* scimitar */
#define LIGHT_MACE  36  /* mace */
#define MORNINGSTAR 37  /* morning star */
#define BROAD_SWORD 38  /* broad sword */
#define MINER_PICK  39  /* miner's pick */
#define GUISARME    40  /* guisarme */
#define WAR_FLAIL   41  /* war flail */
#define CRYSKNIFE   42  /* crysknife */
#define BATTLE_AXE  43  /* battle axe */
#define CUTLASS     44  /* cutlass sword */
#define GLAIVE      45  /* glaive */
#define PERTUSKA    46  /* pertuska */
#define LONG_SWORD  47  /* long sword */
#define LANCE       48  /* lance */
#define RANSEUR     49  /* ranseur */
#define SABRE       50  /* sabre */
#define SPETUM      51  /* spetum */
#define HALBERD     52  /* halberd */
#define TRIDENT     53  /* trident */
#define WAR_PICK    54  /* war pick */
#define BARDICHE    55  /* bardiche */
#define HEAVY_MACE  56  /* mace */
#define SCYTHE      57  /* great scythe */
#define QUARTERSTAFF    58  /* quarter staff */
#define BAST_SWORD  59  /* bastard sword */
#define PIKE        60  /* pike */
#define TWO_FLAIL   61  /* two-handed flail */
#define TWO_MAUL    62  /* two-handed maul */
#define TWO_PICK    63  /* two-handed pick */
#define TWO_SWORD   64  /* two-handed sword */
#define CLAYMORE    65  /* claymore sword */
#define MAXWEAPONS  (sizeof(weaps) / sizeof(struct init_weps))
#define NONE        100 /* no weapon */

/*
 * Armor types
 */
#define SOFT_LEATHER    0
#define CUIRBOLILLI 1
#define HEAVY_LEATHER   2
#define RING_MAIL   3
#define STUDDED_LEATHER 4
#define SCALE_MAIL  5
#define PADDED_ARMOR    6
#define CHAIN_MAIL  7
#define BRIGANDINE  8
#define SPLINT_MAIL 9
#define BANDED_MAIL 10
#define GOOD_CHAIN  11
#define PLATE_MAIL  12
#define PLATE_ARMOR 13
#define MITHRIL     14
#define CRYSTAL_ARMOR   15
#define MAXARMORS   (sizeof(armors) / sizeof(struct init_armor))

/*
 * Food types
 */
#define FD_RATION   0
#define FD_FRUIT    1
#define FD_CRAM     2
#define FD_CAKES    3
#define FD_LEMBA    4
#define FD_MIRUVOR  5
#define MAXFOODS    (sizeof(fd_data) / sizeof(struct magic_item))

/*
 * stuff to do with encumberance
 */
#define F_OK        0   /* have plenty of food in stomach */
#define F_HUNGRY    1   /* player is hungry */
#define F_WEAK      2   /* weak from lack of food */
#define F_FAINT     3   /* fainting from lack of food */

/*
 * return values for get functions
 */
#define NORM    0       /* normal exit */
#define QUIT    1       /* quit option setting */
#define MINUS   2       /* back up one option */

/*
 * These are the types of inventory styles.
 */
#define INV_SLOW    0
#define INV_OVER    1
#define INV_CLEAR   2

/*
 * These will eventually become enumerations
 */
#define MESSAGE     TRUE
#define NOMESSAGE   FALSE
#define POINTS      TRUE
#define NOPOINTS    FALSE
#define LOWERCASE   TRUE
#define UPPERCASE   FALSE
#define WANDER      TRUE
#define NOWANDER    FALSE
#define GRAB        TRUE
#define NOGRAB      FALSE
#define FAMILIAR    TRUE
#define NOFAMILIAR  FALSE
#define MAXSTATS    TRUE
#define NOMAXSTATS  FALSE
#define FORCE       TRUE
#define NOFORCE     FALSE
#define THROWN      TRUE
#define NOTHROWN    FALSE

#define good_monster(m) (on(m, ISCHARMED) || \
            on(m, ISFRIENDLY) || \
            on(m, ISFAMILIAR))

/*
 * Now we define the structures and types
 */

/*
 * level types
 */
typedef enum {
    NORMLEV,        /* normal level */
    POSTLEV,        /* trading post level */
    MAZELEV,        /* maze level */
    THRONE          /* unique monster's throne room */
}       LEVTYPE;

/*
 * Help list
 */

struct h_list {
    char        h_ch;
    char        *h_desc;
};

/*
 * Coordinate data type
 */
typedef struct {
    int     x;
    int     y;
}       coord;

/*
 * Linked list data type
 */
typedef struct linked_list {
    struct linked_list  *l_next;
    struct linked_list  *l_prev;
    char        *l_data;    /* Various structure pointers */
}       linked_list;

/*
 * Stuff about magic items
 */

struct magic_item {
    char        *mi_name;
    char        *mi_abrev;
    int     mi_prob;
    long        mi_worth;
    int     mi_curse;
    int     mi_bless;
};

/*
 * Room structure
 */
struct room {
    coord       r_pos;      /* Upper left corner */
    coord       r_max;      /* Size of room */
    coord       r_exit[MAXDOORS];   /* Where the exits are */
    int     r_flags;    /* Info about the room */
    int     r_nexits;   /* Number of exits */
    short       r_fires;    /* Number of fires in room */
};

/*
 * Initial artifact stats
 */
struct init_artifact {
    char        *ar_name;   /* name of the artifact */
    int     ar_level;   /* first level where it appears */
    int     ar_rings;   /* number of ring effects */
    int     ar_potions; /* number of potion effects */
    int     ar_scrolls; /* number of scroll effects */
    int     ar_wands;   /* number of wand effects */
    int     ar_worth;   /* gold pieces */
    int     ar_weight;  /* weight of object */
};

/*
 * Array of all traps on this level
 */
struct trap {
    coord       tr_pos;     /* Where trap is */
    long        tr_flags;   /* Info about trap (i.e. ISFOUND) */
    char        tr_type;    /* What kind of trap */
    char        tr_show;    /* What disguised trap looks like */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    char        *s_dmg;     /* String describing damage done */
    long            s_exp;      /* Experience */
    long            s_hpt;      /* Hit points */
    int     s_pack;     /* current weight of his pack */
    int     s_carry;    /* max weight he can carry */
    int     s_lvl;      /* Level of mastery */
    int     s_arm;      /* Armor class */
    short       s_power;    /* Spell points */
    short       s_str;      /* Strength */
    short       s_intel;    /* Intelligence */
    short       s_wisdom;   /* Wisdom */
    short       s_dext;     /* Dexterity */
    short       s_const;    /* Constitution */
    short       s_charisma; /* Charisma */
};

/*
 * Structure describing a fighting being (monster at initialization)
 */
struct mstats {
    short s_str;            /* Strength */
    long s_exp;         /* Experience */
    int s_lvl;          /* Level of mastery */
    int s_arm;          /* Armor class */
    char *s_hpt;            /* Hit points */
    char *s_dmg;            /* String describing damage done */
};

/*
 * Structure for monsters and player
 */
typedef struct thing {
    struct linked_list  *t_pack;    /* What the thing is carrying */
    struct stats    t_stats;    /* Physical description */
    struct stats    maxstats;   /* maximum(or initial) stats */
    coord       t_dest; /* Where it is running to */
    coord       t_pos;      /* Position */
    coord       t_oldpos;   /* Last position */
    long            t_flags[16];    /* State word */
    bool        t_turn;     /* If slowed, is it a turn to move */
    bool        t_wasshot;  /* Was character shot last round? */
    short       t_ctype;    /* Character type */
    short       t_index;    /* Index into monster table */
    short       t_no_move;  /* How long the thing can't move */
    short       t_rest_hpt; /* used in hit point regeneration */
    short       t_rest_pow; /* used in spell point regeneration */
    short       t_doorgoal; /* What door are we heading to? */
    char        t_type;     /* What it is */
    char        t_disguise; /* What mimic looks like */
    char        t_oldch;    /* Character that was where it was */
}       thing;

/*
 * Array containing information on all the various types of monsters
 */

struct monster {
    char        *m_name;    /* What to call the monster */
    short       m_carry;    /* Probability of carrying something */
    bool        m_normal;   /* Does monster exist? */
    bool        m_wander;   /* Does monster wander? */
    char        m_appear;   /* What does monster look like? */
    char        *m_intel;   /* Intelligence range */
    long        m_flags[16];    /* Things about the monster */
    char        *m_typesum; /* type of creature can he summon */
    short       m_numsum;   /* how many creatures can he summon */
    short       m_add_exp;  /* Added experience per hit point */
    struct mstats   m_stats;    /* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

typedef struct object {
    coord       o_pos;      /* Where it lives on the screen */
    struct linked_list  *next_obj;  /* The next obj. for stacked
                         * objects */
    struct linked_list  *o_bag; /* bag linked list pointer */
    char        *o_text;    /* What it says if you read it */
    char        *o_damage;  /* Damage if used like sword */
    char        *o_hurldmg; /* Damage if thrown */
    long        o_flags;    /* Information about objects */
    long        ar_flags;   /* general flags */
    int     o_type;     /* What kind of object it is */
    int     o_ident;    /* identifier for object */
    int     o_count;    /* Count for plural objects */
    int     o_which;    /* Which object of a type it is */
    int     o_hplus;    /* Plusses to hit */
    int     o_dplus;    /* Plusses to damage */
    int     o_ac;       /* Armor class */
    int     o_group;    /* Group number for this object */
    int     o_weight;   /* weight of this object */
    char        o_launch;   /* What you need to launch it */
    char        o_mark[MARKLEN];    /* Mark the specific object */
}       object;
/*
 * weapon structure
 */
struct init_weps {
    char        *w_name;    /* name of weapon */
    char        *w_dam;     /* hit damage */
    char        *w_hrl;     /* hurl damage */
    char        w_launch;   /* need to launch it */
    long        w_flags;    /* flags */
    int     w_wght;     /* weight of weapon */
    long        w_worth;    /* worth of this weapon */
};

/*
 * armor structure
 */
struct init_armor {
    char        *a_name;    /* name of armor */
    long        a_worth;    /* worth of armor */
    int     a_prob;     /* chance of getting armor */
    int     a_class;    /* normal armor class */
    int     a_wght;     /* weight of armor */
};

struct matrix {
    int     base;   /* Base to-hit value (AC 10) */
    int     max_lvl;/* Maximum level for changing value */
    int     factor; /* Amount base changes each time */
    int     offset; /* What to offset level */
    int     range;  /* Range of levels for each offset */
};

struct trap *trap_at();
extern char     print_letters[];
extern struct h_list    helpstr[];

/*
 * Now all the global variables
 */
extern coord   shoot_dir;
extern coord   ch_ret;         /* Where chasing takes you */
extern struct linked_list   *arrow, *bolt, *rock, *silverarrow, *fbbolt;
extern struct linked_list   *bullet, *firearrow, *dart, *dagger, *shuriken;
extern struct linked_list   *oil, *grenade;

extern struct trap  traps[];
extern struct room  rooms[];    /* One for each room -- A level */
extern struct room  *oldrp;     /* Roomin(&oldpos) */
extern struct linked_list   *mlist; /* List of monsters on the level */
extern struct thing player;     /* The rogue */
extern struct thing *beast;     /* The last monster attacking */
extern struct monster   monsters[]; /* The initial monster states */
extern struct linked_list   *lvl_obj;   /* List of objects on this
                         * level */
extern struct linked_list   *fam_ptr;   /* A ptr to the familiar */
extern struct linked_list   *curr_mons; /* The mons. currently moving */
extern struct linked_list   *next_mons; /* The mons. after curr_mons */
extern struct object    *cur_weapon;    /* Which weapon he is wielding */
extern struct object    *cur_armor; /* What a well dresssed rogue wears */
extern struct object    *cur_cloak; /* */
extern struct object    *cur_shield;    /* */
extern struct object    *cur_charm; /* */
extern struct object    *cur_shoes; /* */
extern struct object    *cur_hat;   /* */
extern struct object    *cur_ring[];    /* What rings are being worn */
extern struct magic_item    things[];   /* Chances for each type of
                         * item */
extern struct magic_item    s_magic[];  /* Names and chances for
                         * scrolls */
extern struct magic_item    p_magic[];  /* Names and chances for
                         * potions */
extern struct magic_item    r_magic[];  /* Names and chances for
                         * rings */
extern struct magic_item    ws_magic[]; /* Names and chances for
                         * sticks */
extern struct magic_item    fd_data[];  /* Names and chances for food */
extern struct init_weps weaps[];    /* weapons and attributes */
extern struct init_armor    armors[];   /* armors and attributes */
extern struct init_artifact arts[]; /* artifacts and attributes */
extern WINDOW   *cw;            /* Window that the player sees */
extern WINDOW   *hw;            /* Used for the help command */
extern WINDOW   *mw;            /* Used to store mosnters */
extern LEVTYPE  levtype;
extern coord    delta;          /* Change indicated to get_dir() */
extern char *cnames[][15];      /* Character level names */
extern char *s_names[];     /* Names of the scrolls */
extern char *p_colors[];        /* Colors of the potions */
extern char *r_stones[];        /* Stone settings of the rings */
extern char *guess_items[MAXMAGICTYPES][MAXMAGICITEMS]; /* Players guess at what
                                 * magic is */
extern char *ws_type[];     /* Is it a wand or a staff */
extern char *ws_made[];     /* What sticks are made of */
extern char *release;       /* Release number of rogue */
extern char *lastfmt;
extern char *lastarg;
extern char *spacemsg;
extern char *morestr;
extern char *retstr;
extern unsigned long    total;      /* Total dynamic memory bytes */
extern unsigned long    purse;      /* How much gold the rogue has */
extern int  line_cnt;       /* Counter for inventory style */
extern int  newpage;
extern int  resurrect;      /* resurrection counter */
extern int  foodlev;        /* how fast he eats food */
extern int  see_dist;       /* (how far he can see)^2 */
extern int  level;          /* What level rogue is on */
extern int  mpos;           /* Where cursor is on top line */
extern int  ntraps;         /* Number of traps on this level */
extern int  no_command;     /* Number of turns asleep */
extern int  inpack;         /* Number of things in pack */
extern int  lastscore;      /* Score before this turn */
extern int  no_food;        /* Number of levels without food */
extern int  count;          /* Number of times to repeat command */
extern int  dnum;           /* Dungeon number */
extern int  max_level;      /* Deepest player has gone */
extern int  food_left;      /* Amount of food in hero's stomach */
extern int  group;          /* Current group number */
extern int  hungry_state;       /* How hungry is he */
extern int  infest_dam;     /* Damage from parasites */
extern int  lost_str;       /* Amount of strength lost */
extern int  lost_dext;      /* amount of dexterity lost */
extern int  hold_count;     /* Number of monsters holding player */
extern int  trap_tries;     /* Number of attempts to set traps */
extern int  spell_power;        /* Spell power left at this level */
extern int  has_artifact;       /* set for possesion of artifacts */
extern int  picked_artifact;    /* set for any artifacts picked up */
extern int  msg_index;      /* pointer to current message buffer */
extern int  luck;           /* how expensive things to buy thing */
extern int  fam_type;       /* Type of familiar */
extern int  times_prayed;       /* The number of time prayed */
extern int  mons_summoned;      /* Number of summoned monsters */
extern bool char_type;      /* what type of character is player */
extern bool pool_teleport;      /* just teleported from a pool */
extern bool inwhgt;         /* true if from wghtchk() */
extern bool running;        /* True if player is running */
extern bool fighting;       /* True if player is fighting */
extern bool playing;        /* True until he quits */
extern bool wizard;         /* True if allows wizard commands */
extern bool wiz_verbose;        /* True if show debug messages */
extern bool after;          /* True if we want after daemons */
extern bool fight_flush;        /* True if toilet input */
extern bool terse;          /* True if we should be short */
extern bool doorstop;       /* Stop running when we pass a door */
extern bool jump;           /* Show running as series of jumps */
extern bool door_stop;      /* Current status of doorstop */
extern bool firstmove;      /* First move after setting door_stop */
extern bool waswizard;      /* Was a wizard sometime */
extern bool canwizard;      /* Will be permitted to do this */
extern bool askme;          /* Ask about unidentified things */
extern bool moving;         /* move using 'm' command */
extern bool know_items[MAXMAGICTYPES][MAXMAGICITEMS];   /* Does he know what a
                                 * magic item does */
extern bool in_shell;       /* True if executing a shell */
extern bool inv_type;       /* Inven style. Bool so options works */
extern char take;           /* Thing the rogue is taking */
extern char PLAYER;         /* what the player looks like */
extern char prbuf[];        /* Buffer for sprintfs */
extern char outbuf[];       /* Output buffer for stdout */
extern char runch;          /* Direction player is running */
extern char whoami[];       /* Name of player */
extern char fruit[];        /* Favorite fruit */
extern char msgbuf[10][2 * LINELEN];    /* message buffer */
extern char file_name[];        /* Save file name */
extern char score_file[];       /* Score file name */

/*
 * Command line options
 */
extern int  prscore;    /* Print scores */
extern int  prnews;     /* Print news file */
extern int  prversion;  /* Print version info */

/*
 * table sizes
 */
extern int  maxarmors;
extern int  maxartifact;
extern int  maxfoods;
extern int  maxpotions;
extern int  maxrings;
extern int  maxscrolls;
extern int  maxsticks;
extern int  maxweapons;
extern int  nummonst;
extern int  numthings;

/* Ways to die */

#define D_PETRIFY   -1
#define D_ARROW     -2
#define D_DART      -3
#define D_POISON    -4
#define D_BOLT      -5
#define D_SUFFOCATION   -6
#define D_POTION    -7
#define D_INFESTATION   -8
#define D_DROWN     -9
#define D_FALL      -10
#define D_FIRE      -11
#define D_SPELLFUMBLE   -12
#define D_DRAINLIFE -13
#define D_ARTIFACT  -14
#define D_GODWRATH  -15
#define D_CLUMSY    -16
