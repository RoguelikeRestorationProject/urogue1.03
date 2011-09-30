/*
    maze.c  -
   
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

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

struct cell {
	char	y_pos;
	char	x_pos;
};
struct bordercells {
	char	num_pos;    /* number of frontier cells next to you */
	struct cell conn[4];/* the y,x position of above cell */
}	border;

char    *frontier, *bits;
char    *moffset(), *foffset();
int lines, cols;

/*
 * domaze: Draw the maze on this level.
 */
do_maze()
{
	int i, least;
	struct room *rp;
	struct linked_list  *item;
	struct object	*obj;
	struct thing	*mp;
	bool	treas;
	coord	tp;

	for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
	    rp->r_nexits = 0;	/* no exits */
	    rp->r_flags = ISGONE;   /* kill all rooms */
	    rp->r_fires = 0;/* no fires */
	}
	rp = &rooms[0];     /* point to only room */
	rp->r_flags = ISDARK;	/* mazes always dark */
	rp->r_pos.x = 0;    /* room fills whole screen */
	rp->r_pos.y = 1;
	rp->r_max.x = COLS - 1;
	rp->r_max.y = LINES - 3;
	draw_maze();	    /* put maze into window */

	/*
	 * add some gold to make it worth looking for
	 */
	item = spec_item(GOLD, NULL, NULL, NULL);
	obj = OBJPTR(item);
	obj->o_count *= (rnd(10) + 1);	/* add in one large hunk */
	rnd_pos(rp, &tp);
	obj->o_pos = tp;
	add_obj(item, tp.y, tp.x);

	/*
	 * add in some food to make sure he has enough
	 */
	item = spec_item(FOOD, NULL, NULL, NULL);
	obj = OBJPTR(item);
	rnd_pos(rp, &tp);
	obj->o_pos = tp;
	add_obj(item, tp.y, tp.x);

	if (rnd(100) < 5) { /* 5% for treasure maze level */
	    treas = TRUE;
	    least = 20;
	    debug("Treasure maze.");
	}
	else {		/* normal maze level */
	    least = 1;
	    treas = FALSE;
	}
	for (i = 0; i < level + least; i++) {
	    if (!treas && rnd(100) < 50)    /* put in some little buggers */
		continue;

	    /* Put the monster in */
	    item = new_item(sizeof *mp);
	    mp = THINGPTR(item);
	    do {
		rnd_pos(rp, &tp);
	    } until(mvwinch(stdscr, tp.y, tp.x) == FLOOR);

	    new_monster(item, randmonster(NOWANDER, NOGRAB), &tp,
		NOMAXSTATS);

	    /* See if we want to give it a treasure to carry around. */
	    if (rnd(100) < monsters[mp->t_index].m_carry)
		attach(mp->t_pack, new_thing());

	    /* If it carries gold, give it some */
	    if (on(*mp, CARRYGOLD)) {
		item = spec_item(GOLD, NULL, NULL, NULL);
		obj = OBJPTR(item);
		obj->o_count = GOLDCALC + GOLDCALC + GOLDCALC;
		obj->o_pos = mp->t_pos;
		attach(mp->t_pack, item);
	    }

	}
}

/*
 * draw_maze: Generate and draw the maze on the screen
 */
draw_maze()
{
	int i, j, more;
	char	*ptr;

	lines = (LINES - 3) / 2;
	cols = (COLS - 1) / 2;
	bits = malloc((unsigned int) ((LINES - 3) * (COLS - 1)));
	frontier = malloc((unsigned int) (lines * cols));
	ptr = frontier;
	while (ptr < (frontier + (lines * cols)))
	    *ptr++ = TRUE;
	for (i = 0; i < LINES - 3; i++) {
	    for (j = 0; j < COLS - 1; j++) {
		if (i % 2 == 1 && j % 2 == 1)
		    *moffset(i, j) = FALSE; /* floor */
		else
		    *moffset(i, j) = TRUE;  /* wall */
	    }
	}
	for (i = 0; i < lines; i++) {
	    for (j = 0; j < cols; j++) {
		do
		    more = findcells(i, j);
		while (more != 0);
	    }
	}
	crankout();
	free((char *) frontier);
	free((char *) bits);
}

/*
 * moffset: Calculate memory address for bits
 */
char    *
moffset(y, x)
int y, x;
{

	return (bits + (y * (COLS - 1)) + x);
}

/*
 * foffset: Calculate memory address for frontier
 */
char    *
foffset(y, x)
int y, x;
{

	return (frontier + (y * cols) + x);
}

/*
 * findcells: Figure out cells to open up
 */
findcells(y, x)
int x, y;
{
	int rtpos, i;

	*foffset(y, x) = FALSE;
	border.num_pos = 0;
	if (y < lines - 1) {	/* look below */
	    if (*foffset(y + 1, x)) {
		border.conn[border.num_pos].y_pos = y + 1;
		border.conn[border.num_pos].x_pos = x;
		border.num_pos += 1;
	    }
	}
	if (y > 0) {	    /* look above */
	    if (*foffset(y - 1, x)) {
		border.conn[border.num_pos].y_pos = y - 1;
		border.conn[border.num_pos].x_pos = x;
		border.num_pos += 1;

	    }
	}
	if (x < cols - 1) { /* look right */
	    if (*foffset(y, x + 1)) {
		border.conn[border.num_pos].y_pos = y;
		border.conn[border.num_pos].x_pos = x + 1;
		border.num_pos += 1;
	    }
	}
	if (x > 0) {	    /* look left */
	    if (*foffset(y, x - 1)) {
		border.conn[border.num_pos].y_pos = y;
		border.conn[border.num_pos].x_pos = x - 1;
		border.num_pos += 1;

	    }
	}
	if (border.num_pos == 0)/* no neighbors available */
	    return 0;
	else {
	    i = rnd(border.num_pos);
	    rtpos = border.num_pos - 1;
	    rmwall(border.conn[i].y_pos, border.conn[i].x_pos, y, x);
	    return rtpos;
	}
}

/*
 * rmwall: Removes appropriate walls from the maze
 */
rmwall(newy, newx, oldy, oldx)
int newy, newx, oldy, oldx;
{
	int xdif, ydif;

	xdif = newx - oldx;
	ydif = newy - oldy;

	*moffset((oldy * 2) + ydif + 1, (oldx * 2) + xdif + 1) = FALSE;
	findcells(newy, newx);
}


/*
 * crankout: Does actual drawing of maze to window
 */
crankout()
{
	int x, y;

	for (y = 0; y < LINES - 3; y++) {
	    move(y + 1, 0);
	    for (x = 0; x < COLS - 1; x++) {
		if (*moffset(y, x)) {	/* here is a wall */
		    if (y == 0 || y == LINES - 4)
			    /* top or bottom line */
			addch('-');
		    else if (x == 0 || x == COLS - 2)
			    /* left | right side */
			addch('|');
		    else if (y % 2 == 0 && x % 2 == 0) {
			if (*moffset(y, x - 1) ||
			    *moffset(y, x + 1))
			    addch('-');
			else
			    addch('|');
		    }
		    else if (y % 2 == 0)
			addch('-');
		    else
			addch('|');
		}
		else
		    addch(FLOOR);
	    }
	}
}
