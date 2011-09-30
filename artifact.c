/*
    artifact.c  -  This file contains functions for dealing with artifacts

    Last Modified: Dec 28, 1990   

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include "rogue.h"
#include "death.h"

/*
    apply()
	apply an artifact
*/

int
apply()
{
struct linked_list  *item;
struct object   *obj;
int which;
int chance;

if ((item = get_item("activate", ARTIFACT)) == NULL)
    return;
obj = OBJPTR(item);
which = obj->o_which;
if (!(obj->ar_flags & ISACTIVE)) {
    chance = rnd(100) - 10 * rnd(luck);
    debug("Rolled %d.", chance);
    if (chance < 5)
	do_major();
    else if (chance < 50)
	do_minor(obj);
    else
	obj->ar_flags |= ISACTIVE;
}
if (obj->ar_flags & ISACTIVE) {
    switch (which) {
	when TR_PURSE:
	    do_bag(obj);
	when TR_PHIAL:
	    do_phial();
	    when TR_AMULET:
		do_amulet();
	    when TR_PALANTIR:
		do_palantir();
	    when TR_CROWN:
		do_crown();
	    when TR_SCEPTRE:
		do_sceptre();
	    when TR_SILMARIL:
		do_silmaril();
	    when TR_WAND:
		do_wand();
	    otherwise:
		nothing_message(ISCURSED);
		return;
	}
    }
    if (rnd(pstats.s_lvl) < 6)
	do_minor(obj);
    turn_on(player, POWEREAT);
}

/*
 * was the hero carrying a particular artifact
 */

bool
possessed(artifact)
int artifact;
{
    if ((picked_artifact >> artifact) & 1)
	return TRUE;
    else
	return FALSE;
}

/*
 * is the hero carrying a particular artifact
 */

bool
is_carrying(artifact)
int artifact;
{
    if ((has_artifact >> artifact) & 1)
	return TRUE;
    else
	return FALSE;
}

/*
 * is it time to make a new artifact?
 */
bool
make_artifact()
{
    int i;

    mpos = 0;
    debug("Artifact possession and picked flags : %x %x.",
	  has_artifact, picked_artifact);
    for (i = 0; i < maxartifact; i++) {
	if (!possessed(i) && arts[i].ar_level <= level)
	    return TRUE;
    }
    return FALSE;
}

/*
 * make a specified artifact
 */
struct object   *
new_artifact(which, cur)
int which;
struct object   *cur;
{
    if (which >= maxartifact) {
	debug("Bad artifact %d.  Random one created.", which);
	which = rnd(maxartifact);
    }
    if (which < 0) {
	for (which = 0; which < maxartifact; which++)
	    if (!possessed(which) && arts[which].ar_level <= level)
		break;
    }
    debug("Artifact number: %d.", which);
    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_mark[0] = '\0';
    cur->o_type = ARTIFACT;
    cur->o_which = which;
    cur->o_weight = arts[which].ar_weight;
    cur->o_flags = 0;
    cur->o_group = 0;
    cur->o_count = 1;
    cur->o_bag = NULL;
    cur->ar_flags = 0;
    return;
}

/*
 * do_minor: side effects and minor malevolent effects of artifacts
 */
do_minor(tr)
struct object   *tr;
{
    int which;

