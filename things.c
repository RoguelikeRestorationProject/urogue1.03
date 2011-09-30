/*
    things.c  -  Contains functions for dealing with things like potions and scrolls
   
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
 * inv_name: return the name of something as it would appear in an inventory.
 */
char    *
inv_name(obj, lowercase)
struct object   *obj;
bool    lowercase;
{
    char    *pb;

    switch (obj->o_type) {
	when    SCROLL:
	    if (obj->o_count == 1)
		sprintf(prbuf, "A %s%sscroll ",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    blesscurse(obj->o_flags));
	    else
		sprintf(prbuf, "%d %s%sscrolls ",
		    obj->o_count,
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    blesscurse(obj->o_flags));
	    pb = &prbuf[strlen(prbuf)];
	    if (know_items[TYP_SCROLL][obj->o_which])
		sprintf(pb, "of %s", s_magic[obj->o_which].mi_name);
	    else if (guess_items[TYP_SCROLL][obj->o_which])
		sprintf(pb, "called %s", guess_items[TYP_SCROLL][obj->o_which]);
	    else
		sprintf(pb, "titled '%s'", s_names[obj->o_which]);
	when    POTION:
	    if (obj->o_count == 1)
		sprintf(prbuf, "A %s%spotion ",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    blesscurse(obj->o_flags));

	    else
		sprintf(prbuf, "%d %s%spotions ",
		    obj->o_count,
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    blesscurse(obj->o_flags));
	    pb = &prbuf[strlen(prbuf)];
	    if (know_items[TYP_POTION][obj->o_which])
		sprintf(pb, "of %s(%s)", p_magic[obj->o_which].mi_name,
		    p_colors[obj->o_which]);
	    else if (guess_items[TYP_POTION][obj->o_which])
		sprintf(pb, "called %s(%s)", guess_items[TYP_POTION][obj->o_which],
		    p_colors[obj->o_which]);
	    else {
		if (obj->o_count == 1)
		    sprintf(prbuf, "A%s %s potion",
			 obj->o_flags & CANRETURN ? " claimed" :
			vowelstr(p_colors[obj->o_which]),
			p_colors[obj->o_which]);
		else
		    sprintf(prbuf, "%d %s%s potions",
			obj->o_count,
		     obj->o_flags & CANRETURN ? "claimed " : "",
			p_colors[obj->o_which]);
	    }
	when    FOOD:
	    if (obj->o_count == 1)
		sprintf(prbuf, "A%s %s",
		    obj->o_flags & CANRETURN ? " claimed" :
		    vowelstr(fd_data[obj->o_which].mi_name),
		    fd_data[obj->o_which].mi_name);
	    else
		sprintf(prbuf, "%d %s%ss", obj->o_count,
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    fd_data[obj->o_which].mi_name);
	when    WEAPON:
	    if (obj->o_count > 1)
		sprintf(prbuf, "%d ", obj->o_count);
	    else if ((obj->o_flags & (ISZAPPED | CANRETURN | ISPOISON | ISSILVER | ISTWOH)) ||
		 ((obj->o_flags & (ISKNOW | ISZAPPED)) == (ISKNOW | ISZAPPED)))
		strcpy(prbuf, "A ");
	    else
		sprintf(prbuf, "A%s ", vowelstr(weaps[obj->o_which].w_name));
	    pb = &prbuf[strlen(prbuf)];
	    if ((obj->o_flags & ISKNOW) && (obj->o_flags & ISZAPPED))
		sprintf(pb, "charged%s ", charge_str(obj));
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & CANRETURN)
		sprintf(pb, "claimed ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISPOISON)
		sprintf(pb, "poisoned ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISSILVER)
		sprintf(pb, "silver ");
	    if (obj->o_flags & ISTWOH)
		sprintf(pb, "two-handed ");
	    pb = &prbuf[strlen(prbuf)];
	    if (obj->o_flags & ISKNOW)
		sprintf(pb, "%s %s", num(obj->o_hplus, obj->o_dplus),
		weaps[obj->o_which].w_name);
	    else
		sprintf(pb, "%s", weaps[obj->o_which].w_name);
	    if (obj->o_count > 1)
		strcat(prbuf, "s");
	when    ARMOR:
	    if (obj->o_flags & ISKNOW)
		sprintf(prbuf, "%s%s %s",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		   num(armors[obj->o_which].a_class - obj->o_ac, 0),
		    armors[obj->o_which].a_name);
	    else
		sprintf(prbuf, "%s%s",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    armors[obj->o_which].a_name);
	when    ARTIFACT:
	    sprintf(prbuf, "the %s", arts[obj->o_which].ar_name);
	    if (obj->o_flags & CANRETURN)
		strcat(prbuf, " (claimed)");
	when    STICK:
	    sprintf(prbuf, "A %s%s%s ",
		obj->o_flags & CANRETURN ? "claimed " : "",
		blesscurse(obj->o_flags), ws_type[obj->o_which]);
	    pb = &prbuf[strlen(prbuf)];
	    if (know_items[TYP_STICK][obj->o_which])
		sprintf(pb, "of %s%s(%s)", ws_magic[obj->o_which].mi_name,
		    charge_str(obj), ws_made[obj->o_which]);
	    else if (guess_items[TYP_STICK][obj->o_which])
		sprintf(pb, "called %s(%s)", guess_items[TYP_STICK][obj->o_which],
		    ws_made[obj->o_which]);
	    else
		sprintf(&prbuf[2], "%s%s %s",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    ws_made[obj->o_which],
		    ws_type[obj->o_which]);
	when    RING:
	    if (know_items[TYP_RING][obj->o_which])
		sprintf(prbuf, "A%s%s ring of %s(%s)",
		    obj->o_flags & CANRETURN ? " claimed" : "", ring_num(obj),
		    r_magic[obj->o_which].mi_name, r_stones[obj->o_which]);
	    else if (guess_items[TYP_RING][obj->o_which])
		sprintf(prbuf, "A %sring called %s(%s)",
		    obj->o_flags & CANRETURN ? "claimed " : "",
		    guess_items[TYP_RING][obj->o_which], r_stones[obj->o_which]);
	    else
		sprintf(prbuf, "A%s %s ring",
		    obj->o_flags & CANRETURN ? "claimed " :
		    vowelstr(r_stones[obj->o_which]),
		    r_stones[obj->o_which]);
	when    GOLD:
	    sprintf(prbuf, "%d gold pieces", obj->o_count);
	otherwise:
	    debug("Picked up something funny.");
	    sprintf(prbuf, "Something bizarre %s", unctrl(obj->o_type));
    }

    /* Is it marked? */
    if (obj->o_mark[0]) {
	pb = &prbuf[strlen(prbuf)];
	sprintf(pb, " <%s>", obj->o_mark);
    }

    if (obj == cur_armor)
	strcat(prbuf, " (being worn)");
    if (obj == cur_weapon)
	strcat(prbuf, " (weapon in hand)");
    if (obj == cur_ring[LEFT_1])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_2])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_3])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_4])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[LEFT_5])
	strcat(prbuf, " (on left hand)");
    else if (obj == cur_ring[RIGHT_1])
	strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_2])
	strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_3])
	strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_4])
	strcat(prbuf, " (on right hand)");
    else if (obj == cur_ring[RIGHT_5])
	strcat(prbuf, " (on right hand)");
    if (obj->o_flags & ISPROT)
	strcat(prbuf, " [protected]");
    if (lowercase && isupper(prbuf[0]))
	prbuf[0] = tolower(prbuf[0]);
    else if (!lowercase && islower(*prbuf))
	*prbuf = toupper(*prbuf);
    if (!lowercase)
	strcat(prbuf, ".");
    return prbuf;
}

