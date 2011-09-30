/*
    player.c  -  This file contains functions for dealing with special player abilities
   
    Last Modified: Dec 29, 1990

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include "rogue.h"

/*
 * Pray to a deity
 *
 * 00-10 Good stuff happens
 * 11-40 A good deity answers
 * 41-60 Nothing happens
 * 61-90 A bad deity answers, but with good results
 * 91-99 You were better off before
 */
prayer()
{
    int chance, i, times;
    char    ch, num_str[20];
    struct linked_list  *item, *f_mons_a();
    struct thing    *tp;
    bool    got_one = FALSE;
    bool    is_godly;
    static int  times_prayed = 0;

    if (times_prayed > pstats.s_lvl) {
	msg("Are you sure you want to bother the gods?");
	ch = readchar();
	if (tolower(ch) != 'y') {
	    after = FALSE;
	    return;
	}
	else
	    msg("Here goes...");
    }
    msg("You are surrounded by orange smoke...");
    pstats.s_hpt = max_stats.s_hpt;
    pstats.s_power = max_stats.s_power;
    if (rnd(3) == 0)
	luck--;

    if (is_wearing(R_PIETY) || (rnd(luck) == 0 &&
	 (player.t_ctype == C_DRUID || player.t_ctype == C_CLERIC ||
	((player.t_ctype == C_PALADIN || player.t_ctype == C_RANGER)
	 && pstats.s_lvl > 8))))
	is_godly = ISBLESSED;
    else
	is_godly = ISNORMAL;

    if (is_wearing(R_PIETY))
	times_prayed += rnd(2);
    else
	times_prayed++;

    if (wizard) {
	msg("What roll?[0..%d] ", 99);
	ch = get_str(num_str, cw);
	if (ch == ESCAPE) {
	    msg("");
	    return;
	}
	chance = atoi(num_str);
    }
    else {
	chance = rnd(100) + roll(10, luck) - 5;
	if (times_prayed > pstats.s_lvl)
	    chance += 50;
	if (is_godly)
	    chance -= 50;
    }
    chance = max(0, min(chance, 100));

    if (chance == 0) {
	msg("The heavens open and glorious radiance surrounds you!");
	if (is_godly)
	    times = 8;
	else
	    times = 1;

	/*
	 * kill all monsters surrounding the hero except unique ones
	 * This will change when I implement the deity option.  If
	 * The deity is "stronger" than the unique monster, then the
	 * monster will be killed.
	 */
	for (i = 0; i < times; i++) {
	    item = f_mons_a(player.t_pos.y, player.t_pos.x, TRUE);
	    if (item) {
		tp = THINGPTR(item);
		msg("A bolt of eldritch energy strikes down the %s!",
		    monsters[tp->t_index].m_name);
		killed(NULL, item, NOMESSAGE, POINTS);
	    }
	}
    }
    else if (chance == 2) {
	msg("Aule, Lord of Crafts, hears your call.");
	read_scroll(&player, S_MAKEITEMEM, is_godly);
    }

    /* Save 3-9 for other wonderful stuff */
    else if (chance < 15) {
	msg("Orome, Lord of Forests, hears your call.");
	read_scroll(&player, S_SUMMON, is_godly);
    }
    else if (chance < 20) {
	msg("Hermes, the Winged Messenger, hears your call.");
	quaff(&player, P_HASTE, is_godly);
    }
    else if (chance < 25) {
	msg("Lorien, Master of Dreams, hears your call.");
	read_scroll(&player, S_SLEEP, is_godly);
    }
    else if (chance < 30) {
	msg("Este, Lady of Healing, hears your call.");
	quaff(&player, P_RESTORE, is_godly);
	quaff(&player, P_HEALING, is_godly);
    }
    else if (chance < 35) {
	msg("Thor, God of Thunder, hears your call.");
	msg("A bolt of lighting strikes you!");
	read_scroll(&player, S_ELECTRIFY, is_godly);
    }
    else if (chance < 40) {
	msg("Lorien, Master of Illusion, hears your call.");
	quaff(&player, P_DISGUISE, is_godly);
    }

    /*
     * Nothing happens
     */
    else if (chance < 60) {
	msg("Boccob, the Uncaring, ignores you.");
    }

    /*
     * You don't really want one of these gods answering your call
     */
    else if (chance < 65) {
	msg("Jubilex, Master of Slimes and Oozes, hears your call.");
	read_scroll(&player, S_HOLD, is_godly);
	luck++;
    }
    else if (chance < 70) {
	msg("Sauron, Lord of the Ring, hears your call.");
	quaff(&player, P_INVIS, is_godly);
	luck++;
    }
    else if (chance < 75) {
	msg("Orcus, Lord of Undead, hears your call.");
	quaff(&player, P_PHASE, is_godly);
	luck++;
    }
    else if (chance < 80) {
	msg("Incabulos, God of Evil Sendings, hears your call.");
	quaff(&player, P_CLEAR, is_godly);
	luck++;
    }
    else if (chance < 85) {
	msg("Raxivort, Night Flutterer, hears your call.");
	quaff(&player, P_SEEINVIS, is_godly);
	luck++;
    }
    else if (chance < 90) {
	msg("Morgoth, Lord of Fire, hears your call.");
	quaff(&player, P_FIRERESIST, is_godly);
	luck++;
    }

    /*
     * You are in for it now!
     */
    else if (chance < 100) {
	msg("You fall into a horrible trance-like state.");
	no_command += SLEEPTIME;
    }
    if (chance == 100) {
	msg("The heavens open - but wait!");
	msg("A bolt of eldritch energy strikes you!");
	if (pstats.s_hpt > 1)
	    pstats.s_hpt /= 2;
	msg("The gods must be angry with you.");
    }
}