    which = rnd(115);
    debug("Rolled %d.", which);
    switch (which) {
	when 0: seemsg("You develop some acne on your face.");
	when 1:
	    if (on(player, CANSCENT)) {
		msg("A sudden whiff of BO causes you to faint.");
		no_command = STONETIME;
	    }
	    else if (off(player, ISUNSMELL))
		msg("You begin to smell funny.");
	when 2: seemsg("A wart grows on the end of your nose.");
	when 3: hearmsg("Your hear strange noises in the distance.");
	when 4: hearmsg("You hear shuffling in the distance.");
	when 5: hearmsg("You hear clanking in the distance.");
	when 6: hearmsg("You hear water dripping onto the floor.");
	when 7: hearmsg("The dungeon goes strangely silent.");
	when 8: msg("You suddenly feel very warm.");
	when 9: msg("You feel very hot.");
	when 10:    msg("A blast of heat hits you.");
	when 11:    {
	    struct room *rp;
	    static coord    fpos;

	    if (off(player, ISBLIND))
		msg("A pillar of flame leaps up beside you.");
	    else
		msg("You feel something very hot nearby.");

	    if (ntraps + 1 < 2 * MAXTRAPS &&
		fallpos(&hero, &fpos, TRUE)) {
		mvaddch(fpos.y, fpos.x, FIRETRAP);
		traps[ntraps].tr_type = FIRETRAP;
		traps[ntraps].tr_flags = ISFOUND;
		traps[ntraps].tr_show = FIRETRAP;
		traps[ntraps].tr_pos.y = fpos.y;
		traps[ntraps++].tr_pos.x = fpos.x;
		if ((rp = roomin(&hero)) != NULL) {
		    rp->r_flags &= ~ISDARK;
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	}
	when 12: msg("You feel a blast of hot air.");
	when 13: msg("You feel very cold.");
	when 14: msg("You break out in a cold sweat.");
	when 15:
	    if (off(player, ISBLIND) && cur_armor == NULL)
		msg("You are covered with frost.");
	    else if (off(player, ISBLIND))
		msg("Your armor is covered with frost.");
	    else if (cur_armor == NULL)
		msg("Your body feels very cold and you begin to shiver.");
	    else
		msg("Your armor feels very cold.  You hear cracking ice.");
	when 16: msg("A cold wind whistles through the dungeon.");
	when 17:    {
	    int change;

	    change = 18 - pstats.s_str;
	    chg_str(change, TRUE, FALSE);
	    chg_dext(-change, TRUE, FALSE);
	    if (change > 0)
		msg("You feel stronger and clumsier now.");
	    else if (change < 0)
		msg("You feel weaker and more dextrous now.");
	    else
		nothing_message(ISCURSED);
	}
	when 18: msg("You begin to itch all over.");
	when 19: msg("You begin to feel hot and itchy.");
	when 20:
	    msg("You feel a burning itch.");
	    chg_dext(-1, FALSE, TRUE);
	    if (off(player, HASITCH)) {
		turn_on(player, HASITCH);
		fuse(un_itch, 0, roll(4, 6), AFTER);
	    }
	    else
		lengthen(un_itch, roll(4, 6));
	when 21:
	    if (off(player, ISBLIND))
		msg("Your skin begins to flake and peel.");
	    else
		msg("You feel an urge to scratch an itch.");
	when 22: seemsg("Your hair begins to turn grey.");
	when 23: seemsg("Your hair begins to turn white.");
	when 24: seemsg("Some of your hair instantly turns white.");
	when 25: seemsg("You are covered with long white hair.");
	when 26: seemsg("You are covered with long red hair.");
	when 27: msg("You grow a beard.");
	when 28: msg("Your hair falls out.");
	when 29: msg("You feel a burning down below.");
	when 30: msg("Your toes fall off.");
	when 31: msg("You grow some extra toes.");
	when 32: msg("You grow some extra fingers.");
	when 33: msg("You grow an extra thumb.");
	when 34: msg("Your nose falls off.");
	when 35: msg("Your nose gets bigger.");
	when 36: msg("Your nose shrinks.");
	when 37: msg("An eye grows on your forehead.");
	when 38: seemsg("You see beady eyes watching from a distance.");
	when 39: msg("The dungeon rumbles for a moment.");
	when 40: seemsg("A flower grows on the floor next to you.");
	when 41:
	    msg("You are stunned by a psionic blast.");
	    if (on(player, ISHUH))
		lengthen(unconfuse, rnd(40) + (HUHDURATION * 3));
	    else {
		fuse(unconfuse, 0, rnd(40) + (HUHDURATION * 3), AFTER);
		turn_on(player, ISHUH);
	    }
	when 42:
	    msg("You are confused by thousands of voices in your head.");
	    if (on(player, ISHUH))
		lengthen(unconfuse, rnd(10) + (HUHDURATION * 2));
	    else {
		fuse(unconfuse, 0, rnd(10) + (HUHDURATION * 2), AFTER);
		turn_on(player, ISHUH);
	    }
	when 43: hearmsg("You hear voices in the distance.");
	when 44:
	    msg("You feel a strange pull.");
	    teleport();
	    if (off(player, ISCLEAR)) {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8) + HUHDURATION);
		else {
		    fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
	    }
	when 45:
	    msg("You feel less healthy now.");
	    pstats.s_const = max(pstats.s_const - 1, 3);
	    max_stats.s_const = max(max_stats.s_const - 1, 3);
	when 46:
	    msg("You feel weaker now.");
	    chg_str(-1, TRUE, FALSE);
	when 47:
	    msg("You feel less wise now.");
	    pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
	    max_stats.s_wisdom = max(max_stats.s_wisdom - 1, 3);
	when 48:
	    msg("You feel less dextrous now.");
	    chg_dext(-1, TRUE, FALSE);
	when 49:
	    msg("You feel less intelligent now.");
	    pstats.s_intel = max(pstats.s_intel - 1, 3);
	    max_stats.s_intel = max(max_stats.s_intel - 1, 3);
	when 50:
	    msg("A trap door opens underneath your feet.");
	    mpos = 0;
	    level++;
	    new_level(NORMLEV);
	    if (rnd(4) < 2) {
		addmsg("You are damaged by the fall");
		if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
		    addmsg("!  The fall killed you.");
		    endmsg();
		    death(D_FALL);
		    return;
		}
	    }
	    addmsg("!");
	    endmsg();
	    if (off(player, ISCLEAR) && rnd(4) < 3) {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8) + HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);
		turn_on(player, ISHUH);
	    }
	    else
		msg("You feel dizzy for a moment, but it quickly passes.");
	when 51:
	    msg("A maze entrance opens underneath your feet.");
	    mpos = 0;
	    level++;
	    new_level(MAZELEV);
	    if (rnd(4) < 2) {
		addmsg("You are damaged by the fall");
		if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
		    addmsg("!  The fall killed you.");
		    endmsg();
		    death(D_FALL);
		    return;
		}
	    }
	    addmsg("!");
	    endmsg();
	    if (off(player, ISCLEAR) && rnd(4) < 3) {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8) + HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);
		turn_on(player, ISHUH);
	    }
	    else
		msg("You feel dizzy for a moment, but it quickly passes.");
	when 52:
	    hearmsg("You hear a wailing sound in the distance.");
	    aggravate();
	when 53: read_scroll(&player, S_HOLD, ISCURSED);
	when 54:
	    msg("You can't move.");
	    no_command = 3 * HOLDTIME;
	when 55:
	    hearmsg("You hear a buzzing sound.");
	    aggravate();
	when 56:
	    msg("Your limbs stiffen.");
	    no_command = 3 * STONETIME;
	when 57:
	    msg("You feel a rock in your shoe hurting your foot.");
	    turn_on(player, STUMBLER);
	when 58:
	    msg("You get a hollow feeling in your stomach.");
	    food_left -= 500;
	when 59:
	    msg("Your purse feels lighter.");
	    purse = max(purse - 50 - rnd(purse / 2), 0);
	when 60:
	    msg("A pixie appears and grabs gold from your purse.");
	    purse = max(purse - 50 - rnd(50), 0);
	when 61:
	    msg("You feel a tingling sensation all over.");
	    pstats.s_hpt -= rnd(pstats.s_hpt / 3);
	when 62: msg("You feel a pull downwards.");
	when 63: msg("You feel a strange pull downwards.");
	when 64: msg("You feel a peculiar pull downwards.");
	when 65: msg("You have a strange urge to go down.");
	when 66: msg("You feel a pull upwards.");
	when 67: msg("You feel a strange pull upwards.");
	when 68: msg("You have a strange feeling for a moment.");
	when 69: msg("You float in the air for a moment.");
	when 70: msg("You feel very heavy for a moment.");
	when 71: msg("You feel a strange sense of loss.");
	when 72: msg("You feel the earth spinning underneath your feet.");
	when 73: msg("You feel in touch with a Universal Oneness.");
	when 74: hearmsg("You hear voices in the distance.");
	when 75: msg("A strange feeling of power comes over you.");
	when 76: msg("You feel a strange sense of unease.");
	when 77:
	    msg("You feel Lady Luck is looking the other way.");
	    luck++;
	when 78: msg("You feel your pack vibrate for a moment.");
	when 79: msg("You feel someone is watching you.");
	when 80: msg("You feel your hair standing on end.");
	when 81:
	    msg("Wait!  The walls are moving!");
	    new_level(NORMLEV);
	when 82:
	    msg("Wait!  Walls are appearing out of nowhere!");
	    new_level(MAZELEV);
	when 83: blue_light(ISCURSED);
	when 84:
	    msg("Your mind goes blank for a moment.");
	    wclear(cw);
	    light(&hero);
	    status(TRUE);
	when 85:
	    if (on(player, ISDEAF)) {
		msg("You feel your ears burn for a moment.");
		lengthen(hear, 2 * PHASEDURATION);
	    }
	    else {
		msg("You are suddenly surrounded by silence.");
		turn_on(player, ISDEAF);
		fuse(hear, 0, 2 * PHASEDURATION, AFTER);
	    }
	when 86:
	    {
		int baf_curse();
		object  *apply_to_bag();

		apply_to_bag(pack, 0, NULL, baf_curse, NULL);
		if (off(player, ISUNSMELL))
		    msg("You smell a faint trace of burning sulfur.");
	    }
	when 87:
	    msg("You have contracted a parasitic infestation.");
	    infest_dam++;
	    turn_on(player, HASINFEST);
	when 88:    {
		static coord    fear;

		msg("You suddenly feel a chill run up and down your spine.");
		turn_on(player, ISFLEE);
		fallpos(&hero, &fear, TRUE);
		player.t_dest = &fear;
	    }
	when 89:
	    if (cur_weapon != NULL)
		msg("You feel your %s get very hot.",
		    inv_name(cur_weapon, LOWERCASE));
	when 90:
	    if (cur_weapon != NULL)
		msg("Your %s glows white for an instant.",
		    inv_name(cur_weapon, LOWERCASE));
	when 91:
	    if (cur_armor != NULL)
		msg("Your %s gets very hot.", inv_name(cur_armor, LOWERCASE));
	when 92:
	    if (cur_weapon != NULL)
		msg("Your %s suddenly feels very cold.",
		    inv_name(cur_weapon, LOWERCASE));
	when 93:
	    if (cur_armor != NULL)
		msg("Your armor is covered by an oily film.");
	when 94: read_scroll(&player, S_CREATE, ISNORMAL);
	when 95:
	    lower_level(D_POTION);
	when 96:    {
	    int x, y;

	    for (x = -1; x <= 1; x++) {
		for (y = -1; y <= 1; y++) {
		    if (x == 0 && y == 0)
			continue;
		    delta.x = x;
		    delta.y = y;
		    do_zap(&player, WS_POLYMORPH, rnd(2) ? ISCURSED : ISNORMAL);
		}
	    }
	}
	when 97:    {
	    int x, y;

	    for (x = -1; x <= 1; x++) {
		for (y = -1; y <= 1; y++) {
		    if (x == 0 && y == 0)
			continue;
		    delta.x = x;
		    delta.y = y;
		    do_zap(&player, WS_INVIS, ISNORMAL);
		}
	    }
	}
	otherwise:
	    tr->ar_flags &= ~ISACTIVE;
	    hearmsg("You hear a click coming from %s.", inv_name(tr, LOWERCASE));
    }
}

