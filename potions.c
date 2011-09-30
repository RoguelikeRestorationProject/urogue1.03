/*
    potions.c  -  Function(s) for dealing with potions
   
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
#include "death.h"

int shield_ac = 0;      /* AC bonus via P_SHIELD */
char    *find_slot();       /* actually (struct delayed_action *) */

/*
 * quaff - drink a potion (or effect a potion-like spell)
 *
 * quaffer: who does it
 * which:   which P_POTION (-1 means ask from pack)
 * flags:   ISBLESSED, ISCURSED
 */
quaff(quaffer, which, flags)
struct thing    *quaffer;
int which;
int flags;
{
    struct object   *obj;
    struct thing    *th;
    struct stats    *curp = &(quaffer->t_stats);
    struct stats    *maxp = &(quaffer->maxstats);
    bool    blessed = flags & ISBLESSED;
    bool    cursed = flags & ISCURSED;
    bool    is_potion = (which < 0 ? TRUE : FALSE);

    struct linked_list  *item, *titem;
    char    buf[2 * LINELEN];

    if (quaffer != &player) {
	monquaff(quaffer, which, flags);
	return;
    }
    if (is_potion) {    /* A regular potion */
	if ((item = get_item("quaff", POTION)) == NULL)
	    return;
	obj = OBJPTR(item);
	if (obj->o_type != POTION) {
	    msg("You can't drink that!");
	    return;
	}

	/* Calculate its effect */
	flags = obj->o_flags;
	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;
	which = obj->o_which;

	/* remove it from the pack */
	rem_pack(obj);
	discard(item);
	updpack();
    }

    switch (which) {
	when    P_CLEAR:
	    if (cursed) {
		if (off(player, ISCLEAR)) {
		    msg("Wait, what's going on here. Huh? What? Who?");
		    if (on(player, ISHUH))
			lengthen(unconfuse, rnd(8) +
			    HUHDURATION);
		    else
			fuse(unconfuse, 0, rnd(8) +
			    HUHDURATION, AFTER);
		    turn_on(player, ISHUH);
		}
		else
		    msg("You feel dizzy for a moment, but it passes.");
	    }
	    else {
		if (blessed) {  /* Make player immune for the
			 * whole game */
		    extinguish(unclrhead);  /* If we have
				 * a fuse,
				 * put it out */
		    msg("A strong blue aura surrounds your head.");
		}
		else {  /* Just light a fuse for how long
		     * player is safe */
		    if (off(player, ISCLEAR)) {
			fuse(unclrhead, 0,
			    CLRDURATION, AFTER);
			msg("A faint blue aura surrounds your head.");
		    }
		    else {  /* If we have a fuse lengthen
			 * it, else we are permanently
			 * clear. */
			if (find_slot(unclrhead) == NULL)
			    msg("Your blue aura continues to glow strongly.");
			else {
			    lengthen(unclrhead, CLRDURATION);
			    msg("Your blue aura brightens for a moment.");
			}
		    }
		}
		turn_on(player, ISCLEAR);
		/* If player is confused, unconfuse him */
		if (on(player, ISHUH)) {
		    extinguish(unconfuse);
		    unconfuse();
		}
	    }
	when    P_HEALING:
	    if (cursed) {
		if (player.t_ctype != C_PALADIN
		    && !(player.t_ctype == C_NINJA
		    && curp->s_lvl > 12)
		    && !save(VS_POISON)) {
		    feel_message();
		    curp->s_hpt /= 2;
		    curp->s_power /= 2;
		    if ((curp->s_hpt -= 1) <= 0) {
			death(D_POISON);
			return;
		    }
		}
		else
		    msg("You feel momentarily sick.");
	    }
	    else {
		int nsides = (blessed ? 8 : 4);
		int hpt_gain = roll(curp->s_lvl, nsides);
		int power_gain = roll(curp->s_lvl, nsides);

		if (blessed && on(player, ISHUH)) {
		    extinguish(unconfuse);
		    unconfuse();
		}
		curp->s_hpt = min(curp->s_hpt + hpt_gain,
		    maxp->s_hpt);
		if (is_potion) {    /* Do not bump power or
			     * maximums if spell */
		    know_items[TYP_POTION][P_HEALING] =
			TRUE;
		    curp->s_power = min(curp->s_power +
			power_gain, maxp->s_power);
		    if (maxp->s_hpt == curp->s_hpt)
			maxp->s_hpt = curp->s_hpt
			    +=roll(1, nsides);
		    if (maxp->s_power == curp->s_power)
			maxp->s_power = curp->s_power
			    += roll(1, nsides);
		}
		msg("You begin to feel %sbetter.",
		    blessed ? "much " : "");
		if (off(player, PERMBLIND))
		    sight();
	    }
	when    P_GAINABIL:{
	    short   ctype;

	    if (!is_potion || pstats.s_arm <= 0)
		feel_message();
	    else {
		if (blessed) {  /* add to all attributes */
		    add_intelligence(FALSE);
		    add_dexterity(FALSE);
		    add_strength(FALSE);
		    add_wisdom(FALSE);
		    add_const(FALSE);
		}
		else {
		    if (rnd(100) < 70)
		    /* probably change own ability */
			ctype = player.t_ctype;
		    else
			switch (rnd(4)) {
			    when 0: ctype = C_FIGHTER;
			    when 1: ctype = C_MAGICIAN;
			    when 2: ctype = C_CLERIC;
			    when 3: ctype = C_THIEF;
			}
			switch (ctype) {
			    when C_FIGHTER:add_strength(cursed);
			    when C_PALADIN:add_strength(cursed);
			    when C_RANGER:add_strength(cursed);
			    when C_MAGICIAN:add_intelligence(cursed);
			    when C_ILLUSION:add_intelligence(cursed);
			    when C_CLERIC:add_wisdom(cursed);
			    when C_DRUID:add_wisdom(cursed);
			    when C_THIEF:add_dexterity(cursed);
			    when C_ASSASIN:add_dexterity(cursed);
			    when C_NINJA:add_dexterity(cursed);
			    otherwise: msg("You're a strange type!");
			}
		    }
		    if (rnd(100) < 10)
			add_const(cursed);
		    if (rnd(100) < 60)
			curp->s_arm += (cursed ?
			    1 : -1);

		    if (!cursed)
			know_items[TYP_POTION][P_GAINABIL] = TRUE;
		}
	    }
	when    P_MONSTDET:

	    /*
	     * Potion of monster detection, if there are monsters,
	     * detect them
	     */
	    if (is_potion)
		know_items[TYP_POTION][P_MONSTDET] = TRUE;

	    if (cursed) {
		int num = roll(3, 6);
		int i;
		char    ch;
		struct room *rp;
		coord   pos;

		msg("You begin to sense the presence of monsters.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    if (rnd(2))
			ch = 'a' + rnd(26);
		    else
			ch = 'A' + rnd(26);
		    mvwaddch(hw, pos.y, pos.x, ch);
		}
		waddstr(cw, morestr);
		overlay(hw, cw);
		wrefresh(cw);
		wait_for(' ');
		msg("");
	    }
	    else if (mlist != NULL) {
		msg("You begin to sense the presence of monsters.");
		waddstr(cw, morestr);
		overlay(mw, cw);
		wrefresh(cw);
		wait_for(' ');
		msg("");
		if (blessed)
		    turn_on(player, BLESSMONS);
	    }
	    else
		nothing_message(flags);
	when    P_TREASDET:

	    /*
	     * Potion of magic detection.  Show the potions and
	     * scrolls
	     */
	    if (cursed) {
		int num = roll(3, 3);
		int i;
		char    ch;
		struct room *rp;
		coord   pos;

		msg("You sense the presence of magic on this level.");
		wclear(hw);
		for (i = 1; i < num; i++) {
		    rp = &rooms[rnd_room()];
		    rnd_pos(rp, &pos);
		    if (rnd(9) == 0)
			ch = BMAGIC;
		    else if (rnd(9) == 0)
			ch = CMAGIC;
		    else
			ch = MAGIC;
		    mvwaddch(hw, pos.y, pos.x, ch);
		}
		waddstr(cw, morestr);

		overlay(hw, cw);
		wrefresh(cw);
		wait_for(' ');
		msg("");
		if (is_potion)
		    know_items[TYP_POTION][P_TREASDET] =
			TRUE;
		break;
	    }
	    if (blessed)
		turn_on(player, BLESSMAGIC);
	    if (lvl_obj != NULL) {
		struct linked_list  *mobj;
		struct object   *tp;
		bool    show;

		show = FALSE;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL;
		    mobj = next(mobj)) {
		    tp = OBJPTR(mobj);
		    if (is_magic(tp)) {
			char    mag_type = MAGIC;

		    if (blessed)
			if (tp->o_flags & ISCURSED)
			    mag_type = CMAGIC;
			else if (tp->o_flags &
			    ISBLESSED)
			    mag_type = BMAGIC;
			show = TRUE;
			mvwaddch(hw, tp->o_pos.y,
			    tp->o_pos.x, mag_type);
		    }
		}
		for (titem = mlist; titem != NULL;
		    titem = next(titem)) {
		    struct linked_list  *pitem;

		    th = THINGPTR(titem);
		    for (pitem = th->t_pack;
			pitem != NULL;
			pitem = next(pitem)) {
			if (is_magic(OBJPTR(pitem))) {
			    show = TRUE;
			    mvwaddch(hw,
				th->t_pos.y,
				th->t_pos.x,
				MAGIC);
			}
		    }
		}
		if (show) {
		    msg("You sense the presence of magic on this level.");
		    if (is_potion)
			know_items[TYP_POTION][P_TREASDET] = TRUE;
		    waddstr(cw, morestr);
		    overlay(hw, cw);
		    wrefresh(cw);
		    wait_for(' ');
		    msg("");
		    break;
		}
	    }
	    nothing_message(flags);
	when    P_SEEINVIS:
	    if (cursed) {
		if (off(player, ISBLIND) &&
		    !is_wearing(R_SEEINVIS)) {
		    msg("A cloak of darkness falls around you.");
		    turn_on(player, ISBLIND);
		    fuse(sight, 0, SEEDURATION, AFTER);
		    look(FALSE);
		}
		else
		    msg("Your eyes stop tingling for a moment.");
	    }
	    else if (off(player, PERMBLIND)) {
		if (is_potion)
		    know_items[TYP_POTION][P_SEEINVIS] =
			TRUE;
		if (off(player, CANSEE)) {
		    turn_on(player, CANSEE);
		    msg("Your eyes begin to tingle.");
		    fuse(unsee, 0, blessed ? SEEDURATION * 3 : SEEDURATION, AFTER);
		    light(&hero);
		}
		else if (find_slot(unsee) != NULL) {
		    nothing_message(ISNORMAL);
		    lengthen(unsee, blessed ? SEEDURATION * 3 : SEEDURATION);
		}
		sight();
	    }
	when    P_PHASE:
	    if (cursed) {
		msg("You can't move.");
		no_command = HOLDTIME;
	    }
	    else {
		short   duration = (blessed ? 3 : 1);

		if (is_potion)
		    know_items[TYP_POTION][P_PHASE] = TRUE;
		if (on(player, CANINWALL))
		    lengthen(unphase, duration *
			PHASEDURATION);
		else {
		    fuse(unphase, 0, duration *
			PHASEDURATION, AFTER);
		    turn_on(player, CANINWALL);
		}
		msg("You feel %slight-headed!",
		    blessed ? "very " : "");
	    }
	when    P_RAISELEVEL:
	    if (cursed || (!is_potion && pstats.s_lvl > 20))
		lower_level(D_POTION);
	    else {
		msg("You suddenly feel %smore skillful.",
		    blessed ? "much " : "");
		know_items[TYP_POTION][P_RAISELEVEL] = TRUE;
		raise_level();
		if (blessed)
		    raise_level();
	    }
	when    P_HASTE:
	    if (cursed) {   /* Slow player down */
		if (on(player, ISHASTE)) {
		    extinguish(nohaste);
		    nohaste();
		}
		else {
		    msg("You feel yourself moving %sslower.",
			on(player, ISSLOW) ? "even " : "");
		    if (on(player, ISSLOW))
			lengthen(noslow, rnd(4) + 4);
		    else if (!is_wearing(R_FREEDOM)) {
			turn_on(player, ISSLOW);
			player.t_turn = TRUE;
			fuse(noslow, 0, rnd(4) + 4, AFTER);
		    }
		}
	    }
	    else {
		if (off(player, ISSLOW))
		    msg("You feel yourself moving %sfaster.",
			blessed ? "much " : "");
		add_haste(blessed);
		if (is_potion)
		    know_items[TYP_POTION][P_HASTE] = TRUE;
	    }
	when    P_RESTORE:{
	    int i;

	    msg("You are surrounded by an orange mist.");
	    if (is_potion)
		know_items[TYP_POTION][P_RESTORE] = TRUE;

	    if (lost_str) {
		for (i = 0; i < lost_str; i++)
		    extinguish(res_strength);
		lost_str = 0;
	    }
	    res_strength();

	    if (lost_dext) {
		for (i = 0; i < lost_dext; i++)
		    extinguish(un_itch);
		lost_dext = 0;
	    }
	    res_dexterity();

	    res_wisdom();
	    res_intelligence();
	    curp->s_const = maxp->s_const;
	}
	when    P_INVIS:
	    if (cursed) {
		msg("You feel very noticable.");
		quaff(&player, P_SHIELD, ISCURSED);
	    }
	    else if (off(player, ISINVIS)) {
		turn_on(player, ISINVIS);
		if (on(player, ISDISGUISE)) {
		    turn_off(player, ISDISGUISE);
		    extinguish(undisguise);
		    msg("Your skin feels itchy for a moment.");
		}
		msg("You have a tingling feeling all over your body.");
		fuse(appear, 0, blessed ? WANDERTIME * 3 :
		    WANDERTIME, AFTER);
		PLAYER = IPLAYER;
		light(&hero);
		if (is_potion)
		    know_items[TYP_POTION][P_INVIS] = TRUE;
	    }
	    else
		lengthen(appear, blessed ? WANDERTIME * 3 :
		    WANDERTIME);
	when    P_SMELL:
	    if (cursed) {
		if (on(player, CANSCENT)) {
		    turn_off(player, CANSCENT);
		    extinguish(unscent);
		    msg("You no longer smell monsters around you.");
		}
		else if (on(player, ISUNSMELL)) {
		    lengthen(scent, PHASEDURATION);
		    msg("You feel your nose tingle.");
		}
		else {
		    turn_on(player, ISUNSMELL);
		    fuse(scent, 0, PHASEDURATION, AFTER);
		    msg("You can't smell anything now.");
		}
	    }
	    else {
		short   duration = (blessed ? 3 : 1);

		if (is_potion)
		    know_items[TYP_POTION][P_SMELL] = TRUE;
		if (on(player, CANSCENT))
		    lengthen(unscent, duration *
			PHASEDURATION);
		else {
		    fuse(unscent, 0, duration *
			PHASEDURATION, AFTER);
		    turn_on(player, CANSCENT);
		}
		msg("You begin to smell monsters all around you.");
	    }
	when    P_HEAR:
	    if (cursed) {
		if (on(player, CANHEAR)) {
		    turn_off(player, CANHEAR);
		    extinguish(hear);
		    msg("You no longer hear monsters around you.");
		}
		else if (on(player, ISDEAF)) {
		    lengthen(hear, PHASEDURATION);
		    msg("You feel your ears burn.");
		}
		else {
		    fuse(hear, 0, PHASEDURATION, AFTER);
		    turn_on(player, ISDEAF);
		    msg("You are surrounded by a sudden silence.");
		}
	    }
	    else {
		short   duration = (blessed ? 3 : 1);

		if (is_potion)
		    know_items[TYP_POTION][P_HEAR] = TRUE;
		if (on(player, CANHEAR))
		    lengthen(unhear, duration *
			PHASEDURATION);
		else {
		    fuse(unhear, 0, duration *
			PHASEDURATION, AFTER);
		    turn_on(player, CANHEAR);
		}
		msg("You begin to hear monsters all around you.");
	    }
	when    P_SHERO:
	    if (cursed) {
		if (on(player, SUPERHERO)) {
		    msg("You feel ordinary again.");
		    turn_off(player, SUPERHERO);
		    extinguish(unshero);
		    extinguish(unbhero);
		}
		else if (on(player, ISUNHERO)) {
		    msg("Your feeling of vulnerability increases.");
		    lengthen(shero, 5 + rnd(5));
		}
		else {
		    msg("You feel suddenly vulnerable.");
		    if (curp->s_hpt == 1) {
			death(D_POTION);
			return;
		    }
		    curp->s_hpt /= 2;
		    chg_str(-2, FALSE, TRUE);
		    chg_dext(-2, FALSE, TRUE);
		    no_command = 3 + rnd(HEROTIME);
		    turn_on(player, ISUNHERO);
		    fuse(shero, 0, HEROTIME +
			rnd(HEROTIME), AFTER);
		}
	    }
	    else {
		if (on(player, ISFLEE)) {
		    turn_off(player, ISFLEE);
		    msg("You regain your composure.");
		}
		if (on(player, ISUNHERO)) {
		    extinguish(shero);
		    shero();
		}
		else if (on(player, SUPERHERO)) {
		    if (find_slot(unbhero))
			lengthen(unbhero, HEROTIME + 2
			    * rnd(HEROTIME));
		    else if (find_slot(unshero) && !blessed)
			lengthen(unshero, HEROTIME + 2
			    * rnd(HEROTIME));
		    else {
			extinguish(unshero);
			unshero();
			fuse(unbhero, 0, 2 * (HEROTIME +
			    rnd(HEROTIME)), AFTER);
		    }
		    msg("Your feeling of invulnerablity grows stronger.");
		}
		else {
		    turn_on(player, SUPERHERO);
		    chg_str(10, FALSE, FALSE);
		    chg_dext(5, FALSE, FALSE);
		    quaff(quaffer, P_HASTE, ISBLESSED);
		    quaff(quaffer, P_CLEAR, ISNORMAL);
		    if (blessed) {
			fuse(unbhero, 0, HEROTIME +
			    rnd(HEROTIME), AFTER);
			msg("You suddenly feel invincible.");
		    }
		    else {
			fuse(unshero, 0, HEROTIME +
			    rnd(HEROTIME), AFTER);
			msg("You suddenly feel invulnerable.");
		    }
		    if (is_potion)
			know_items[TYP_POTION][P_SHERO] = TRUE;
		}
	    }
	when    P_DISGUISE:
	    if (off(player, ISDISGUISE) && off(player, ISINVIS)) {
		turn_on(player, ISDISGUISE);
		msg("Your body shimmers a moment and then changes.");
		fuse(undisguise, 0, blessed ? GONETIME * 3 :
		    GONETIME, AFTER);
		if (rnd(2))
		    PLAYER = 'a' + rnd(26);
		else
		    PLAYER = 'A' + rnd(26);
		light(&hero);
		if (is_potion)
		    know_items[TYP_POTION][P_DISGUISE] =
			TRUE;
	    }
	    else if (off(player, ISINVIS))
		lengthen(undisguise, blessed ? GONETIME * 3 :
		    GONETIME);
	    else
		msg("You have an itchy feeling under your skin.");
	when    P_FIRERESIST:
	    if (cursed) {
		if (!is_wearing(R_FIRERESIST)) {
		    msg("Your teeth start clattering.");
		    if (on(player, ISHASTE)) {
			extinguish(nohaste);
			nohaste();
		    }
		    else {
			msg("You feel yourself moving %sslower.",
			    on(player, ISSLOW)
			    ? "even "
			    : "");
			if (on(player, ISSLOW))
			    lengthen(noslow,
				rnd(4) + 4);
			else if (!is_wearing(R_FREEDOM)) {
			    turn_on(player, ISSLOW);
			    player.t_turn = TRUE;
			    fuse(noslow, 0, rnd(4) + 4, AFTER);
			}
		    }
		}
		else
		    msg("You feel a brief chill.");
	    }
	    else {
		if (is_potion)
		    know_items[TYP_POTION][P_FIRERESIST] =
			TRUE;
		if (blessed) {
		    extinguish(unhot);
		    msg("You feel a strong continuous warm glow.");
		}
		else {
		    if (off(player, NOFIRE)) {
			fuse(unhot, 0, PHASEDURATION,
			    AFTER);
			msg("You feel a warm glow.");
		    }
		    else {
			if (find_slot(unhot) == NULL)
			    msg("Your warm glow continues.");
			else {
			    lengthen(unhot,
				PHASEDURATION);
			    msg("Your feel a hot flush.");
			}
		    }
		}
		turn_on(player, NOFIRE);
		if (on(player, NOCOLD)) {
		    turn_off(player, NOCOLD);
		    extinguish(uncold);
		}
	    }
	when    P_COLDRESIST:
	    if (cursed) {
		if (!is_wearing(R_COLDRESIST)) {
		    msg("Your feel feverishly hot.");
		    if (on(player, ISHASTE)) {
			extinguish(nohaste);
			nohaste();
		    }
		    else {
			msg("You feel yourself moving %sslower.",
			    on(player, ISSLOW)
			    ? "even "
			    : "");
			if (on(player, ISSLOW))
			    lengthen(noslow,
				rnd(4) + 4);
			else if (!is_wearing(R_FREEDOM)) {
			    turn_on(player, ISSLOW);
			    player.t_turn = TRUE;
			    fuse(noslow, 0, rnd(4) + 4, AFTER);
			}
		    }
		}
		else
		    msg("You feel a brief touch of heat rash.");
	    }
	    else {
		if (is_potion)
		    know_items[TYP_POTION][P_COLDRESIST] =
			TRUE;
		if (blessed) {
		    extinguish(uncold);
		    msg("You feel a strong continuous cool breeze.");
		}
		else {
		    if (off(player, NOCOLD)) {
			fuse(uncold, 0, PHASEDURATION,
			    AFTER);
			msg("You feel a cool breeze.");
		    }
		    else {
			if (find_slot(uncold) == NULL)
			    msg("Your cool feeling continues.");
			else {
			    lengthen(uncold,
				PHASEDURATION);
			    msg("The cool breeze blows more strongly.");
			}
		    }
		}
		turn_on(player, NOCOLD);
		if (on(player, NOFIRE)) {
		    extinguish(unhot);
		    turn_off(player, NOFIRE);
		}
	    }
	when    P_HASOXYGEN:
	    if (cursed) {
		if (!is_wearing(R_BREATHE)) {
		    msg("You can't breathe.");
		    no_command = HOLDTIME;
		}
		else {
		    msg("You feel a momentary shortness of breath.");
		}
	    }
	    else {
		short   duration = (blessed ? 3 : 1);

		if (is_potion)
		    know_items[TYP_POTION][P_HASOXYGEN] =
			TRUE;
		if (on(player, HASOXYGEN))
		    lengthen(unbreathe, duration *
			PHASEDURATION);
		else {
		    fuse(unbreathe, 0, duration *
			PHASEDURATION, AFTER);
		    turn_on(player, HASOXYGEN);
		}
		if (!is_wearing(R_BREATHE))
		    msg("The air seems %sless polluted.",
			blessed ? "much " : "");
	    }
	when    P_LEVITATION:
	    if (cursed) {
		msg("You can't move.");
		no_command = HOLDTIME;
	    }
	    else {
		short   duration = (blessed ? 3 : 1);

		if (is_potion)
		    know_items[TYP_POTION][P_LEVITATION] =
			TRUE;
		if (on(player, CANFLY))
		    lengthen(unfly, duration * WANDERTIME);
		else {
		    fuse(unfly, 0, duration * WANDERTIME, AFTER);
		    turn_on(player, CANFLY);
		}
		if (!is_wearing(R_LEVITATION))
		msg("You %sbegin to float in the air!",
		    blessed ? "quickly " : "");
	    }
	when    P_REGENERATE:
	    if (cursed) {
		quaff(quaffer, P_HEALING, ISCURSED);
		quaff(quaffer, P_HASTE, ISCURSED);
	    }
	    else {
		short   duration = (blessed ? 3 : 1) *
		    HUHDURATION;

		if (is_potion)
		    know_items[TYP_POTION][P_REGENERATE] = TRUE;
		if (on(player, SUPEREAT))
		    lengthen(unsupereat, duration);
		else {
		    fuse(unsupereat, 0, duration, AFTER);
		    turn_on(player, SUPEREAT);
		}
		if (on(player, ISREGEN))
		    lengthen(unregen, duration);
		else {
		    fuse(unregen, 0, duration, AFTER);
		    turn_on(player, ISREGEN);
		}
		if (!is_wearing(R_REGEN))
		    msg("You feel %shealthier!",
			blessed ? "much " : "");
	    }
	when    P_SHIELD:{
	    int adjustment = 0;

	    if (on(player, HASSHIELD)) {    /* cancel old spell */
		extinguish(unshield);
		unshield();
	    }

	    if (cursed)
		adjustment = 2;

	    else if (blessed) {
		msg("Your skin feels very rigid.");
		switch (player.t_ctype) {
		    when C_FIGHTER:
		    case C_PALADIN:
		    case C_RANGER:
			adjustment = -3;
		    otherwise:
			adjustment = -5;
		}
	    }
	    else {
		msg("Your skin hardens.");
		adjustment = -2;
	    }

	    pstats.s_arm += adjustment;
	    shield_ac += adjustment;
	    turn_on(player, HASSHIELD);
	    fuse(unshield, 0, (blessed ? 3 : 1) * SEEDURATION, AFTER);
	    if (is_potion)
		know_items[TYP_POTION][P_SHIELD] = TRUE;
	}
	when    P_TRUESEE:
	    if (cursed) {
		turn_on(player, PERMBLIND);
		if (on(player, ISBLIND)) {
		    msg("The gloom around you thickens.");
		    lengthen(sight, SEEDURATION);
		}
		else {
		    msg("A mantle of darkness falls around you.");
		    turn_on(player, ISBLIND);
		    fuse(sight, 0, SEEDURATION, AFTER);
		    look(FALSE);
		}
		look(FALSE);
	    }
	    else if (on(player, PERMBLIND)) {
		if (blessed || is_potion) {
		    turn_off(player, PERMBLIND);
		    sight();
		    goto let_there_be_light;
		}
		else
		    nothing_message(ISBLESSED);
	    }
	    else
    let_there_be_light:
	    if (off(player, CANSEE)) {
		turn_on(player, CANSEE);
		msg("You feel especially perceptive.");
		fuse(untruesee, 0, blessed ? SEEDURATION * 3
		    : SEEDURATION, AFTER);
		light(&hero);
	    }
	    else if (find_slot(unsee) != NULL) {
		nothing_message(ISNORMAL);
		lengthen(untruesee, blessed ? SEEDURATION * 3
		    : SEEDURATION);
	    }
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }

    status(FALSE);
    if (is_potion) {
	if (!cursed && know_items[TYP_POTION][which] &&
	    guess_items[TYP_POTION][which]) {
	    free(guess_items[TYP_POTION][which]);
	    guess_items[TYP_POTION][which] = NULL;
	}
	else if (askme && !know_items[TYP_POTION][which] &&
	    guess_items[TYP_POTION][which] == NULL) {
	    msg(terse ? "Call it: " :
		"What do you want to call it? ");
	    if (get_str(buf, cw) == NORM) {
		guess_items[TYP_POTION][which] =
		    new((unsigned int) strlen(buf) + 1);
		strcpy(guess_items[TYP_POTION][which], buf);
	    }
	}
	food_left += (blessed ? rnd(100) : (cursed ? -rnd(100) :
	    rnd(50)));
    }
}

