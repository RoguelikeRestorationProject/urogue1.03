/*
    magic.c  -  This file contains functions for casting magic spells

    Last Modified: Jan 5, 1991

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990, 1991 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include "rogue.h"
#include "death.h"

/* for printing out messages */
#define CAST_NORMAL 0x000   /* cast normal version */
#define CAST_CURSED 0x001   /* cast cursed version */
#define CAST_BLESSED    0x002   /* cast blessed version */
#define CAST_CROWN  0x010   /* crown helped out */
#define CAST_SEPTRE 0x020   /* septre helped out */

#define MAX_SPELLS  100 /* Max # sorted_spells */
#define MIN_FUMBLE_CHANCE   5
#define MAX_FUMBLE_CHANCE   95

struct spells {
    int sp_level;   /* level of casting spell */
    int sp_which;   /* which scroll or potion */
    int sp_flags;   /* scroll, blessed, known */
    int sp_cost;    /* generated in incant()  */
};

char    *spell_name();
char    *spell_abrev();
struct spells   *pick_monster_spell();

/*
 * Cost for each level of spells level:
 *          0  1   2   3   4   5    6    7    8
 */
int spell_cost[] = {1, 5, 17, 29, 53, 91, 159, 247, 396};

/*
 * Spells that a monster can cast
 */
#define     M_SELFTELEP 0
#define     M_OFFENSE   10
#define     M_HLNG2     1
#define     M_REGENERATE    2
#define     M_HLNG      3
#define     NUM_RUN     4
#define     M_HASTE     4
#define     M_SEEINVIS  5
#define     M_SHERO     6
#define     M_PHASE     7
#define     M_INVIS     8
#define     M_CANCEL    9

struct spells   monst_spells[] = {
    {5, S_SELFTELEP, SCR_MAGIC},
    {4, P_HEALING, POT_MAGIC | _TWO_},
    {3, P_REGENERATE, POT_MAGIC},
    {2, P_HEALING, POT_MAGIC},

    {4, P_HASTE, POT_MAGIC},
    {2, P_SEEINVIS, POT_MAGIC},
    {3, P_SHERO, POT_MAGIC},
    {5, P_PHASE, POT_MAGIC},

    {4, P_INVIS, POT_MAGIC},
    {4, WS_CANCEL, ZAP_MAGIC},

    /* In reverse order of damage ability */
    {6, WS_ELECT, ZAP_MAGIC | _TWO_},
    {6, WS_FIRE, ZAP_MAGIC | _TWO_},
    {6, WS_COLD, ZAP_MAGIC | _TWO_},
    {6, WS_MISSILE, ZAP_MAGIC | _TWO_},
    {5, WS_ELECT, ZAP_MAGIC},
    {5, WS_FIRE, ZAP_MAGIC},
    {5, WS_COLD, ZAP_MAGIC},
    {4, WS_ELECT, ZAP_MAGIC | ISCURSED},
    {4, WS_FIRE, ZAP_MAGIC | ISCURSED},
    {4, WS_COLD, ZAP_MAGIC | ISCURSED},
    {3, WS_MISSILE, ZAP_MAGIC},
    {1, WS_MISSILE, ZAP_MAGIC | ISCURSED},

    {-1, -1, -1}
};

/*
 * Spells that a player can cast Non-mus only know ISKNOW spells until found
 * in the dungeon Special classes know their spells one level lower, and
 * blessed one above
 */
struct spells   player_spells[] = {
    {1, WS_KNOCK, ZAP_MAGIC | ISKNOW},
    {1, S_SUMFAMILIAR, SCR_MAGIC | ISKNOW},
    {1, S_GFIND, SCR_MAGIC | ISKNOW},
    {1, P_MONSTDET, POT_MAGIC | ISKNOW | SP_DRUID},
    {1, P_TREASDET, POT_MAGIC | ISKNOW | SP_MAGIC},
    {1, S_FOODDET, SCR_MAGIC | ISKNOW | SP_CLERIC},
    {1, S_LIGHT, SCR_MAGIC | ISKNOW | SP_ILLUSION},