/*
 * do_major: major malevolent effects
 *
 *  0.  read_scroll(S_SELFTELEPORT, ISCURSED)
 *  1.  PERMBLIND for twice normal duration
 *  2.  new_level(THRONE);
 *  3.  turn_on(player, SUPEREAT);
 *  4.  lengthen(noslow, 20 + rnd(20));
 *  5.  lower_level(D_POTION) * roll(1,4)
 *  6.  change stats
 *  7.  FIRETRAP
 *  8.  armor crumbles
 *  9.  weapon crumbles
 *  10. weapon crumbles
 *  11. curse weapon
 */
do_major()
{
    int which;

    which = rnd(12);
    debug("Rolled %d.", which);
    switch (which) {
	when 0: read_scroll(&player, S_SELFTELEP, ISCURSED);
	when 1:
	    quaff(&player, P_TRUESEE, ISCURSED);
	    quaff(&player, P_TRUESEE, ISCURSED);
	when 2: new_level(THRONE);
	when 3:
	    /* Turn off other body-affecting spells */
	    if (on(player, ISREGEN)) {
		extinguish(unregen);
		turn_off(player, ISREGEN);
		unregen();
	    }
	    if (on(player, NOCOLD)) {
		extinguish(uncold);
		turn_off(player, NOCOLD);
		uncold();
	    }
	    if (on(player, NOFIRE)) {
		extinguish(unhot);
		turn_off(player, NOFIRE);
		unhot();
	    }
	    if (on(player, SUPEREAT)) {
		lengthen(unsupereat, 2 * PHASEDURATION);
		msg("Your body temperature rises still further.");
	    }
	    else {
		msg("You feel very warm all over.");
		fuse(unsupereat, 0, 2 * PHASEDURATION, AFTER);
		turn_on(player, SUPEREAT);
	    }
	when 4:
	    msg("You feel yourself moving %sslower.",
		on(player, ISSLOW) ? "even " : "");
	    if (on(player, ISSLOW))
		lengthen(noslow, PHASEDURATION);
	    else {
		turn_on(player, ISSLOW);
		player.t_turn = TRUE;
		fuse(noslow, 0, PHASEDURATION, AFTER);
	    }
	when 5: {
	    int i, num = roll(1, 4);

	    for (i = 1; i < num; i++)
		lower_level(D_POTION);
	}
	when 6:
	    if (rnd(2))
		add_intelligence(TRUE);
	    if (rnd(2))
		chg_dext(-1, TRUE, FALSE);
	    if (rnd(2))
		chg_str(-1, TRUE, FALSE);
	    if (rnd(2))
		add_wisdom(TRUE);
	    if (rnd(2))
		add_const(TRUE);
	when 7: {
	    static coord    fires;
	    struct room *rp;

	    if (ntraps + 1 >= MAXTRAPS) {
		msg("You feel a puff of hot air.");
		return;
	    }
	    for (; ntraps < 2 * MAXTRAPS; ntraps++) {
		if (!fallpos(&hero, &fires, TRUE))
		    break;
		mvaddch(fires.y, fires.x, FIRETRAP);
		traps[ntraps].tr_type = FIRETRAP;
		traps[ntraps].tr_flags |= ISFOUND;
		traps[ntraps].tr_show = FIRETRAP;
		traps[ntraps].tr_pos.x = fires.x;
		traps[ntraps].tr_pos.y = fires.y;
		if ((rp = roomin(&hero)) != NULL)
		    rp->r_flags &= ~ISDARK;
	    }
	}
	when 8: {
	    object  *obj;
	    int bafcweapon();
	    object  *apply_to_bag();

	    if (cur_weapon == NULL) {
		msg("You feel your hands tingle a moment.");
		pstats.s_dmg = "1d2";
		return;
	    }
	    obj = apply_to_bag(pack, 0, NULL, bafcweapon, NULL);
	    if (obj->o_flags & ISMETAL)
		msg("Your %s melts and disappears.", inv_name(obj, LOWERCASE));
	    else
		msg("Your %s crumbles in your hands.", inv_name(obj, LOWERCASE));
	    obj->o_flags &= ~ISCURSED;
	    dropcheck(obj);
	    del_bag(pack, obj);

	}
	when 9: {
	    object  *obj;
	    object  *apply_to_bag();
	    int bafcarmor();

	    if (cur_armor == NULL) {
		msg("Your body tingles a moment.");
		return;
	    }
	    obj = apply_to_bag(pack, 0, NULL, bafcarmor, NULL);
	    msg("Your %s crumbles into small black powdery dust.",
		inv_name(obj, LOWERCASE));
	    obj->o_flags &= ~ISCURSED;
	    dropcheck(obj);
	    del_bag(pack, obj);
	}
	otherwise:
	    if (cur_weapon == NULL) {
		seemsg("Your hand glows yellow for an instant.");
		pstats.s_dmg = "1d3";
		return;
	    }
	    seemsg("Your %s glows bright red for a moment.",
		   weaps[cur_weapon->o_which].w_name);
	    if (cur_weapon->o_hplus > 0)
		cur_weapon->o_hplus = -rnd(3);
	    else
		cur_weapon->o_hplus -= rnd(3);
	    if (cur_weapon->o_dplus > 0)
		cur_weapon->o_dplus = -rnd(3);
	    else
		cur_weapon->o_dplus -= rnd(3);
	    cur_weapon->o_flags = ISCURSED | ISLOST;
	    cur_weapon->o_ac = 0;
    }
}

