/*
    newlvl.c  -  Dig and draw a new level
   
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

new_level(ltype)
LEVTYPE ltype;          /* designates type of level to create */
{
    int rm, i, cnt;
    struct linked_list  *item, *nitem;
    struct thing    *tp;
    struct linked_list  *fpack;
    bool    going_down = TRUE;
    coord   stairs;

    /* Start player off right */
    turn_off(player, ISHELD);
    turn_off(player, ISFLEE);
    extinguish(suffocate);
    hold_count = 0;
    trap_tries = 0;
    no_food++;

    if (level >= max_level)
	max_level = level;
    else
	going_down = FALSE;

    /*
     * Free up the monsters on the last level
     */
    if (fam_ptr != NULL) {  /* save what familiar is carrying */
	fpack = (THINGPTR(fam_ptr))->t_pack;
	(THINGPTR(fam_ptr))->t_pack = NULL;
	fam_ptr = NULL; /* just in case */
    }
    for (i = 1; i <= mons_summoned; i++)
	extinguish(unsummon);
    mons_summoned = 0;

    for (item = mlist; item != NULL; item = nitem) {
	tp = THINGPTR(item);
	nitem = next(item);
	if (on(*tp, ISUNIQUE))  /* Put alive UNIQUE on next level */
	    monsters[tp->t_index].m_normal = TRUE;
	killed(NULL, item, NOMESSAGE, NOPOINTS);
    }

    free_list(lvl_obj); /* Free up previous objects (if any) */

    wclear(cw);
    wclear(mw);
    clear();
    levtype = ltype;
    switch (ltype) {
	when    THRONE:
	do_throne();    /* do monster throne stuff */
	when    MAZELEV:
	do_maze();
	when    POSTLEV:
	default:
	if (going_down && (level == 0 || (level % 15) == 0)) {
	    do_post();
	    level++;
	    levtype = ltype = NORMLEV;
	}
	do_rooms(); /* Draw rooms */

	do_passages();  /* Draw passages */
    }

    /*
     * Place the staircase down.
     */
    cnt = 0;
    do {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &stairs);
    } while (!(mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000));
    addch(STAIRS);

    put_things(ltype);  /* Place objects (if any) */

    if (has_artifact && level == 1)
	create_lucifer(&stairs);

    /*
     * Place the traps
     */
    ntraps = 0;     /* No traps yet */
    if (levtype == NORMLEV) {
	if (rnd(10) < level) {
	    char    ch;

	    i = ntraps = min(MAXTRAPS, rnd(level / 4) + 1);

	    /*
	     * maybe a lair
	     */
	    if (level > 50 && ltype == NORMLEV && rnd(wizard ? 3 :
		10) == 0) {
		cnt = 0;
		do {
		    rm = rnd_room();
		    if (rooms[rm].r_flags & ISTREAS)
			continue;
		    rnd_pos(&rooms[rm], &stairs);
		} until(mvinch(stairs.y, stairs.x) == FLOOR
		       || cnt++ > 5000);
		addch(LAIR);
		i--;
		traps[i].tr_flags = 0;
		traps[i].tr_type = LAIR;
		traps[i].tr_show = FLOOR;
		traps[i].tr_pos = stairs;
	    }
	    while (i--) {
		cnt = 0;
		do {
		    rm = rnd_room();
		    if (rooms[rm].r_flags & ISTREAS)
			continue;
		    rnd_pos(&rooms[rm], &stairs);
		} until(mvinch(stairs.y, stairs.x) == FLOOR
		    || cnt++ > 5000);

		traps[i].tr_flags = 0;
		switch (rnd(11)) {
		    when 0:
			ch = TRAPDOOR;
		    when 1:
			ch = BEARTRAP;
		    when 2:
			ch = SLEEPTRAP;
		    when 3:
			ch = ARROWTRAP;
		    when 4:
			ch = TELTRAP;
		    when 5:
			ch = DARTTRAP;
		    when 6:
			ch = POOL;
			if (rnd(10))
			    traps[i].tr_flags =
				ISFOUND;
		    when 7:
			ch = MAZETRAP;
		    when 8:
			ch = FIRETRAP;
		    when 9:
			ch = POISONTRAP;
		    when 10:
			ch = RUSTTRAP;
		}
		addch(ch);
		traps[i].tr_type = ch;
		traps[i].tr_show = FLOOR;
		traps[i].tr_pos = stairs;
	    }
	}
    }

    do {            /* Position hero */
	rm = rnd_room();
	if (levtype != THRONE && (rooms[rm].r_flags & ISTREAS))
	    continue;
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR &&
	   DISTANCE(hero.y, hero.x, stairs.y, stairs.x) > 16);
    oldrp = &rooms[rm]; /* Set the current room */
    player.t_oldpos = player.t_pos; /* Set the current position */

    if (levtype != POSTLEV && levtype != THRONE) {
	if (on(player, BLESSMAP) && rnd(5) == 0) {
	    read_scroll(&player, S_MAP, ISNORMAL);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMAP);
	}
	if (player.t_ctype == C_THIEF || on(player, BLESSGOLD) &&
	    rnd(5) == 0) {
	    read_scroll(&player, S_GFIND, ISNORMAL);
	    if (rnd(3) == 0)
		turn_off(player, BLESSGOLD);
	}

	if (player.t_ctype == C_RANGER || on(player, BLESSFOOD) &&
	    rnd(5) == 0) {
	    read_scroll(&player, S_FOODDET, ISNORMAL);
	    if (rnd(3) == 0)
		turn_off(player, BLESSFOOD);
	}
	if (player.t_ctype == C_MAGICIAN ||
	    player.t_ctype == C_ILLUSION ||
	    on(player, BLESSMAGIC) && rnd(5) == 0) {
	    quaff(&player, P_TREASDET, ISNORMAL);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMAGIC);
	}
	if (player.t_ctype == C_DRUID || on(player, BLESSMONS) &&
	    rnd(5) == 0) {
	    quaff(&player, P_MONSTDET, ISNORMAL);
	    if (rnd(3) == 0)
		turn_off(player, BLESSMONS);
	}
	else if (player.t_ctype == C_CLERIC ||
	    player.t_ctype == C_PALADIN || is_wearing(R_PIETY))
	    undead_sense();
    }

    if (is_wearing(R_AGGR))
	aggravate();

    if (is_wearing(R_ADORNMENT) ||
	cur_armor != NULL && cur_armor->o_which == MITHRIL) {
	bool    greed = FALSE;

	for (item = mlist; item != NULL; item = next(item)) {
	    tp = THINGPTR(item);
	    if (on(*tp, ISGREED)) {
		turn_on(*tp, ISRUN);
		turn_on(*tp, ISMEAN);
		tp->t_dest = hero;
		greed = TRUE;
	    }
	}
	if (greed)
	    msg("An uneasy feeling comes over you.");
    }

    if (is_carrying(TR_PALANTIR)) { /* Palantir shows all */
	msg("The Palantir reveals all!");
	overlay(stdscr, cw);    /* Wizard mode 'f' command */
	overlay(mw, cw);/* followed by 'm' command */
    }
    if (is_carrying(TR_PHIAL)) {    /* Phial lights your way */
	if (!is_carrying(TR_PALANTIR))
	    msg("The Phial banishes the darkness!");
	for (i = 0; i < MAXROOMS; i++)
	    rooms[i].r_flags &= ~ISDARK;
    }
    if (is_carrying(TR_AMULET)) {   /* Amulet describes the level */
	level_eval();
    }


    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
    light(&hero);

    /*
     * Summon familiar if player has one
     */
    if (on(player, HASFAMILIAR)) {
	summon_monster((short) fam_type, FAMILIAR, MESSAGE);
	if (fam_ptr != NULL) {  /* add old pack to new */
	    tp = THINGPTR(fam_ptr);
	    if (tp->t_pack == NULL)
		tp->t_pack = fpack;
	    else if (fpack != NULL) {
		for (item = tp->t_pack; item->l_next != NULL;
		    item = next(item))
		    ;
		item->l_next = fpack;
		fpack->l_prev = item;
	    }
	}
	else
	    free_list(fpack);
    }

    status(TRUE);
}

