/*
    rogue.c  -  Now all the global variables
   
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

struct trap traps[2 * MAXTRAPS];        /* twice as much for special effects */
struct room rooms[MAXROOMS];        /* One for each room -- A level */
struct room *oldrp;             /* Roomin(&player.t_oldpos) */
struct thing    player;             /* The rogue */
struct thing    *beast;             /* The last beast that attacked us */
struct object   *cur_armor;         /* What a well dresssed rogue wears */
struct object   *cur_weapon = NULL;     /* ... and wields */
struct object   *cur_ring[10];          /* His rings */
struct object   *cur_shield = NULL;     /* future expansion */
struct object   *cur_cloak = NULL;
struct object   *cur_shoes = NULL;
struct object   *cur_hat = NULL;
struct object   *cur_charm = NULL;      /* Currently charmed monsters */
struct linked_list  *fam_ptr = NULL;    /* A ptr to the familiar */
struct linked_list  *lvl_obj = NULL;    /* Treasure on this level */
struct linked_list  *mlist = NULL;      /* Monsters on this level */
struct linked_list  *curr_mons = NULL;  /* The mons. currently moving */
struct linked_list  *next_mons = NULL;  /* The mons. after curr_mons */

/*
 * Command line options
 */
int prscore;        /* Print scores */
int prnews;         /* Print news file */
int prversion;      /* Print version info */

int foodlev = 1;        /* how fast he eats food */
int ntraps;         /* Number of traps on this level */
int seed;           /* Random number seed */
int dnum;           /* Dungeon number */
int max_level;      /* Deepest player has gone */
int lost_dext;      /* amount of lost dexterity */
int mpos = 0;
int no_command = 0;
int level = 0;
int inpack = 0;
int see_dist = 3;
int no_food = 0;
int count = 0;
int food_left = HUNGERTIME;
int group = 1;
int hungry_state = F_OK;
int infest_dam = 0;
int lost_str = 0;
int lastscore = -1;
int hold_count = 0;
int trap_tries = 0;
int has_artifact = 0;
int picked_artifact = 0;
int msg_index = 0;
int luck = 0;
int resurrect = 0;
int line_cnt;       /* Inventory stuff */
int newpage;        /* Inventory stuff */
int fam_type = 0;       /* The type of familiar */
int mons_summoned = 0;  /* Number of summoned monsters */
char    curpurch[15];       /* name of item ready to buy */
char    PLAYER = VPLAYER;   /* what the player looks like */
char    take;           /* Thing the rogue is taking */
char    prbuf[2 * LINELEN]; /* Buffer for sprintfs */
char    outbuf[LINELEN];    /* Output buffer for stdout */
char    runch;          /* Direction player is running */
char    whoami[2 * LINELEN];    /* Name of player */
char    fruit[2 * LINELEN]; /* Favorite fruit */
char    msgbuf[10][2 * LINELEN];    /* message buffer */
char    file_name[2 * LINELEN]; /* Save file name */
char    score_file[2 * LINELEN];/* Score file name */
char    home[2 * LINELEN];  /* User's home directory */
char    *lastfmt;
char    *lastarg;
WINDOW  *cw;            /* Window that the player sees */
WINDOW  *hw;            /* Used for the help command */
WINDOW  *mw;            /* Used to store mosnters */
unsigned long   purse = 0;
unsigned long   total = 0;
bool    char_type = C_NOTSET;   /* what type of character is player */
bool    inv_type = INV_OVER;    /* Overwrite style of inventory */
bool    pool_teleport = FALSE;  /* just teleported from a pool */
bool    inwhgt = FALSE;     /* true if from wghtchk() */
bool    after;          /* True if we want after daemons */
bool    waswizard;      /* Was a wizard sometime */
bool    canwizard;      /* Will be permitted to do this */
bool    playing = TRUE;
bool    running = FALSE;
bool    fighting = FALSE;
bool    wizard = FALSE;
bool    wiz_verbose = TRUE;
bool    fight_flush = FALSE;
bool    terse = FALSE;
bool    door_stop = FALSE;
bool    jump = FALSE;
bool    doorstop = TRUE;
bool    firstmove = FALSE;
bool    askme = FALSE;
bool    moving = FALSE;
bool    in_shell = FALSE;
coord   delta;          /* Change indicated to get_dir() */
LEVTYPE levtype;        /* type of level i'm on */

char    *spacemsg = "--Press SPACE to continue--";
char    *morestr = "--More--";
char    *retstr = "[Press RETURN to continue]";