/*
 * rem_obj: Remove an object from the level.
 *
 */
rem_obj(item, dis)
struct linked_list  *item;
int dis;
{
    struct linked_list  *llptr;
    struct object   *objptr, *op;
    int x, y;

    detach(lvl_obj, item);
    objptr = OBJPTR(item);

    if (llptr = objptr->next_obj) {
	detach((objptr->next_obj), llptr);
	attach(lvl_obj, llptr);
	op = OBJPTR(llptr);
	if (op->next_obj = objptr->next_obj)
	    objptr->next_obj->l_prev = NULL;
	y = op->o_pos.y;
	x = op->o_pos.x;
	if (cansee(y, x))
	    mvwaddch(cw, y, x, op->o_type);
	mvaddch(y, x, op->o_type);
    }
    else {
	y = objptr->o_pos.y;
	x = objptr->o_pos.x;

	/*
	 * problems if dropped in rock
	 */
	mvaddch(y, x, (roomin(&(objptr->o_pos)) == NULL
	    ? PASSAGE : FLOOR));
    }
    if (dis)
	discard(item);
}

/*
 * add_obj: adds an object to the level
 *
 */
add_obj(item, y, x)
struct linked_list  *item;
int y, x;
{
    struct linked_list  *llptr;
    struct object   *objptr;

    llptr = find_obj(y, x, TRUE);
    if (llptr) {
	objptr = OBJPTR(llptr);
	attach((objptr->next_obj), item);
    }
    else {
	attach(lvl_obj, item);
	objptr = OBJPTR(item);
	objptr->next_obj = NULL;
	mvaddch(y, x, objptr->o_type);
    }
}