    {2, WS_CLOSE, ZAP_MAGIC | ISKNOW},
    {2, S_IDENTIFY, SCR_MAGIC | ISKNOW},
    {2, WS_HIT, ZAP_MAGIC | ISKNOW | SP_PRAYER},
    {2, P_SHIELD, POT_MAGIC | ISKNOW | SP_MAGIC},
    {2, P_COLDRESIST, POT_MAGIC | SP_WIZARD},
    {2, P_SEEINVIS, POT_MAGIC | SP_ILLUSION},
    {2, S_CONFUSE, SCR_MAGIC | SP_CLERIC},
    {2, P_SMELL, POT_MAGIC | SP_DRUID},
    {2, WS_MISSILE, ZAP_MAGIC | SP_MAGIC},
    {2, P_HEAR, POT_MAGIC},

    {3, P_CLEAR, POT_MAGIC | ISKNOW},
    {3, P_HEALING, POT_MAGIC | ISKNOW | SP_PRAYER},
    {3, S_CURING, SCR_MAGIC | ISKNOW | SP_PRAYER},
    {3, WS_MONSTELEP, ZAP_MAGIC | SP_MAGIC},
    {3, WS_CANCEL, ZAP_MAGIC | SP_WIZARD},
    {3, S_SELFTELEP, SCR_MAGIC | SP_WIZARD},
    {3, P_FIRERESIST, POT_MAGIC | SP_WIZARD | SP_DRUID},
    {3, S_MAP, SCR_MAGIC | SP_ILLUSION | SP_DRUID},
    {3, S_REMOVECURSE, SCR_MAGIC | SP_PRAYER},
    {3, S_HOLD, SCR_MAGIC | SP_CLERIC},
    {3, S_SLEEP, SCR_MAGIC | SP_DRUID},
    {3, P_HASOXYGEN, POT_MAGIC | SP_DRUID},
    {3, WS_XENOHEALING, ZAP_MAGIC | SP_DRUID},
    {3, P_RESTORE, POT_MAGIC},

    {4, S_MSHIELD, SCR_MAGIC | ISKNOW | SP_ILLUSION},
    {4, P_INVIS, POT_MAGIC | SP_ILLUSION},
    {4, S_REFLECT, SCR_MAGIC | SP_ILLUSION},
    {4, P_TRUESEE, POT_MAGIC | SP_ILLUSION},
    {4, P_REGENERATE, POT_MAGIC | SP_CLERIC},
    {4, WS_DRAIN, ZAP_MAGIC | SP_CLERIC},
    {4, P_HASTE, POT_MAGIC | SP_ILLUSION | SP_CLERIC},
    {4, P_LEVITATION, POT_MAGIC | SP_WIZARD | SP_DRUID},
    {4, WS_WEB, ZAP_MAGIC | SP_MAGIC},
    {4, P_PHASE, POT_MAGIC},

    {5, P_SHERO, POT_MAGIC | ISKNOW},
    {5, S_PETRIFY, SCR_MAGIC | SP_MAGIC},
    {5, S_SCARE, SCR_MAGIC | _TWO_ | SP_PRAYER},
    {5, WS_COLD, ZAP_MAGIC | SP_DRUID},
    {5, WS_FIRE, ZAP_MAGIC | SP_CLERIC},
    {5, WS_ELECT, ZAP_MAGIC | SP_WIZARD},
    {5, WS_ANTIMATTER, ZAP_MAGIC | SP_ILLUSION},
    {5, S_ELECTRIFY, SCR_MAGIC | SP_ILLUSION},

    {6, WS_DISINTEGRATE, ZAP_MAGIC | ISKNOW},
    {6, S_OWNERSHIP, SCR_MAGIC | SP_ALL},

    {7, S_ENCHANT, SCR_MAGIC | SP_MAGIC},

    /*
     * Too powerful
     *  { 6,    P_GAINABIL, POT_MAGIC|SP_PRAYER},
     *  { 6,    P_RAISELEVEL,   POT_MAGIC|SP_CLERIC},
     *  { 6,    S_GENOCIDE, SCR_MAGIC|SP_DRUID},
     *  { 6,    S_MAKEITEMEM,   SCR_MAGIC|SP_WIZARD},
     */

