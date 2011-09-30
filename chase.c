/*
    chase.c  -  Code for one object to chase another

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

#include <ctype.h>
#include "rogue.h"
#include "stepok.h"

#define MAXINT  32767
#define MININT  -32767

/* Movement penalties */
#define SHOTPENALTY 2       /* In line of sight of missile */
#define DOORPENALTY 1       /* Moving out of current room */

coord   ch_ret;         /* Where chasing takes you */
struct object   *pick_weap();   /* find monster's favorite weapon */

/*
 * runners: Make all the running monsters move. with monsters now fighting
 * each other, this routine have been enhanced and may need more work yet
 */

runners()
{
    struct linked_list  *item;
    struct thing    *tp;

    for (item = mlist; item != NULL; item = next_mons)
    {
	curr_mons = item;
	next_mons = next(curr_mons);
    tp = THINGPTR(item);

    if (on(*tp, ISHELD) && rnd(tp->t_stats.s_str +
	    tp->t_stats.s_lvl) > 10 + rnd(50))
	{
	    turn_off(*tp, ISHELD);
	    turn_off(*tp, ISDISGUISE);
	    turn_on(*tp, ISRUN);

	    tp->t_dest = hero;

	    if (tp->t_stats.s_hpt < rnd(tp->maxstats.s_hpt))
		turn_on(*tp, ISFLEE);

	    if (cansee(tp->t_pos.y, tp->t_pos.x))
		msg("The %s breaks free!", monsters[tp->t_index].m_name);
	}

    if (off(*tp, ISHELD) && on(*tp, ISRUN))
	{
	    bool flee = FALSE;
	    flee = on(*tp, ISFLEE) ||
	(SAME_POS(tp->t_dest,hero) && on(player, ISINWALL) &&
	 off(*tp, CANINWALL) && off(*tp, ISFAMILIAR));

	    if (off(*tp, ISSLOW) || tp->t_turn)
	    {
		doctor(tp);
		do_chase(tp, flee);
	    }

	    if (curr_mons && (on(*tp, ISHASTE) ||
		((on(*tp, CANFLY) || on(*tp, ISFAST)) &&
	    DISTANCE(hero.y, hero.x, tp->t_pos.y, tp->t_pos.x) >= 4)))
	    {
	doctor(tp);
	do_chase(tp, flee);
	}

	    if (curr_mons)
	    {
	tp->t_turn ^= TRUE;
	tp->t_wasshot ^= FALSE; /* Not shot anymore */
	    }

	}
    }
    curr_mons = next_mons = NULL;
}

/*
 * do_chase: Make one thing chase another.
 */