/*
 * drop: put something down
 */
drop(item)
struct linked_list  *item;
{
    char    ch = mvwinch(stdscr, hero.y, hero.x);
    struct object   *obj;

    switch (ch) {
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
	case POOL:
	case ARTIFACT:
	    if (item == NULL && (item = get_item("drop", 0)) == NULL)
		return (FALSE);
	otherwise:
	    msg("You can't drop something here.");
	    return (FALSE);
    }

    obj = OBJPTR(item);
    if (!dropcheck(obj))
	return (FALSE);

    /*
     * Curse a dropped scare monster scroll
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE) {
	if (obj->o_flags & ISBLESSED)
	    obj->o_flags &= ~ISBLESSED;
	else
	    obj->o_flags |= ISCURSED;
    }

    /*
     * Take it out of the pack
     */
    if (obj->o_count >= 2 && obj->o_group == 0) {
	struct linked_list  *nitem = new_item(sizeof *obj);

	obj->o_count--;
	obj = OBJPTR(nitem);
	*obj = *(OBJPTR(item));
	obj->o_count = 1;
	item = nitem;
    }
    else
	rem_pack(obj);

    if (ch == POOL) {
	msg("The pool bubbles briefly as your %s sinks out of sight.",
	    inv_name(obj, TRUE));
	discard(item);
	item = NULL;
    }
    else {          /* Link it into the level object list */
	add_obj(item, hero.y, hero.x);
	mvaddch(hero.y, hero.x, obj->o_type);
    }
    if (obj->o_type == ARTIFACT)
	has_artifact &= ~(1 << obj->o_which);

    updpack();
    return (TRUE);
}

/*
 * do special checks for dropping or unweilding|unwearing|unringing
 */
dropcheck(op)
struct object   *op;
{
    char    *find_slot();       /* really (struct delayed_action *) */

    if (op == NULL)
	return TRUE;
    if (op != cur_armor && op != cur_weapon &&
	op != cur_ring[LEFT_1] && op != cur_ring[LEFT_2] &&
	op != cur_ring[LEFT_3] && op != cur_ring[LEFT_4] &&
	op != cur_ring[LEFT_5] &&
	op != cur_ring[RIGHT_1] && op != cur_ring[RIGHT_2] &&
	op != cur_ring[RIGHT_3] && op != cur_ring[RIGHT_4] &&
	op != cur_ring[RIGHT_5])
	return TRUE;
    if (op->o_flags & ISCURSED) {
	msg("You can't.  It appears to be cursed.");
	return FALSE;
    }
    if (op == cur_weapon)
	cur_weapon = NULL;
    else if (op == cur_armor) {
	waste_time();
	cur_armor = NULL;
    }
    else if (op == cur_ring[LEFT_1] || op == cur_ring[LEFT_2] ||
	 op == cur_ring[LEFT_3] || op == cur_ring[LEFT_4] ||
	 op == cur_ring[LEFT_5] ||
	 op == cur_ring[RIGHT_1] || op == cur_ring[RIGHT_2] ||
	 op == cur_ring[RIGHT_3] || op == cur_ring[RIGHT_4] ||
	 op == cur_ring[RIGHT_5]) {
	if (op == cur_ring[LEFT_1])
	    cur_ring[LEFT_1] = NULL;
	else if (op == cur_ring[LEFT_2])
	    cur_ring[LEFT_2] = NULL;
	else if (op == cur_ring[LEFT_3])
	    cur_ring[LEFT_3] = NULL;
	else if (op == cur_ring[LEFT_4])
	    cur_ring[LEFT_4] = NULL;
	else if (op == cur_ring[LEFT_5])
	    cur_ring[LEFT_5] = NULL;
	else if (op == cur_ring[RIGHT_1])
	    cur_ring[RIGHT_1] = NULL;
	else if (op == cur_ring[RIGHT_2])
	    cur_ring[RIGHT_2] = NULL;
	else if (op == cur_ring[RIGHT_3])
	    cur_ring[RIGHT_3] = NULL;
	else if (op == cur_ring[RIGHT_4])
	    cur_ring[RIGHT_4] = NULL;
	else if (op == cur_ring[RIGHT_5])
	    cur_ring[RIGHT_5] = NULL;
	switch (op->o_which) {
	    when    R_ADDSTR:
		chg_str(-op->o_ac, FALSE, FALSE);
	    when    R_ADDHIT:
		chg_dext(-op->o_ac, FALSE, FALSE);
	    when    R_ADDINTEL:
		pstats.s_intel -= op->o_ac;
	    when    R_ADDWISDOM:
		pstats.s_wisdom -= op->o_ac;
	    when    R_SEEINVIS:
		if (find_slot(unsee) == NULL) {
		    turn_off(player, CANSEE);
		    msg("The tingling feeling leaves your eyes.");
		}
		light(&hero);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
	    when    R_LIGHT:{
		if (roomin(&hero) != NULL) {
		    light(&hero);
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
		}
	    }
	}
    }
    return TRUE;
}