/* Lower a level of experience */

lower_level(who)
short   who;
{
    int fewer, nsides, i;

    if (--pstats.s_lvl == 0) {
	death(who); /* All levels gone */
	return;
    }
    msg("You suddenly feel less skillful.");
    pstats.s_exp = 1L;
    init_exp();
    for (i = 2; i <= pstats.s_lvl; i++) {
	if (max_stats.s_exp < 0x3fffffffL)  /* 2^30 - 1 */
	    max_stats.s_exp *= 2L;  /* twice as many for next */
    }
    switch (player.t_ctype) {
	when    C_FIGHTER: nsides = 12;
	when    C_PALADIN: nsides = 12;
	when    C_RANGER: nsides = 12;
	when    C_MAGICIAN: nsides = 4;
	when    C_ILLUSION: nsides = 4;
	when    C_CLERIC: nsides = 8;
	when    C_DRUID: nsides = 8;
	when    C_THIEF: nsides = 6;
	when    C_ASSASIN: nsides = 6;
	when    C_NINJA: nsides = 6;
    }
    fewer = max(1, roll(1, 16 - nsides) + int_wis_bonus());
    pstats.s_power -= fewer;
    max_stats.s_power -= fewer;

    fewer = max(1, roll(1, nsides) + const_bonus());
    pstats.s_hpt -= fewer;
    max_stats.s_hpt -= fewer;
    if (pstats.s_hpt < 1)
	pstats.s_hpt = 1;
    if (max_stats.s_hpt < 1) {
	death(who);
	return;
    }
}