do_chase(th, flee)
struct thing    *th;
bool    flee;
{
    struct room *rer, *ree; /* room of chaser, room of
			 * chasee */
    struct room *old_room,      /* old room of monster */
	*new_room;      /* new room of monster */
    int mindist = MAXINT, maxdist = MININT, dist = MININT,
	    last_door = -1; /* Door we just came
			 * from */
    int i;
    bool    stoprun = FALSE,    /* TRUE means we are there */
	rundoor;        /* TRUE means run to a door */
    struct linked_list  *item;
    coord   this;           /* Temporary destination for chaser */
    bool    hit_bad,        /* TRUE means hit bad monster */
	mon_attack;     /* TRUE means find a monster to hit */
    char    sch;

    /* Make sure the monster can move */
    if (th->t_no_move != 0) {
	th->t_no_move--;
	return;
    }

    /*
     * Bad monsters check for a good monster to hit, friendly monsters
     * check for a bad monster to hit.
     */
    mon_attack = FALSE;
    if (good_monster(*th)) {
	hit_bad = TRUE;
	mon_attack = TRUE;
    }
    else if (on(*th, ISMEAN)) {
	hit_bad = FALSE;
	mon_attack = TRUE;
    }

    if (mon_attack) {
	struct linked_list  *mon_to_hit, *f_mons_a();

	if ((mon_to_hit = f_mons_a(th->t_pos.y, th->t_pos.x, hit_bad))) {
	    mon_mon_attack(th, mon_to_hit, pick_weap(th), NOTHROWN);
	    return;
	}
    }

    rer = roomin(&th->t_pos);   /* Find room of chaser */
    ree = roomin(&th->t_dest);  /* Find room of chasee */

    /*
     * We don't count doors as inside rooms for this routine
     */
    if (mvwinch(stdscr, th->t_pos.y, th->t_pos.x) == DOOR)
	rer = NULL;
    this = th->t_dest;

    /*
     * If we are not in a corridor and not a phasing monster, then if we
     * are running after the player, we run to a door if he is not in the
     * same room. If we are fleeing, we run to a door if he IS in the
     * same room.  Note:  We don't bother with doors in mazes.
     */
    if (levtype != MAZELEV && rer != NULL && off(*th, CANINWALL)) {
	if (flee)
	    rundoor = (rer == ree);
	else
	    rundoor = (rer != ree);
    }
    else
	rundoor = FALSE;

    if (rundoor) {
	coord   exit;   /* A particular door */
	int exity, exitx;   /* Door's coordinates */

	if (th->t_doorgoal != -1) { /* Do we already have the
			 * goal? */
	    this = rer->r_exit[th->t_doorgoal];
	    dist = 0;   /* Indicate that we have our door */
	}

	else
	    for (i = 0; i < rer->r_nexits; i++) {   /* Loop through doors */
		exit = rer->r_exit[i];
		exity = exit.y;
		exitx = exit.x;

		/* Avoid secret doors */
		if (mvwinch(stdscr, exity, exitx) == DOOR) {
		    /* Were we just on this door? */
		    if (ce(exit, th->t_oldpos))
			last_door = i;

		    else {
			dist = DISTANCE(th->t_dest.y, th->t_dest.x, exity, exitx);

			/*
			 * If fleeing, we want to
			 * maximize distance from
			 * door to what we flee, and
			 * minimize distance from
			 * door to us.
			 */
			if (flee)
			    dist -= DISTANCE(th->t_pos.y, th->t_pos.x, exity, exitx);

			/*
			 * Maximize distance if
			 * fleeing, otherwise
			 * minimize it
			 */
			if ((flee && (dist > maxdist)) ||
			    (!flee && (dist < mindist))) {
			    th->t_doorgoal = i; /* Use this door */
			    this = exit;
			    mindist = maxdist = dist;
			}
		    }
		}
	    }

	/* Could we not find a door? */
	if (dist == MININT) {
	    /* If we were on a door, go ahead and use it */
	    if (last_door != -1) {
		th->t_doorgoal = last_door;
		this = th->t_oldpos;
		dist = 0;   /* Indicate that we found a
			 * door */
	    }
	}

	/* Indicate that we do not want to flee from the door */
	if (dist != MININT)
	    flee = FALSE;
    }

    else
	th->t_doorgoal = -1;    /* Not going to any door */

    /*
     * this now contains what we want to run to this time so we run to
     * it.  If we hit it we either want to fight it or stop running
     */
    if (!chase(th, &this, flee)) {
	if (ce(ch_ret, hero)) {
	    /* merchants try to sell something */
	    if (on(*th, CANSELL)) {
		sell(th);
		return;
	    }
	    else if (off(*th, ISFRIENDLY) && off(*th, ISCHARMED)
	    && (off(*th, CANFLY) || (on(*th, CANFLY) && rnd(2))))
		attack(th, pick_weap(th), FALSE);
	    return;
	}
	else if (on(*th, NOMOVE))
	    stoprun = TRUE;
    }

    if (!curr_mons)
	return;     /* Did monster get itself killed? */

    if (on(*th, NOMOVE))
	return;

    /* If we have a scavenger, it can pick something up */
    if ((item = find_obj(ch_ret.y, ch_ret.x, TRUE)) != NULL) {
	struct object   *obj = OBJPTR(item);

	if (on(*th, ISSCAVENGE) ||
	    ((on(*th, CANWIELD) || on(*th, CANSHOOT))
	     && (obj->o_type == WEAPON || obj->o_type == ARMOR))
	    || (on(*th, CANCAST) && is_magic(obj))) {
	    rem_obj(item, FALSE);
	    attach(th->t_pack, item);
	}
    }

    mvwaddch(cw, th->t_pos.y, th->t_pos.x, th->t_oldch);
    sch = mvwinch(cw, ch_ret.y, ch_ret.x);

    /* Get old and new room of monster */
    old_room = roomin(&th->t_pos);
    new_room = roomin(&ch_ret);

    /* If the monster can illuminate rooms, check for a change */
    if (on(*th, HASFIRE)) {
	/* Is monster entering a room? */
	if (old_room != new_room && new_room != NULL) {
	    new_room->r_flags |= HASFIRE;
	    new_room->r_fires++;
	    if (cansee(ch_ret.y, ch_ret.x) && new_room->r_fires == 1)
		light(&hero);
	}

	/* Is monster leaving a room? */
	if (old_room != new_room && old_room != NULL) {
	    if (--(old_room->r_fires) <= 0) {
		old_room->r_flags &= ~HASFIRE;
		if (cansee(th->t_pos.y, th->t_pos.x))
		    light(&th->t_pos);
	    }
	}
    }

    /*
     * If monster is entering player's room and player can see it, stop
     * the player's running.
     */
    if (new_room != old_room && new_room != NULL &&
	new_room == ree && cansee(ch_ret.y, ch_ret.x) &&
	(off(*th, ISINVIS) || (off(*th, ISSHADOW) || rnd(10) == 0) ||
	 on(player, CANSEE)) && off(*th, CANSURPRISE))
	running = FALSE;

    if (rer != NULL && (rer->r_flags & ISDARK) &&
	!(rer->r_flags & HASFIRE) && sch == FLOOR &&
    DISTANCE(ch_ret.y, ch_ret.x, th->t_pos.y, th->t_pos.x) < see_dist &&
	off(player, ISBLIND))
	th->t_oldch = ' ';
    else
	th->t_oldch = sch;

    if (cansee(ch_ret.y, ch_ret.x) &&
	off(*th, ISINWALL) &&
	((off(*th, ISINVIS) && (off(*th, ISSHADOW) || rnd(100) < 10)) ||
	 on(player, CANSEE)) &&
	off(*th, CANSURPRISE))
	mvwaddch(cw, ch_ret.y, ch_ret.x, th->t_type);
    mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');
    mvwaddch(mw, ch_ret.y, ch_ret.x, th->t_type);

    /* Record monster's last position (if new one is different) */
    if (!ce(ch_ret, th->t_pos))
	th->t_oldpos = th->t_pos;
    th->t_pos = ch_ret; /* Mark the monster's new position */

    /* If the monster is on a trap, trap it */
    sch = mvinch(ch_ret.y, ch_ret.x);
    if (isatrap(sch)) {
	debug("Monster trapped by %c.", sch);
	if (cansee(ch_ret.y, ch_ret.x))
	    th->t_oldch = sch;
	be_trapped(th, &ch_ret);
    }


    /* And stop running if need be */
    if (stoprun && ce(th->t_pos, th->t_dest))
	turn_off(*th, ISRUN);
}