/*
 * return a new thing
 */
struct linked_list  *
new_thing()
{
    int j, k;
    struct linked_list  *item;
    struct object   *cur;
    short   blesschance = rnd(100);
    short   cursechance = rnd(100);

    item = new_item(sizeof *cur);
    cur = OBJPTR(item);
    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_count = 1;
    cur->o_group = 0;
    cur->o_flags = 0;
    cur->o_weight = 0;
    cur->o_mark[0] = '\0';

    /*
     * Decide what kind of object it will be If we haven't had food for a
     * while, let it be food.
     */
    switch (no_food > 3 ? TYP_FOOD : pick_one(things, numthings)) {
	when    TYP_POTION:
	    cur->o_type = POTION;
	    cur->o_which = pick_one(p_magic, maxpotions);
	    cur->o_weight = things[TYP_POTION].mi_wght;
	    if (cursechance < p_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < p_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	when    TYP_SCROLL:
	    cur->o_type = SCROLL;
	    cur->o_which = pick_one(s_magic, maxscrolls);
	    cur->o_weight = things[TYP_SCROLL].mi_wght;
	    if (cursechance < s_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < s_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	when    TYP_FOOD:
	    no_food = 0;
	    cur->o_type = FOOD;
	    cur->o_which = pick_one(fd_data, maxfoods);
	    cur->o_weight = 2;
	    cur->o_count += extras();
	when    TYP_WEAPON:
	    cur->o_type = WEAPON;
	    cur->o_which = rnd(maxweapons);
	    init_weapon(cur, cur->o_which);
	    if (cursechance < 10) {
		short   bad = (rnd(10) < 1) ? 2 : 1;

		cur->o_flags |= ISCURSED;
		cur->o_hplus -= bad;
		cur->o_dplus -= bad;
	    }
	    else if (blesschance < 15) {
		short   good = (rnd(10) < 1) ? 2 : 1;

		cur->o_hplus += good;
		cur->o_dplus += good;
	    }
	when    TYP_ARMOR:
	    cur->o_type = ARMOR;
	    for (j = 0; j < maxarmors; j++)
		if (blesschance < armors[j].a_prob)
		    break;
	    if (j == maxarmors) {
		debug("Picked a bad armor %d", blesschance);
		j = 0;
	    }
	    cur->o_which = j;
	    cur->o_ac = armors[j].a_class;
	    if (((k = rnd(100)) < 20) && j != MITHRIL) {
		cur->o_flags |= ISCURSED;
		cur->o_ac += rnd(3) + 1;
	    }
	    else if (k < 28 || j == MITHRIL)
		cur->o_ac -= rnd(3) + 1;
	    if (j == MITHRIL)
		cur->o_flags |= ISPROT;
	    cur->o_weight = armors[j].a_wght;
	when    TYP_RING:
	    cur->o_type = RING;
	    cur->o_which = pick_one(r_magic, maxrings);
	    cur->o_weight = things[TYP_RING].mi_wght;
	    if (cursechance < r_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < r_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	    switch (cur->o_which) {
		when    R_ADDSTR:
		case R_ADDWISDOM:
		case R_ADDINTEL:
		case R_PROTECT:
		case R_ADDHIT:
		case R_ADDDAM:
		case R_CARRYING:
		    cur->o_ac = rnd(2) + 1; /* From 1 to 3 */
			if (cur->o_flags & ISCURSED)
		    cur->o_ac = -cur->o_ac;
		    if (cur->o_flags & ISBLESSED)
			cur->o_ac++;
		when    R_RESURRECT:
		case R_TELCONTROL:
		case R_VREGEN:
		case R_REGEN:
		case R_PIETY:
		case R_WIZARD:
		    cur->o_ac = 0;
		    if (cur->o_flags & ISCURSED)
			cur->o_ac = -1;
		    if (cur->o_flags & ISBLESSED)
			cur->o_ac = 1;
		when    R_DIGEST:
		    if (cur->o_flags & ISCURSED)
		    cur->o_ac = -1;
		    else if (cur->o_flags & ISBLESSED)
			cur->o_ac = 2;
		    else
			cur->o_ac = 1;
		otherwise:
		    cur->o_ac = 0;
	    }
	when    TYP_STICK:
	    cur->o_type = STICK;
	    cur->o_which = pick_one(ws_magic, maxsticks);
	    fix_stick(cur);
	    if (cursechance < ws_magic[cur->o_which].mi_curse)
		cur->o_flags |= ISCURSED;
	    else if (blesschance < ws_magic[cur->o_which].mi_bless)
		cur->o_flags |= ISBLESSED;
	otherwise:
	    debug("Picked a bad kind of object");
	    wait_for(' ');
    }
    return item;
}

/*
 * provide a new item tailored to specification
 */
struct linked_list  *
spec_item(type, which, hit, damage)
int type, which, hit, damage;
{
    struct linked_list  *item;
    struct object   *obj;

    item = new_item(sizeof *obj);
    obj = OBJPTR(item);

    obj->o_count = 1;
    obj->o_group = 0;
    obj->o_type = type;
    obj->o_which = which;
    obj->o_damage = obj->o_hurldmg = "0d0";
    obj->o_hplus = 0;
    obj->o_dplus = 0;
    obj->o_flags = 0;
    obj->o_mark[0] = '\0';
    obj->o_text = NULL;
    obj->o_launch = 0;
    obj->o_weight = 0;

    switch (type) {     /* Handle special characteristics */
	when    WEAPON:
	    init_weapon(obj, which);
	    obj->o_hplus = hit;
	    obj->o_dplus = damage;
	    obj->o_ac = 10;
	when    ARMOR:
	    obj->o_ac = armors[which].a_class - hit;
	when    RING:
	    obj->o_ac = hit;
	when    STICK:
	    fix_stick(obj);
	    obj->o_charges = hit;
	when    GOLD:
	    obj->o_type = GOLD;
	    obj->o_count = GOLDCALC;
	    obj->o_ac = 11;
    }
    if (hit > 0 || damage > 0)
	obj->o_flags |= ISBLESSED;
    else if (hit < 0 || damage < 0)
	obj->o_flags |= ISCURSED;

    return (item);
}

/*
 * pick an item out of a list of nitems possible magic items
 */
pick_one(magic, nitems)
struct magic_item   *magic;
int nitems;
{
    int i;
    struct magic_item   *end;
    struct magic_item   *start = magic;

    for (end = &magic[nitems], i = rnd(1000); magic < end; magic++)
	if (i <= magic->mi_prob)
	    break;
    if (magic == end) {
	if (wizard) {
	    add_line("Bad pick_one: %d from %d items", i, nitems);
	    for (magic = start; magic < end; magic++)
		add_line("%s: %d%%", magic->mi_name, magic->mi_prob);
	    end_line();
	}
	magic = start;
    }
    return magic - start;
}


/*
 * blesscurse - returns whether the object is  blessed or cursed
 */

char    *
blesscurse(flags)
long    flags;
{
    if (flags & ISKNOW) {
	if (flags & ISCURSED)
	    return ("cursed ");
	if (flags & ISBLESSED)
	    return ("blessed ");
	return ("normal ");
    }
    return ("");
}

/*
 * extras: Return the number of extra food items to be created
 */
int
extras()
{
    int i = rnd(100);

    if (i < 10)
	return (2);
    else if (i < 20)
	return (1);
    else
	return (0);
}

/*
 * name_type
 *
 * Returns a string identifying a pack item's type
 */

char    *
name_type(type)
char    type;
{
    switch (type) {
	when    ARMOR:
	    return ("armor");
	when    WEAPON:
	    return ("weapons");
	when    SCROLL:
	    return ("scrolls");
	when    RING:
	    return ("rings");
	when    STICK:
	    return ("staffs");
	when    POTION:
	    return ("potions");
	when    FOOD:
	    return ("food");
	when    GOLD:
	    return ("gold");
    otherwise:
	    return ("items");
    }
}

/*
 * make_item
 *
 * adds a linked_list structure to the top of an object for interfacing with
 * other routines
 */
linked_list *
make_item(obj_p)
object  *obj_p;
{
    linked_list *item_p = (linked_list *) ur_alloc(sizeof(linked_list));

    next(item_p) = prev(item_p) = NULL;
    item_p->l_data = (char *) obj_p;
    return (item_p);
}

/*
 * is_member
 *
 * Checks to see if a character is a member of an array
 */
is_member(list_p, member)
char    *list_p;
char    member;
{
    while (*list_p != 0) {
	if (*list_p++ == member)
	    return (TRUE);
    }
    return (FALSE);
}
