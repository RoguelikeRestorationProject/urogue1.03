/*
    sticks.c  -  Functions to implement the various sticks 
   
    Last Modified: Dec 30, 1990

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

/*
 * Functions to implement the various sticks one might find while wandering
 * around the dungeon.
 */

#include <ctype.h>
#include "rogue.h"
#include "death.h"

/*
 * for WS_HIT, WS_WEB, etc
 */
static struct object    obolt =
{
    {0,0},NULL,NULL,"",0,"1d4",0,0,'*',0,0,0,100,1,0,0,0,0,0
};
struct object   null_stick = {0};
char    *bolt_name;

/*
 * Mask for cancelling special abilities The flags listed here will be the
 * ones left on after the cancellation takes place
 */
#define CANC0MASK ( ISBLIND     | ISINWALL  | ISRUN     | \
	    ISFLEE      | ISMEAN    | ISGREED   | \
	    CANSHOOT    | ISHELD    | ISHUH     | \
	    ISSLOW      | ISHASTE   | ISCLEAR   | \
	    ISUNIQUE)
#define CANC1MASK ( HASDISEASE  | DIDSUFFOCATE  | CARRYGOLD     | \
	    HASITCH     | CANSELL   | CANBBURN  | \
	    CANSPEAK    | CANFLY    | ISFRIENDLY)
#define CANC2MASK ( HASINFEST   | NOMOVE    | ISSCAVENGE    | \
	    DOROT       | HASSTINK  | DIDHOLD)
#define CANC3MASK ( ISUNDEAD    | CANBREATHE    | CANCAST   | \
	    HASOXYGEN)
#define CANC4MASK ( CANTRAMPLE  | CANSWIM   | CANWIELD  | \
	    ISFAST      | CANBARGAIN    | CANSPORE  | \
	    ISLARGE     | ISSMALL   | ISFLOCK   | \
	    ISSWARM     | CANSTICK  | CANTANGLE | \
	    SHOOTNEEDLE | CANZAP    | HASARMOR  | \
	    CANTELEPORT | ISBERSERK | ISFAMILIAR    | \
	    HASFAMILIAR | SUMMONING)
#define CANC5MASK ( CANREFLECT  | MAGICATTRACT  | HASSHIELD | \
	    HASMSHIELD)
#define CANC6MASK ( 0 )
#define CANC7MASK ( 0 )
#define CANC8MASK ( 0 )
#define CANC9MASK ( 0 )
#define CANCAMASK ( 0 )
#define CANCBMASK ( 0 )
#define CANCCMASK ( 0 )
#define CANCDMASK ( 0 )
#define CANCEMASK ( 0 )
#define CANCFMASK ( 0 )

fix_stick(cur)
struct object   *cur;
{
    if (strcmp(ws_type[cur->o_which], "staff") == 0) {
	cur->o_weight = 100;
	cur->o_charges = 5 + rnd(10);
	cur->o_damage = "2d3";
	switch (cur->o_which) {
	    when    WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		cur->o_damage = "2d8";
	    when    WS_LIGHT:
		cur->o_charges = 20 + rnd(10);
	}
    }
    else {          /* A wand */
	cur->o_damage = "1d3";
	cur->o_weight = 60;
	cur->o_charges = 3 + rnd(5);
	switch (cur->o_which) {
	    when    WS_HIT:
		cur->o_hplus = 3;
		cur->o_dplus = 3;
		cur->o_damage = "1d8";
	    when    WS_LIGHT:
		cur->o_charges = 10 + rnd(10);
	}
    }
    cur->o_hurldmg = "1d1";

}

/*
 * do_zap - zap a stick (or effect a stick-like spell)
 *
 * zapper:  who does it
 * got_dir: need to ask for direction?
 * which:   which WS_STICK (-1 means ask from pack)
 * flags:   ISBLESSED, ISCURSED
 */