/*
 * runto: Set a monster running after something or stop it from running (for
 * when it dies)
 */

runto(runner, spot)
coord   *runner;
coord   *spot;
{
    struct linked_list  *item;
    struct thing    *tp;

    /*
     * If we couldn't find him, something is funny
     */
    if ((item = find_mons(runner->y, runner->x)) == NULL) {
	debug("CHASER '%s'", unctrl(winat(runner->y, runner->x)));
	return;
    }
    tp = THINGPTR(item);

    /*
     * Start the beastie running
     */
    tp->t_dest = *spot;
    turn_on(*tp, ISRUN);
    turn_off(*tp, ISDISGUISE);
}

/*
 * chase: Find the spot for the chaser(er) to move closer to the chasee(ee).
 * Returns TRUE if we want to keep on chasing later FALSE if we reach the
 * goal.
 */

chase(tp, ee, flee)
struct thing    *tp;
coord   *ee;
bool    flee;
{
    int x, y;
    int dist, thisdist, monst_dist = MAXINT;
    struct linked_list  *weapon;
    coord   *er = &tp->t_pos, *shoot_dir;
    char    ch, mch;
    bool    next_player = FALSE;

    /*
     * Take care of shooting directions
     */
    if (on(*tp, CANBREATHE) || on(*tp, CANSHOOT) || on(*tp, CANCAST))
	if (good_monster(*tp)) {
	    shoot_dir = find_shoot(tp); /* find a mean monster */
	    if (wizard && shoot_dir)
		msg("Found monster to attack towards (%d,%d).",
		    shoot_dir->x, shoot_dir->y);
	}
	else
	    shoot_dir = can_shoot(er, ee);  /* shoot hero */

    /*
     * If the thing is confused, let it move randomly. Some monsters are
     * slightly confused all of the time.
     */
    if ((on(*tp, ISHUH) && rnd(10) < 8) ||
	((on(*tp, ISINVIS) || on(*tp, ISSHADOW)) && rnd(100) < 20) ||
	(on(player, ISINVIS) && off(*tp, CANSEE))) {    /* Player is invisible */
	    /* get a valid random move */
	    ch_ret = *rndmove(tp);
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);

	if (on(*tp, ISHUH) && rnd(20) == 0) /* monster might lose
			     * confusion */
	    turn_off(*tp, ISHUH);

	/*
	 * check to see if random move takes creature away from
	 * player if it does then turn off ISHELD
	 */
	if (dist > 1 && on(*tp, DIDHOLD)) {
	    turn_off(*tp, DIDHOLD);
	    turn_on(*tp, CANHOLD);
	    if (--hold_count == 0)
		turn_off(player, ISHELD);
	}
    }

    /* If we can breathe, we may do so */
    else if (on(*tp, CANBREATHE) && (shoot_dir) && (rnd(100) < 67) &&
	 (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6)) &&
    (DISTANCE(er->y, er->x, ee->y, ee->x) < BOLT_LENGTH * BOLT_LENGTH)) {
	short   chance;
	char    *breath;

	/* Will it breathe at random */
	if (on(*tp, CANBRANDOM)) {
	    if (rnd(level / 20) == 0 && tp->t_index != nummonst + 1
		&& !(good_monster(*tp)))
		turn_off(*tp, CANBRANDOM);

	    /* Select type of breath */
	    chance = rnd(100);
	    if (chance < 11)
		breath = "acid";
	    else if (chance < 22)
		breath = "flame";
	    else if (chance < 33)
		breath = "lightning bolt";
	    else if (chance < 44)
		breath = "chlorine gas";
	    else if (chance < 55)
		breath = "ice";
	    else if (chance < 66)
		breath = "nerve gas";
	    else if (chance < 77)
		breath = "sleeping gas";
	    else if (chance < 88)
		breath = "slow gas";
	    else
		breath = "fear gas";
	}

	/* Or can it breathe acid? */
	else if (on(*tp, CANBACID)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBACID);
	    breath = "acid";
	}

	/* Or can it breathe fire */
	else if (on(*tp, CANBFIRE)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBFIRE);
	    breath = "flame";
	}

	/* Or can it breathe electricity? */
	else if (on(*tp, CANBBOLT)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBBOLT);
	    breath = "lightning bolt";
	}

	/* Or can it breathe gas? */
	else if (on(*tp, CANBGAS)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBGAS);
	    breath = "chlorine gas";
	}

	/* Or can it breathe ice? */
	else if (on(*tp, CANBICE)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBICE);
	    breath = "ice";
	}

	else if (on(*tp, CANBPGAS)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBPGAS);
	    breath = "nerve gas";
	}

	else if (on(*tp, CANBSGAS)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBSGAS);
	    breath = "sleeping gas";
	}

	else if (on(*tp, CANBSLGAS)) {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBSLGAS);
	    breath = "slow gas";
	}

	else {
	    if (rnd(level / 20) == 0)
		if (!(good_monster(*tp)))
		    turn_off(*tp, CANBFGAS);
	    breath = "fear gas";
	}

	shoot_bolt(tp, *er, *shoot_dir, (tp == THINGPTR(fam_ptr)),
	       tp->t_index, breath, roll(tp->t_stats.s_lvl, 6));

	ch_ret = *er;
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
	if (!curr_mons)
	    return (TRUE);
    }

    /*
     * If we can cast spells we might do so - even if adjacent fleeing
     * monsters are restricted to certain spells
     */
    else if (shoot_dir && on(*tp, CANCAST) &&
	 (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6))) {
	incant(tp, *shoot_dir);
	ch_ret = *er;
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
    }

    /*
     * Should we shoot or throw something? fleeing monsters may to shoot
     * anyway if far enough away
     */
    else if (shoot_dir && on(*tp, CANSHOOT) && (weapon = get_hurl(tp)) &&
	 (off(*tp, ISFLEE) || rnd(DISTANCE(er->y, er->x, ee->y, ee->x)) > 2) &&
	 (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6))) {
	missile(shoot_dir->y, shoot_dir->x, weapon, tp);
	ch_ret = *er;
	dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);
    }

    /*
     * Otherwise, find the empty spot next to the chaser that is closest
     * to the chasee.
     */
    else {
	int ey, ex;
	struct room *rer, *ree;
	int dist_to_old = MININT;   /* Dist from goal to old
			     * position */

	/* Get rooms */
	rer = roomin(er);
	ree = roomin(ee);

	/*
	 * This will eventually hold where we move to get closer If
	 * we can't find an empty spot, we stay where we are.
	 */
	dist = flee ? 0 : MAXINT;
	ch_ret = *er;

	/* Are we at our goal already? */
	if (!flee && ce(ch_ret, *ee))
	    return (FALSE);

	ey = er->y + 1;
	ex = er->x + 1;
	for (x = er->x - 1; x <= ex; x++)
	    for (y = er->y - 1; y <= ey; y++) {
		coord   tryp;

		/* Don't try off the board */
		if ((x < 0) || (x >= COLS) || (y < 1) || (y >= LINES - 2))
		    continue;

		/*
		 * Don't try the player if not going after
		 * the player
		 */
		/* or he's disguised and monster is dumb */
		if (((off(*tp, ISFLEE) && !ce(hero, *ee)) ||
		     (on(player, ISDISGUISE) && (rnd(tp->t_stats.s_lvl) < 6))
		     || good_monster(*tp))
		    && x == hero.x && y == hero.y)
		    continue;

		tryp.x = x;
		tryp.y = y;

		/*
		 * Is there a monster on this spot closer to
		 * our goal? Don't look in our spot or where
		 * we were.
		 */
		if (!ce(tryp, *er) && !ce(tryp, tp->t_oldpos) &&
		    isalpha(mch = mvwinch(mw, y, x))) {
		    int test_dist;

		    test_dist = DISTANCE(y, x, ee->y, ee->x);
		    if (test_dist <= 25 &&  /* Let's be fairly close */
			test_dist < monst_dist) {

			/*
			 * Could we really move
			 * there?
			 */
			mvwaddch(mw, y, x, ' ');    /* Temporarily blank
					 * monst */
			if (diag_ok(er, &tryp, tp))
			    monst_dist = test_dist;
			mvwaddch(mw, y, x, mch);    /* Restore monster */
		    }
		}

		if (!diag_ok(er, &tryp, tp))
		    continue;
		ch = mvwinch(cw, y, x); /* Screen character */
		if (on(*tp, ISFLEE) && (ch == PLAYER))
		    next_player = TRUE;

		/*
		 * Stepping on player is NOT okay if we are
		 * fleeing
		 */

		if (step_ok(y, x, NOMONST, tp) &&
		    (off(*tp, ISFLEE) || ch != PLAYER)) {

		    /*
		     * If it is a trap, an intelligent
		     * monster may not step on it (unless
		     * our hero is on top!)
		     */
		    if (isatrap(ch)) {
			if (!(ch == RUSTTRAP) &&
			    !(ch == FIRETRAP && on(*tp, NOFIRE)) &&
			    rnd(10) < tp->t_stats.s_intel &&
			(y != hero.y || x != hero.x))
			    continue;
		    }

		    /*
		     * OK -- this place counts
		     */
		    thisdist = DISTANCE(y, x, ee->y, ee->x);

		    /*
		     * Adjust distance if we are being
		     * shot at
		     */
		    if (tp->t_wasshot && tp->t_stats.s_intel > 5 &&
			ce(hero, *ee)) {
			/* Move out of line of sight */
			if (straight_shot(tryp.y, tryp.x, ee->y, ee->x, (coord *) NULL)) {
			    if (flee)
				thisdist -= SHOTPENALTY;
			    else
				thisdist += SHOTPENALTY;
			}

			/*
			 * But do we want to leave
			 * the room?
			 */
			else if (rer && rer == ree && ch == DOOR)
			    thisdist += DOORPENALTY;
		    }

		    /*
		     * Don't move to the last position if
		     * we can help it
		     */
		    if (ce(tryp, tp->t_oldpos))
			dist_to_old = thisdist;

		    else if ((flee && (thisdist > dist)) ||
			 (!flee && (thisdist < dist))) {
			ch_ret = tryp;
			dist = thisdist;
		    }
		}
	    }

	/*
	 * If we are running from the player and he is in our way, go
	 * ahead and slug him.
	 */
	if (next_player && DISTANCE(er->y, er->x, ee->y, ee->x) < dist &&
	    step_ok(tp->t_dest.y, tp->t_dest.x, NOMONST, tp)) {
	    ch_ret = tp->t_dest;    /* Okay to hit player */
	    return FALSE;
	}


	/*
	 * If we can't get closer to the player (if that's our goal)
	 * because other monsters are in the way, just stay put
	 */
	if (!flee && ce(hero, *ee) && monst_dist < MAXINT &&
	    DISTANCE(er->y, er->x, hero.y, hero.x) < dist)
	    ch_ret = *er;

	/* Do we want to go back to the last position? */
	else if (dist_to_old != MININT &&   /* It is possible to
			     * move back */
	     ((flee && dist == 0) ||    /* No other possible
			     * moves */
	      (!flee && dist == MAXINT))) {
	    /* Do we move back or just stay put (default)? */
	    dist = DISTANCE(er->y, er->x, ee->y, ee->x);    /* Current distance */
	    if (!flee || (flee && (dist_to_old > dist)))
		ch_ret = tp->t_oldpos;
	}
    }

    /* Make sure we have the real distance now */
    dist = DISTANCE(ch_ret.y, ch_ret.x, ee->y, ee->x);

    /* Mark monsters in a wall */
    switch (mvinch(ch_ret.y, ch_ret.x)) {
    case WALL:
    case '-':
    case '|':
	turn_on(*tp, ISINWALL);
    otherwise:
	turn_off(*tp, ISINWALL);
    }

    if (off(*tp, ISFLEE) &&
	(!SAME_POS(tp->t_dest,hero) || off(player, ISINWALL) || on(*tp, CANINWALL)))
	return (dist != 0);

    /* May actually hit here from a confused move */
    else
	return (!ce(ch_ret, hero));
}