    {-1, -1, -1}
};

/*
 * Incantation: Cast a spell
 */
incant(caster, shoot_dir)
struct thing    *caster;
coord   shoot_dir;
{
    int i;
    struct stats    *curp;
    struct stats    *maxp;
    bool    is_player = (caster == &player);
    int points_casters;
    char    *casters_name = (on(player, ISBLIND)) ? "it" :
	    monsters[caster->t_index].m_name;
    struct spells   *sp;
    char    *cast_name;     /* = spell_name(sp) */
    char    *spell_type;        /* spell or prayer */
    int casting_cost;       /* from spell_cost[] */
    int spell_roll;     /* sucess/fail 1D100 die roll */
    int fumble_chance;      /* Spell fumble chance */
    int num_fumbles = 0;    /* for fumble_spell() */
    int bless_or_curse = ISNORMAL;  /* blessed or cursed? */
    int message_flags = CAST_NORMAL;    /* which message to print out */
    int class_casters;  /* For determining ISKNOW */
    int stat_casters;   /* s_intel or s_wisdom */
    int level_casters;  /* spellcasting level */
    char    buf[2 * LINELEN];

    curp = &(caster->t_stats);
    maxp = &(caster->maxstats);
    points_casters = curp->s_power;

    if (points_casters <= 0) {
	if (is_player)
	    msg("You don't have any spell points.");
	return;
    }

    /*
     * Paladins, Rangers, ringwearers, and monsters cast at 4 levels
     * below. Other non-specialists at 8 below
     */
    level_casters = curp->s_lvl;
    switch (caster->t_ctype) {
	when    C_PALADIN:
	    level_casters -= 4;
	case C_CLERIC:
	    class_casters = SP_CLERIC;
	    stat_casters = curp->s_wisdom;
	when    C_RANGER:
	    level_casters -= 4;
	case C_DRUID:
	    class_casters = SP_DRUID;
	    stat_casters = curp->s_wisdom;
	when    C_MAGICIAN:
	    class_casters = SP_WIZARD;
	    stat_casters = curp->s_intel;
	when    C_ILLUSION:
	    class_casters = SP_ILLUSION;
	    stat_casters = curp->s_intel;
	when    C_MONSTER:
	    if (off(*caster, ISUNIQUE))
		level_casters -= 4;
	    class_casters = 0x0;
	    stat_casters = curp->s_intel;
	otherwise:
	    if (is_wearing(R_WIZARD)) {
		level_casters -= 4;
		class_casters = (rnd(4) ? SP_WIZARD :
		    SP_ILLUSION);
		stat_casters = curp->s_intel;
	    }
	    else if (is_wearing(R_PIETY)) {
		level_casters -= 4;
		class_casters = (rnd(4) ? SP_CLERIC : SP_DRUID);
		stat_casters = curp->s_wisdom;
	    }
	    else {
		level_casters -= 8;
		class_casters = 0x0;
		stat_casters = (rnd(2) ? curp->s_wisdom :
		    curp->s_intel);
	    }
    }
    /* Bug - What about when WIS == INT? */
    spell_type = (stat_casters == curp->s_intel) ? "spell" : "prayer";

    if (!is_player && (sp = pick_monster_spell(caster)) == NULL)
	    return;
    else {
	int num_spells = -1;    /* num of spells cheap enough */
	int sort_spells();  /* for qsort() */
	static struct spells    sorted_spells[MAX_SPELLS];

	sorted_spells[0].sp_cost = -1;
	for (sp = player_spells; sp->sp_level != -1; sp++) {
	    if (sp->sp_flags & class_casters) { /* Does class know
				 * spell? */
		int rnd_number = rnd(2 * sp->sp_level) -
			sp->sp_level;

		/* Knows normal spell one level below others */
		casting_cost = spell_cost[sp->sp_level - 1] +
		    rnd_number;
		if (points_casters >= casting_cost) {
		    sorted_spells[++num_spells] = *sp;
		    sorted_spells[num_spells].sp_cost = casting_cost;
		    sorted_spells[num_spells].sp_level = sp->sp_level - 1;
		}

		/* Knows blessed spell one level above others */
		casting_cost = spell_cost[sp->sp_level + 1] +
		    rnd_number;
		if (points_casters >= casting_cost) {
		    sorted_spells[++num_spells] = *sp;
		    sorted_spells[num_spells].sp_level = sp->sp_level + 1;
		    sorted_spells[num_spells].sp_cost = casting_cost;
		    sorted_spells[num_spells].sp_flags |= ISBLESSED;
		}
	    }

	    /*
	     * If class doesn't know spell, see if its a ISKNOW
	     */
	    else if (sp->sp_flags & ISKNOW) {
		int rnd_number = rnd(4 * sp->sp_level) -
			sp->sp_level;

		casting_cost = spell_cost[sp->sp_level] +
			rnd_number;
		if (points_casters >= casting_cost) {
		    sorted_spells[++num_spells] = *sp;
		    sorted_spells[num_spells].sp_cost = casting_cost;
		}
	    }
	    /* else this spell is unknown */
	}

	if (sorted_spells[0].sp_cost == -1) {
	    msg("You don't have enough %s points.", spell_type);
	    after = FALSE;
	    return;
	}
	qsort(sorted_spells, num_spells + 1, sizeof(struct spells),
	    sort_spells);

	do {        /* Prompt for spells */
	    struct spells   *which_spell = NULL;

	    buf[0] = '\0';
	    msg("");/* Get rid of --More-- */
	    msg("Which %s are you casting [%d points left] (* for list)? ",
		spell_type, points_casters);

	    switch (get_str(buf, cw)) {
		when NORM:
		when QUIT:  /* ESC - lose turn */
		    return;
		otherwise:
		    continue;
	    }
	    if (buf[0] == '*') {    /* print list */
		add_line("Cost  Abbreviation    Full Name");
		for (i = 0; i <= num_spells; i++) {
		    sp = &sorted_spells[i];
		    sprintf(buf, "[%3d] %-12s\t%s",
			sp->sp_cost, spell_abrev(sp),
			spell_name(sp));
		    add_line(buf);
		}
		end_line();
		sp = NULL;
		continue;
	    }
	    if (isupper((int) buf[0])) {    /* Uppercase
			     * Abbreviation */
		for (i = 0; i <= num_spells; i++) {
		    sp = &sorted_spells[i];
		    if ((strcmp(spell_abrev(sp), buf) == 0)) {
			which_spell = sp;
			break;
		    }
		}
	    }
	    else {  /* Full Spell Name */
		for (i = 0; i <= num_spells; i++) {
		    sp = &sorted_spells[i];
		    if ((strcmp(spell_name(sp), buf) == 0)) {
			which_spell = sp;
			break;
		    }
		}
	    }
	    sp = which_spell;

	} while (sp == NULL);

    }

    /*
     * Common monster and player code
     */
    cast_name = spell_name(sp);

    fumble_chance = (10 * sp->sp_level / 4 - 10 * level_casters / 13) * 5;
    if (cur_weapon != NULL && wield_ok(caster, cur_weapon, FALSE) == FALSE) {
	switch (caster->t_ctype) {
	    when    C_MAGICIAN:
	    case C_ILLUSION:
		msg("You should have both hands free.");
		fumble_chance += rnd(level_casters) * 5;
	    when    C_CLERIC:
	    case C_DRUID:
	    case C_PALADIN:
		msg("Your god looks askance at the weapon you wield.");
		fumble_chance += rnd(level_casters) * 5;
	    otherwise:
		break;
	}
    }

    if (fumble_chance >= MAX_FUMBLE_CHANCE)
	fumble_chance = MAX_FUMBLE_CHANCE;
    else if (fumble_chance <= MIN_FUMBLE_CHANCE + sp->sp_level)
	fumble_chance = MIN_FUMBLE_CHANCE + sp->sp_level;

    if (fumble_chance > (30 + rnd(50))) {
	if (is_player) {
	    char    answer;

	    msg("Are you sure you want to try for that hard a %s? [n]",
		spell_type);
	    answer = readchar();
	    if (tolower(answer) != 'y') {
		after = FALSE;
		return;
	    }
	    else
		msg("Here goes...");
	}
	else {      /* Only if the monster is desperate */
	    if (curp->s_hpt > maxp->s_hpt / 2)
		return;
	}
    }

    spell_roll = rnd(100);

    debug("%s(%d) cast '%s' fumble %%%d (rolled %d) ",
	  monsters[caster->t_index].m_name, curp->s_power, cast_name,
	  fumble_chance, spell_roll);
    caster->t_rest_hpt = caster->t_rest_pow = 0;
    if (!is_player) {   /* Stop running. */
	running = FALSE;
	msg("The %s is casting '%s'.", casters_name, cast_name);
    }

    /* The Crown of Might insures that your spells never fumble */
    if (spell_roll < fumble_chance) {
	if (is_carrying(TR_CROWN))
	    message_flags |= CAST_CROWN;
	else {
	    message_flags |= CAST_CURSED;

	    curp->s_power -= min(curp->s_power,
		(2 * sp->sp_cost)); /* 2x cost */
	    num_fumbles = rnd(((fumble_chance - spell_roll) / 10)
		 + 1) + rnd(sp->sp_level) + rnd(curp->s_lvl);
	    num_fumbles = min(10, max(0, num_fumbles));

	    if (num_fumbles >= 6 && rnd(1) == 0)
		bless_or_curse = ISCURSED;
	    else if (num_fumbles < 4) {
		if (is_player)
		    msg("Your %s fails.", spell_type);
		return;
	    }

	}
    }
    else if (spell_roll > MAX_FUMBLE_CHANCE) {
	if (is_player) {
	    message_flags |= CAST_BLESSED;
	    pstats.s_exp += 3 * sp->sp_cost * curp->s_lvl;
	    check_level();
	}
	maxp->s_power += sp->sp_cost;
	bless_or_curse = ISBLESSED;
    }
    else {
	if (is_player) {/* extra exp for sucessful spells */
	    if (player.t_ctype == C_MAGICIAN
		|| player.t_ctype == C_ILLUSION) {
		pstats.s_exp += sp->sp_cost * curp->s_lvl;
		check_level();
	    }

	}
	bless_or_curse = sp->sp_flags & ISBLESSED;
	curp->s_power -= sp->sp_cost;
    }

    /* The Sceptre of Might blesses all your spells */
    if (is_player && (bless_or_curse & ISBLESSED == 0) &&
	is_carrying(TR_SCEPTRE)) {
	message_flags |= CAST_SEPTRE;
	bless_or_curse = ISBLESSED;
    }

    if (sp->sp_flags & POT_MAGIC)
	quaff(caster, sp->sp_which, bless_or_curse);
    else if (sp->sp_flags & SCR_MAGIC)
	    read_scroll(caster, sp->sp_which, bless_or_curse);
    else if (sp->sp_flags & ZAP_MAGIC) {
	if (is_player) {
	    do {    /* Must pick a direction */
		msg("Which direction?");
	    } while (get_dir() == FALSE);
	}
	else {
	    delta.x = shoot_dir.x;
	    delta.y = shoot_dir.y;
	}
	do_zap(caster, sp->sp_which, bless_or_curse);
    }
    else
	msg("What a strange %s!", spell_type);

    /*
     * Print messages and take fumbles *after* spell has gone off. This
     * makes ENCHANT, etc more dangerous
     */
    if (is_player) {
	if (message_flags & CAST_SEPTRE)
	    msg("The Sceptre enhanced your %s.", spell_type);
	if (message_flags & CAST_CROWN)
	    msg("The Crown wordlessly corrected your %s.",
		spell_type);
	switch (message_flags & 01) {
	    when    CAST_CURSED:
		msg("You botched your '%s' %s.", cast_name,
		    spell_type);
		fumble_spell(caster, num_fumbles);
	    when    CAST_NORMAL:
		msg("You sucessfully cast your '%s' %s.",
		    cast_name, spell_type);
	    when    CAST_BLESSED:
		msg("Your '%s' %s went superbly.", cast_name,
		    spell_type);
	}
    }
}