res_dexterity()
{
    if (lost_dext) {
	chg_dext(lost_dext, FALSE, FALSE);
	lost_dext = 0;
    }
    else {
	pstats.s_dext = max_stats.s_dext + ring_value(R_ADDHIT) +
	    (on(player, POWERDEXT) ? 10 : 0) +
	    (on(player, SUPERHERO) ? 5 : 0);
    }

}


/*
 * res_wisdom: Restore player's wisdom
 */

res_wisdom()
{
    int ring_str;

    /* Discount the ring value */
    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    if (pstats.s_wisdom < max_stats.s_wisdom)
	pstats.s_wisdom = max_stats.s_wisdom;

    /* Redo the rings */
    pstats.s_wisdom += ring_str;
}

/*
 * res_intelligence: Restore player's intelligence
 */

res_intelligence()
{
    int ring_str;

    /* Discount the ring value */
    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    if (pstats.s_intel < max_stats.s_intel)
	pstats.s_intel = max_stats.s_intel;

    /* Redo the rings */
    pstats.s_intel += ring_str;
}


/*
 * Increase player's strength
 */

add_strength(cursed)
bool    cursed;
{

    if (cursed) {
	msg("You feel slightly weaker now.");
	chg_str(-1, FALSE, FALSE);
    }
    else {
	msg("You feel stronger now.  What bulging muscles!");
	if (lost_str != 0) {
	    lost_str--;
	    chg_str(1, FALSE, FALSE);
	}
	else
	    chg_str(1, TRUE, FALSE);
    }
}