/*
 * do_phial: handle powers of the Phial of Galadriel
 */
do_phial()
{
    int which;

    /* Prompt for action */
    msg("How do you wish to apply the Phial of Galadriel (* for list)? ");
    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > 1) {
	add_line("[a] total healing");
	add_line("[b] total monster confusion");
	end_line();
	msg("");
	msg("Which power do you wish to use? ");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > 1) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successsful.");
    switch (which) {
	when 0:
	    pstats.s_hpt = max_stats.s_hpt += rnd(pstats.s_lvl) + 1;
	    pstats.s_power = max_stats.s_power += rnd(pstats.s_lvl) + 1;
	when 1: {
	    struct linked_list  *mi;
	    struct thing    *tp;

	    for (mi = mlist; mi != NULL; mi = next(mi)) {
		tp = THINGPTR(mi);
		if (off(*tp, ISUNIQUE) || !save_throw(*tp, VS_MAGIC))
		    turn_on(*tp, ISHUH);
	    }
	}
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_palantir: handle powers of the Palantir of Might
 */
do_palantir()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Palantir of Might (* for list)? ");
    limit = 3;
    if (is_carrying(TR_SCEPTRE))
	limit += 1;
    if (is_carrying(TR_CROWN))
	limit += 1;

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > limit) {
	msg("");
	add_line("[a] monster detection");
	add_line("[b] gold detection");
	add_line("[c] magic detection");
	add_line("[d] food detection");
	if (limit >= 4)
	    add_line("[e] teleportation");
	if (limit >= 5)
	    add_line("[f] clear thought");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > limit) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	when 0: quaff(&player, P_MONSTDET, ISNORMAL);
	when 1: read_scroll(&player, S_GFIND, ISNORMAL);
	when 2: quaff(&player, P_TREASDET, ISNORMAL);
	when 3: read_scroll(&player, S_FOODDET, ISNORMAL);
	when 4: read_scroll(&player, S_SELFTELEP, ISNORMAL);
	when 5: quaff(&player, P_CLEAR, ISNORMAL);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_silmaril: handle powers of the Silamril of Ea
 */