/*
 * spell_name - returns pointer to spell name
 */
char    *
spell_name(sp)
struct spells   *sp;
{
    static char spellname[2 * BUFSIZ];
    char    *ret_val;

    if (sp->sp_flags & POT_MAGIC)
	ret_val = strcpy(spellname, p_magic[sp->sp_which].mi_name);
    else if (sp->sp_flags & SCR_MAGIC)
	ret_val = strcpy(spellname, s_magic[sp->sp_which].mi_name);
    else if (sp->sp_flags & ZAP_MAGIC)
	ret_val = strcpy(spellname, ws_magic[sp->sp_which].mi_name);
    else
	ret_val = strcpy(spellname, "unknown spell type");
    if (sp->sp_flags & ISBLESSED)
	strcat(spellname, " 2");
    return (spellname);
}

/*
 * spell_abrev - returns pointer to capital letter spell abbreviation
 */
char    *
spell_abrev(sp)
struct spells   *sp;
{
    static char spellname[2 * BUFSIZ];
    char    *ret_val;

    if (sp->sp_flags & POT_MAGIC)
	ret_val = strcpy(spellname, p_magic[sp->sp_which].mi_abrev);
    else if (sp->sp_flags & SCR_MAGIC)
	ret_val = strcpy(spellname, s_magic[sp->sp_which].mi_abrev);
    else if (sp->sp_flags & ZAP_MAGIC)
	ret_val = strcpy(spellname, ws_magic[sp->sp_which].mi_abrev);
    else
	ret_val = strcpy(spellname, "?????");
    if (sp->sp_flags & ISBLESSED)
	strcat(spellname, " 2");
    return (spellname);
}