/*
 * Increase player's intelligence
 */
add_intelligence(cursed)
bool    cursed;
{
    int ring_str;   /* Value of ring strengths */

    /* Undo any ring changes */
    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    /* Now do the potion */
    if (cursed) {
	msg("You feel slightly less intelligent now.");
	pstats.s_intel = max(pstats.s_intel - 1, 3);
    }
    else {
	msg("You feel more intelligent now.  What a mind!");
	pstats.s_intel = min(pstats.s_intel + 1, 25);
    }

    /* Adjust the maximum */
    if (max_stats.s_intel < pstats.s_intel)
	max_stats.s_intel = pstats.s_intel;

    /* Now put back the ring changes */
    pstats.s_intel += ring_str;
}


/*
 * Increase player's wisdom
 */

add_wisdom(cursed)
bool    cursed;
{
    int ring_str;   /* Value of ring strengths */

    /* Undo any ring changes */
    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    /* Now do the potion */
    if (cursed) {
	msg("You feel slightly less wise now.");
	pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
    }
    else {
	msg("You feel wiser now.  What a sage!");
	pstats.s_wisdom = min(pstats.s_wisdom + 1, 25);
    }

    /* Adjust the maximum */
    if (max_stats.s_wisdom < pstats.s_wisdom)
	max_stats.s_wisdom = pstats.s_wisdom;

    /* Now put back the ring changes */
    pstats.s_wisdom += ring_str;
}