/* Routines for thieves */

/*
 * gsense: Sense gold returns TRUE if gold was detected
 */

gsense()
{
    if (lvl_obj != NULL) {
	struct linked_list  *gitem;
	struct object   *cur;
	int gtotal = 0;

	wclear(hw);
	for (gitem = lvl_obj; gitem != NULL; gitem = next(gitem)) {
	    cur = OBJPTR(gitem);
	    if (cur->o_type == GOLD) {
		gtotal += cur->o_count;
		mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, GOLD);
	    }
	}
	if (gtotal) {
	    msg("You sense gold!");
	    overlay(hw, cw);
	    return (TRUE);
	}
    }
    nothing_message();
    return (FALSE);
}


/*
 * is_stealth: is player quiet about something
 */

bool
is_stealth(tp)
struct thing    *tp;
{
    return (rnd(25) < tp->t_stats.s_dext ||
	(tp == &player && is_wearing(R_STEALTH)));
}

/*
 * steal: Steal in direction given in delta
 */

steal()
{
    struct linked_list  *item;
    struct thing    *tp;
    char    *mname;
    coord   new_pos;
    short   thief_bonus;
    char    *unsuccess = "";
    char    *gain = "";
    char    *notice = "is not";

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES - 3 ||
	new_pos.x < 0 || new_pos.x > COLS - 1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("%so one to steal from.", (terse) ? "N" : "There is n");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL)
	return;
    tp = THINGPTR(item);
    mname = monsters[tp->t_index].m_name;

    /* Can player steal something unnoticed? */
    if (player.t_ctype == C_THIEF || player.t_ctype == C_NINJA)
	thief_bonus = 10;
    else
	thief_bonus = -50;
    if (rnd(50) >= 3 * pstats.s_dext + thief_bonus) {
	runto(&new_pos, &hero);
	turn_off(*tp, ISFRIENDLY);
	notice = "is";
    }
    if (rnd(100) <
	(thief_bonus + 2 * pstats.s_dext + 5 * pstats.s_lvl -
	 5 * (tp->t_stats.s_lvl - 3))) {
	struct linked_list  *s_item, *pack_ptr;
	short   count = 0;

	s_item = NULL;  /* Start stolen goods out as nothing */

	/* Find a good item to take */
	if (tp->t_pack != NULL) {
	    /* Count up the number of items in the monster's pack */
	    for (pack_ptr = tp->t_pack; pack_ptr != NULL;
		pack_ptr = next(pack_ptr))
		count++;

	    /* Pick one */
	    count = rnd(count);

	    /* Take it from the monster */
	    for (pack_ptr = tp->t_pack; --count == 0;
		pack_ptr = next(pack_ptr));
	    s_item = pack_ptr;
	    detach(tp->t_pack, s_item);

	    /* Give it to player */
	    if (add_pack(s_item, MESSAGE) == FALSE) {
		(OBJPTR(s_item))->o_pos = hero;
		fall(&player, s_item, TRUE);
	    }

	    /* Get points for stealing from unfriendly monsters */
	    if (off(*tp, ISFRIENDLY)) {
		if (player.t_ctype == C_THIEF)
		    pstats.s_exp += 2 * tp->t_stats.s_exp
			/ 3;
		else
		    pstats.s_exp += tp->t_stats.s_exp /
			min(pstats.s_lvl, 10);
		check_level();
	    }
	}
	else {
	    gain = " gains you nothing and";
	}
    }
    else {
	unsuccess = " unsuccessful";
    }
    msg("Your%s attempt%s %s noticed.", unsuccess, gain, notice);
}