/*
 * fumble_spell - he blew it.  Make him pay
 */
fumble_spell(caster, num_fumbles)
struct thing    *caster;        /* the fumblee */
int num_fumbles;        /* high means you fumbled real bad */
{
    struct stats    *curp = &(caster->t_stats);
    struct stats    *maxp = &(caster->maxstats);
    bool    is_player = (caster == &player);

    debug("Taking %d fumbles.", num_fumbles);
    switch (num_fumbles) {
    case 10:
	/* Lose ability */
	if (rnd(5) == 0)
	    quaff(caster, P_GAINABIL, ISCURSED);
	break;
    case 9:
	/* Lose max spell points */
	if (rnd(4) == 0) {
	    maxp->s_power -= rnd(10);
	    if (maxp->s_power <= 5)
		maxp->s_power = 5;
	}
	break;
    case 8:
	/* Lose all current spell points */
	if (rnd(3) == 0)
	    curp->s_power = 0;
	else
	    curp->s_power /= 2;
	break;
    case 7:
	/* Freeze */
	if (rnd(2) == 0) {
	    if (is_player)
		no_command++;
	    else
		caster->t_no_move++;
	}
	break;
    case 6:
	break;
	/* Cast a cursed spell - see below */
    case 5:
	/* Become dazed and confused */
	if (rnd(5) == 0)
	    quaff(caster, P_CLEAR, ISCURSED);
	break;
    case 4:
	/* Lose hit points */
	if (is_player)
	    feel_message();
	if ((curp->s_hpt -= rnd(10)) <= 0) {
	    if (is_player)
		death(D_SPELLFUMBLE);
	    else
		killed(caster, find_mons(caster->t_pos.y, caster->t_pos.x),
		       NOMESSAGE, NOPOINTS);
	    return;
	}
	break;
    case 3:
	break;
	/* Spell fails */
    case 2:
	/* Freeze */
	if (is_player)
	    no_command++;
	else
	    caster->t_no_move++;
	break;
    default:
    case 1:
	/* Take double spell points - handled in incant() */
	break;
    }
}