/*
 * roomin: Find what room some coordinates are in. NULL means they aren't in
 * any room.
 */

struct room *
roomin(cp)
coord   *cp;
{
    struct room *rp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
	if (cp->x <= rp->r_pos.x + (rp->r_max.x - 1)
	    && rp->r_pos.x <= cp->x
	    && cp->y <= rp->r_pos.y + (rp->r_max.y - 1)
	    && rp->r_pos.y <= cp->y)
	    return rp;
    return NULL;
}

/*
 * find_mons: Find the monster from his corrdinates
 */

struct linked_list  *
find_mons(y, x)
int y;
int x;
{
    struct linked_list  *item;

    for (item = mlist; item != NULL; item = next(item)) {
	struct thing    *th = THINGPTR(item);

	if (th->t_pos.y == y && th->t_pos.x == x)
	    return item;
    }
    return NULL;
}

/*
 * Find an unfriendly monster around us to hit
 */
struct linked_list  *
f_mons_a(y, x, hit_bad)
int y, x;
bool    hit_bad;
{
    int row, col;
    struct linked_list  *item;
    struct thing    *tp;

    for (row = x - 1; row <= x + 1; row++)
	for (col = y - 1; col <= y + 1; col++)
	    if (row == x && col == y)
		continue;
	    else if (col > 0 && row > 0 &&
		isalpha(mvwinch(mw, col, row)) &&
		 ((item = find_mons(col, row)) != NULL)) {
		tp = THINGPTR(item);
		if ((good_monster(*tp) && !hit_bad) ||
		    (!good_monster(*tp) && hit_bad))
		    return (item);
	    }
    return (NULL);
}


