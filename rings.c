/*
    rings.c  -  routines dealing specifically with rings
   
    Last Modified: Dec 29, 1990

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
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

ring_on()
{
    struct object   *obj;
    struct linked_list  *item;
    int ring;
    char    buf[2 * LINELEN];

    if ((item = get_item("put on", RING)) == NULL)
	return;
    obj = OBJPTR(item);
    if (obj->o_type != RING) {
	msg("You can't put that on!");
	return;
    }

    /*
     * find out which hand to put it on
     */
    if (is_current(obj)) {
	msg("Already wearing that!");
	return;
    }

    if (cur_ring[LEFT_1] == NULL)
	ring = LEFT_1;
    else if (cur_ring[LEFT_2] == NULL)
	ring = LEFT_2;
    else if (cur_ring[LEFT_3] == NULL)
	ring = LEFT_3;
    else if (cur_ring[LEFT_4] == NULL)
	ring = LEFT_4;
    else if (cur_ring[LEFT_5] == NULL)
	ring = LEFT_5;
    else if (cur_ring[RIGHT_1] == NULL)
	ring = RIGHT_1;
    else if (cur_ring[RIGHT_2] == NULL)
	ring = RIGHT_2;
    else if (cur_ring[RIGHT_3] == NULL)
	ring = RIGHT_3;
    else if (cur_ring[RIGHT_4] == NULL)
	ring = RIGHT_4;
    else if (cur_ring[RIGHT_5] == NULL)
	ring = RIGHT_5;
    else {
	if (terse)
	    msg("Wearing enough rings.");
	else
	    msg("You already have on eight rings.");
	return;
    }
    cur_ring[ring] = obj;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which) {
	when    R_ADDSTR:
	    pstats.s_str += obj->o_ac;
	when    R_ADDHIT:
	    pstats.s_dext += obj->o_ac;
	when    R_ADDINTEL:
	    pstats.s_intel += obj->o_ac;
	when    R_ADDWISDOM:
	    pstats.s_wisdom += obj->o_ac;
	when    R_FREEDOM:
	    turn_off(player, ISHELD);
	    hold_count = 0;
	when    R_TRUESEE:
	    if (off(player, PERMBLIND)) {
		turn_on(player, CANTRUESEE);
		msg("You become more aware of your surroundings.");
		sight();
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when    R_SEEINVIS:
	    if (off(player, PERMBLIND)) {
		turn_on(player, CANTRUESEE);
		msg("Your eyes begin to tingle.");
		sight();
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
	when    R_AGGR:
	    aggravate();
	when    R_CARRYING:
	    updpack();
	when    R_LEVITATION:
	    msg("You begin to float in the air!");
	when    R_LIGHT:
	    if (roomin(&hero) != NULL) {
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    }
    }

    status(FALSE);
    if (know_items[TYP_RING][obj->o_which] &&
	guess_items[TYP_RING][obj->o_which]) {
	free(guess_items[TYP_RING][obj->o_which]);
	guess_items[TYP_RING][obj->o_which] = NULL;
    }
    else if (!know_items[TYP_RING][obj->o_which] &&
	 askme &&
	 (obj->o_flags & ISKNOW) == 0 &&
	 guess_items[TYP_RING][obj->o_which] == NULL) {
	mpos = 0;
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (get_str(buf, cw) == NORM) {
	    guess_items[TYP_RING][obj->o_which] =
		new((unsigned int) strlen(buf) + 1);
	    strcpy(guess_items[TYP_RING][obj->o_which], buf);
	}
	msg("");
    }
}

ring_off()
{
    struct object   *obj;
    struct linked_list  *item;

    if (cur_ring[LEFT_1] == NULL && cur_ring[LEFT_2] == NULL &&
	cur_ring[LEFT_3] == NULL && cur_ring[LEFT_4] == NULL &&
	cur_ring[LEFT_5] == NULL &&
	cur_ring[RIGHT_1] == NULL && cur_ring[RIGHT_2] == NULL &&
	cur_ring[RIGHT_3] == NULL && cur_ring[RIGHT_4] == NULL &&
	cur_ring[RIGHT_5] == NULL) {
	if (terse)
	    msg("No rings.");
	else
	    msg("You aren't wearing any rings.");
	return;
    }
    else if ((item = get_item("remove", RING)) == NULL)
	return;
    mpos = 0;
    obj = OBJPTR(item);
    if ((obj = OBJPTR(item)) == NULL)
	msg("%s wearing that!", terse ? "Not" : "You are not");
    if (dropcheck(obj)) {
	switch (obj->o_which) {
	    when    R_SEEINVIS:
		msg("Your eyes stop tingling.");
	    when    R_CARRYING:
		updpack();
	    when    R_LEVITATION:
		msg("You float gently to the ground.");
	    when    R_LIGHT:
		if (roomin(&hero) != NULL) {
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    when    R_TRUESEE:
		msg("Your sensory perceptions return to normal.");
	}
	msg("Was wearing %s.", inv_name(obj, LOWERCASE));
    }
}

/*
 * how much food does this ring use up?
 */
ring_eat(hand)
int hand;
{
    int ret_val = 0;
    int ac;

    if (cur_ring[hand] != NULL) {
	switch (cur_ring[hand]->o_which) {
	    case R_REGEN:
	    case R_VREGEN:
		ret_val = rnd(pstats.s_lvl > 10 ? 10
		    : pstats.s_lvl);
	    case R_ALERT:
	    case R_SUSABILITY:
		return 1;
	    case R_DIGEST:
		ac = cur_ring[hand]->o_ac;
		if (ac < 0 && rnd(1 - (ac / 3)) == 0)
		    ret_val = -ac + 1;
		else if (rnd((ac / 2) + 2) == 0)
		    ret_val = -1 - ac;
	    case R_SEARCH:
		ret_val = rnd(100) < 33;
	}
    }
    ret_val += rnd(luck);
    return (ret_val);
}

/*
 * print ring bonuses
 */
char    *
ring_num(obj)
struct object   *obj;
{
    static char buf[10];

    buf[0] = '\0';
    if (obj->o_flags & ISKNOW) {
	switch (obj->o_which) {
	    when    R_PROTECT:
	    case R_ADDSTR:
	    case R_ADDDAM:
	    case R_ADDHIT:
	    case R_ADDINTEL:
	    case R_ADDWISDOM:
	    case R_CARRYING:
	    case R_VREGEN:
	    case R_RESURRECT:
	    case R_TELCONTROL:
	    case R_REGEN:
	    case R_PIETY:
	    case R_WIZARD:
		buf[0] = ' ';
		strcpy(&buf[1], num(obj->o_ac, 0));
	    when    R_DIGEST:
		buf[0] = ' ';
		strcpy(&buf[1], num(obj->o_ac < 0 ?
		obj->o_ac : obj->o_ac - 1, 0));
	    otherwise:
		if (obj->o_flags & ISCURSED)
		    strcpy(buf, " cursed");
	}
    }
    return (buf);
}

/*
 * ring_value -     Return the effect of the specified ring
 */
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
ring_value(type)
int type;
{
    int result = 0;

    if (ISRING(LEFT_1, type))
	result += cur_ring[LEFT_1]->o_ac;
    if (ISRING(LEFT_2, type))
	result += cur_ring[LEFT_2]->o_ac;
    if (ISRING(LEFT_3, type))
	result += cur_ring[LEFT_3]->o_ac;
    if (ISRING(LEFT_4, type))
	result += cur_ring[LEFT_4]->o_ac;
    if (ISRING(LEFT_5, type))
	result += cur_ring[LEFT_5]->o_ac;
    if (ISRING(RIGHT_1, type))
	result += cur_ring[RIGHT_1]->o_ac;
    if (ISRING(RIGHT_2, type))
	result += cur_ring[RIGHT_2]->o_ac;
    if (ISRING(RIGHT_3, type))
	result += cur_ring[RIGHT_3]->o_ac;
    if (ISRING(RIGHT_4, type))
	result += cur_ring[RIGHT_4]->o_ac;
    if (ISRING(RIGHT_5, type))
	result += cur_ring[RIGHT_5]->o_ac;
    return (result);
}