/*
 * learn_new_spells - go through player_spells and ISKNOW identified potions,
 * scrolls, and sticks
 */
learn_new_spells()
{
    struct spells   *sp;
    int kludge;

    for (sp = player_spells; sp->sp_level != -1; sp++) {
	if (sp->sp_flags & POT_MAGIC)
	    kludge = TYP_POTION;
	else if (sp->sp_flags & SCR_MAGIC)
	    kludge = TYP_SCROLL;
	else if (sp->sp_flags & ZAP_MAGIC)
	    kludge = TYP_STICK;
	if (know_items[kludge][sp->sp_which]) {
	    if ((sp->sp_flags & ISKNOW) == FALSE)
		debug("Learned new spell '%s'", spell_name(sp));
	    sp->sp_flags |= ISKNOW;
	}
    }
}

/*
 * pick_monster_spell - decide which spell from monst_spells will be cast
 * returns pointer to spell in monst_spells
 */
struct spells   *
pick_monster_spell(caster)
struct thing    *caster;
{
    struct spells   *sp = NULL;
    struct stats    *curp = &(caster->t_stats);
    int level_casters = curp->s_lvl;
    int stat_casters = curp->s_intel;
    int points_casters = curp->s_power;

    /* Discover castable spells */
    for (sp = monst_spells; sp->sp_level != -1; sp++) {
	int rnd_number = rnd(2 * sp->sp_level) - sp->sp_level;
	int casting_cost = spell_cost[sp->sp_level] + rnd_number;

	if (points_casters >= casting_cost)
	    sp->sp_flags |= ISKNOW;
    }