/*
 * affect: cleric affecting undead
 */

affect()
{
    struct linked_list  *item;
    struct thing    *tp;
    char    *mname;
    coord   new_pos;
    bool    is_godly;
    int effective_level;

    if (player.t_ctype != C_CLERIC && player.t_ctype != C_PALADIN &&
	!is_wearing(R_PIETY)) {
	msg("Only clerics and paladins can affect undead.");
	return;
    }

    is_godly = (player.t_ctype == C_CLERIC || player.t_ctype == C_PALADIN);
    if (is_godly && is_wearing(R_PIETY))
	effective_level = 2 * pstats.s_lvl;
    else
	effective_level = pstats.s_lvl;

    new_pos.y = hero.y + delta.y;
    new_pos.x = hero.x + delta.x;

    /* Anything there? */
    if (new_pos.y < 0 || new_pos.y > LINES - 3 ||
	new_pos.x < 0 || new_pos.x > COLS - 1 ||
	mvwinch(mw, new_pos.y, new_pos.x) == ' ') {
	msg("Nothing to affect.");
	return;
    }

    if ((item = find_mons(new_pos.y, new_pos.x)) == NULL) {
	debug("Affect what @ %d,%d?", new_pos.y, new_pos.x);
	return;
    }
    tp = THINGPTR(item);
    mname = monsters[tp->t_index].m_name;

    if (off(*tp, ISUNDEAD)) {
	msg("Your holy symbol has no effect on the %s.", mname);
	goto annoy;
    }
    if (on(*tp, WASTURNED)) {
	msg("Your holy symbol merely enrages the %s.", mname);
	goto annoy;
    }

    /* Can cleric destroy it? */
    if (effective_level >= 3 * tp->t_stats.s_lvl) {
	msg("You have destroyed the %s.", mname);
	killed(&player, item, NOMESSAGE, POINTS);
	return;
    }

    /* Can cleric turn it? */
    if (rnd(100) + 1 >
	(100 * ((2 * tp->t_stats.s_lvl) - effective_level)) /
	    effective_level) {
	msg("You have turned the %s.", mname);
	turn_on(*tp, WASTURNED);    /* One turn per monster */
	turn_on(*tp, ISRUN);
	turn_on(*tp, ISFLEE);

	/* If monster was suffocating, stop it */
	if (on(*tp, DIDSUFFOCATE)) {
	    turn_off(*tp, DIDSUFFOCATE);
	    extinguish(suffocate);
	}

	/* If monster held us, stop it */
	if (on(*tp, DIDHOLD) && (--hold_count == 0))
	    turn_off(player, ISHELD);
	turn_off(*tp, DIDHOLD);
	return;
    }

    msg("The %s momentarily recoils from your holy symbol.", mname);
annoy:
    if (off(*tp, WASTURNED))
	runto(&new_pos, &hero);
}

/*
 * undead_sense: cleric or paladin finding the ungodly
 */

undead_sense()
{
    struct linked_list  *item;
    struct thing    *tp;
    bool    show = FALSE;

    wclear(hw);
    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	if (on(*tp, ISUNDEAD)) {
	    mvwaddch(hw, tp->t_pos.y, tp->t_pos.x, '&');
	    show = TRUE;
	}
    }
    if (show) {
	msg("You feel the presense of the ungodly.");
	overlay(hw, cw);
	wrefresh(cw);
	wclear(hw);
    }
}