do_silmaril()
{
    int which;

    /* Prompt for action */
    msg("How do you wish to apply the Silamril of Ea (* for list)? ");

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > 2) {
	msg("");
	add_line("[a] magic mapping");
	add_line("[b] petrification");
	add_line("[c] stairwell downwards");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > 2) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	when 0: read_scroll(&player, S_MAP, ISNORMAL);
	when 1: read_scroll(&player, S_PETRIFY, ISNORMAL);
	when 2:
	    msg("A stairwell opens beneath your feet and you go down.");
	    level++;
	    new_level(NORMLEV);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_amulet: handle powers of the Amulet of Yendor
 */
do_amulet()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Amulet of Yendor (* for list)? ");

    limit = 0;
    if (is_carrying(TR_PURSE))
	limit += 1;
    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }
    if (which < 0 || which > limit) {
	msg("");
	add_line("[a] level evaluation");
	if (limit >= 1)
	    add_line("[b] invisibility");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > limit) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	when 0: level_eval();
	when 1: quaff(&player, P_INVIS, ISNORMAL);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_bag: handle powers of the Magic Purse of Yendor as a bag of holding
 */
do_bag(obj)
struct object   *obj;
{
    int which, limit;
    object  *get_object();

    /* Prompt for action */
    msg("How do you wish to apply the Magic Purse of Yendor (* for list)? ");

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }

    limit = 2;
    if (is_carrying(TR_AMULET))
	limit += 1;

    if (which < 0 || which > limit) {
	msg("");
	add_line("[a] inventory");
	add_line("[b] add to bag");
	add_line("[c] remove from bag");
	if (limit >= 3)
	    add_line("[d] see invisible");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > limit) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	when 0:
	    inventory(obj->o_bag, NULL);
	when 1: {
	    object  *new_obj_p; /* what the user
			     * selected */

	    if ((new_obj_p = get_object(pack, "add", NULL, NULL)) != NULL) {
		rem_pack(new_obj_p);    /* free up pack slot */
		push_bag(&obj->o_bag, new_obj_p,
		    new_obj_p->o_type);
		pack_report(new_obj_p, MESSAGE,
		    "You just added ");
	    }
	}
	when 2: {
	    object  *obj_p;
	    linked_list *item_p;
	    linked_list *make_item();

	    if ((obj_p = get_object(obj->o_bag, "remove",
		 NULL, NULL)) != NULL) {
		item_p = make_item(obj_p);  /* attach upper
				 * structure */
		if (add_pack(item_p, MESSAGE) != FALSE)
		    pop_bag(obj->o_bag, obj_p);
	    }
	}
	when 3: quaff(&player, P_TRUESEE, ISBLESSED);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_sceptre: handle powers of the Sceptre of Might
 */