do_zap(zapper, which, flags)
struct thing    *zapper;
int which;
int flags;
{
    struct linked_list  *item, *nitem;
    struct object   *obj, *nobj;
    struct room *rp;
    struct thing    *tp;
    char    *mname;
    int y, x;
    bool    blessed = flags & ISBLESSED;
    bool    cursed = flags & ISCURSED;
    bool    is_stick = (which < 0 ? TRUE : FALSE);
    bool    got_one = TRUE;
    int bff_zappable();
    object  *get_object();

    if (zapper != &player) {
	monster_do_zap(zapper, which, flags);
	return;
    }
    if (is_stick) {
	if ((obj = get_object(pack, "zap with", STICK, bff_zappable))
	    == NULL)
	    return;
	if (obj->o_type != STICK && !(obj->o_flags & ISZAPPED)) {
	    msg("You can't zap with that!");
	    return;
	}
	if (obj->o_type != STICK)   /* an electrified weapon */
	    which = WS_ELECT;
	else
	    which = obj->o_which;
	if (obj->o_charges < 1) {
	    nothing_message(flags);
	    return;
	}
	obj->o_charges--;
	if (delta.y == 0 && delta.x == 0)
	    do {
		delta.y = rnd(3) - 1;
		delta.x = rnd(3) - 1;
	    } while (delta.y == 0 && delta.x == 0);

	flags = obj->o_flags;
	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;
    }
    else
	obj = &null_stick;

    /* Find out who the target is */
    y = hero.y;
    x = hero.x;
    while (shoot_ok(winat(y, x))) {
	y += delta.y;
	x += delta.x;
    }
    if (x >= 0 && x < COLS && y >= 1 && y < LINES - 2 &&
	    isalpha(mvwinch(mw, y, x))) {
	item = find_mons(y, x);
	tp = THINGPTR(item);
	mname = on(player, ISBLIND) ? "monster" :
	    monsters[tp->t_index].m_name;
	if (on(*tp, CANSELL)) {
	    luck++;
	    aggravate();
	}
    }
    else
	got_one = FALSE;

    switch (which) {
	when    WS_LIGHT:
	    /* Reddy Kilowat wand.  Light up the room */
	    if (blue_light(flags) && is_stick)
		if (is_stick)
		    know_items[TYP_STICK][WS_LIGHT] = TRUE;
	when    WS_DRAIN:

	    /*
	     * Take away 1/2 of hero's hit points, then take it away
	     * evenly from the monsters in the room or next to hero if he
	     * is in a passage. Leave the monsters alone if the stick is
	     * cursed. Drain 1/3rd of hero's hit points if blessed.
	     */
	    if (pstats.s_hpt < 2) {
		death(D_DRAINLIFE);
		return;
	    }
	    if (cursed)
		pstats.s_hpt /= 2;

	    else if ((rp = roomin(&hero)) == NULL)
		drain(hero.y - 1, hero.y + 1, hero.x - 1,
		hero.x + 1);
	    else
		drain(rp->r_pos.y, rp->r_pos.y + rp->r_max.y,
		      rp->r_pos.x, rp->r_pos.x + rp->r_max.x);
	    if (blessed)
		pstats.s_hpt *= 2.0 / 3.0;
	when    WS_POLYMORPH:
	case WS_MONSTELEP:
	case WS_CANCEL:{
	    char    oldch;
	    int rm;
	    int save_adj = 0;

	    if (got_one) {

		/*
		 * if the monster gets the saving throw,
		 * leave the case
		 */
		if (blessed)
		    save_adj = -5;
		if (cursed)
		    if (which == WS_POLYMORPH)
			save_adj = -5;  /* not save vs becoming
				 * tougher */
		    else
			save_adj = 5;

		if (save_throw(VS_MAGIC - save_adj, tp)) {
		    nothing_message(flags);
		    break;
		}
		else if (is_stick)
		    know_items[TYP_STICK][which] = TRUE;

		/* Unhold player */
		if (on(*tp, DIDHOLD)) {
		    turn_off(*tp, DIDHOLD);
		    if (--hold_count == 0)
			turn_off(player, ISHELD);
		}
		/* unsuffocate player */
		if (on(*tp, DIDSUFFOCATE)) {
		    turn_off(*tp, DIDSUFFOCATE);
		    extinguish(suffocate);
		}

		if (which == WS_POLYMORPH) {
		    int which_new;
		    int charmed;

		    detach(mlist, item);
		    charmed = on(*tp, ISCHARMED);
		    oldch = tp->t_oldch;
		    delta.y = y;
		    delta.x = x;
		    if (!blessed && !cursed)
			which_new = randmonster(WANDER, GRAB);
		    else {

			/*
			 * duplicate randmonster()
			 * for now
			 */

			/*
			 * Eventually fix to take
			 * level
			 */
			int cur_level, range, i;

			if (blessed)
			    cur_level = level / 2;
			if (cursed)
			    cur_level = level * 2;
			range = 4 * NLEVMONS;
			i = 0;
			do {
			    if (i++ > range * 10) { /* just in case all have
					 * be genocided */
				i = 0;
				if (--cur_level <= 0)
				    fatal("Rogue could not find a monster to make");
			    }
			    which_new = NLEVMONS * (cur_level - 1) + (rnd(range) - (range - 1 - NLEVMONS));
			    if (which_new < 1)
				which_new = rnd(NLEVMONS) + 1;
			    if (which_new > nummonst - NUMSUMMON - 1) {
				if (blessed)
				    which_new = rnd(range) + (nummonst - NUMSUMMON - 1) - (range - 1);
				else if (which_new > nummonst - 1)
				    which_new = rnd(range + NUMSUMMON) + (nummonst - 1) - (range + NUMSUMMON - 1);
			    }
			}
			while (!monsters[which_new].m_normal);
		    }
		    new_monster(item, which_new, &delta, NOMAXSTATS);
		    mname = on(player, ISBLIND) ? "monster" : monsters[tp->t_index].m_name;
		    if (!cursed && charmed)
			turn_on(*tp, ISCHARMED);
		    if (off(*tp, ISRUN))
			runto(&delta, &hero);
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_type);
		    tp->t_oldch = oldch;
		    seemsg(terse ? "A new %s!" : "You have created a new %s!", mname);
		}
		else if (which == WS_CANCEL) {
		    tp->t_flags[0] &= CANC0MASK;
		    tp->t_flags[1] &= CANC1MASK;
		    tp->t_flags[2] &= CANC2MASK;
		    tp->t_flags[3] &= CANC3MASK;
		    tp->t_flags[4] &= CANC4MASK;
		    tp->t_flags[5] &= CANC5MASK;
		    tp->t_flags[6] &= CANC5MASK;
		    tp->t_flags[7] &= CANC7MASK;
		    tp->t_flags[8] &= CANC8MASK;
		    tp->t_flags[9] &= CANC9MASK;
		    tp->t_flags[10] &= CANCAMASK;
		    tp->t_flags[11] &= CANCBMASK;
		    tp->t_flags[12] &= CANCCMASK;
		    tp->t_flags[13] &= CANCDMASK;
		    tp->t_flags[14] &= CANCEMASK;
		    tp->t_flags[15] &= CANCFMASK;
		}
		else {  /* A teleport stick */
		    if (cursed) {   /* Teleport monster to
			     * player */
			if ((y == (hero.y + delta.y)) &&
			  (x == (hero.x + delta.x)))
			    nothing_message(flags);
			else {
			    tp->t_pos.y = hero.y + delta.y;
			    tp->t_pos.x = hero.x + delta.x;
			}
		    }
		    else if (blessed) { /* Get rid of monster */
			killed(NULL, item, NOMESSAGE, NOPOINTS);
			return;
		    }
		    else {
			int i = 0;

			do {    /* Move monster to
			     * another room */
			    rm = rnd_room();
			    rnd_pos(&rooms[rm], &tp->t_pos);
			} while (winat(tp->t_pos.y, tp->t_pos.x) !=
			     FLOOR && i++ < 500);
		    }

		    /* Now move the monster */
		    if (isalpha(mvwinch(cw, y, x)))
			mvwaddch(cw, y, x, tp->t_oldch);
		    tp->t_dest = hero;
		    turn_on(*tp, ISRUN);
		    turn_off(*tp, ISDISGUISE);
		    mvwaddch(mw, y, x, ' ');
		    mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, tp->t_oldch);
		    if (tp->t_pos.y != y || tp->t_pos.x != x)
			tp->t_oldch = mvwinch(cw, tp->t_pos.y, tp->t_pos.x);
		}
	    }
	}
	when    WS_MISSILE:{
	    int damage;
	    int nsides = 4;
	    char    ch;

	    if (is_stick)
		know_items[TYP_STICK][which] = TRUE;

	    /* Magic Missiles *always* hit, no saving throw */
	    obolt.o_type = '*';
	    do_motion(&obolt, delta.y, delta.x, &player);
	    ch = winat(obolt.o_pos.y, obolt.o_pos.x);

	    if (cursed)
		nsides /= 2;
	    else if (blessed)
		nsides *= 2;
	    damage = roll(pstats.s_lvl, nsides);

	    if (isalpha(ch)) {
		debug("Missiled %s for %d (%d)",
		 mname, damage, tp->t_stats.s_hpt - damage);
		if ((tp->t_stats.s_hpt -= damage) <= 0) {
		    seemsg("The missile kills the %s.", mname);
		    killed(&player, item, NOMESSAGE, POINTS);
		}
		else {
		    seemsg("The missile hits the %s", mname);
		    runto(&obolt.o_pos, &hero);
		    summon_help(tp, NOFORCE);
		}
	    }
	    if (obolt.o_pos.y >= 0 && obolt.o_pos.x >= 0 &&
		obolt.o_pos.y < LINES && obolt.o_pos.x < COLS)
		mvwaddch(cw, obolt.o_pos.y, obolt.o_pos.x,
		     show(obolt.o_pos.y, obolt.o_pos.x));
	}
	when    WS_HIT:{
	    char    ch;
	    struct object   strike; /* don't want to change
			 * sticks attributes */

	    if (is_stick)
		know_items[TYP_STICK][which] = TRUE;
	    obolt.o_type = '@';
	    do_motion(&obolt, delta.y, delta.x, &player);
	    ch = winat(obolt.o_pos.y, obolt.o_pos.x);

	    if (isalpha(ch)) {
		static char buf[20];
		int nsides, ndice;

		strike = *obj;
		if (blessed)
		    strike.o_hplus = 12;
		else if (cursed)
		    strike.o_hplus = 3;
		else
		    strike.o_hplus = 6;

		if (!is_stick || strcmp(ws_type[which], "staff") == 0)
		    ndice = 3;
		else
		    ndice = 2;
		if (blessed)
		    nsides = 16;
		else if (cursed)
		    nsides = 4;
		else
		    nsides = 8;
		sprintf(buf, "%dd%d", ndice, nsides);

		strike.o_damage = buf;
		fight(&tp->t_pos, &strike, NOTHROWN);
	    }
	}
	when    WS_SLOW_M:
	    if (got_one) {
		if (cursed) {
		    if (off(*tp, ISSLOW))
			turn_on(*tp, ISHASTE);
		    else
			turn_off(*tp, ISSLOW);
		}
		else if (blessed) {
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    turn_off(*tp, ISRUN);
		    turn_on(*tp, ISHELD);
		    return;
		}
		else {
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    if (off(*tp, ISHASTE))
		    turn_on(*tp, ISSLOW);
		    else
			turn_off(*tp, ISHASTE);
		    tp->t_turn = TRUE;
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	when    WS_CHARGE:
	    if (know_items[TYP_STICK][which] != TRUE && is_stick) {
		msg("This is a wand of charging.");
		know_items[TYP_STICK][which] = TRUE;
	    }
	    if ((nitem = get_item("charge", STICK)) != NULL) {
		nobj = OBJPTR(nitem);
		if ((nobj->o_charges == 0) && (nobj->o_which != WS_CHARGE)) {
		    fix_stick(nobj);
		    if (blessed)
			nobj->o_charges *= 2;
		    else if (cursed)
			nobj->o_charges /= 2;
		}
		else {
		    if (blessed)
			nobj->o_charges += 2;
		    else if (cursed)
			nobj->o_charges -= 1;
		    else
			nobj->o_charges += 1;
		}
	    }
	when    WS_ELECT:
	case WS_FIRE:
	case WS_COLD:{
	    char    *name;
	    int damage;
	    int ndice = 3 + pstats.s_lvl;
	    int nsides;

	    if (is_stick) {
		know_items[TYP_STICK][which] = TRUE;
		if (strcmp(ws_type[which], "staff") == 0)
		    nsides = 8;
		else
		    nsides = 4;
	    }
	    else
		nsides = 6;

	    if (cursed)
		nsides /= 2;
	    else if (blessed)
		nsides *= 2;

	    switch (which) {
		when    WS_ELECT:
		name = "lightning bolt";

		if (rnd(2))
		    ndice += rnd(obj->o_charges / 10);
		else
		    nsides += rnd(obj->o_charges / 10);
		when    WS_FIRE:
		name = "flame";
		when    WS_COLD:
		name = "ice";
	    }

	    damage = roll(ndice, nsides);
	    shoot_bolt(&player, hero, delta, POINTS, D_BOLT, name, damage);
	}
	when    WS_ANTIMATTER:{
	    int m1, m2, x1, y1;
	    char    ch;
	    struct linked_list  *ll;
	    struct thing    *lt;

	    if (is_stick)
		know_items[TYP_STICK][which] = TRUE;
	    y1 = hero.y;
	    x1 = hero.x;
	    do {
		y1 += delta.y;
		x1 += delta.x;
		ch = winat(y1, x1);
	    } while (ch == PASSAGE || ch == FLOOR);

	    for (m1 = x1 - 1; m1 <= x1 + 1; m1++) {
		for (m2 = y1 - 1; m2 <= y1 + 1; m2++) {
		    ch = winat(m2, m1);
		    if (m1 == hero.x && m2 == hero.y)
			continue;
		    if (ch != ' ') {
			ll = find_obj(m2, m1, TRUE);
			while (ll != NULL) {
			    rem_obj(ll, TRUE);
			    ll = find_obj(m2, m1, TRUE);
			}
			ll = find_mons(m2, m1);
			if (ll != NULL) {
			    lt = THINGPTR(ll);
			    if (on(*lt, CANSELL)) {
				luck++;
				aggravate();
			    }
			    if (off(*lt, CANINWALL)) {
				check_residue(lt);
				detach(mlist, ll);
				discard(ll);
				mvwaddch(mw, m2, m1, ' ');
			    }
			}
			mvaddch(m2, m1, ' ');
			mvwaddch(cw, m2, m1, ' ');
		    }
		}
	    }
	    touchwin(cw);
	    touchwin(mw);
	}
	when    WS_CONFMON:
	case WS_PARALYZE:
	    if (got_one) {
		if (save_throw(VS_MAGIC - (blessed ? 5 : (cursed ? -5 : 0)), tp))
		    nothing_message(flags);
		else {
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    switch (which) {
			when    WS_CONFMON:
			    seemsg("The %s looks bewildered.", mname);
			    turn_on(*tp, ISHUH);
			when    WS_PARALYZE:
			    seemsg("The %s looks stunned.", mname);
			    tp->t_no_move = FREEZETIME;
		    }
		}
		delta.y = y;
		delta.x = x;
		runto(&delta, &hero);
	    }
	    else
		nothing_message(flags);
	when    WS_XENOHEALING:{
	    int hpt_gain = roll(zapper->t_stats.s_lvl, (blessed ? 8 : 4));
	    int mons_hpt = tp->t_stats.s_hpt;
	    if (got_one) {
		if (cursed) {
		    if (!save_throw(VS_MAGIC, tp)) {
			if ((mons_hpt -= hpt_gain) <= 0) {
			    seemsg("The %s shrivels up and dies!", mname);
			    killed(&player, item, NOMESSAGE, POINTS);
			}
			else
			    seemsg("Wounds appear all over the %s.", mname);
		    }
		    else
			nothing_message(flags);
		    delta.y = y;
		    delta.x = x;
		    runto(&delta, &hero);
		}
		else {
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    mons_hpt = min(mons_hpt + hpt_gain, tp->maxstats.s_hpt);
		    seemsg("The %s appears less wounded!", mname);
		}
	    }
	    else
		nothing_message(flags);
	}
	when    WS_DISINTEGRATE:
	    if (got_one) {
		if (cursed) {
		    if (on(*tp, ISUNIQUE)) {
			if (on(*tp, CANSUMMON))
			    summon_help(tp, FORCE);
			else
			    msg("The %s is very upset.", mname);
			turn_on(*tp, ISHASTE);
		    }
		    else {
			int m1, m2;
			coord   mp;
			struct linked_list  *titem;
			char    ch;
			struct thing    *th;
			for (m1 = tp->t_pos.x - 1; m1 <= tp->t_pos.x + 1; m1++) {
			    for (m2 = tp->t_pos.y - 1; m2 <= tp->t_pos.y + 1; m2++) {
				ch = winat(m2, m1);
				if (shoot_ok(ch) && ch != PLAYER) {
				    mp.x = m1;  /* create it */
				    mp.y = m2;
				    titem = new_item(sizeof(struct thing));
				    new_monster(titem, (short) tp->t_index, &mp, NOMAXSTATS);
				    th = THINGPTR(titem);
				    turn_on(*th, ISMEAN);
				    runto(&mp, &hero);
				}
			    }
			}
		    }
		    delta.y = y;
		    delta.x = x;
		    turn_on(*tp, ISMEAN);
		    runto(&delta, &hero);
		}
		else {  /* if its a UNIQUE it might still live */
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    if (on(*tp, ISUNIQUE) &&
			save_throw(VS_MAGIC - (blessed ? -5 : 0), tp)) {
			tp->t_stats.s_hpt = tp->t_stats.s_hpt / 2 - 1;
			if (tp->t_stats.s_hpt < 1) {
			    killed(&player, item, NOMESSAGE, POINTS);
			    seemsg("The %s fades away.", mname);
			}
			else {
			    delta.y = y;
			    delta.x = x;
			    runto(&delta, &hero);
			    if (on(*tp, CANSUMMON))
				summon_help(tp, FORCE);
			    else
				seemsg("The %s is very upset.", mname);
			}
		    }
		    else {
			if (on(*tp, CANSELL)) {
			    luck++;
			    aggravate();
			}
			seemsg("The %s turns to dust and blows away.", mname);
			killed(&player, item, NOMESSAGE, POINTS);
		    }
		}
	    }
	when    WS_NOTHING:
	    nothing_message(flags);
	when    WS_INVIS:{
	    if (cursed) {
		int x1, y1, x2, y2;
		bool    zapped = FALSE;
		if ((rp = roomin(&hero)) == NULL) {
		    x1 = max(hero.x - 1, 0);
		    y1 = max(hero.y - 1, 0);
		    x2 = min(hero.x + 1, COLS - 1);
		    y2 = min(hero.y + 1, LINES - 3);
		}
		else {
		    x1 = rp->r_pos.x;
		    y1 = rp->r_pos.y;
		    x2 = rp->r_pos.x + rp->r_max.x;
		    y2 = rp->r_pos.y + rp->r_max.y;
		}
		for (item = mlist; item != NULL; item = next(item)) {
		    tp = THINGPTR(item);
		    if (tp->t_pos.x >= x1 && tp->t_pos.x <= x2 &&
			tp->t_pos.y >= y1 && tp->t_pos.y <= y2) {
			turn_on(*tp, ISINVIS);
			turn_on(*tp, ISRUN);
			turn_off(*tp, ISDISGUISE);
			runto(&tp->t_pos, &hero);
			zapped = TRUE;
		    }
		}
		if (zapped)
		    seemsg("The monsters seem to have all disappeared.");
		else
		    nothing_message(flags);
	    }
	    else {
		if (got_one) {
		    if (is_stick)
			know_items[TYP_STICK][which] = TRUE;
		    if (blessed) {
			turn_off(*tp, ISINVIS);
			turn_off(*tp, ISSHADOW);
			seemsg("The %s appears.", mname);
		    }
		    else {
			turn_on(*tp, ISINVIS);
			seemsg("The %s disappears.", mname);
		    }
		}
		else
		    nothing_message(flags);
	    }
	    light(&hero);
	}
	when    WS_BLAST:{
	    char    ch;
	    struct linked_list  *item, *ip;
	    struct object   *obj;
	    struct trap *trp;

	    if (is_stick)
		know_items[TYP_STICK][which] = TRUE;
	    item = spec_item(WEAPON, GRENADE, 0, 0);
	    obj = OBJPTR(item);
	    obj->o_count = 1;
	    obj->o_flags |= ISKNOW;
	    hearmsg("BOOOM!");
	    aggravate();
	    obj->o_pos.x = hero.x;
	    obj->o_pos.y = hero.y;
	    for (;;) {
		obj->o_pos.y += delta.y;
		obj->o_pos.x += delta.x;
		if (!ce(obj->o_pos, hero) &&
		    cansee(obj->o_pos.y, obj->o_pos.x) &&
		    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
		      show(obj->o_pos.y, obj->o_pos.x));
		}
		if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x))
		    && ch != DOOR && !ce(obj->o_pos, hero)) {
		    if (cansee(obj->o_pos.y, obj->o_pos.x) &&
			ntraps + 1 < 2 * MAXTRAPS &&
			mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
			mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
			wrefresh(cw);
		    }
		    if (isatrap(ch)) {
			trp = trap_at(obj->o_pos.y, obj->o_pos.x);
			if (trp != NULL) {
			    trp->tr_type = TRAPDOOR;
			    trp->tr_flags |= ISFOUND;
			    trp->tr_show = TRAPDOOR;
			}
		    }
		    else if (isalpha(ch))
			hit_monster(obj->o_pos.y, obj->o_pos.x, obj, &player);
		    else if ((ch == FLOOR || ch == PASSAGE)
			 && ntraps + 1 < 2 * MAXTRAPS) {
			mvaddch(obj->o_pos.y, obj->o_pos.x, TRAPDOOR);
			traps[ntraps].tr_type = TRAPDOOR;
			traps[ntraps].tr_flags = ISFOUND;
			traps[ntraps].tr_show = TRAPDOOR;
			traps[ntraps].tr_pos.y = obj->o_pos.y;
			traps[ntraps++].tr_pos.x = obj->o_pos.x;
		    }
		    else if (ch == POTION || ch == SCROLL || ch == FOOD
			 || ch == WEAPON || ch == RING || ch == ARMOR
			 || ch == STICK || ch == ARTIFACT || ch == GOLD) {
			ip = find_obj(obj->o_pos.y, obj->o_pos.x, TRUE);
			while (ip) {    /* BOOM destroys all
				 * stacked objects */
			    rem_obj(ip, TRUE);
			    ip = find_obj(obj->o_pos.y, obj->o_pos.x, TRUE);
			}
			mvaddch(obj->o_pos.y, obj->o_pos.x,
			    roomin(&obj->o_pos) == NULL ? PASSAGE : FLOOR);
		    }
		    continue;
		}
		break;
	    }
	    discard(item);
	}
	when    WS_WEB:{
	    char    ch;

	    if (is_stick)
		know_items[TYP_STICK][which] = TRUE;
	    obolt.o_type = '#';
	    do_motion(&obolt, delta.y, delta.x, &player);
	    ch = winat(obolt.o_pos.y, obolt.o_pos.x);

	    if (isalpha(ch)) {
		if (save_throw(tp))
		    seemsg("The %s evades your web.", mname);
		else {
		    seemsg("The %s is webbed.", mname);
		    turn_off(*tp, ISRUN);
		    turn_on(*tp, ISHELD);
		}
	    }
	    if (obolt.o_pos.y >= 0 && obolt.o_pos.x >= 0 &&
		obolt.o_pos.y < LINES && obolt.o_pos.x < COLS)
		mvwaddch(cw, obolt.o_pos.y, obolt.o_pos.x,
		     show(obolt.o_pos.y, obolt.o_pos.x));
	}
	when    WS_KNOCK:
	case WS_CLOSE:
	    if (blessed) {  /* Do all doors in room */
		char    new_door = (which == WS_KNOCK ? DOOR : SECRETDOOR);
		struct room *rp = roomin(&hero);
		if (rp != NULL)
		    for (x = 0; x < rp->r_nexits; x++) {
			move(rp->r_exit[x].y, rp->r_exit[x].x);
			addch(new_door);
		    }
		else    /* Do all adjacent doors */
		    for (x = hero.x - 1; x <= hero.x + 1; x++)
			for (y = hero.y - 1; y <= hero.y + 1; y++)
			    switch (winat(y, x)) {
				when    DOOR:
				case SECRETDOOR:
				    addch(new_door);
			    }
		light(&hero);
	    }
	    else if (cursed)/* do opposite spell */
		do_zap(zapper, (which == WS_KNOCK ? WS_CLOSE : WS_KNOCK), ISBLESSED);
	    else {
		coord   zap_loc;
		char    loc;

		zap_loc.y = hero.y + delta.y;
		zap_loc.x = hero.x + delta.x;
		loc = winat(zap_loc.y, zap_loc.x);

		switch (loc) {
		    when    SECRETDOOR:
			if (which == WS_KNOCK) {
			    mvaddch(zap_loc.y, zap_loc.x, DOOR);
			    seemsg("A secret door stands revealed before you!");
			    if (is_stick)
				know_items[TYP_STICK][which] = TRUE;
			}
			else
			    goto n_doing;
		    when    DOOR:
			if (which == WS_CLOSE) {
			    mvaddch(zap_loc.y, zap_loc.x, SECRETDOOR);
			    msg("You sucessfully block off the door.");
			    if (is_stick)
				know_items[TYP_STICK][which] = TRUE;
			}
			else
			    goto n_doing;
		    otherwise:
	n_doing:
			nothing_message(flags);
		}
	    }
	otherwise:
	    msg("What a bizarre schtick!");
    }
}