/*
 * put_things: put potions and scrolls on this level
 */

put_things(ltype)
LEVTYPE ltype;          /* designates type of level to create */
{
    int i, rm, cnt;
    struct linked_list  *item;
    struct object   *cur;
    bool    got_unique = FALSE;
    int length, width, maxobjects;
    coord   tp;

    /*
     * Once you have found an artifact, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (has_artifact && level < max_level && ltype != THRONE)
	return;

    /*
     * There is a chance that there is a treasure room on this level
     * Increasing chance after level 15
     */
    if (ltype != MAZELEV && rnd(80) < level - 15) {
	int i, j;
	struct room *rp;

	/* Count the number of free spaces */
	i = 0;      /* 0 tries */
	do {
	    rp = &rooms[rnd_room()];
	    width = rp->r_max.y - 2;
	    length = rp->r_max.x - 2;
	} until((width * length <= MAXTREAS) || (i++ > MAXROOMS * 4));

	/* Mark the room as a treasure room */
	rp->r_flags |= ISTREAS;

	/* Make all the doors secret doors */
	for (i = 0; i < rp->r_nexits; i++) {
	    move(rp->r_exit[i].y, rp->r_exit[i].x);
	    addch(SECRETDOOR);
	}

	/* Put in the monsters and treasures */
	for (j = 1; j < rp->r_max.y - 1; j++)
	    for (i = 1; i < rp->r_max.x - 1; i++) {
		coord   trp;

		trp.y = rp->r_pos.y + j;
		trp.x = rp->r_pos.x + i;

		/* Monsters */
		if ((rnd(100) < (MAXTREAS * 100) /
		    (width * length)) &&
		    (mvwinch(mw, rp->r_pos.y + j,
		    rp->r_pos.x + i) == ' ')) {
		    struct thing    *tp;

		    /* Make a monster */
		    item = new_item(sizeof *tp);
		    tp = THINGPTR(item);

		    /*
		     * Put it there and aggravate it
		     * (unless it can escape) only put
		     * one UNIQUE per treasure room at
		     * most
		     */
		    if (got_unique)
			new_monster(item,
			    randmonster(NOWANDER,
			    GRAB), &trp,
			    NOMAXSTATS);
		    else {
			new_monster(item,
			    randmonster(NOWANDER,
			    NOGRAB), &trp,
			    NOMAXSTATS);
			if (on(*tp, ISUNIQUE))
			    got_unique = TRUE;
		    }
		    turn_off(*tp, ISFRIENDLY);
		    turn_on(*tp, ISMEAN);
		    if (off(*tp, CANINWALL)) {
			tp->t_dest = hero;
			turn_on(*tp, ISRUN);
		    }
		}

		/* Treasures */
		if ((rnd(100) < (MAXTREAS * 100) /
		    (width * length)) &&
		    (mvinch(rp->r_pos.y + j,
		    rp->r_pos.x + i) == FLOOR)) {
		    item = new_thing();
		    cur = OBJPTR(item);
		    cur->o_pos = trp;
		    add_obj(item, trp.y, trp.x);
		}
	    }
    }

    /*
     * Do MAXOBJ attempts to put things on a level, maybe
     */
    maxobjects = (ltype == THRONE) ? rnd(3 * MAXOBJ) + 35 : MAXOBJ;
    for (i = 0; i < maxobjects; i++)
	if (rnd(100) < 40 || ltype == THRONE) {

	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing();
	    cur = OBJPTR(item);

	    /*
	     * Put it somewhere
	     */
	    cnt = 0;
	    do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &tp);
	    } until(winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	    cur->o_pos = tp;
	    add_obj(item, tp.y, tp.x);
	}

    /*
     * If he is really deep in the dungeon and he hasn't found an
     * artifact yet, put it somewhere on the ground
     */
    if (make_artifact()) {
	item = new_item(sizeof *cur);
	cur = OBJPTR(item);
	new_artifact(-1, cur);
	cnt = 0;
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &tp);
	} until(winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	cur->o_pos = tp;
	add_obj(item, tp.y, tp.x);
    }
}

