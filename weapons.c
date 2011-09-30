/*
    weapons.c  -  Functions for dealing with problems brought about by weapons
   
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

#include <ctype.h>
#include "rogue.h"

/*
 * missile: Fire a missile in a given direction
 */

missile(ydelta, xdelta, item, tp)
int ydelta, xdelta;
struct linked_list  *item;
struct thing    *tp;
{
    struct object   *obj;
    struct linked_list  *nitem;

    if (item == NULL)   /* Get which thing we are hurling */
	return;
    obj = OBJPTR(item);
    if (!dropcheck(obj) || is_current(obj))
	return;

    /*
     * Get rid of the thing. If it is a non-multiple item object, or if
     * it is the last thing, just drop it. Otherwise, create a new item
     * with a count of one.
     */
    if (obj->o_count < 2) {
	if (tp->t_pack == pack)
	    rem_pack(obj);
	else
	    detach(tp->t_pack, item);
    }
    else {
	obj->o_count--;
	nitem = (struct linked_list *) new_item(sizeof *obj);
	obj = OBJPTR(nitem);
	*obj = *(OBJPTR(item));
	obj->o_count = 1;
	item = nitem;
    }
    switch (obj->o_type) {
	when    ARTIFACT:
	    has_artifact &= ~(1 << obj->o_which);
	when    SCROLL:
	    if (obj->o_which == S_SCARE && obj->o_flags & ISBLESSED)
		obj->o_flags &= ~ISBLESSED;
	    else
		obj->o_flags |= ISCURSED;
    }

    updpack();
    do_motion(obj, ydelta, xdelta, tp);

    /*
     * AHA! Here it has hit something. If it is a wall or a door, or if
     * it misses (combat) the monster, put it on the floor
     */
    if (!hit_monster(obj->o_pos.y, obj->o_pos.x, obj, tp)) {
	if (obj->o_type == WEAPON && obj->o_which == GRENADE) {
	    static coord    fpos;

	    hearmsg("BOOOM!");
	    aggravate();
	    if (ntraps + 1 < 2 * MAXTRAPS &&
		fallpos(&obj->o_pos, &fpos, TRUE)) {
		mvaddch(fpos.y, fpos.x, TRAPDOOR);
		traps[ntraps].tr_type = TRAPDOOR;
		traps[ntraps].tr_flags = ISFOUND;
		traps[ntraps].tr_show = TRAPDOOR;
		traps[ntraps].tr_pos.y = fpos.y;
		traps[ntraps++].tr_pos.x = fpos.x;
		light(&hero);
	    }
	    discard(item);
	}
	else if (obj->o_flags & ISLOST) {
	    if (obj->o_type == WEAPON)
		addmsg("The %s", weaps[obj->o_which].w_name);
	    else
		addmsg(inv_name(obj, LOWERCASE));
	    msg(" vanishes in a puff of greasy smoke.");
	    discard(item);
	}
	else {
	    fall(&player, item, TRUE);
	    if (obj->o_flags & CANRETURN)
		msg("You have %s.", inv_name(obj, LOWERCASE));
	}
    }
    else if (obj->o_flags & ISOWNED) {
	add_pack(item, NOMESSAGE);
	msg("You have %s.", inv_name(obj, LOWERCASE));
    }
    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
 * do the actual motion on the screen done by an object traveling across the
 * room
 */
do_motion(obj, ydelta, xdelta, tp)
struct object   *obj;
int ydelta, xdelta;
struct thing    *tp;
{

    /*
     * Come fly with us ...
     */
    obj->o_pos = tp->t_pos;
    for (;;) {
	int ch;

	/*
	 * Erase the old one
	 */
	if (!ce(obj->o_pos, tp->t_pos) &&
	    cansee(obj->o_pos.y, obj->o_pos.x) &&
	    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
	    mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
		show(obj->o_pos.y, obj->o_pos.x));
	}