/*
 * diag_ok: Check to see if the move is legal if it is diagonal
 */

diag_ok(sp, ep, flgptr)
coord   *sp, *ep;
struct thing    *flgptr;
{
    if (ep->x == sp->x || ep->y == sp->y)
	return TRUE;
    return (step_ok(ep->y, sp->x, MONSTOK, flgptr) &&
	step_ok(sp->y, ep->x, MONSTOK, flgptr));
}

/*
 * cansee: returns true if the hero can see a certain coordinate.
 */

cansee(y, x)
int y, x;
{
    struct room *rer;
    coord   tp;

    if (on(player, ISBLIND))
	return FALSE;
    tp.y = y;
    tp.x = x;
    rer = roomin(&tp);

    /*
     * We can only see if the hero in the same room as the coordinate and
     * the room is lit or if it is close.
     */
    return ((rer != NULL &&
	 rer == roomin(&hero) &&
	 (!(rer->r_flags & ISDARK) || (rer->r_flags & HASFIRE)) &&
	 (levtype != MAZELEV || /* Maze level needs direct line */
	  maze_view(tp.y, tp.x))) ||
	DISTANCE(y, x, hero.y, hero.x) < see_dist);
}

static coord    shoot_dir;

coord   *
find_shoot(tp)
struct thing    *tp;
{
    struct room *rtp;
    int ulx, uly, xmx, ymx, xmon, ymon, tpx, tpy, row, col;
    struct linked_list  *mon;
    struct thing    *ick;