/*
 * drain: Do drain hit points from player shtick
 */

drain(ymin, ymax, xmin, xmax)
int ymin, ymax, xmin, xmax;
{
    int i, j, count;
    struct thing    *ick;
    struct linked_list  *item;

    /*
     * First count how many things we need to spread the hit points among
     */
    for (count = 0, i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)))
		count++;

    if (count == 0) {
	msg("You have a tingling feeling.");
	return;
    }
    else
	msg("You feel weaker.");
    count = pstats.s_hpt / count;
    pstats.s_hpt /= 2;

    /*
     * Now zot all of the monsters
     */
    for (i = ymin; i <= ymax; i++)
	for (j = xmin; j <= xmax; j++)
	    if (isalpha(mvwinch(mw, i, j)) && ((item = find_mons(i, j)) != NULL)) {
		ick = THINGPTR(item);
		if (on(*ick, CANSELL)) {
		    luck++;
		    aggravate();
		}
		if ((ick->t_stats.s_hpt -= count) < 1)
		    killed(&player, item,
			   cansee(i, j) && !on(*ick, ISINVIS), POINTS);
	    }
}

/*
 * charge a wand for wizards.
 */
char    *
charge_str(obj)
struct object   *obj;
{
    static char buf[20];

    if (!(obj->o_flags & ISKNOW))
	buf[0] = '\0';
    else if (terse)
	sprintf(buf, " [%d]", obj->o_charges);
    else if (obj->o_charges == 1)
	sprintf(buf, " [%d charge]", obj->o_charges);
    else
	sprintf(buf, " [%d charges]", obj->o_charges);
    return buf;
}