	/*
	 * Get the new position
	 */
	obj->o_pos.y += ydelta;
	obj->o_pos.x += xdelta;
	if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) &&
	    ch != DOOR && !ce(obj->o_pos, hero)) {

	    /*
	     * It hasn't hit anything yet, so display it If it
	     * alright.
	     */
	    if (cansee(obj->o_pos.y, obj->o_pos.x) &&
		mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
		mvwaddch(cw, obj->o_pos.y, obj->o_pos.x,
		    obj->o_type);
		wrefresh(cw);
	    }
	    continue;
	}
	break;
    }
}

/*
 * fall: Drop an item someplace around here.
 */

fall(tp, item, pr)
struct linked_list  *item;
struct thing    *tp;
bool    pr;
{
    struct object   *obj;
    struct room *rp;
    static coord    fpos;

    obj = OBJPTR(item);
    rp = roomin(&(tp->t_pos));
    if (obj->o_flags & CANRETURN) {
	add_pack(item, NOMESSAGE);
	msg("You have %s.", inv_name(obj, LOWERCASE));
	return;
    }
    else if (fallpos(&obj->o_pos, &fpos, TRUE)) {
	if (obj->o_flags & CANBURN && obj->o_type == WEAPON
	    && obj->o_which == MOLOTOV
	    && ntraps + 1 < 2 * MAXTRAPS) {
	    mvaddch(fpos.y, fpos.x, FIRETRAP);
	    traps[ntraps].tr_type = FIRETRAP;
	    traps[ntraps].tr_flags = ISFOUND;
	    traps[ntraps].tr_show = FIRETRAP;
	    traps[ntraps].tr_pos.y = fpos.y;
	    traps[ntraps++].tr_pos.x = fpos.x;
	    if (rp != NULL)
		rp->r_flags &= ~ISDARK;
	}
	else {
	    obj->o_pos = fpos;
	    add_obj(item, fpos.y, fpos.x);
	}
	if (rp != NULL &&
	    (!(rp->r_flags & ISDARK) ||
	     (rp->r_flags & HASFIRE))) {
	    light(&hero);
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
	}
	return;
    }
    if (pr) {
	if (cansee(obj->o_pos.y, obj->o_pos.x)) {
	    if (obj->o_type == WEAPON)
		addmsg("The %s", weaps[obj->o_which].w_name);
	    else
		addmsg(inv_name(obj, LOWERCASE));
	    msg(" vanishes as it hits the ground.");
	}
    }
    discard(item);
}

/*
 * init_weapon: Set up the initial goodies for a weapon
 */

init_weapon(weap, type)
struct object   *weap;
char    type;
{
    struct init_weps    *iwp = &weaps[type];

    weap->o_damage = iwp->w_dam;
    weap->o_hurldmg = iwp->w_hrl;
    weap->o_launch = iwp->w_launch;
    weap->o_flags = iwp->w_flags;
    weap->o_weight = iwp->w_wght;
    if (weap->o_flags & ISMANY) {
	weap->o_count = rnd(8) + 8;
	weap->o_group = ++group;
    }
    else
	weap->o_count = 1;
}

/*
 * hit_monster - does the missile hit the target
 */

hit_monster(y, x, weapon, thrower)
int y, x;
struct object   *weapon;
struct thing    *thrower;
{
    struct linked_list  *mon;
    static coord    target;

    target.y = y;
    target.x = x;

    if (thrower == &player)
	return (fight(&target, weapon, THROWN));

    if (ce(target, hero)) {
	if (good_monster(*thrower)) {
	    if (on(*thrower, ISFAMILIAR))
		msg("Please get out of the way, Master!  I nearly hit you.");
	    else
		msg("Get out of the way %s!", whoami);
	    return (FALSE);
	}
	return (attack(thrower, weapon, THROWN));
    }

    if ((mon = find_mons(y, x)) != NULL)
	return (mon_mon_attack(thrower, mon, weapon, THROWN));
    else
	return (FALSE);
}

/*
 * num: Figure out the plus number for armor/weapons
 */

char    *num(n1, n2)
int n1, n2;
{
    static char numbuf[2 * LINELEN];

    if (n1 == 0 && n2 == 0)
	return "+0";

    if (n2 == 0)
	sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1);
    else
	sprintf(numbuf, "%s%d, %s%d", n1 < 0 ? "" : "+",
	    n1, n2 < 0 ? "" : "+", n2);

    return (numbuf);
}

