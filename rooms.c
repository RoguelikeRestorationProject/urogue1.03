/*
    rooms.c  -  Draw the nine rooms on the screen
   
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
#include "stepok.h"

do_rooms()
{
    int i;
    struct room *rp;
    struct linked_list  *item;
    struct thing    *tp;
    int left_out;
    coord   top;
    coord   bsze;
    coord   mp;

    /*
     * bsze is the maximum room size
     */
    bsze.x = COLS / 3;
    bsze.y = (LINES - 2) / 3;

    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	rp->r_nexits = rp->r_flags = rp->r_fires = 0;

    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
	rooms[rnd_room()].r_flags |= ISGONE;

    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++) {
	bool    has_gold = FALSE;

	/*
	 * Find upper left corner of box that this room goes in
	 */
	top.x = (i % 3) * bsze.x;
	top.y = i / 3 * bsze.y + 1;
	if (rp->r_flags & ISGONE) {

	    /*
	     * Place a gone room.  Make certain that there is a
	     * blank line for passage drawing.
	     */
	    do {
		rp->r_pos.x = top.x + rnd(bsze.x - 2) + 1;
		rp->r_pos.y = top.y + rnd(bsze.y - 2) + 1;
		rp->r_max.x = -COLS;
		rp->r_max.x = -LINES;
	    } while (rp->r_pos.y < 1 || rp->r_pos.y > LINES - 3);
	    continue;
	}
	if (rnd(80) < level - 15)
	    rp->r_flags |= ISDARK;

	/*
	 * Find a place and size for a random room
	 */
	do {
	    rp->r_max.x = rnd(bsze.x - 4) + 4;
	    rp->r_max.y = rnd(bsze.y - 4) + 4;
	    rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
	    rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
	} while (rp->r_pos.y == 0);

	/* Draw the room */
	draw_room(rp);

	/*
	 * Put the gold in
	 */
	if (rnd(100) < 50 && (!has_artifact || level >= max_level)) {
	    struct linked_list  *item;
	    struct object   *cur;
	    coord   tp;

	    has_gold = TRUE;    /* This room has gold in it */

	    item = spec_item(GOLD, NULL, NULL, NULL);
	    cur = OBJPTR(item);

	    /* Put it somewhere */
	    rnd_pos(rp, &tp);
	    cur->o_pos = tp;
	    /* Put the gold into the level list of items */
	    add_obj(item, tp.y, tp.x);

	    if (roomin(&tp) != rp) {
		endwin();
		abort();
	    }
	}

	/*
	 * Put the monster in
	 */
	if (rnd(100) < (has_gold ? 80 : 40)) {
	    short   which;
	    int i, count;

	    item = new_item(sizeof *tp);
	    tp = THINGPTR(item);
	    do {
		rnd_pos(rp, &mp);
	    } while (mvwinch(stdscr, mp.y, mp.x) != FLOOR);

	    which = randmonster(NOWANDER, NOGRAB);
	    new_monster(item, which, &mp, NOMAXSTATS);

	    /*
	     * See if we want to give it a treasure to carry
	     * around.
	     */
	    if (rnd(100) < monsters[tp->t_index].m_carry)
		attach(tp->t_pack, new_thing());

	    /*
	     * If it has a fire, mark it
	     */
	    if (on(*tp, HASFIRE)) {
		rp->r_flags |= HASFIRE;
		rp->r_fires++;
	    }

	    /*
	     * If it carries gold, give it some
	     */
	    if (on(*tp, CARRYGOLD)) {
		struct object   *cur;

		item = spec_item(GOLD, NULL, NULL, NULL);
		cur = OBJPTR(item);
		cur->o_count = GOLDCALC + GOLDCALC + GOLDCALC;
		cur->o_pos = tp->t_pos;
		attach(tp->t_pack, item);
	    }

	    i = rnd(7);
	    if (on(*tp, ISSWARM) && i < 5)
		count = roll(2, 4);
	    else if (on(*tp, ISFLOCK) && i < 5)
		count = roll(1, 4);
	    else
		count = 0;

	    for (i = 1; i <= count; i++) {
		coord   *mpos;

		if ((mpos = place_mons(mp.y, mp.x)) != NULL) {
		    struct linked_list  *nitem;

		    nitem = new_item(sizeof(struct thing));
		    new_monster(nitem, which, mpos,
			NOMAXSTATS);

		    /*
		     * If the monster is on a trap, trap
		     * it
		     */
		    if (isatrap(mvinch(mpos->y, mpos->x)))
			be_trapped(THINGPTR(nitem), mp);
		    if (on(*tp, ISFRIENDLY))
			turn_on(*(THINGPTR(nitem)),
			    ISFRIENDLY);
		    else
			turn_off(*(THINGPTR(nitem)),
			    ISFRIENDLY);
		}
	    }
	    if (count > 0) {
		int boost = rnd(3) + 1;

		if (on(*tp, LOWCAST) || on(*tp, MEDCAST) ||
		    on(*tp, HIGHCAST))
		    turn_on(*tp, CANCAST);
		tp->t_stats.s_hpt += 3 * boost;
		tp->t_stats.s_arm -= 2 * boost;
		tp->t_stats.s_lvl += 2 * boost;
		tp->t_stats.s_str += 2 * boost;
		tp->t_stats.s_intel += 2 * boost;
		tp->t_stats.s_exp += 4 * boost *
		    monsters[which].m_add_exp;
	    }
	}
    }
}

/*
 * Draw a box around a room
 */

draw_room(rp)
struct room *rp;
{
    int j, k;

    move(rp->r_pos.y, rp->r_pos.x + 1);
    vert(rp->r_max.y - 2);  /* Draw left side */
    move(rp->r_pos.y + rp->r_max.y - 1, rp->r_pos.x);
    horiz(rp->r_max.x); /* Draw bottom */
    move(rp->r_pos.y, rp->r_pos.x);
    horiz(rp->r_max.x); /* Draw top */
    vert(rp->r_max.y - 2);  /* Draw right side */

    /*
     * Put the floor down
     */
    for (j = 1; j < rp->r_max.y - 1; j++) {
	move(rp->r_pos.y + j, rp->r_pos.x + 1);
	for (k = 1; k < rp->r_max.x - 1; k++)
	    addch(FLOOR);
    }
}

/*
 * horiz: draw a horizontal line
 */

horiz(cnt)
int cnt;
{
    while (cnt--)
	addch('-');
}

/*
 * vert: draw a vertical line
 */

vert(cnt)
int cnt;
{
    int x, y;

    getyx(stdscr, y, x);
    x--;
    while (cnt--) {
	move(++y, x);
	addch('|');
    }
}

/*
 * rnd_pos: pick a random spot in a room
 */

rnd_pos(rp, cp)
struct room *rp;
coord   *cp;
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
}
/*
 * Pick a room that is really there
 */

rnd_room()
{
    int rm;

    if (levtype != NORMLEV)
	rm = 0;
    else
	do {
	    rm = rnd(MAXROOMS);
	} while (rooms[rm].r_flags & ISGONE);
    return rm;
}
