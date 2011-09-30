/*
    fight.c  -  All the fighting gets done here

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
    
    See the file LICENSE.TXT for full copyright and licensing information.*/
*/

#include <ctype.h>
#include "rogue.h"
#include "death.h"

/*
 * This are the beginning experience levels for all players all further
 * experience levels are computed by multiplying by 2
 */
static long e_levels[10] = {
    113L,           /* Fighter */
    142L,           /* Paladin */
    129L,           /* Ranger */
    87L,            /* Cleric */
    114L,           /* Druid */
    135L,           /* Magician */
    129L,           /* Illusionist */
    72L,            /* Thief */
    86L,            /* Assasin */
    259L            /* Ninja */
};
struct matrix   att_mat[11] = {
    /* Base, Max_lvl, Factor, Offset, Range */
    {10, 17, 2, 1, 2},  /* fi */
    {10, 17, 2, 1, 2},  /* pa */
    {10, 17, 2, 1, 2},  /* ra */
    {10, 19, 2, 1, 3},  /* cl */
    {10, 19, 2, 1, 3},  /* dr */
    {9, 21, 2, 1, 5},   /* mu */
    {9, 21, 2, 1, 5},   /* il */
    {10, 21, 2, 1, 4},  /* th */
    {10, 21, 2, 1, 4},  /* as */
    {10, 21, 2, 1, 4},  /* nj */
    {7, 25, 1, 0, 2}    /* mn */
};

do_fight(y, x, tothedeath)
{
    if (!tothedeath && pstats.s_hpt < max_stats.s_hpt / 3) {
	if (!terse)
	    msg("That's not wise.");
	after = fighting = FALSE;
	return;
    }
    if (isalpha(winat(hero.y + y, hero.x + x))) {
	after = fighting = TRUE;
	do_move(y, x);
    }
    else {
	if (fighting == FALSE)
	    msg("Nothing there.");
	after = fighting = FALSE;
    }
}

/*
 * fight: The player attacks the monster.
 */

fight(mp, weap, thrown)
coord   *mp;
struct object   *weap;
bool    thrown;
{
    struct thing    *tp;
    struct linked_list  *item;
    bool    did_hit = TRUE;
    char    *mname;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL) {
	debug("Fight what @ %d,%d", mp->y, mp->x);
	return;
    }
    tp = THINGPTR(item);
    mname = (on(player, ISBLIND)) ? "it" : monsters[tp->t_index].m_name;

    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    player.t_rest_hpt = player.t_rest_pow = 0;
    tp->t_rest_hpt = tp->t_rest_pow = 0;

    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (off(player, ISBLIND)) {
	if (on(*tp, ISDISGUISE) && (tp->t_type != tp->t_disguise)) {
	    msg("Wait! That's a %s!", mname);
	    turn_off(*tp, ISDISGUISE);
	    did_hit = thrown;
	}
	if (on(*tp, CANSURPRISE)) {
	    turn_off(*tp, CANSURPRISE);
	    if ((player.t_ctype == C_RANGER && rnd(6) != 0) ||
		(player.t_ctype == C_NINJA && rnd(pstats.s_lvl / 2)
		 != 0))
		msg("You notice a %s trying to hide!", mname);
	    else {
		msg("Wait! There's a %s!", mname);
		did_hit = thrown;
	    }
	}
    }

    /* Protection from Normal Missiles */
    if (thrown && on(*tp, HASMSHIELD)) {
	msg("The %s slows as it approaches %s.",
	    weaps[weap->o_which].w_name, mname);
	did_hit = FALSE;
    }

    if (did_hit) {
	did_hit = FALSE;
	if (!can_blink(tp) &&
	    (off(*tp, MAGICHIT) || (weap != NULL &&
		   (weap->o_hplus > 0 || weap->o_dplus > 0))) &&
	    (off(*tp, BMAGICHIT) || (weap != NULL &&
		   (weap->o_hplus > 2 || weap->o_dplus > 2))) &&
	    roll_em(&player, tp, weap, thrown, cur_weapon)) {
	    did_hit = TRUE;
	    tp->t_wasshot = TRUE;

	    if (thrown) {
		if (weap != NULL && weap->o_type == WEAPON
		    && weap->o_which == GRENADE) {
		    hearmsg("BOOOM!");
		    aggravate();
		}
		thunk(weap, mname);
	    }
	    else
		hit(mname);

	    /* hitting a friendly monster is curtains */
	    if (on(*tp, ISFRIENDLY)) {
		turn_off(*tp, ISFRIENDLY);
		turn_on(*tp, ISMEAN);
	    }

	    /* Charmed monsters become uncharmed */
	    if (on(*tp, ISCHARMED)) {
		turn_off(*tp, ISCHARMED);
		turn_on(*tp, ISMEAN);
	    }

	    /*
	     * If the player hit a rust monster, he better have a
	     * + weapon
	     */
	    if (on(*tp, CANRUST)) {
		if (!thrown && (weap != NULL) &&
		    (weap->o_flags & ISMETAL) &&
		    !(weap->o_flags & ISPROT) &&
		    !(weap->o_flags & ISSILVER) &&
		(weap->o_hplus < 1) && (weap->o_dplus < 1)) {
		    if (rnd(100) < 50)
			weap->o_hplus--;
		    else
			weap->o_dplus--;
		    msg(terse ? "Your %s weakens!"
			: "Your %s appears to be weaker now!",
			weaps[weap->o_which].w_name);
		}
		else if (!thrown && weap != NULL &&
		    (weap->o_flags & ISMETAL))
		    msg(terse ? "" : "The rust vanishes from your %s!",
			weaps[weap->o_which].w_name);
	    }

	    /* flammable monsters die from burning weapons */
	    if (thrown && on(*tp, CANBBURN) &&
		(weap->o_flags & CANBURN) &&
		!save_throw(VS_WAND, tp)) {
		msg("The %s vanishes in a ball of flame.",
		    monsters[tp->t_index].m_name);
		tp->t_stats.s_hpt = 0;
	    }

	    /* spores explode and infest hero  */
	    if (on(*tp, CANSPORE)) {
		msg("The %s explodes in a cloud of dust.",
		    monsters[tp->t_index].m_name);
		if (is_wearing(R_HEALTH) ||
		    player.t_ctype == C_PALADIN ||
		    (player.t_ctype == C_NINJA && pstats.s_lvl
		     > 6) ||
		    thrown && rnd(50) > 0 ||
		    rnd(20) > 0)
		    msg("The dust makes it hard to breath.");
		else {
		    msg(terse ? "You have been infested."
			: "You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		}
		tp->t_stats.s_hpt = 0;
	    }

	    /*
	     * fireproof monsters laugh at you when burning
	     * weapon hits
	     */
	    if (thrown && on(*tp, NOFIRE) && (weap->o_flags & CANBURN))
		msg("The %s laughs as the %s bounces.",
		    monsters[tp->t_index].m_name,
		    weaps[weap->o_which].w_name);

	    /* sharp weapons have no effect on NOSHARP monsters */
	    if (on(*tp, NOSHARP) && (weap != NULL) &&
		(weap->o_flags & ISSHARP)) {
		msg("The %s has no effect on the %s!",
		    weaps[weap->o_which].w_name,
		    monsters[tp->t_index].m_name);
		fighting = FALSE;
	    }

	    /* metal weapons pass through NOMETAL monsters */
	    if (on(*tp, NOMETAL) && (weap != NULL) &&
		(weap->o_flags & ISMETAL)) {
		msg("The %s passes through the %s!",
		    weaps[weap->o_which].w_name,
		    monsters[tp->t_index].m_name);
		fighting = FALSE;
	    }

	    /*
	     * If the player hit something that shrieks, wake the
	     * dungeon
	     */
	    if (on(*tp, CANSHRIEK)) {
		turn_off(*tp, CANSHRIEK);
		if (on(player, CANHEAR)) {
		    msg("You are stunned by the %s's shriek.", mname);
		    no_command += 4 + rnd(8);
		}
		else if (off(player, ISDEAF))
		    msg("The %s emits a piercing shriek.", mname);
		else
		    msg("The %s seems to be trying to make some noise.", mname);
		aggravate();
		if (rnd(wizard ? 3 : 39) == 0 && cur_armor
		    != NULL
		    && cur_armor->o_which == CRYSTAL_ARMOR) {

		    struct linked_list  *item;
		    struct object   *obj;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = OBJPTR(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    }
		    else {
			msg("Your armor shatters from the shriek.");
			cur_armor = NULL;
			del_pack(item);
		    }
		}
	    }

	    /*
	     * If the player hit something that can surprise, it
	     * can't now
	     */
	    if (on(*tp, CANSURPRISE))
		turn_off(*tp, CANSURPRISE);

	    /*
	     * If the player hit something that can summon, it
	     * will try to
	     */
	    summon_help(tp, NOFORCE);

	    /* Can the player confuse? */
	    if (on(player, CANHUH) && !thrown) {
		seemsg("Your hands stop glowing red!");
		seemsg("The %s appears confused.", mname);
		turn_on(*tp, ISHUH);
		turn_off(player, CANHUH);
	    }

	    /* Merchants just disappear if hit */

	    /*
	     * increases prices and curses objects from now on
	     * though
	     */
	    if (on(*tp, CANSELL)) {
		msg("The %s disappears with his wares with a BOOM and a flash.", mname);
		killed(NULL, item, NOMESSAGE, NOPOINTS);
		aggravate();
		luck++;
	    }

	    else if (tp->t_stats.s_hpt <= 0)
		killed(&player, item, MESSAGE, POINTS);

	    /*
	     * If the monster is fairly intelligent and about to
	     * die, it may turn tail and run.
	     */
	    else if ((tp->t_stats.s_hpt < max(10, tp->maxstats.s_hpt / 10)) &&
		 (rnd(25) < tp->t_stats.s_intel)) {
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

		if (on(*tp, CANTELEPORT)) {
		    int rm;

		    /*
		     * Erase the monster from the old
		     * position
		     */
		    if (isalpha(mvwinch(cw, tp->t_pos.y, tp->t_pos.x)))
			mvwaddch(cw, tp->t_pos.y, tp->t_pos.x, tp->t_oldch);
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, ' ');

		    /* Get a new position */
		    do {
			rm = rnd_room();
			rnd_pos(&rooms[rm], &tp->t_pos);
		    } until(winat(tp->t_pos.y, tp->t_pos.x) == FLOOR);

		    /* Put it there */
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, tp->t_type);
		    tp->t_oldch = mvwinch(cw, tp->t_pos.y, tp->t_pos.x);
		    seemsg("The %s seems to have disappeared!", mname);
		}
	    }
	}
	else if (thrown)
	    bounce(weap, mname);
	else
	    miss(mname);
    }
    if (curr_mons)
	runto(mp, &hero);   /* after so that backstabbing can
		     * happen */
    count = 0;
    return did_hit;
}