    rtp = roomin(&tp->t_pos);   /* Find room of chaser */
    if (rtp == NULL)
	return NULL;
    ulx = rtp->r_pos.x;
    uly = rtp->r_pos.y;
    xmx = rtp->r_max.x;
    ymx = rtp->r_max.y;

    tpx = tp->t_pos.x;
    tpy = tp->t_pos.y;

    for (col = ulx; col < (ulx + xmx); col++)
	for (row = uly; row < (uly + ymx); row++) {
	    if (row > 0 && col > 0 && isalpha(mvwinch(mw, row, col))) {
		if ((mon = find_mons(row, col))) {
		    ick = THINGPTR(mon);
		    xmon = ick->t_pos.x;
		    ymon = ick->t_pos.y;
		    if (!(good_monster(*ick))) {
			if (straight_shot(tpy, tpx, ymon, xmon, &shoot_dir))
			    return (&shoot_dir);
		    }
		}
	    }
	}
    return (NULL);
}

/*
 * Can_shoot determines if the monster (er) has a direct line of shot at the
 * player (ee).  If so, it returns the direction in which to shoot.
 */

coord   *
can_shoot(er, ee)
coord   *er, *ee;
{
    int ery, erx, eey, eex;

    /* Make sure we are chasing the player */
    if (!ce((*ee), hero))
	return (NULL);

    /* They must be in the same room */
    if (roomin(er) != roomin(&hero))
	return (NULL);

    ery = er->y;
    erx = er->x;
    eey = ee->y;
    eex = ee->x;

    /* Will shoot unless next to player, then 80% prob will fight */
    if ((DISTANCE(ery, erx, eey, eex) < 4) && (rnd(100) < 80))
	return (NULL);

    /* Do we have a straight shot? */
    if (!straight_shot(ery, erx, eey, eex, &shoot_dir))
	return (NULL);
    else
	return (&shoot_dir);
}