/*
 * shoot_bolt fires a bolt from the given starting point in the given
 * direction
 */

bool
shoot_bolt(shooter, start, dir, get_points, reason, name, damage)
struct thing    *shooter;
coord   start, dir;
bool    get_points;     /* should player get exp points? */
short   reason;         /* reason for dying */
char    *name;          /* fire, nerve, cold, etc */
int damage;         /* make zapee suffer */
{
    char    dirch, ch;
    bool    change;
    short   y, x;
    coord   pos;
    coord   spotpos[BOLT_LENGTH];
    bool    ret_val = FALSE;/* True if breathing monster gets killed */
    struct linked_list  *item;
    struct thing    *tp;
    char    *mname;
    int bounced;    /* where along BOLT_LENGTH it hit a wall */
    int player_damage;  /* damage if player saved */
    bool    no_effect;  /* zap does not effect zapee */
    int take_that[BOLT_LENGTH + 1]; /* damage to each monster */

    /* last spot for player */

    debug("%s does %d damage", name, damage);

    switch (dir.y + dir.x) {
	when 0: dirch = '/';
	when 1: case -1:
	    dirch = (dir.y == 0 ? '-' : '|');
	when 2: case -2:
	    dirch = '\\';
    }
    pos.y = start.y + dir.y;
    pos.x = start.x + dir.x;
    change = FALSE;
    for (y = 0; y < BOLT_LENGTH + 1; y++)
	take_that[y] = 0;

    bounced = 0;
    for (y = 0; y < BOLT_LENGTH; y++) {
	no_effect = FALSE;
	ch = winat(pos.y, pos.x);
	spotpos[y] = pos;

	/* Bolt damage dimishes over space */
	damage = max(1, damage - (y / 3));

	/* Are we at hero? */
	if (ce(pos, hero))
	    goto at_hero;

	switch (ch) {
	    case SECRETDOOR:
	    case '|':
	    case '-':
	    case ' ':
		bounced = y;
		if (dirch == '-' || dirch == '|') {
		    dir.y = -dir.y;
		    dir.x = -dir.x;
		}
		else
		    switch (ch) {
			case '|':
			case '-':
			case SECRETDOOR:{
			    struct room *rp;

			    rp = roomin(&pos);
			    if (pos.y == rp->r_pos.y ||
				pos.y == rp->r_pos.y + rp->r_max.y - 1) {
				dir.y = -dir.y;
				change ^= TRUE;
			    }
			    if (pos.x == rp->r_pos.x ||
				pos.x == rp->r_pos.x + rp->r_max.x - 1) {
				dir.x = -dir.x;
				change ^= TRUE;
			    }
			}
			otherwise:  {   /* A wall */
			    char    chy = mvinch(pos.y - dir.y, pos.x + dir.x), chx = mvinch(pos.y + dir.y, pos.x - dir.x);

			    if (chy != WALL && chy != SECRETDOOR &&
			     chy != '-' && chy != '|') {
				dir.y = -dir.y;
				change = TRUE;
			    }
			    else if (chx != WALL && chx != SECRETDOOR &&
			     chx != '-' && chx != '|') {
				dir.x = -dir.x;
				change = TRUE;
			    }
			    else {
				dir.y = -dir.y;
				dir.x = -dir.x;
			    }
			}
		    }

		/* Do we change how the bolt looks? */
		if (change) {
		    change = FALSE;
		    if (dirch == '\\')
			dirch = '/';
		    else if (dirch == '/')
			dirch = '\\';
		}
	    otherwise:
		if (isalpha(ch)) {  /* hit a monster */
		    item = find_mons(pos.y, pos.x);
		    if (item == NULL) {
			debug("Can't find monster %c @ %d %d.",
			      ch, pos.y, pos.x);
			continue;
		    }
		    tp = THINGPTR(item);
		    mname = on(player, ISBLIND) ? "monster" : monsters[tp->t_index].m_name;

		    /*
		     * Disguised monsters stay hidden if they
		     * save
		     */
		    if (on(*tp, ISDISGUISE) && save_throw(VS_MAGIC, tp) &&
			(tp->t_type != tp->t_disguise) && off(player, ISBLIND)) {
			msg("Wait! That's a %s!", mname);
			turn_off(*tp, ISDISGUISE);
		    }

		    tp->t_wasshot = TRUE;
		    if (on(*tp, CANSELL)) {
			luck++;
			aggravate();
		    }

		    /* Hit the monster -- does it do damage? */
		    if (strcmp(name, "ice") == 0) {
			if (on(*tp, NOCOLD) || on(*tp, ISUNDEAD))
			    no_effect = TRUE;
		    }
		    else if (strcmp(name, "flame") == 0) {
			if (on(*tp, NOFIRE))
			    no_effect = TRUE;
			if (on(*tp, CANBBURN)) {
			    seemsg("The %s is burned to death by the flame.",
				   mname);
			    take_that[y] += tp->t_stats.s_hpt + 1;
			    ret_val = TRUE;
			}
		    }
		    else if (strcmp(name, "lightning bolt") == 0) {
			if (on(*tp, NOBOLT))
			    no_effect = TRUE;
			if (on(*tp, BOLTDIVIDE)) {
			    no_effect = TRUE;
			    if (creat_mons(tp, tp->t_index, NOMESSAGE))
				seemsg("The %s divides the %s.", name, mname);
			}
		    }
		    if (no_effect == TRUE) {
			msg("The %s has no effect on the %s.", name,
			    on(player, ISBLIND) ? "monster" : mname);
		    }
		    else {
			take_that[(bounced ? bounced-- : y)] +=
			    save_throw(VS_MAGIC, tp) ? damage / 2 : damage;
		    }
		}
		else if (pos.y == hero.y && pos.x == hero.x) {
	at_hero:
		    player_damage = damage;
		    running = fighting = FALSE;
		    bounced = 0;

		    if (cur_armor != NULL &&
			cur_armor->o_which == CRYSTAL_ARMOR &&
			(strcmp(name, "acid") == 0)) {
			player_damage = 0;
			msg("The acid splashes harmlessly against your armor!");
		    }
		    else if (((cur_armor != NULL &&
			   cur_armor->o_which == CRYSTAL_ARMOR) ||
			  (on(player, ISELECTRIC)) ||
			  is_wearing(R_ELECTRESIST)) &&
			(strcmp(name, "lightning bolt") == 0)) {
			player_damage = 0;
			if (rnd(100) < 75
			    && cur_weapon != NULL
			    && shooter != &player) {
			    cur_weapon->o_flags |= ISZAPPED;
			    cur_weapon->o_charges += (10 + rnd(15));
			}
			if (cur_weapon != NULL && cur_armor != NULL)
			    msg("Your armor and %s are covered with dancing blue lights!", weaps[cur_weapon->o_which].w_name);
			else if (cur_armor != NULL)
			    msg("Your armor is covered with dancing blue lights!");
			else if (cur_weapon != NULL)
			    msg("Your %s is covered with dancing blue lights!", weaps[cur_weapon->o_which].w_name);
			else
			    msg("You are momentarily covered with dancing blue lights.");
		    }
		    else if ((is_wearing(R_FIRERESIST) || on(player, NOFIRE)) &&
			 (strcmp(name, "flame") == 0)) {
			player_damage = 0;
			msg("The flame bathes you harmlessly.");
		    }
		    else if ((is_wearing(R_COLDRESIST) || on(player, NOCOLD)) &&
			 (strcmp(name, "ice") == 0)) {
			player_damage = 0;
			msg("The ice cracks and quickly melts around you.");
		    }
		    else if (save(VS_MAGIC)) {
			take_that[BOLT_LENGTH] += player_damage / 2;
			debug("Player dodges %s for %d.", name, player_damage / 2);
		    }
		    else {
			debug("Player zapped by %s for %d.", name, player_damage);
			take_that[BOLT_LENGTH] += player_damage;
		    }

		    /* Check for gas with special effects */
		    if (off(player, HASOXYGEN) && !is_wearing(R_BREATHE) && !save(VS_BREATH)) {
			if (strcmp(name, "nerve gas") == 0) {
			    if (no_command == 0) {
				msg("The nerve gas paralyzes you.");
				no_command = FREEZETIME;
			    }
			}
			else if (strcmp(name, "sleeping gas") == 0) {
			    if (no_command == 0) {
				msg("The sleeping gas puts you to sleep.");
				no_command = SLEEPTIME;
			    }
			}
			else if (strcmp(name, "slow gas") == 0
			     && !is_wearing(R_FREEDOM)) {
			    msg("You feel yourself moving %sslower.",
				on(player, ISSLOW) ? "even " : "");
			    if (on(player, ISSLOW))
				lengthen(noslow, rnd(10) + 4);
			    else {
				turn_on(player, ISSLOW);
				player.t_turn = TRUE;
				fuse(noslow, 0, rnd(10) + 4, AFTER);
			    }
			}
			else if (strcmp(name, "fear gas") == 0) {
			    if (!(on(player, ISFLEE) &&
				  SAME_POS(player.t_dest,shooter->t_pos)) &&
				(shooter != &player)) {
				if (off(player, SUPERHERO)
				    && player.t_ctype != C_PALADIN) {
				    turn_on(player, ISFLEE);
				    player.t_dest = shooter->t_pos;
				    msg("The fear gas terrifies you.");
				}
				else
				    msg("The fear gas has no effect.");
			    }
			}
		    }
		}
		mvwaddch(cw, pos.y, pos.x, dirch);
		wrefresh(cw);
	    }
	    pos.y += dir.y;
	    pos.x += dir.x;
	}

	/*
	 * Now that we have determined the damage for the length of the bolt,
	 * apply it to each monster (and then the player) in turn
	 */
	for (x = 0; x < BOLT_LENGTH; x++) {
	    ch = winat(spotpos[x].y, spotpos[x].x);

	    if (take_that[x] != 0 && isalpha(ch)) {
		if ((item = find_mons(spotpos[x].y, spotpos[x].x)) == NULL) {
		    debug("Can't find monster %c @ %d %d.",
			  ch, spotpos[x].y, spotpos[x].x);
		    continue;
		}
		else
		    tp = THINGPTR(item);
		mname = on(player, ISBLIND) ? "monster" : monsters[tp->t_index].m_name;

		debug("Zapped %s for %d (%d)",
		      mname, take_that[x], tp->t_stats.s_hpt - take_that[x]);
		if ((tp->t_stats.s_hpt -= take_that[x]) <= 0) {
		    if (cansee(tp->t_pos.y, tp->t_pos.x))
			msg("The %s kills the %s.", name,
			    on(player, ISBLIND) ? "monster" : mname);
		    killed(shooter, item, NOMESSAGE, get_points);
		    ret_val = TRUE;
		}
		else if (get_points) {
		    runto(&spotpos[x], &hero);
		    summon_help(tp, NOFORCE);
		}
	    }
	    if (spotpos[x].y >= 0 && spotpos[x].x >= 0 &&
		spotpos[x].y < LINES && spotpos[x].x < COLS)
		mvwaddch(cw, spotpos[x].y, spotpos[x].x,
		     show(spotpos[x].y, spotpos[x].x));
	}
	if (take_that[BOLT_LENGTH] != 0) {
	    if (tp != THINGPTR(fam_ptr)) {
		if (terse)
		    msg("The %s hits you.", name);
		else
		    msg("You are hit by the %s.", name);
		if ((pstats.s_hpt -= take_that[BOLT_LENGTH]) <= 0) {
		    death(reason);
		    return;
		}
	    }
	}

    return (ret_val);
}