do_sceptre()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Sceptre of Might (* for list)? ");

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }

    limit = 5;
    if (is_carrying(TR_CROWN))
	limit += 1;
    if (is_carrying(TR_PALANTIR))
	limit += 1;

    if (which < 0 || which > limit) {
	msg("");
	add_line("[a] cancellation");
	add_line("[b] polymorph monster");
	add_line("[c] slow monster");
	add_line("[d] teleport monster");
	add_line("[e] monster confusion");
	add_line("[f] paralyze monster");
	if (limit >= 6)
	    add_line("[g] drain life");
	if (limit >= 7)
	    add_line("[h] smell monster");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > limit) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 7) {
	msg("Your finger slips.");
	which = rnd(6);
	if (wizard) {
	    msg("What wand? (%d)", which);
	    if (get_str(prbuf, cw) == NORM) {
		which = atoi(prbuf);
		if (which < 0 || which > 5) {
		    msg("Invalid selection.");
		    which = rnd(6);
		    msg("Rolled %d.", which);
		}
	    }
	}
    }

    switch (which) {
	when 0: if (get_dir())
	    do_zap(&player, WS_CANCEL, ISBLESSED);
	when 1: if (get_dir())
	    do_zap(&player, WS_POLYMORPH, ISBLESSED);
	when 2: if (get_dir())
	    do_zap(&player, WS_SLOW_M, ISBLESSED);
	when 3: if (get_dir())
	    do_zap(&player, WS_MONSTELEP, ISBLESSED);
	when 4: if (get_dir())
	    do_zap(&player, WS_CONFMON, ISBLESSED);
	when 5: if (get_dir())
	    do_zap(&player, WS_PARALYZE, ISBLESSED);
	when 6: if (get_dir())
	    do_zap(&player, WS_DRAIN, ISBLESSED);
	when 7: quaff(&player, P_SMELL, ISBLESSED);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * do_wand: handle powers of the Wand of Yendor
 */
do_wand()
{
    int which, i;

    /* Prompt for action */
    msg("How do you wish to apply the Wand of Yendor (* for list)? ");

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }

    if (which < 0 || which >= maxsticks) {
	msg("");
	for (i = 0; i < maxsticks; i++) {
	    sprintf(prbuf, "[%c] %s", i + 'a', ws_magic[i].mi_name);
	    add_line(prbuf);
	}
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which >= maxsticks) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 12) {
	msg("Your finger slips.");
	which = rnd(maxsticks);
	if (wizard) {
	    msg("What wand? (%d)", which);
	    if (get_str(prbuf, cw) == NORM) {
		which = atoi(prbuf);
		if (which < 0 || which >= maxsticks) {
		    msg("Invalid selection.");
		    which = rnd(maxsticks);
		    msg("Rolled %d.", which);
		}
	    }
	}
    }

    if (get_dir())
	do_zap(&player, which, ISBLESSED);
}