/*
 * straight_shot: See if there is a straight line of sight between the two
 * given coordinates.  If shooting is not NULL, it is a pointer to a
 * structure which should be filled with the direction to shoot (if there is
 * a line of sight).  If shooting, monsters get in the way.  Otherwise, they
 * do not.
 */

bool
straight_shot(ery, erx, eey, eex, shooting)
int ery, erx, eey, eex;
coord   *shooting;
{
    int dy, dx; /* Deltas */
    char    ch;

    /* Does the monster have a straight shot at player */
    if ((ery != eey) && (erx != eex) &&
	(abs(ery - eey) != abs(erx - eex)))
	return (FALSE);

    /* Get the direction to shoot */
    if (eey > ery)
	dy = 1;
    else if (eey == ery)
	dy = 0;
    else
	dy = -1;

    if (eex > erx)
	dx = 1;
    else if (eex == erx)
	dx = 0;
    else
	dx = -1;

    /* Make sure we have free area all the way to the player */
    ery += dy;
    erx += dx;
    while ((ery != eey) || (erx != eex)) {
	switch (ch = winat(ery, erx)) {
	case '|':
	case '-':
	case WALL:
	case DOOR:
	case SECRETDOOR:
	    return (FALSE);
	default:
	    if (shooting && isalpha(ch))
		return (FALSE);
	}
	ery += dy;
	erx += dx;
    }

    if (shooting) {     /* If we are shooting -- put in the
		 * directions */
	shooting->y = dy;
	shooting->x = dx;
    }
    return (TRUE);
}

/*
 * get_hurl: returns the weapon that the monster will "throw" if it has one
 */

static struct linked_list   *arrow, *bolt, *rock, *silverarrow, *fbbolt;
static struct linked_list   *bullet, *firearrow, *dart, *dagger, *shuriken;
static struct linked_list   *oil, *grenade;

struct linked_list  *
get_hurl(tp)
struct thing    *tp;
{
    struct linked_list  *pitem;
    bool    bow = FALSE, crossbow = FALSE, sling = FALSE, footbow = FALSE;

    /* Don't point to anything to begin with */
    arrow = bolt = rock = silverarrow = fbbolt = NULL;
    bullet = firearrow = dart = dagger = shuriken = NULL;
    oil = grenade = NULL;

    for (pitem = tp->t_pack; pitem != NULL; pitem = next(pitem))
	if ((OBJPTR(pitem))->o_type == WEAPON)
	    switch ((OBJPTR(pitem))->o_which) {
		when    BOW:bow = TRUE;
		when    CROSSBOW:crossbow = TRUE;
		when    SLING:sling = TRUE;
		when    FOOTBOW:footbow = TRUE;
		when    ROCK:rock = pitem;
		when    ARROW:arrow = pitem;
		when    SILVERARROW:silverarrow = pitem;
		when    BOLT:bolt = pitem;
		when    FBBOLT:fbbolt = pitem;
		when    BULLET:bullet = pitem;
		when    FLAMEARROW:firearrow = pitem;
		when    DART:dart = pitem;
		when    DAGGER:dagger = pitem;
		when    SHIRIKEN:shuriken = pitem;
		when    MOLOTOV:oil = pitem;
		when    GRENADE:grenade = pitem;
	    }