    /*
     * Decide which spell to cast
     */
    if (curp->s_hpt < rnd(caster->maxstats.s_hpt)) { /* think defense */
	int i;
	static int  run_or_heal[NUM_RUN] =
	{M_SELFTELEP, M_HLNG2, M_HLNG, M_REGENERATE};

	for (i = 0; i < NUM_RUN; i++) {
	    sp = &monst_spells[run_or_heal[i]];
	    if ((sp->sp_flags & ISKNOW) && rnd(1))
		return (sp);
	}
    }

    if (on(*caster, ISSLOW)) {  /* cancel a slow */
	sp = &monst_spells[M_HASTE];
	if (sp->sp_flags & ISKNOW)
	    return (sp);
    }

    if (on(*caster, ISFLEE)) {  /* stop running away */
	sp = &monst_spells[M_SHERO];
	if (sp->sp_flags & ISKNOW)
	    return (sp);
    }

    if (on(player, ISINVIS) || on(player, ISDISGUISE)) {
	if (off(*caster, CANSEE)) { /* look for him */
	    sp = &monst_spells[M_SEEINVIS];
	    if (sp->sp_flags & ISKNOW)
		return (sp);
	}
	else if (off(*caster, ISINVIS)) {   /* go invisible */
	    sp = &monst_spells[M_INVIS];
	    if (sp->sp_flags & ISKNOW)
		return (sp);
	}
    }

    if (on(player, CANINWALL) && (off(*caster, CANINWALL)) &&
	(rnd(5) == 0)) {
	sp = &monst_spells[M_PHASE];
	if (sp->sp_flags & ISKNOW)
	    return (sp);
    }

    if (rnd(5) == 0 && has_defensive_spell(player)) {
	sp = &monst_spells[M_CANCEL];
	if (sp->sp_flags & ISKNOW)
	    return (sp);
    }

    /* Cast an offensive spell */
    for (sp = &monst_spells[M_OFFENSE]; sp->sp_level != 1; sp++) {
	if ((rnd(3) == 0) && (sp->sp_flags & ISKNOW))
	    return (sp);
	if ((rnd(3) == 0) && (sp->sp_flags & ISKNOW)) {
	    if (sp->sp_which != WS_MISSILE &&
		DISTANCE(caster->t_dest.y, caster->t_dest.x,
		hero.y, hero.x) > BOLT_LENGTH)
		continue;
	    else
		return (sp);
	}
    }
    return (NULL);
}

/*
 * sort_spells -    called by qsort()
 */
sort_spells(sp1, sp2)
struct spells   *sp1, *sp2;
{
    int diff = sp1->sp_cost - sp2->sp_cost;

    if (diff != 0)
	return (diff);
    else
	return (strcmp(spell_name(sp1), spell_name(sp1)));
}