/*
 * Increase player's dexterity
 */

add_dexterity(cursed)
bool    cursed;
{
    /* Now do the potion */
    if (cursed) {
	msg("You feel less dextrous now.");
	chg_dext(-1, FALSE, TRUE);
    }
    else {
	msg("You feel more dextrous now.  Watch those hands!");
	if (lost_dext != 0) {
	    lost_dext--;
	    chg_dext(1, FALSE, FALSE);
	}
	else
	    chg_dext(1, TRUE, FALSE);
    }
}


/*
 * Increase player's constitution
 */

add_const(cursed)
bool    cursed;
{
    /* Do the potion */
    if (cursed) {
	msg("You feel slightly less healthy now.");
	pstats.s_const = max(pstats.s_const - 1, 3) +
	    (on(player, POWERCONST) ? 10 : 0);
    }
    else {
	msg("You feel healthier now.");
	pstats.s_const = min(pstats.s_const + 1, 25) +
	    (on(player, POWERCONST) ? 10 : 0);
    }

    /* Adjust the maximum */
    if (max_stats.s_const < pstats.s_const - (on(player, POWERCONST) ? 10 : 0))
	max_stats.s_const = pstats.s_const;
}

/*
 * monquaff - monster gets the effect
 */
monquaff(quaffer, which, flags)
struct thing    *quaffer;
int which;
int flags;
{
    struct stats    *curp = &(quaffer->t_stats);
    struct stats    *maxp = &(quaffer->maxstats);
    bool    blessed = flags & ISBLESSED;
    bool    cursed = flags & ISCURSED;

    switch (which) {
	when    P_SEEINVIS:
	    if (cursed)
		turn_on(*quaffer, ISHUH);
	    else
		turn_on(*quaffer, CANSEE);
	when    P_GAINABIL:
	    if (cursed)
		curp->s_intel /= 2;
	    else
		curp->s_power = maxp->s_power;
	when    P_CLEAR:
	    if (cursed)
		turn_on(*quaffer, ISHUH);
	    else
		turn_on(*quaffer, ISHUH);
	when    P_HEALING:
	    if (cursed) {
		curp->s_hpt /= 2;
		curp->s_power /= 2;
	    }
	    else {
		int nsides = (blessed ? 8 : 4);
		int hpt_gain = roll(curp->s_lvl, nsides);
		int power_gain = roll(curp->s_lvl, nsides);

		curp->s_hpt = min(curp->s_hpt + hpt_gain,
		    maxp->s_hpt);
		curp->s_power = min(curp->s_power + power_gain,
		     maxp->s_power);
	    }
	when    P_HASTE:
	    if (cursed) {
		if (on(*quaffer, ISHASTE))
		    turn_off(*quaffer, ISHASTE);
		else
		    turn_on(*quaffer, ISSLOW);
	    }
	    else
		turn_on(*quaffer, ISHASTE);
	when    P_INVIS:
	    turn_on(*quaffer, ISINVIS);
	    if (cansee(quaffer->t_pos.y, quaffer->t_pos.x))
		seemsg("The monster dissappears!");
	when    P_REGENERATE:
	    if (cursed) {
		quaff(quaffer, P_HEALING, ISCURSED);
		quaff(quaffer, P_HASTE, ISCURSED);
	    }
	    else
		turn_on(*quaffer, ISREGEN);
	when    P_SHERO:
	    if (on(*quaffer, ISFLEE))
		turn_off(*quaffer, ISFLEE);
	    else {
		turn_on(*quaffer, SUPERHERO);
		quaff(quaffer, P_HASTE, ISBLESSED);
		quaff(quaffer, P_CLEAR, ISNORMAL);
	    }
	when    P_PHASE:
	    if (cursed)
		quaffer->t_no_move += HOLDTIME;
	    else
		turn_on(*quaffer, CANINWALL);
	otherwise:
	    debug("'%s' is a strange potion for a monster to quaff!",
		p_magic[which].mi_name);
    }
}