    if (grenade)
	return (grenade);
    if (bow && silverarrow)
	return (silverarrow);
    if (crossbow && bolt)
	return (bolt);
    if (bow && firearrow)
	return (firearrow);
    if (oil)
	return (oil);
    if (footbow && fbbolt)
	return (fbbolt);
    if (bow && arrow)
	return (arrow);
    if (sling && bullet)
	return (bullet);
    if (sling && rock)
	return (rock);
    if (shuriken)
	return (shuriken);
    if (dagger)
	return (dagger);
    if (silverarrow)
	return (silverarrow);
    if (firearrow)
	return (firearrow);
    if (fbbolt)
	return (fbbolt);
    if (bolt)
	return (bolt);
    if (bullet)
	return (bullet);
    if (dart)
	return (dart);
    if (rock)
	return (rock);
    return (NULL);
}

/*
 * pick_weap: returns the biggest weapon that the monster will wield if it
 * has a non-launching or non-missile weapon returns NULL if no weapon, or
 * bare hands is better
 */

struct object   *
pick_weap(tp)
struct thing    *tp;
{
    int weap_dam = maxdamage(tp->t_stats.s_dmg);
    struct object   *ret_obj = NULL;
    struct linked_list  *pitem;

    if (on(*tp, CANWIELD)) {
	for (pitem = tp->t_pack; pitem != NULL; pitem = next(pitem)) {
	    struct object   *obj = OBJPTR(pitem);

	    if (obj->o_type != WEAPON && !(obj->o_flags & (ISLAUNCHER | ISMISL)) &&
		maxdamage(obj->o_damage) > weap_dam) {
		weap_dam = maxdamage(obj->o_damage);
		ret_obj = obj;
	    }
	}
    }
    return (ret_obj);
}

/*
 * Canblink checks if the monster can teleport (blink).  If so, it will try
 * to blink the monster next to the player.
 */

bool
can_blink(tp)
struct thing    *tp;
{
    short   y, x, index = 9;
    coord   tryp;       /* To hold the coordinates for use in diag_ok */
    bool    spots[9], found_one = FALSE;

    /*
     * First, can the monster even blink?  And if so, there is only a 50%
     * chance that it will do so.  And it won't blink if it is running.
     */
    if (off(*tp, CANBLINK) || (on(*tp, ISHELD)) ||
	on(*tp, ISFLEE) ||
	(on(*tp, ISSLOW) && off(*tp, ISHASTE) && !(tp->t_turn)) ||
	(rnd(12) < 6))
	return (FALSE);


    /* Initialize the spots as illegal */
    do {
	spots[--index] = FALSE;
    } while (index > 0);

    /* Find a suitable spot next to the player */
    for (y = hero.y - 1; y < hero.y + 2; y++)
	for (x = hero.x - 1; x < hero.x + 2; x++, index++) {

	    /*
	     * Make sure x coordinate is in range and that we are
	     * not at the player's position
	     */
	    if (x < 0 || x >= COLS || index == 4)
		continue;

	    /* Is it OK to move there? */
	    if (!step_ok(y, x, NOMONST, tp))
		spots[index] = FALSE;
	    else {

		/*
		 * OK, we can go here.  But don't go there if
		 * monster can't get at player from there
		 */
		tryp.y = y;
		tryp.x = x;
		if (diag_ok(&tryp, &hero, tp)) {
		    spots[index] = TRUE;
		    found_one = TRUE;
		}
	    }
	}

    /* If we found one, go to it */
    if (found_one) {
	/* Find a legal spot */
	while (spots[index = rnd(9)] == FALSE)
	    continue;

	/* Get the coordinates */
	y = hero.y + (index / 3) - 1;
	x = hero.x + (index % 3) - 1;

	/* Move the monster from the old space */
	mvwaddch(cw, tp->t_pos.y, tp->t_pos.x, tp->t_oldch);

	/* Move it to the new space */
	tp->t_oldch = mvwinch(cw, y, x);

	if (cansee(y, x) &&
	    off(*tp, ISINWALL) &&
	    ((off(*tp, ISINVIS) &&
	      (off(*tp, ISSHADOW) || rnd(100) < 10)) || on(player, CANSEE)) &&
	    off(*tp, CANSURPRISE))
	    mvwaddch(cw, y, x, tp->t_type);
	mvwaddch(mw, tp->t_pos.y, tp->t_pos.x, ' ');    /* Clear old position */
	mvwaddch(mw, y, x, tp->t_type);
	tp->t_pos.y = y;
	tp->t_pos.x = x;
    }

    return (found_one);
}