/*
 * attack: The monster attacks the player
 */

attack(mp, weapon, thrown)
struct thing    *mp;
struct object   *weapon;
bool    thrown;
{
    char    *mname;
    bool    did_hit = FALSE;
    char    *find_slot();   /* actually (struct delayed_action *) */
    /* If the monster is in a wall, it cannot attack */
    if (on(*mp, ISINWALL))
	return (FALSE);

    /*
     * If two monsters start to gang up on our hero, stop fight mode
     */
    if (fighting) {
	if (beast == NULL)
	    beast = mp;
	else if (beast != mp)
	    fighting = FALSE;
    }

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    player.t_rest_hpt = player.t_rest_pow = 0;
    mp->t_rest_hpt = mp->t_rest_pow = 0;

    if (on(*mp, ISDISGUISE) && off(player, ISBLIND))
	turn_off(*mp, ISDISGUISE);
    mname = on(player, ISBLIND) ? "the monster" :
	monsters[mp->t_index].m_name;
    if (roll_em(mp, &player, weapon, thrown, wield_weap(weapon, mp)) &&
	(!thrown || off(player, HASMSHIELD))) {
	did_hit = TRUE;

	m_thunk(weapon, mname);

	if (pstats.s_hpt <= 0) {
	    death(mp->t_index); /* Bye bye life ... */
	    return TRUE;
	}

	/* surprising monsters appear after they shoot at you */
	if (thrown && on(*mp, CANSURPRISE))
	    turn_off(*mp, CANSURPRISE);
	else {

	    /*
	     * If a vampire hits, it may take half your hit
	     * points
	     */
	    if (on(*mp, CANSUCK) &&
		!save(VS_MAGIC)) {
		if (pstats.s_hpt == 1) {
		    death(mp->t_index);
		    return TRUE;
		}
		else {
		    pstats.s_hpt /= 2;
		    msg("You feel your life force being drawn from you.");
		}
	    }

	    /*
	     * strong monsters can shatter or gong crystalline
	     * armor
	     */
	    if (cur_armor != NULL && cur_armor->o_which == CRYSTAL_ARMOR) {
		if (rnd(mp->t_stats.s_str + (cur_armor->o_ac / 2)) > 20) {

		    struct linked_list  *item;
		    struct object   *obj;

		    for (item = pack; item != NULL; item = next(item)) {
			obj = OBJPTR(item);
			if (obj == cur_armor)
			    break;
		    }
		    if (item == NULL) {
			debug("Can't find crystalline armor being worn.");
		    }
		    else {
			msg("Your armor is shattered by the blow.");
			cur_armor = NULL;
			del_pack(item);
		    }
		}
		else if (rnd(mp->t_stats.s_str) > 15) {
		    msg("Your armor rings from the blow.");
		    aggravate();
		}
	    }

	    /* Stinking monsters reduce the player's strength */
	    if (on(*mp, CANSTINK)) {
		turn_off(*mp, CANSTINK);
		if (player.t_ctype != C_PALADIN
		    && !(player.t_ctype == C_NINJA && pstats.s_lvl > 12)
		    && !save(VS_POISON)) {
		    if (on(player, CANSCENT)) {
			msg("You pass out from the stench of the %s.", mname);
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("The stench of the %s sickens you.", mname);
		    if (on(player, HASSTINK))
			lengthen(unstink, STINKTIME);
		    else {
			turn_on(player, HASSTINK);
			fuse(unstink, 0, STINKTIME,
			    AFTER);
		    }
		}
	    }

	    /* chilling monster reduces strength permanently */
	    if (on(*mp, CANCHILL) &&
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {

		msg("You cringe at the %s's chilling touch.", mname);
		if (!is_wearing(R_SUSABILITY)) {
		    chg_str(-1, FALSE, TRUE);
		    if (lost_str == 0)
			fuse(res_strength, 0, CHILLTIME, AFTER);
		    else
			lengthen(res_strength, CHILLTIME);
		}
	    }

	    /* itching monsters reduce dexterity (temporarily) */
	    if (on(*mp, CANITCH) && player.t_ctype != C_PALADIN
	    && !(player.t_ctype == C_NINJA && pstats.s_lvl > 12)
		&& !save(VS_POISON)) {
		msg("The claws of the %s scratch you!", mname);
		if (is_wearing(R_SUSABILITY)) {
		    msg("The scratch has no effect.");
		}
		else {
		    msg("You feel a burning itch.");
		    turn_on(player, HASITCH);
		    chg_dext(-1, FALSE, TRUE);
		    fuse(un_itch, 0, roll(4, 6), AFTER);
		}
	    }

	    /* a hugging monster may SQUEEEEEEEZE */
	    if (on(*mp, CANHUG) &&
		(cur_armor == NULL || cur_armor->o_which != CRYSTAL_ARMOR)) {
		if (roll(1, 20) >= 18 || roll(1, 20) >= 18) {
		    msg("The %s squeezes you against itself.", mname);
		    if ((pstats.s_hpt -= roll(2, 8)) <= 0) {
			death(mp->t_index);
			return TRUE;
		    }
		}
	    }

	    /* a trampling monster may step on the player */
	    if (on(*mp, CANTRAMPLE)) {
		if (roll(1, 20) >= 16 || roll(1, 20) >= 16) {
		    msg("The %s steps on you.", mname);
		    if ((pstats.s_hpt -= roll(3, mp->t_stats.s_lvl)) <= 0) {
			death(mp->t_index);
			return TRUE;
		    }
		}
	    }

	    /*
	     * a disease-carrying monster may transmit the
	     * disease
	     */
	    if (on(*mp, CANDISEASE) &&
		(rnd(pstats.s_const) < mp->t_stats.s_lvl) &&
		off(player, HASDISEASE)) {

		if (is_wearing(R_HEALTH)
		    || (player.t_ctype == C_PALADIN)
		    || (player.t_ctype == C_NINJA &&
		    pstats.s_lvl > 6))
		    msg("The wound heals quickly.");
		else {
		    turn_on(player, HASDISEASE);
		    fuse(cure_disease, 0, roll(4, 4) *
			SICKTIME, AFTER);
		    msg(terse ? "You have been diseased."
		    : "You have contracted a disease!");
		}
	    }

	    /* a rust monster will weaken your armor */
	    if (on(*mp, CANRUST)) {
		if (cur_armor != NULL &&
		    cur_armor->o_which != SOFT_LEATHER &&
		    cur_armor->o_which != HEAVY_LEATHER &&
		    cur_armor->o_which != CUIRBOLILLI &&
		    cur_armor->o_which != PADDED_ARMOR &&
		    cur_armor->o_which != CRYSTAL_ARMOR &&
		    cur_armor->o_which != MITHRIL &&
		    !(cur_armor->o_flags & ISPROT) &&
		    cur_armor->o_ac < pstats.s_arm + 1) {
		    msg(terse ? "Your armor weakens!"
			: "Your armor appears to be weaker now. Oh my!");
		    cur_armor->o_ac++;
		}
		else if (cur_armor != NULL &&
		     (cur_armor->o_flags & ISPROT) &&
		    cur_armor->o_which != SOFT_LEATHER &&
		     cur_armor->o_which != HEAVY_LEATHER &&
		     cur_armor->o_which != CUIRBOLILLI &&
		    cur_armor->o_which != PADDED_ARMOR &&
		     cur_armor->o_which != CRYSTAL_ARMOR &&
		     cur_armor->o_which != MITHRIL)
		    msg(terse ? "" : "The rust vanishes instantly!");
	    }

	    /*
	     * If a surprising monster hit you, you can see it
	     * now
	     */
	    if (on(*mp, CANSURPRISE))
		turn_off(*mp, CANSURPRISE);

	    /*
	     * an infesting monster will give you a parasite or
	     * rot
	     */
	    if (on(*mp, CANINFEST) && rnd(pstats.s_const) < mp->t_stats.s_lvl) {
		if (is_wearing(R_HEALTH) || (player.t_ctype == C_PALADIN)
		    || (player.t_ctype == C_NINJA && pstats.s_lvl > 6))
		    msg("The wound quickly heals.");
		else {
		    turn_off(*mp, CANINFEST);
		    msg(terse ? "You have been infested."
			: "You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		}
	    }

	    /* Some monsters have poisonous bites */
	    if (on(*mp, CANPOISON) && !save(VS_POISON)) {
		if (is_wearing(R_SUSABILITY) || (player.t_ctype == C_PALADIN)
		    || (player.t_ctype == C_NINJA && pstats.s_lvl > 12))
		    msg(terse ? "Sting has no effect."
			: "A sting momentarily weakens you.");
		else {
		    chg_str(-1, FALSE, FALSE);
		    msg(terse ? "A sting has weakened you." :
			"You feel a sting in your arm and now feel weaker.");
		}
	    }

	    /* a hideous monster may cause fear by touching */
	    if (on(*mp, TOUCHFEAR)) {
		turn_off(*mp, TOUCHFEAR);
		if (!save(VS_WAND)
		    && !(on(player, ISFLEE) && (SAME_POS(player.t_dest,mp->t_pos)))) {
		    if (off(player, SUPERHERO)
		     && (player.t_ctype != C_PALADIN)) {
			turn_on(player, ISFLEE);
			player.t_dest = mp->t_pos;
			msg("The %s's touch terrifies you.", mname);
		    }
		    else
			msg("The %s's touch feels cold and clammy.",
			    mname);
		}
	    }

	    /* some monsters will suffocate our hero */
	    if (on(*mp, CANSUFFOCATE) && (rnd(100) < 15) &&
		(find_slot(suffocate) == NULL)) {
		turn_on(*mp, DIDSUFFOCATE);
		msg("The %s is beginning to suffocate you.",
		    mname);
		fuse(suffocate, 0, roll(4, 2), AFTER);
	    }

	    /* don't look now, you will get turned to stone */
	    if (on(*mp, TOUCHSTONE)) {
		turn_off(*mp, TOUCHSTONE);
		if (on(player, CANINWALL))
		    msg("The %s's touch has no effect.", mname);
		else {
		    if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
			msg("Your body begins to solidify.");
			msg("You are turned to stone !!! --More--");
			wait_for(' ');
			death(D_PETRIFY);
			return TRUE;
		    }
		    else {
			msg("The %s's touch stiffens your limbs.", mname);
			no_command = rnd(STONETIME) + 2;
		    }
		}
	    }

	    /* Undead might drain energy levels */
	    if ((on(*mp, CANDRAIN) || on(*mp, DOUBLEDRAIN)) && rnd(100) < 15) {
		if (is_carrying(TR_AMULET))
		    msg("The Amulet protects you from the %s's negative energy!", mname);
		else {
		    lower_level(mp->t_index);
		    if (on(*mp, DOUBLEDRAIN))
			lower_level(mp->t_index);
		}
		turn_on(*mp, DIDDRAIN);
	    }

	    /* permanently drain a wisdom point */
	    if (on(*mp, DRAINWISDOM) && rnd(100) < 15) {
		int ring_str;   /* Value of ring
			     * strengths */

		/* Undo any ring changes */
		ring_str = ring_value(R_ADDWISDOM) +
		    (on(player, POWERWISDOM) ? 10 : 0);
		pstats.s_wisdom -= ring_str;
		msg("You feel slightly less wise now.");
		pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
		max_stats.s_wisdom = pstats.s_wisdom;

		/* Now put back the ring changes */
		pstats.s_wisdom += ring_str;
	    }

	    /* permanently drain a intelligence point */
	    if (on(*mp, DRAINBRAIN) && rnd(100) < 15) {
		int ring_str;   /* Value of ring
			     * strengths */

		/* Undo any ring changes */
		ring_str = ring_value(R_ADDINTEL) +
		    (on(player, POWERINTEL) ? 10 : 0);
		pstats.s_intel -= ring_str;
		msg("You feel slightly less intelligent now.");
		pstats.s_intel = max(pstats.s_intel - 1, 3);
		max_stats.s_intel = pstats.s_intel;

		/* Now put back the ring changes */
		pstats.s_intel += ring_str;
	    }

	    /* Violet fungi and others hold the hero */
	    if (on(*mp, CANHOLD) && off(*mp, DIDHOLD)
		&& !is_wearing(R_FREEDOM)) {
		turn_on(player, ISHELD);
		turn_on(*mp, DIDHOLD);
		hold_count++;
	    }

	    /* suckers will suck blood and run away */
	    if (on(*mp, CANDRAW)) {
		turn_off(*mp, CANDRAW);
		turn_on(*mp, ISFLEE);
		msg("The %s sates itself with your blood.", mname);
		if ((pstats.s_hpt -= 12) <= 0) {
		    death(mp->t_index);
		    return TRUE;
		}
	    }

	    /* el stinkos will force a reduction in strength */
	    if (on(*mp, CANSMELL)) {
		turn_off(*mp, CANSMELL);
		if (save(VS_MAGIC) || is_wearing(R_SUSABILITY))
		    msg("You smell an unpleasant odor.");
		else {
		    short   odor_str = -(rnd(6) + 1);

		    if (on(player, CANSCENT)) {
			msg("You pass out from a foul odor.");
			no_command += 4 + rnd(8);
		    }
		    else if (off(player, ISUNSMELL))
			msg("You are overcome by a foul odor.");
		    if (lost_str == 0) {
			chg_str(odor_str, FALSE, TRUE);
			fuse(res_strength, 0, SMELLTIME, AFTER);
		    }
		    else
			lengthen(res_strength, SMELLTIME);
		}
	    }

	    /* Paralyzation */
	    if (on(*mp, CANPARALYZE)) {
		turn_off(*mp, CANPARALYZE);
		if (!save(VS_PARALYZATION) && no_command == 0) {
		    if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		    else {
			msg("The %s's touch paralyzes you.", mname);
			no_command = FREEZETIME;
		    }
		}
	    }

	    /* Rotting */
	    if (on(*mp, CANROT)) {
		turn_off(*mp, CANROT);
		turn_on(*mp, DOROT);
	    }

	    /* some monsters steal gold */
	    if (on(*mp, STEALGOLD)) {
		long    lastpurse;
		struct linked_list  *item;
		struct object   *obj;

		lastpurse = purse;
		purse -= GOLDCALC;
		if (!save(VS_MAGIC))
		    purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		if (purse < 0)
		    purse = 0;
		if (purse != lastpurse) {
		    msg("Your purse feels lighter.");

		    /* Give the gold to the thief */
		    for (item = mp->t_pack; item != NULL; item = next(item)) {
			obj = OBJPTR(item);
			if (obj->o_type == GOLD) {
			    obj->o_count += lastpurse - purse;
			    break;
			}
		    }

		    /* Did we do it? */
		    if (item == NULL) { /* Then make some */
			item = new_item(sizeof *obj);
			obj = OBJPTR(item);
			obj->o_type = GOLD;
			obj->o_count = lastpurse - purse;
			obj->o_hplus = obj->o_dplus = 0;
			obj->o_damage = obj->o_hurldmg = "0d0";
			obj->o_ac = 11;
			obj->o_group = 0;
			obj->o_flags = 0;
			obj->o_mark[0] = '\0';
			obj->o_pos = mp->t_pos;

			attach(mp->t_pack, item);
		    }
		}

		if (rnd(2))
		    turn_on(*mp, ISFLEE);
		turn_on(*mp, ISINVIS);
	    }

	    /* other monsters steal magic */
	    if (on(*mp, STEALMAGIC)) {
		struct linked_list  *list, *steal;
		struct object   *obj;
		int worth = 0;

		steal = NULL;
		for (list = pack; list != NULL; list = next(list)) {
		    obj = OBJPTR(list);
		    if (rnd(33) == 0) {
			if (obj->o_flags & ISBLESSED)
			    obj->o_flags &= ~ISBLESSED;
			else
			    obj->o_flags |= ISCURSED;
			msg("You feel nimble fingers reach into you pack.");
		    }
		    if ((obj != cur_armor &&
			 obj != cur_weapon &&
			 obj != cur_ring[LEFT_1] &&
			 obj != cur_ring[LEFT_2] &&
			 obj != cur_ring[LEFT_3] &&
			 obj != cur_ring[LEFT_4] &&
			 obj != cur_ring[LEFT_5] &&
			 obj != cur_ring[RIGHT_1] &&
			 obj != cur_ring[RIGHT_2] &&
			 obj != cur_ring[RIGHT_3] &&
			 obj != cur_ring[RIGHT_4] &&
			 obj != cur_ring[RIGHT_5] &&
			 !(obj->o_flags & ISPROT) &&
			 is_magic(obj)
			 || level > 95)
			&& get_worth(obj) > worth) {
			steal = list;
			worth = get_worth(obj);
		    }
		}
		if (steal != NULL) {
		    struct object   *obj;

		    obj = OBJPTR(steal);
		    if (obj->o_count > 1 && obj->o_group == 0) {
			int oc;
			struct linked_list  *nitem;
			struct object   *op;

			oc = --(obj->o_count);
			obj->o_count = 1;
			nitem = new_item(sizeof *obj);
			op = OBJPTR(nitem);
			*op = *obj;
			msg("The %s stole %s!", mname, inv_name(obj, LOWERCASE));
			obj->o_count = oc;
			attach(mp->t_pack, nitem);
		    }
		    else {
			msg("The %s stole %s!", mname, inv_name(obj, LOWERCASE));
			obj->o_flags &= ~ISCURSED;
			dropcheck(obj);
			rem_pack(obj);
			attach(mp->t_pack, steal);
			if (obj->o_type == ARTIFACT)
			    has_artifact &= ~(1 << obj->o_which);
		    }
		    if (obj->o_flags & ISOWNED) {
			turn_on(*mp, NOMOVE);
			msg("The %s is transfixed by your ownership spell.",
			    mname);
		    }
		    if (rnd(2))
			turn_on(*mp, ISFLEE);
		    turn_on(*mp, ISINVIS);
		    updpack();
		}
	    }
	}
    }
    else {
	/* If the thing was trying to surprise, no good */
	if (on(*mp, CANSURPRISE))
	    turn_off(*mp, CANSURPRISE);

	if (on(*mp, DOROT)) {
	    if (player.t_ctype != C_PALADIN
		|| !(player.t_ctype == C_NINJA && pstats.s_lvl > 6)) {
		msg(terse ? "You feel weaker."
		 : "Your skin crawls and you feel weaker.");
		pstats.s_hpt -= 2;
		if (pstats.s_hpt <= 0) {
		    death(mp->t_index); /* Bye bye life ... */
		    return TRUE;
		}
	    }
	    else
		msg("You feel something cold and clammy against you.");
	}
	m_bounce(weapon, mname);
    }
    if (fight_flush)
	flushout();
    count = 0;
    status(FALSE);
    return (did_hit);
}


/*
 * mon_mon_attack: A monster attacks another monster
 */
mon_mon_attack(attacker, mon, weapon, thrown)
struct thing    *attacker;
struct linked_list  *mon;
struct object   *weapon;
bool    thrown;
{
    struct thing    *attackee = THINGPTR(mon);
    bool    did_hit = FALSE;
    bool    visible = cansee(attackee->t_pos.y, attackee->t_pos.x);
    char    *mname1 = monsters[attacker->t_index].m_name;
    char    *mname2 = monsters[attackee->t_index].m_name;

    /*
     * Similar monsters don't hit each other
     */
    if (attacker->t_index == attackee->t_index) {
	if (attacker == THINGPTR(fam_ptr) && visible)
	    msg("Master, I cannot hit one of my brethen.");
	return FALSE;
    }

    /*
     * stop running and any healing
     */
    attackee->t_rest_hpt = attackee->t_rest_pow = 0;
    attacker->t_rest_hpt = attacker->t_rest_pow = 0;


    if (roll_em(attacker, attackee, weapon, thrown,
	wield_weap(weapon, attacker))) {
	did_hit = TRUE;

	if (visible && on(*attackee, CANSURPRISE))
	    turn_off(*attackee, CANSURPRISE);
	if (visible && weapon != NULL)
	    msg("The %s's %s hits the %s.", mname1,
		weaps[weapon->o_which].w_name, mname2);
	else if (visible)
	    msg("The %s hits the %s.", mname1, mname2);

	if (attackee->t_stats.s_hpt <= 0) {
	    killed(attacker, mon, MESSAGE,
		 on(*attacker, ISFAMILIAR) ? POINTS : NOPOINTS);
	    return TRUE;
	}

    }
    else if (visible && weapon != NULL)
	msg("The %s's %s misses the %s.", mname1,
	    weaps[weapon->o_which].w_name, mname2);
    else if (visible)
	msg("The %s misses the %s.", mname1, mname2);

    if (off(*attackee, ISMEAN) && off(*attackee, ISFAMILIAR))
	turn_on(*attackee, ISRUN);
    if (fight_flush)
	flushout();
    count = 0;
    status(FALSE);
    return (did_hit);
}


/*
 * swing: returns true if the swing hits
 */

swing(class, at_lvl, op_arm, wplus)
short   class;
int at_lvl, op_arm, wplus;
{
    int res = rnd(20) + 1;
    int need;

    need = att_mat[class].base -
	att_mat[class].factor *
	((min(at_lvl, att_mat[class].max_lvl) -
	  att_mat[class].offset) / att_mat[class].range) +
	(10 - op_arm);
    if (need > 20 && need <= 25)
	need = 20;

    return (res + wplus >= need);
}

/*
 * init_exp: set up initial experience level change threshold
 */

void    init_exp()
{
    max_stats.s_exp = e_levels[player.t_ctype];
}

/*
 * next_exp_level: Do the next level arithmetic Returns number of levels to
 * jump
 */
int
next_exp_level(print_message)
bool    print_message;
{
    int level_jump = 0;

    while (pstats.s_exp >= max_stats.s_exp) {
	pstats.s_exp -= max_stats.s_exp;    /* excess experience
			     * points */
	level_jump++;
	if (max_stats.s_exp < 0x3fffffffL)  /* 2^30 - 1 */
	    max_stats.s_exp *= 2L;  /* twice as many for next */
    }
    if (print_message)
	msg("You need %d more points to attain the %stitle of %s.",
	    max_stats.s_exp - pstats.s_exp,
	    (pstats.s_lvl > 14 ? "next " : ""),
	    cnames[player.t_ctype][min(pstats.s_lvl, 14)]);

    return (level_jump);
}

/*
 * check_level: Check to see if the guy has gone up a level.
 */
check_level()
{
    int num_jumped, j, add;
    int nsides;

    if ((num_jumped = next_exp_level(NOMESSAGE)) <= 0)
	return;

    pstats.s_lvl += num_jumped; /* new experience level */
    switch (player.t_ctype) {
	when    C_MAGICIAN:
	case C_ILLUSION:
	    nsides = 4;
	when    C_THIEF:
	case C_ASSASIN:
	case C_NINJA:
	case C_MONSTER:
	default:
	    nsides = 6;
	when    C_CLERIC:
	case C_DRUID:
	    nsides = 8;
	when    C_FIGHTER:
	case C_PALADIN:
	case C_RANGER:
	    nsides = 12;
	break;
    }

    /* Take care of multi-level jumps */
    for (add = 0, j = 0; j < num_jumped; j++) {
	int increase = roll(1, nsides) + const_bonus();

	add += max(1, increase);
    }
    max_stats.s_hpt += add;
    pstats.s_hpt += add;
    msg("Welcome, %s, to level %d.",
	cnames[player.t_ctype][min(pstats.s_lvl - 1, 14)], pstats.s_lvl);
    next_exp_level(MESSAGE);

    /* Now add new spell points and learn new spells */
    nsides = 16 - nsides;
    for (add = 0, j = 0; j < num_jumped; j++) {
	int increase = roll(1, nsides) + int_wis_bonus();

	add += max(1, increase);
    }
    max_stats.s_power += add;
    pstats.s_power += add;
    learn_new_spells();

    /*
     * Create a more powerful familiar (if player has one)
     */
    if (on(player, HASFAMILIAR) && on(player, CANSUMMON))
	summon_monster((short) 0, FAMILIAR, NOMESSAGE);
}

/*
 * roll_em: Roll several attacks
 */

roll_em(att_er, def_er, weap, thrown, cur_weapon)
struct thing    *att_er, *def_er;
struct object   *weap;
bool    thrown;
struct object   *cur_weapon;
{
    struct stats    *att = &att_er->t_stats;
    struct stats    *def = &def_er->t_stats;
    int ndice, nsides, nplus, def_arm;
    char    *cp;
    int prop_hplus = 0, prop_dplus = 0;
    bool    is_player = (att_er == &player);
    bool    did_hit = FALSE;
    char    *strchr();

    if (weap == NULL)
	cp = att->s_dmg;
    else if (!thrown)
	cp = weap->o_damage;
    else if ((weap->o_flags & ISMISL) && cur_weapon != NULL &&
	 cur_weapon->o_which == weap->o_launch)
    {
	cp = weap->o_hurldmg;
	prop_hplus = cur_weapon->o_hplus;
	prop_dplus = cur_weapon->o_dplus;
    }
    else
	cp = (weap->o_flags & ISMISL ? weap->o_damage :
	    weap->o_hurldmg);

    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	/* Is attacker weak? */
	if (on(*att_er, HASSTINK))
	    hplus -= 2;

	if (is_player)
	{
	    hplus += hitweight();   /* adjust for encumberence */
	    dplus += hung_dam();    /* adjust damage for hungry
			 * player */
	    dplus += ring_value(R_ADDDAM);
	}
	ndice = atoi(cp);
	if (cp == NULL || (cp = strchr(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (cp != NULL && (cp = strchr(cp, '+')) != NULL)
	    nplus = atoi(++cp);
	else
	    nplus = 0;

	if (def == &pstats)
	{
	    if (on(*att_er, NOMETAL) && cur_armor != NULL &&
		(cur_armor->o_which == RING_MAIL ||
		 cur_armor->o_which == SCALE_MAIL ||
		 cur_armor->o_which == CHAIN_MAIL ||
		 cur_armor->o_which == SPLINT_MAIL ||
		 cur_armor->o_which == BANDED_MAIL ||
		 cur_armor->o_which == GOOD_CHAIN ||
		 cur_armor->o_which == PLATE_MAIL ||
		 cur_armor->o_which == PLATE_ARMOR))
		def_arm = def->s_arm;
	    else if (cur_armor != NULL)
		def_arm = cur_armor->o_ac - 10 + pstats.s_arm;
	    else
		def_arm = def->s_arm;
	    def_arm -= ring_value(R_PROTECT);
	}
	else
	    def_arm = def->s_arm;

	if ((weap != NULL && weap->o_type == WEAPON &&
	     (weap->o_flags & ISSILVER) &&
	    !save_throw(VS_MAGIC, def_er)) ||
	    swing(att_er->t_ctype, att->s_lvl,
	    def_arm - dext_prot(def->s_dext),
	   hplus + str_plus(att->s_str) + dext_plus(att->s_dext)))
	{
	    damage = roll(ndice, nsides) + dplus + nplus +
		add_dam(att->s_str);

	    /* Rangers do +1/lvl vs. ISLARGE */
	    if (att_er->t_ctype == C_RANGER && on(*def_er, ISLARGE))
		damage += pstats.s_lvl;

	    /* Ninja do +1 per lvl/2 */
	    if (att_er->t_ctype == C_NINJA)
		damage += pstats.s_lvl / 2;

	    /* Check for half damage monsters */
	    if (on(*def_er, HALFDAMAGE) && (weap != NULL) &&
		!((weap->o_flags & CANBURN) &&
		on(*def_er, CANBBURN)))
		damage /= 2;

	    /* undead get twice damage from silver weapons */
	    if (on(*def_er, ISUNDEAD) &&
		(weap != NULL) && (weap->o_flags & ISSILVER))
		damage *= 2;

	    /* Check for fireproof monsters */
	    if (on(*def_er, NOFIRE) && (weap != NULL) &&
		(weap->o_flags & CANBURN))
		damage = 0;

	    /* Check for metal proof monsters */
	    if (on(*def_er, NOMETAL) && (weap != NULL) &&
		(weap->o_flags & ISMETAL))
		damage = 0;

	    /* Check for monsters that ignore sharp weapons  */
	    if (on(*def_er, NOSHARP) && (weap != NULL) &&
		(weap->o_flags & ISSHARP))
		damage = 0;

	    /* Check for poisoned weapons */
	    if ((weap != NULL) && (weap->o_flags & ISPOISON)
		&& off(*def_er, ISUNDEAD)
		&& !save_throw(VS_POISON, def_er))
		damage = max(damage, (def->s_hpt / 2) + 5);

	    /* Check for no-damage and division */
	    if (on(*def_er, BLOWDIVIDE) && rnd(3) == 0 &&
		!((weap != NULL) && (weap->o_flags & CANBURN))) {
		damage = 0;
		creat_mons(def_er, def_er->t_index, NOMESSAGE);
	    }

	    damage = max(0, damage);

	    /*
	     * sleeping monsters are backstabbed by certain
	     * player classes
	     */
	    if (is_player && !thrown && damage > 0 &&
		(off(*def_er, ISRUN) || def_er->t_no_move > 0) &&
		(player.t_ctype == C_THIEF ||
		 player.t_ctype == C_NINJA ||
		 player.t_ctype == C_ASSASIN)
		&& (wield_ok(&player, cur_weapon, NOMESSAGE))
		&& (wear_ok(&player, cur_armor, NOMESSAGE))) {
		damage *= (pstats.s_lvl / 4 + 2);
		msg("You backstabbed the %s %d times!",
		    monsters[def_er->t_index].m_name,
		    (pstats.s_lvl / 4) + 2);
		if (player.t_ctype == C_NINJA ||
		    player.t_ctype == C_ASSASIN)
		    pstats.s_exp += def_er->t_stats.s_exp
			/ 2;
	    }

	    def->s_hpt -= damage;   /* Do the damage */
	    debug("Hit %s for %d (%d) ",
		  monsters[def_er->t_index].m_name, damage,
		  def_er->t_stats.s_hpt);

	    if (is_player && is_wearing(R_VREGEN)) {
		damage = (ring_value(R_VREGEN) * damage) / 3;
		pstats.s_hpt = min(max_stats.s_hpt,
		    pstats.s_hpt + damage);
	    }

	    /*
	     * stun monsters when taking more than 1/3 their max
	     * hpts
	     */
	    if (is_player && !thrown && !did_hit && player.t_ctype == C_FIGHTER &&
		damage > def_er->maxstats.s_hpt / 3) {
		if (def->s_hpt > 0) {
		    msg("The %s has been stunned! HP=%d",
		      monsters[def_er->t_index].m_name,def->s_hpt);
		    def_er->t_no_move += rnd(4) + 1;
		}
		pstats.s_exp += def_er->t_stats.s_exp / 4;
	    }

	    did_hit = TRUE;
	}
	if (cp == NULL || (cp = strchr(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname: Figure out the monsters name
 */

char    *
prname(who)
char    *who;
{
    static char *mon = "monster";

    if (on(player, ISBLIND))
	return mon;
    else
	return who;
}

/*
 * hit: Print a message to indicate a succesful hit
 */

hit(ee)
char    *ee;
{
    char    *s;

    if (fighting)
	return;
    addmsg("You");
    if (terse)
	addmsg(" hit");
    else {
	switch (rnd(15)) {
	    otherwise:  s = " hit";
	    when 1: s = " score an excellent hit on";
	    when 2: s = " injure";
	    when 3: s = " swing and hit";
	    when 4: s = " damage";
	    when 5: s = " barely nick";
	    when 6: s = " scratch";
	    when 7: s = " gouge a chunk out of";
	    when 8: s = " severely wound";
	    when 9: s = " counted coup on";
	    when 10:    s = " drew blood from";
	    when 11:    s = " nearly decapitate";
	    when 12:    s = " deal a wacking great blow to";
	}
	addmsg("%s the %s", s, prname(ee));
    }
    addmsg(".");
    endmsg();
}

/*
 * miss: Print a message to indicate a poor swing
 */
miss(ee)
char    *ee;
{
    char    *s;

    if (fighting)
	return;
    addmsg("You");
    if (terse)
	addmsg(" miss");
    else {
	switch (rnd(10)) {
	    otherwise:  s = " miss";
	    when 1: s = " swing and miss";
	    when 2: s = " barely miss";
	    when 3: s = " don't hit";
	    when 4: s = " wildly windmill around";
	    when 5: s = " almost fumble while missing";
	}
	addmsg("%s the %s", s, prname(ee));
    }
    addmsg(".");
    endmsg();
}

/*
 * save_throw: See if a creature save against something
 */
save_throw(which, tp)
int which;
struct thing    *tp;
{
    int need;
    int ring_bonus = 0;
    int armor_bonus = 0;
    int class_bonus = 0;

    if (tp == &player) {
	if (player.t_ctype == C_PALADIN)
	    class_bonus = 2;
	ring_bonus = ring_value(R_PROTECT);
	if (cur_armor != NULL && (which == VS_WAND ||
	    which == VS_MAGIC)) {
	    if (cur_armor->o_which == MITHRIL)
		armor_bonus += 5;
	    armor_bonus += (armors[cur_armor->o_which].a_class
		    - cur_armor->o_ac);
	}
    }

    need = 14 + which - tp->t_stats.s_lvl / 2 - ring_bonus -
	armor_bonus - class_bonus;

    /* Roll of 1 always fails; 20 always saves */

    if (need < 1)
	need = 1;
    else if (need > 20)
	need = 20;
    return (roll(1, 20) >= need);
}

/*
 * save: See if he saves against various nasty things
 */

save(which)
int which;
{
    return save_throw(which, &player);
}

/*
 * dext_plus: compute to-hit bonus for dexterity
 */

dext_plus(dexterity)
int dexterity;
{
    return ((dexterity - 10) / 3);
}


/*
 * dext_prot: compute armor class bonus for dexterity
 */

dext_prot(dexterity)
int dexterity;
{
    return ((dexterity - 9) / 2);
}

/*
 * str_plus: compute bonus/penalties for strength on the "to hit" roll
 */

static int  strtohit[] =
{
    0, 0, 0, -3, -2, -2, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 3, 3, 4, 4, 5, 6, 7
};

str_plus(str)
short   str;
{
    int ret_val = str;

    if (str < 3)
	ret_val = 3;
    else if (str > 25)
	ret_val = 25;

    return (strtohit[ret_val]);
}

/*
 * add_dam: compute additional damage done for exceptionally high or low
 * strength
 */
static int  str_damage[] =
{
    0, 0, 0, -1, -1, -1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 2, 7, 8, 9, 10, 11, 12, 14
};

add_dam(str)
short   str;
{
    int ret_val = str;

    if (str < 3)
	ret_val = 3;
    else if (str > 25)
	ret_val = 25;

    return (str_damage[ret_val]);
}

/*
 * hung_dam: Calculate damage depending on players hungry state
 */
hung_dam()
{
    int howmuch;

    switch (hungry_state) {
	case F_OK:
	case F_HUNGRY:
	    howmuch = 0;
	when F_WEAK: howmuch = -1;
	when F_FAINT: howmuch = -2;
    }
    return howmuch;
}

/*
 * raise_level: The guy just magically went up a level.
 */

raise_level()
{
    pstats.s_exp = max_stats.s_exp;
    check_level();
}

/*
 * thunk: A missile hits a monster
 */

thunk(weap, mname)
struct object   *weap;
char    *mname;
{
    if (fighting)
	return;
    if (weap->o_type == WEAPON)
	msg("The %s hits the %s.", weaps[weap->o_which].w_name,
	    prname(mname));
    else
	msg("You hit the %s.", prname(mname));
}

/*
 * m_thunk: A missile from a monster hits the player
 */
m_thunk(weap, mname)
struct object   *weap;
char    *mname;
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("The %s's %s hits you.", prname(mname),
	    weaps[weap->o_which].w_name);
    else
	msg("The %s hits you.", prname(mname));
}

/*
 * bounce: A missile misses a monster
 */

bounce(weap, mname)
struct object   *weap;
char    *mname;
{
    if (fighting)
	return;
    if (weap->o_type == WEAPON)
	msg("The %s misses the %s.", weaps[weap->o_which].w_name,
	    prname(mname));
    else
	msg("You missed the %s.", prname(mname));
}

/*
 * m_bounce: A missile from a monster misses the player
 */

m_bounce(weap, mname)
struct object   *weap;
char    *mname;
{
    if (fighting)
	return;
    if (weap != NULL && weap->o_type == WEAPON)
	msg("The %s's %s misses you.", prname(mname),
	    weaps[weap->o_which].w_name);
    else
	msg("The %s misses you.", prname(mname));
}

/*
 * remove a monster from the screen
 */
remove_monster(mp, item)
coord   *mp;
struct linked_list  *item;
{
    struct thing    *tp = THINGPTR(item);
    char    ch = tp->t_oldch;

    mvwaddch(mw, mp->y, mp->x, ' ');
    if ((int) ch < 33 || ch == ' ')
	ch = mvwinch(stdscr, mp->y, mp->x);
    if (cansee(mp->y, mp->x))
	mvwaddch(cw, mp->y, mp->x, ch);
    detach(mlist, item);
    discard(item);
}

/*
 * is_magic: Returns true if an object radiates magic
 */

is_magic(obj)
struct object   *obj;
{
    switch (obj->o_type) {
	case ARMOR:
	    return obj->o_ac != armors[obj->o_which].a_class;
	when    WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when    POTION:
	case SCROLL:
	case STICK:
	case RING:
	case ARTIFACT:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed: Called to put a monster to death
 */

killed(killer, item, print_message, give_points)
struct thing    *killer;    /* Who did the dirty deed */
struct linked_list  *item;  /* Who died */
bool    print_message, give_points; /* Give killer exp points? */
{
    struct linked_list  *pitem, *nitem;
    struct thing    *tp = THINGPTR(item);
    bool    visible = cansee(tp->t_pos.y, tp->t_pos.x);
    bool    is_player = (killer == (&player));

    if (item == curr_mons)
	curr_mons = NULL;
    else if (item == next_mons)
	next_mons = next(next_mons);

    if (on(*tp, WASSUMMONED)) {
	extinguish(unsummon);
	turn_off(player, HASSUMMONED);
    }

    if (print_message && visible) {
	if (is_player)
	    addmsg(terse ? "Defeated " : "You have defeated ");
	else
	    addmsg("The %s has defeated ",
		monsters[killer->t_index].m_name);

	if (on(player, ISBLIND))
	    msg("it.");
	else {
	    if (!terse)
		addmsg("the ");
	    msg("%s.", monsters[tp->t_index].m_name);
	}
    }

    if (killer != NULL && item == fam_ptr) {    /* The player's familiar
			     * died */
	turn_off(player, HASFAMILIAR);
	fam_ptr = NULL;
	msg("An incredible wave of sadness sweeps over you.");
	/* There should be some penalty here... */
    }

    check_residue(tp);

    if (is_player) {
	fighting = FALSE;
	if (on(*tp, ISFRIENDLY)) {
	    msg("You feel a slight chill run up and down your spine.");
	    luck++;
	}
    }
    if (give_points) {
	if (killer != NULL) {
	    killer->t_stats.s_exp += tp->t_stats.s_exp;
	    if (on(*killer, ISFAMILIAR))
		pstats.s_exp += tp->t_stats.s_exp;
	}

	if (is_player) {
	    switch (player.t_ctype) {
		when    C_CLERIC:
		case C_PALADIN:
		if (on(*tp, ISUNDEAD) || on(*tp, ISUNIQUE)) {
		    pstats.s_exp += tp->t_stats.s_exp / 2;
		    msg("You are to be commended for smiting the ungodly.");
		}
		when    C_DRUID:
		case C_RANGER:
		if (on(*tp, ISLARGE)) {
		    pstats.s_exp += tp->t_stats.s_exp / 2;
		    msg("Congratulations on smiting a dangerous monster.");
		}
		when    C_MAGICIAN:
		case C_ILLUSION:
		if (on(*tp, DRAINBRAIN)) {
		    pstats.s_exp += tp->t_stats.s_exp / 2;
		    msg("Congratulations on smiting a dangerous monster.");
		}

	    }
	}
	check_level();
    }

    /* Empty the monsters pack */
    for (pitem = tp->t_pack; pitem != NULL; pitem = nitem) {
	struct object   *obj = OBJPTR(pitem);

	nitem = next(pitem);

	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);
	if (killer == NULL)
	    discard(pitem);
	else
	    fall(killer, pitem, FALSE);
    }
    remove_monster(&tp->t_pos, item);
}


/*
 * Returns a pointer to the weapon the monster is wielding corresponding to
 * the given thrown weapon
 */

struct object   *
wield_weap(weapon, mp)
struct object   *weapon;
struct thing    *mp;
{
    int look_for;
    struct linked_list  *pitem;

    if (weapon == NULL)
	return (NULL);
    switch (weapon->o_which) {
	when    BOLT:
	    look_for = CROSSBOW;
	when    ARROW:
	    look_for = BOW;
	when    SILVERARROW:
	case FLAMEARROW:
	    look_for = BOW;
	when    ROCK:
	case BULLET:
	    look_for = SLING;
	otherwise:
	    return (NULL);
    }

    for (pitem = mp->t_pack; pitem; pitem = next(pitem))
	if ((OBJPTR(pitem))->o_which == look_for)
	    return (OBJPTR(pitem));

    return (NULL);
}

/*
 * Summon - see whether to summon help Returns TRUE if help comes, FALSE
 * otherwise
 */
summon_help(mons, force)
struct thing    *mons;
bool    force;
{
    char    *helpname;
    int which, i;
    char    *mname = monsters[mons->t_index].m_name;
    char    *strcmp();

    /* Try to summon if less than 1/3 max hit points */
    if (on(*mons, CANSUMMON) &&
	(force == FORCE ||
      (mons->t_stats.s_hpt < mons->maxstats.s_hpt / 3) &&
      (rnd(40 * 10) < (mons->t_stats.s_lvl * mons->t_stats.s_intel)))) {
	turn_off(*mons, CANSUMMON);
	msg("The %s summons its attendants!", mname);
	helpname = monsters[mons->t_index].m_typesum;
	for (which = 1; which < nummonst; which++) {
	    if (strcmp(helpname, monsters[which].m_name) == NULL)
		break;
	}
	if (which >= nummonst) {
	    debug("Couldn't find summoned one.");
	    return (FALSE);
	}

	/* summoned monster was genocided */
	if (!monsters[which].m_normal) {
	    msg("The %s becomes very annoyed at you!", mname);
	    if (on(*mons, ISSLOW))
		turn_off(*mons, ISSLOW);
	    else
		turn_on(*mons, ISHASTE);
	    return (FALSE);
	}
	else
	    for (i = 0; i < monsters[mons->t_index].m_numsum; i++) {
		struct linked_list  *ip;
		struct thing    *tp;

		if ((ip = creat_mons(mons, which, NOMESSAGE)) != NULL) {
		    tp = THINGPTR(ip);
		    turn_off(*tp, ISFRIENDLY);
		}
	    }
    }
    return (TRUE);
}

/*
 * maxdamage() -    return the max damage a weapon can do
 */
maxdamage(cp)
char    *cp;
{
    int ndice, nsides, nplus;

    ndice = atoi(cp);
    if (cp == NULL || (cp = strchr(cp, 'd')) == NULL)
	return (0);
    nsides = atoi(++cp);
    if (cp != NULL && (cp = strchr(cp, '+')) != NULL)
	nplus = atoi(++cp);
    else
	nplus = 0;

    return (ndice * nsides + nplus);
}