int throne_monster = 0; /* identifies monster type in THRONE room */

/*
 * do_throne: Put a monster's throne room and monsters on the screen
 */
do_throne()
{
    coord   mp;
    int save_level;
    int i;
    struct room *rp;
    struct thing    *tp;
    struct linked_list  *item;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++) {
	rp->r_nexits = 0;   /* no exits */
	rp->r_flags = ISGONE;   /* kill all rooms */
    }
    rp = &rooms[0];     /* point to only room */
    rp->r_flags = 0;    /* this room NOT gone */
    rp->r_max.x = 40;
    rp->r_max.y = 10;   /* 10 * 40 room */
    rp->r_pos.x = (COLS - rp->r_max.x) / 2; /* center horizontal */
    rp->r_pos.y = 3;    /* 2nd line */
    draw_room(rp);      /* draw the only room */

    save_level = level;
    level = max(2 * level, level + roll(4, 6));

    if (throne_monster == 0)/* Who has he offended? */
	do {
	    throne_monster = nummonst - roll(1, NUMSUMMON);
	} until(monsters[throne_monster].m_normal);

    /* Create summoning monster */
    item = new_item(sizeof *tp);
    tp = THINGPTR(item);
    do {
	rnd_pos(rp, &mp);
    } until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);

    new_monster(item, throne_monster, &mp, MAXSTATS);
    turn_on(*tp, CANSEE);
    turn_off(*tp, ISFRIENDLY);

    if (on(*tp, CANSUMMON)) /* summon his helpers */
	summon_help(tp, FORCE);
    else {
	for (i = roll(4, 10); i >= 0; i--) {
	    item = new_item(sizeof *tp);
	    tp = THINGPTR(item);
	    do {
		rnd_pos(rp, &mp);
	    } until(mvwinch(stdscr, mp.y, mp.x) == FLOOR);
	    new_monster(item, randmonster(NOWANDER, NOGRAB), &mp,
		MAXSTATS);
	    turn_on(*tp, CANSEE);
	    turn_off(*tp, ISFRIENDLY);
	}
    }

    level = save_level + roll(2, 3);    /* send the hero down */
    aggravate();
    throne_monster = 0;
}

/*
 * create_lucifer - special surprise on the way back up create Lucifer
 * with more than the usual god abilities
 */
create_lucifer(stairs)
coord   *stairs;
{
    struct linked_list  *item = new_item(sizeof(struct thing));
    struct thing    *tp = THINGPTR(item);

    new_monster(item, nummonst + 1, stairs, MAXSTATS);
    turn_on(*tp, CANINWALL);
    turn_on(*tp, CANHUH);
    turn_on(*tp, CANBLINK);
    turn_on(*tp, CANSNORE);
    turn_on(*tp, CANDISEASE);
    turn_on(*tp, NOCOLD);
    turn_on(*tp, TOUCHFEAR);
    turn_on(*tp, BMAGICHIT);
    turn_on(*tp, NOFIRE);
    turn_on(*tp, NOBOLT);
    turn_on(*tp, CANBLIND);
    turn_on(*tp, CANINFEST);
    turn_on(*tp, CANSMELL);
    turn_on(*tp, CANPARALYZE);
    turn_on(*tp, CANSTINK);
    turn_on(*tp, CANCHILL);
    turn_on(*tp, CANFRIGHTEN);
    turn_on(*tp, CANHOLD);
    turn_on(*tp, CANBRANDOM);
}