/* 15 named levels */
char    *cnames[C_NOTSET][15] = {
    {"Veteran", "Warrior",
	"Swordsman", "Hero",    /* Fighter */
	"Swashbuckler", "Myrmidon",
	"Champion", "Superhero",
	"Lord", "Lord",
	"Lord", "Lord",
	"Lord", "Lord",
	"Lord"
    },
    {"Gallant", "Keeper",
	"Protector", "Defender",    /* Paladin */
	"Warder", "Guardian",
	"Chevalier", "Justiciar",
	"Paladin", "Paladin",
	"Paladin", "Paladin",
	"Paladin", "Paladin",
	"Paladin"
    },
    {"Runner", "Strider",
	"Scout", "Courser", /* Ranger */
	"Tracker", "Guide",
	"Pathfinder", "Ranger",
	"Ranger Knight", "Ranger Lord",
	"Ranger Lord", "Ranger Lord",
	"Ranger Lord", "Ranger Lord",
	"Ranger Lord"
    },
    {"Acolyte", "Adept",
	"Priest", "Curate", /* Cleric */
	"Prefect", "Canon",
	"Lama", "Patriarch",
	"High Priest", "High Priest",
	"High Priest", "High Priest",
	"High Priest", "High Priest",
	"High Priest"
    },
    {"Aspirant", "Ovate",   /* Druid */
	"Initiate of the 1st Circle", "Initiate of the 2nd Circle",
	"Initiate of the 3rd Circle", "Initiate of the 4th Circle",
	"Initiate of the 5th Circle", "Initiate of the 6th Circle",
	"Initiate of the 7th Circle", "Initiate of the 8th Circle",
	"Initiate of the 9th Circle", "Druid",
	"Archdruid", "The Great Druid",
	"The Grand Druid"
    },

    {"Prestidigitator", "Evoker",
	"Conjurer", "Theurgist",    /* Magic User */
	"Thaumaturgist", "Magician",
	"Enchanter", "Warlock",
	"Sorcerer", "Necromancer",
	"Wizard", "Wizard",
	"Wizard", "Wizard",
	"Wizard"
    },
    {"Prestidigitator", "Minor Trickster",
	"Trickster", "Master Trickster",    /* Illusionist */
	"Cabalist", "Visionist",
	"Phantasmist", "Apparitionist",
	"Spellbinder", "Illusionist",
	"Illusionist", "Illusionist",
	"Illusionist", "Illusionist",
	"Illusionist"
    },
    {"Rogue", "Footpad",
	"Cutpurse", "Robber",   /* Thief */
	"Burglar", "Filcher",
	"Sharper", "Magsman",
	"Thief", "Master Thief",
	"Master Thief", "Master Thief",
	"Master Thief", "Master Thief",
	"Master Thief"
    },
    {"Bravo", "Rutterkin",
	"Waghalter", "Murderer",    /* Assasin */
	"Thug", "Killer",
	"Cutthroat", "Executioner",
	"Assassin", "Expert Assassin",
	"Senior Assassin", "Chief Assassin",
	"Prime Assassin", "Guildmaster Assassin",
	"Grandfather of Assassins"
    },
    {"Ninja", "Ninja",
	"Ninja", "Ninja",   /* Ninja */
	"Ninja", "Ninja",
	"Ninja", "Ninja",
	"Ninja", "Ninja",
	"Ninja", "Ninja",
	"Ninja", "Ninja",
	"Ninja"
    }
};

struct h_list   helpstr[] = {
    '?', "  prints help",
    '/', "  identify object",
    'h', "  left",
    'j', "  down",
    'k', "  up",
    'l', "  right",
    'y', "  up & left",
    'u', "  up & right",
    'b', "  down & left",
    'n', "  down & right",
    '<', "SHIFT><dir> run that way",
    'm', "<dir> move onto without picking up",
    't', "<dir> throw something",
    'z', "<dir> zap a wand or staff",
    '>', "  go down a staircase",
    's', "  search for trap/secret door",
    '.', "  rest for a while",
    ',', "  pick up an object",
    'i', "  inventory all items",
    'I', "  inventory type of item",
    'q', "  quaff potion",
    'r', "  read paper",
    'e', "  eat food",
    'w', "  wield a weapon",
    'W', "  wear armor",
    'T', "  take armor off",
    'P', "  put on ring",
    'R', "  remove ring",
    'A', "  activate/apply an artifact",
    'd', "  drop object",
    'C', "  call object (generic)",
    'M', "  mark object (specific)",
    'o', "  examine/set options",
    'c', "  cast a spell/say a prayer",
    'p', "  pray for help (risky)",
    'a', "  affect the undead",
    '^', "  set a trap",
    'D', "  dip something (into a pool)",
    19, "<dir>  take (steal) from (direction)", /* ctrl-t */
    17, "   redraw screen", /* ctrl-r */
    15, "   back up to 10 previous messages",   /* ctrl-p */
    ESCAPE, "   cancel command",
    'v', "  print program version number",
    'S', "  save game",
    'Q', "  quit",
    '=', "  listen for monsters",
    'f', "<dir> fight monster",
    'F', "<dir> fight monster to the death",

    /*
     * Wizard commands.  Identified by (h_ch != 0 && h_desc == 0).
     */
    '-', 0,
    22, "   enter wizard mode", /* ctrl-w */
    22, "v  toggle wizard verbose mode",
    22, "e  exit wizard mode",
    22, "r  random number check",
    22, "s  system statistics",
    22, "F  food statistics",
    22, "f  floor map",
    22, "m  see monster",
    22, "M  create monster",
    22, "c  create item",
    22, "i  inventory level",
    22, "I  identify item",
    22, "t  random teleport",
    22, "g  goto level",
    22, "C  charge item",
    22, "w  print worth of object",
    22, "o  improve stats and pack",
    0, 0
};