/*
 * monster_do_zap - monster gets the effect
 */
monster_do_zap(zapper, which, flags)
struct thing    *zapper;
int which;
int flags;
{
    struct stats    *curp = &(zapper->t_stats);
    struct stats    *maxp = &(zapper->maxstats);
    bool    blessed = flags & ISBLESSED;
    bool    cursed = flags & ISCURSED;
    bool    shoot = FALSE;
    int damage;
    int ndice, nsides;
    char    ch;

    switch (which) {
	when    WS_MISSILE:
	    bolt_name = "magic missile";
	    do_motion(&obolt, delta.y, delta.x, zapper);
	    ch = winat(obolt.o_pos.y, obolt.o_pos.x);
	    ndice = curp->s_lvl;
	    nsides = 4;
	    if (cursed)
		nsides /= 2;
	    else if (blessed)
		nsides *= 2;
	    damage = roll(ndice, nsides);
	    if (ch == PLAYER) {
		if (save(VS_MAGIC)) /* help the player */
		    msg("You evade the %s.", bolt_name);
		else {
		    if (terse)
			msg("The %s hits you.", bolt_name);
		    else
			msg("You are hit by the %s.", bolt_name);
		    if ((pstats.s_hpt -= damage) <= 0)
			death(D_BOLT);
		}
	    }
	    else if (isalpha(ch)) {
		struct linked_list  *item = find_mons(obolt.o_pos.y, obolt.o_pos.x);

		if (item != NULL) {
		    struct thing    *tp = THINGPTR(item);
		    bool    see_it = cansee(obolt.o_pos.y, obolt.o_pos.x);

		    if ((tp->t_stats.s_hpt -= damage) <= 0)
			killed(zapper, item, see_it, (zapper == THINGPTR(fam_ptr)));
		    else if (see_it)
			msg("The %s hits the monster.", bolt_name);
		}
	    }
	    if (obolt.o_pos.y >= 0 && obolt.o_pos.x >= 0 &&
		obolt.o_pos.y < LINES && obolt.o_pos.x < COLS)
		mvwaddch(cw, obolt.o_pos.y, obolt.o_pos.x,
		     show(obolt.o_pos.y, obolt.o_pos.x));
	when    WS_CANCEL:
	    cancel_player(off(*zapper, ISUNIQUE));
	when    WS_COLD:
	    shoot = TRUE;
	    bolt_name = "cold";
	when    WS_FIRE:
	    shoot = TRUE;
	    bolt_name = "fire";
	when    WS_ELECT:
	    shoot = TRUE;
	    bolt_name = "lightning";
	otherwise:
	    debug("'%s' is a strange stick for a monster to zap!",
		  ws_magic[which].mi_name);
    }
    if (shoot) {
	ndice = curp->s_lvl / 2 + 1;
	nsides = 6;

	if (cursed)
	    nsides /= 2;
	else if (blessed)
	    nsides *= 2;
	damage = roll(ndice, nsides);
	shoot_bolt(zapper, zapper->t_pos, delta,
	    (zapper == THINGPTR(fam_ptr)), D_BOLT,
	    bolt_name, damage);
    }
}

struct powers {
    int p_flag;
    int     (*p_function) ();
};

/*
 * Order in which powers will attempt to be cancelled
 */
struct powers   player_powers[] = {
    {ISHASTE, nohaste},
    {ISELECTRIC, unelectrify},
    {CANINWALL, unphase},
    {CANFLY, unfly},
    {ISINVIS, appear},
    {CANREFLECT, ungaze},
    {ISDISGUISE, undisguise},
    {HASMSHIELD, unmshield},
    {ISREGEN, unregen},
    {CANSEE, unsee},
    {NOCOLD, uncold},
    {NOFIRE, unhot},
    {HASOXYGEN, unbreathe},
    {HASSHIELD, unshield},
    {-1, NULL}
};

cancel_player(not_unique)
bool    not_unique;
{
    struct powers   *pp;
    bool    no_effect = TRUE;

    for (pp = player_powers; pp->p_flag != -1; pp++) {
	if (on(player, pp->p_flag) && !save(VS_MAGIC)) {
	    (*pp->p_function) ();
	    extinguish(pp->p_function);
	    no_effect = FALSE;
	    if (not_unique) /* Gods might cancel everything */
		break;
	}
    }
    if (no_effect)
	nothing_message(ISNORMAL);
}