/*
 * do_crown: handle powers of the Crown of Might
 */
do_crown()
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Crown of Might (* for list)? ");

    which = (short) ((getchar() & 0177) - 'a');
    if (which == (short) ESCAPE - (short) 'a') {
	after = FALSE;
	return;
    }

    limit = 9;
    if (is_carrying(TR_PALANTIR))
	limit += 1;
    if (is_carrying(TR_SCEPTRE))
	limit += 1;

    if (which < 0 || which > limit) {
	msg("");
	add_line("[a] add strength");
	add_line("[b] add intelligence");
	add_line("[c] add wisdom");
	add_line("[d] add dexterity");
	add_line("[e] add constitution");
	add_line("[f] normal strength");
	add_line("[g] normal intelligence");
	add_line("[h] normal wisdom");
	add_line("[i] normal dexterity");
	add_line("[j] normal constitution");
	if (limit >= 10)
	    add_line("[k] disguise");
	if (limit >= 11)
	    add_line("[l] super heroism");
	end_line();
	msg("Which power do you wish to use?");
	which = (short) ((getchar() & 0177) - 'a');
	while (which < 0 || which > limit) {
	    if (which == (short) ESCAPE - (short) 'a') {
		after = FALSE;
		return;
	    }
	    msg("");
	    msg("Please enter one of the listed powers: ");
	    which = (short) ((getchar() & 0177) - 'a');
	}
	msg("Your attempt is successful.");
    }
    else
	msg("Your attempt is successful.");
    switch (which) {
	when 0:
	    if (off(player, POWERSTR)) {
		turn_on(player, POWERSTR);
		chg_str(10, FALSE, FALSE);
		msg("You feel much stronger now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 1:
	    if (off(player, POWERINTEL)) {
		pstats.s_intel += 10;
		turn_on(player, POWERINTEL);
		msg("You feel much more intelligent now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 2:
	    if (off(player, POWERWISDOM)) {
		pstats.s_wisdom += 10;
		turn_on(player, POWERWISDOM);
		msg("Your feel much wiser know.");
	    }
	    else
		nothing_message(ISCURSED);
	when 3:
	    if (off(player, POWERDEXT)) {
		turn_on(player, POWERDEXT);
		chg_dext(10, FALSE, FALSE);
		msg("You feel much more dextrous now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 4:
	    if (off(player, POWERCONST)) {
		pstats.s_const += 10;
		turn_on(player, POWERCONST);
		msg("You feel much healthier now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 5:
	    if (on(player, POWERSTR)) {
		turn_off(player, POWERSTR);
		chg_str(-10, FALSE, FALSE);
		msg("Your muscles bulge less now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 6:
	    if (on(player, POWERINTEL)) {
		pstats.s_intel = max(pstats.s_intel - 10,
			     3 + ring_value(R_ADDINTEL));
		turn_off(player, POWERINTEL);
		msg("You feel less intelligent now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 7:
	    if (on(player, POWERWISDOM)) {
		pstats.s_wisdom = max(pstats.s_wisdom - 10,
			      3 + ring_value(R_ADDWISDOM));
		turn_off(player, POWERWISDOM);
		msg("You feel less wise now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 8:
	    if (on(player, POWERDEXT)) {
		turn_off(player, POWERDEXT);
		chg_dext(-10, FALSE, FALSE);
		msg("You feel less dextrous now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 9:
	    if (on(player, POWERCONST)) {
		pstats.s_const -= 10;
		turn_off(player, POWERCONST);
		msg("You feel less healthy now.");
	    }
	    else
		nothing_message(ISCURSED);
	when 10: quaff(&player, P_DISGUISE, ISNORMAL);
	when 11: quaff(&player, P_SHERO, ISNORMAL);
	otherwise:
	    msg("What a strange thing to do!!");
    }
}

/*
 * level_eval - have amulet evaluate danger on this level
 */

level_eval()
{
    int count = 0;
    int max_nasty = 0;
    struct linked_list  *item;
    struct thing    *tp;
    char    *colour, *temp;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	count++;
	max_nasty = max(max_nasty,
		  (10 - tp->t_stats.s_arm) * tp->t_stats.s_hpt);
    }

    if (count < 3)
	colour = "black";
    else if (count < 6)
	colour = "red";
    else if (count < 9)
	colour = "orange";
    else if (count < 12)
	colour = "yellow";
    else if (count < 15)
	colour = "green";
    else if (count < 18)
	colour = "blue";
    else if (count < 25)
	colour = "violet";
    else
	colour = "pink with purple polka dots";

    if (max_nasty < 10)
	temp = "feels cold and lifeless";
    else if (max_nasty < 30)
	temp = "feels cool";
    else if (max_nasty < 200)
	temp = "feels warm and soft";
    else if (max_nasty < 1000)
	temp = "feels warm and slippery";
    else if (max_nasty < 5000)
	temp = "feels hot and dry";
    else if (max_nasty < 10000)
	temp = "feels too hot to hold";
    else if (max_nasty < 20000)
	temp = "burns your hand";
    else
	temp = "jumps up and down shrieking 'DANGER! DANGER'";

    msg("The amulet glows %s and %s.", colour, temp);
}