/*
 * wield: Pull out a certain weapon
 */

wield()
{
    struct linked_list  *item;
    struct object   *obj, *oweapon;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon)) {
	cur_weapon = oweapon;
	return;
    }
    cur_weapon = oweapon;
    if ((item = get_item("wield", WEAPON)) == NULL) {
	after = FALSE;
	return;
    }

    obj = OBJPTR(item);
    if (is_current(obj)) {
	after = FALSE;
	return;
    }
    wield_ok(&player, obj, TRUE);

    msg("%sielding %s.", (terse ? "W" : "You are now w"),
	inv_name(obj, LOWERCASE));
    cur_weapon = obj;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
fallpos(pos, newpos, passages)
coord   *pos, *newpos;
bool    passages;
{
    int y, x, cnt, ch;

    cnt = 0;
    for (y = pos->y - 1; y <= pos->y + 1; y++)
	for (x = pos->x - 1; x <= pos->x + 1; x++)

	    /*
	     * check to make certain the spot is empty, if it is,
	     * put the object there, set it in the level list and
	     * re-draw the room if he can see it
	     */
	    if (y == hero.y && x == hero.x)
		continue;
	    else
		switch (ch = winat(y, x)) {
		    case GOLD:
		    case POTION:
		    case SCROLL:
		    case FOOD:
		    case WEAPON:
		    case ARMOR:
		    case RING:
		    case STICK:
		    case FLOOR:
		    case PASSAGE:
		    case ARTIFACT:
			newpos->y = y;
			newpos->x = x;
			cnt++;
		}
    return (cnt);
}

/*
 * wield_ok: enforce player class weapons restrictions
 */

bool
wield_ok(wieldee, obj, print_message)
struct thing    *wieldee;
struct object   *obj;
bool    print_message;
{
    int which = obj->o_which;
    bool    ret_val = TRUE;
    int class_type = wieldee->t_ctype;

    if (obj->o_type != WEAPON) {
	ret_val = FALSE;
	return (ret_val);
    }

    else
	switch (class_type) {
	    when    C_MAGICIAN: /* need one hand free */
	    case C_ILLUSION:
		if (obj->o_flags & ISTWOH)
		    ret_val = FALSE;
	    when    C_THIEF:    /* need portable weapon  */
	    case C_ASSASIN:
	    case C_NINJA:
		if (obj->o_flags & ISTWOH)
		    ret_val = FALSE;
	    when    C_CLERIC:   /* No sharp weapons */
		if (obj->o_flags & ISSHARP)
		    ret_val = FALSE;
	    when    C_DRUID:    /* No non-silver metal
			 * weapons */
		if (obj->o_flags & ISMETAL &&
		    !(obj->o_flags & ISSILVER))
		    ret_val = FALSE;
	    when    C_PALADIN:  /* must wield sharp stuff */
		if ((obj->o_flags & ISSHARP) == FALSE)
		    ret_val = FALSE;
	    when    C_FIGHTER:  /* wield anything */
	    case C_RANGER:
	    case C_MONSTER:
		;
	    otherwise:  /* Unknown class */
		debug("Unknown class %d.", class_type);
	    break;
	}

    if (itemweight(obj) > 18 * pstats.s_str) {
	if (wieldee == &player && print_message == TRUE)
	    msg("That is too heavy for you to swing effectively!");
	ret_val = FALSE;
	return (ret_val);
    }

    if (ret_val == FALSE && print_message == MESSAGE)
	switch (class_type) {
	    when    C_MAGICIAN:
	    case C_ILLUSION:
		msg("You'll find it hard to cast spells while wielding that!");
	    when    C_THIEF:
	    case C_ASSASIN:
	    case C_NINJA:
		msg("Don't expect to backstab anyone while wielding that!");
	    when    C_CLERIC:
	    case C_DRUID:
	    case C_PALADIN:
		msg("Your god strongly disapproves of your wielding that!");
	    case C_FIGHTER:
	    case C_RANGER:
	    case C_MONSTER:
		break;
	}
    return (ret_val);
}
