/*
    misc.c  -  all sorts of miscellaneous routines
   
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

/*
 * tr_name: print the name of a trap
 */
static char ret_string[40];

char    *
tr_name(ch)
char    ch;
{
    char    *s, *prefix;

    ret_string[0] = '\0';
    switch (ch) {
	when TRAPDOOR:
	    s = "trapdoor.";
	when BEARTRAP:
	    s = "beartrap.";
	when SLEEPTRAP:
	    s = "sleeping gas trap.";
	when ARROWTRAP:
	    s = "arrow trap.";
	when TELTRAP:
	    s = "teleport trap.";
	when DARTTRAP:
	    s = "dart trap.";
	when POOL:
	    s = "shimmering pool.";
	when MAZETRAP:
	    s = "maze entrance.";
	when FIRETRAP:
	    s = "fire trap.";
	when POISONTRAP:
	    s = "poison pool trap.";
	when LAIR:
	    s = "monster lair.";
	when RUSTTRAP:
	    s = "rust trap.";
	otherwise:
	    ;
    }
    prefix = (terse ? "A " : "You found a ");
    strcpy(ret_string, prefix);
    strcat(ret_string, s);
    return (ret_string);
}

/*
 * Look: A quick glance all around the player
 */

look(wakeup)
bool    wakeup;
{
    int x, y;
    char    ch, och;
    int oldx, oldy;
    bool    inpass, horiz, vert, do_light = FALSE, do_blank = FALSE;
    int passcount = 0;
    struct room *rp;
    int ey, ex;

    /* Are we moving vertically or horizontally? */
    if (runch == 'h' || runch == 'l')
	horiz = TRUE;
    else
	horiz = FALSE;
    if (runch == 'j' || runch == 'k')
	vert = TRUE;
    else
	vert = FALSE;

    getyx(cw, oldy, oldx);  /* Save current position */

    /*
     * Blank out the floor around our last position and check for moving
     * out of a corridor in a maze.
     */
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) &&
	!(oldrp->r_flags & HASFIRE) && off(player, ISBLIND))
	do_blank = TRUE;
    for (x = player.t_oldpos.x - 1; x <= player.t_oldpos.x + 1; x++)
	for (y = player.t_oldpos.y - 1; y <= player.t_oldpos.y + 1;
		y++) {
	    ch = show(y, x);
	    if (do_blank && (y != hero.y || x != hero.x) &&
		ch == FLOOR)
		mvwaddch(cw, y, x, ' ');

	    /* Moving out of a corridor? */
	    if (levtype == MAZELEV &&
		(ch != '|' && ch != '-') && /* Not a wall */
		((vert && x != player.t_oldpos.x &&
		y == player.t_oldpos.y) ||
		 (horiz && y != player.t_oldpos.y &&
		x == player.t_oldpos.x)))
		do_light = TRUE;    /* Just came to a turn */
	}

    inpass = ((rp = roomin(&hero)) == NULL);    /* Are we in a passage? */

    /* Are we coming out of a wall into a corridor in a maze? */
    och = show(player.t_oldpos.y, player.t_oldpos.x);
    ch = show(hero.y, hero.x);
    if (levtype == MAZELEV && (och == '|' || och == '-' ||
	och == SECRETDOOR) &&
	(ch != '|' && ch != '-' && ch != SECRETDOOR))
	do_light = off(player, ISBLIND);    /* Light it up if not
			     * blind */

    /* Look around the player */
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS)
	    for (y = hero.y - 1; y <= ey; y++) {
		if (y <= 0 || y >= LINES - 2)
		    continue;
		if (isalpha(mvwinch(mw, y, x))) {
		    struct linked_list  *it;
		    struct thing    *tp;

		    if (wakeup)
			it = wake_monster(y, x);
		    else
			it = find_mons(y, x);
		    if (it == NULL)
			continue;
		    tp = THINGPTR(it);
		    tp->t_oldch = mvinch(y, x);
		    if (isatrap(tp->t_oldch)) {
			struct trap *trp =
			    trap_at(y, x);

			tp->t_oldch = (trp->tr_flags &
			    ISFOUND) ? tp->t_oldch
			    : trp->tr_show;
		    }
		    if (tp->t_oldch == FLOOR &&
			(rp->r_flags & ISDARK)
			&& !(rp->r_flags & HASFIRE) &&
			off(player, ISBLIND))
			tp->t_oldch = ' ';
		}

		/*
		 * Secret doors show as walls
		 */
		if ((ch = show(y, x)) == SECRETDOOR)
		    ch = secretdoor(y, x);

		/*
		 * Don't show room walls if he is in a
		 * passage and check for maze turns
		 */
		if (off(player, ISBLIND)) {
		    if (y == hero.y && x == hero.x
			|| (inpass && (ch == '-' ||
			ch == '|')))
			continue;

		    /* Are we at a crossroads in a maze? */
		    if (levtype == MAZELEV &&
			(ch != '|' && ch != '-') &&
			/* Not a wall */
			((vert && x != hero.x &&
			y == hero.y) ||
			 (horiz && y != hero.y &&
			x == hero.x)))
			do_light = TRUE;
			/* Just came to a turn */
		}
		else if (y != hero.y || x != hero.x)
		    continue;

		wmove(cw, y, x);
		waddch(cw, ch);
		if (door_stop && !firstmove && running) {
		    switch (runch) {
			when 'h':
			    if (x == ex)
				continue;
			when 'j':
			    if (y == hero.y - 1)
				continue;
			when 'k':
			    if (y == ey)
				continue;
			when 'l':
			    if (x == hero.x - 1)
				continue;
			when 'y':
			    if ((x + y) -
				(hero.x +
				hero.y) >= 1)
				continue;
			when 'u':
			    if ((y - x) -
				(hero.y -
				hero.x) >= 1)
				continue;
			when 'n':
			    if ((x + y) -
				(hero.x +
				hero.y) <= -1)
				continue;
			when 'b':
			    if ((y - x) -
				(hero.y -
				hero.x) <= -1)
			    continue;
		    }
		    switch (ch) {
			case DOOR:
			    if (x == hero.x ||
				y == hero.y)
				running = FALSE;
			    break;
			case PASSAGE:
			    if (x == hero.x ||
				y == hero.y)
				passcount++;
			    break;
			case FLOOR:

			/*
			 * Stop by new passages in a
			 * maze (floor next to us)
			 */
			    if ((levtype ==
				MAZELEV) &&
				((horiz &&
				x == hero.x &&
				y != hero.y) ||
				(vert &&
				y == hero.y &&
				x != hero.x)))
				running = FALSE;
			case '|':
			case '-':
			case ' ':
			    break;
			default:
			    running = FALSE;
			    break;
		    }
		}
	    }
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;

    /*
     * Do we have to light up the area (just stepped into a new
     * corridor)?
     */
    if (do_light && wakeup &&   /* wakeup will be true on a normal
		     * move */
	!(rp->r_flags & ISDARK) &&  /* We have some light */
	!ce(hero, player.t_oldpos)) /* Don't do anything if we didn't
		     * move */
	light(&hero);

    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    if (wakeup) {
	player.t_oldpos = hero; /* Don't change if we didn't move */
	oldrp = rp;
    }
}

/*
 * secret_door: Figure out what a secret door looks like.
 */
static coord    cp;

secretdoor(y, x)
int y, x;
{
    struct room *rp;

    cp.y = y;
    cp.x = x;
    if ((rp = roomin(&cp)) != NULL) {
	if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
	    return ('-');
	else
	    return ('|');
    }
    return ('p');
}

/*
 * find_obj: find the unclaimed object at y, x
 */

static struct linked_list   *next_ll;

struct linked_list  *
find_obj(y, x, start)
int y;
int x, start;
{
    struct linked_list  *obj, *sobj;
    struct object   *op;

    if (start)
	sobj = lvl_obj;
    else
	sobj = next_ll;

    for (obj = sobj; obj != NULL; obj = next(obj)) {
	op = OBJPTR(obj);
	if (op && op->o_pos.y == y && op->o_pos.x == x) {
	    next_ll = next(obj);
	    if (!next_ll)
		next_ll = lvl_obj;
	    return obj;
	}
    }
    next_ll = lvl_obj;
    return NULL;
}

/*
 * eat: He wants to eat something, so let him try
 */

eat()
{
    struct object   *obj;
    int amount;

    object  *get_object();

    if ((obj = get_object(pack, "eat", FOOD, NULL)) == NULL)
	return;
    if (--obj->o_count == 0)
	discard_pack(obj);
    switch (obj->o_which) {
	when    FD_RATION:
	    amount = HUNGERTIME + rnd(400) - 200;
	    if (rnd(100) > 70) {
		msg("Yuk, this food tastes awful.");
		pstats.s_exp++;
		check_level();
	    }
	    else
		msg("Yum, that tasted good.");
	when    FD_FRUIT:
	    amount = 200 + rnd(HUNGERTIME);
	    msg("My, that was a yummy %s.", fruit);
	when    FD_CRAM:
	    amount = rnd(HUNGERTIME / 2) + 600;
	    msg("The cram tastes dry in your mouth.");
	when    FD_CAKES:
	    amount = (HUNGERTIME / 3) + rnd(600);
	    msg("Yum, the honey cakes tasted good.");
	when    FD_LEMBA:
	    amount = (HUNGERTIME / 2) + rnd(900);
	    quaff(&player, P_HEALING, ISNORMAL);
	when    FD_MIRUVOR:
	    amount = (HUNGERTIME / 3) + rnd(500);
	    quaff(&player, P_HEALING, ISNORMAL);
	    quaff(&player, P_RESTORE, ISNORMAL);
	otherwise:
	    msg("What a strange thing to eat!");
	    amount = HUNGERTIME;
    }
    food_left += amount;
    if (obj->o_flags & ISBLESSED) {
	food_left += 2 * amount;
	msg("You have a tingling feeling in your mouth.");
    }
    else if (food_left > STOMACHSIZE) {
	food_left = STOMACHSIZE;
	msg("You feel satiated and too full to move.");
	no_command = HOLDTIME;
    }
    hungry_state = F_OK;
    updpack();
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * Used to modify the player's strength it keeps track of the highest it has
 * been, just in case
 */

chg_str(amt, both, lost)
int amt;
bool    both, lost;
{
    int ring_str;   /* ring strengths */
    struct stats    *ptr;   /* for speed */

    ptr = &pstats;
    ring_str = ring_value(R_ADDSTR) + (on(player, POWERSTR) ? 10 : 0) +
	(on(player, SUPERHERO) ? 10 : 0);
    ptr->s_str -= ring_str;
    ptr->s_str += amt;

    if (ptr->s_str < 3) {
	ptr->s_str = 3;
	lost = FALSE;
    }
    else if (ptr->s_str > 25)
	ptr->s_str = 25;

    if (both)
	max_stats.s_str = ptr->s_str;
    if (lost)
	lost_str -= amt;

    ptr->s_str += ring_str;
    if (ptr->s_str < 0)
	ptr->s_str = 0;

    updpack();
}

/*
 * Used to modify the player's dexterity it keeps track of the highest it has
 * been, just in case
 */

chg_dext(amt, both, lost)
int amt;
bool    both, lost;
{
    int ring_dext;  /* ring strengths */
    struct stats    *ptr;   /* for speed */

    ptr = &pstats;
    ring_dext = ring_value(R_ADDHIT) + (on(player, POWERDEXT) ? 10 : 0) +
	(on(player, SUPERHERO) ? 5 : 0);
    ptr->s_dext -= ring_dext;
    ptr->s_dext += amt;

    if (ptr->s_dext < 3) {
	ptr->s_dext = 3;
	lost = FALSE;
    }
    else if (ptr->s_dext > 25)
	ptr->s_dext = 25;

    if (both)
	max_stats.s_dext = ptr->s_dext;
    if (lost)
	lost_dext -= amt;

    ptr->s_dext += ring_dext;
    if (ptr->s_dext < 0)
	ptr->s_dext = 0;
}

/*
 * add_haste: add a haste to the player
 */

add_haste(blessed)
bool    blessed;
{
    short   hasttime;

    if (blessed)
	hasttime = 10;
    else
	hasttime = 6;

    if (on(player, ISSLOW)) {   /* Is person slow? */
	extinguish(noslow);
	noslow();

	if (blessed)
	    hasttime = 4;
	else
	    return;
    }

    if (on(player, ISHASTE)) {
	msg("You faint from exhaustion.");
	no_command += rnd(hasttime);
	lengthen(nohaste, rnd(hasttime) + (roll(1, 4) * hasttime));
    }
    else {
	turn_on(player, ISHASTE);
	fuse(nohaste, 0, roll(hasttime, hasttime), AFTER);
    }
}

/*
 * aggravate: aggravate all the monsters on this level
 */

aggravate()
{
    struct linked_list  *mi;
    struct thing    *tp;

    for (mi = mlist; mi != NULL; mi = next(mi)) {
	tp = THINGPTR(mi);
	runto(&tp->t_pos, &hero);
	turn_off(*tp, ISFRIENDLY);
    }
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char    *
vowelstr(str)
char    *str;
{
    switch (*str) {
    case 'a':
    case 'A':
    case 'e':
    case 'E':
    case 'i':
    case 'I':
    case 'o':
    case 'O':
    case 'u':
    case 'U':
	return "n";
    default:
	return "";
    }
}

/*
 * see if the object is one of the currently used items
 */
is_current(obj)
struct object   *obj;
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon ||
	obj == cur_ring[LEFT_1] || obj == cur_ring[LEFT_2] ||
	obj == cur_ring[LEFT_3] || obj == cur_ring[LEFT_4] ||
	obj == cur_ring[LEFT_5] ||
	obj == cur_ring[RIGHT_1] || obj == cur_ring[RIGHT_2] ||
	obj == cur_ring[RIGHT_3] || obj == cur_ring[RIGHT_4] ||
	obj == cur_ring[RIGHT_5]) {
	msg(terse ? "In use." : "That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
get_dir()
{
    char    *prompt;
    bool    gotit;

    if (terse)
	prompt = "Direction? ";
    else {
	prompt = "Which direction? ";
	msg(prompt);
    }

    do {
	gotit = TRUE;
	switch (readchar()) {
	    when 'h': case 'H':
		delta.y = 0;
		delta.x = -1;
	    when 'j': case 'J':
		delta.y = 1;
		delta.x = 0;
	    when 'k': case 'K':
		delta.y = -1;
		delta.x = 0;
	    when 'l': case 'L':
		delta.y = 0;
		delta.x = 1;
	    when 'y': case 'Y':
		delta.y = -1;
		delta.x = -1;
	    when 'u': case 'U':
		delta.y = -1;
		delta.x = 1;
	    when 'b': case 'B':
		delta.y = 1;
		delta.x = -1;
	    when 'n': case 'N':
		delta.y = 1;
		delta.x = 1;
	    when    ESCAPE:
		return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until(gotit);
    if (on(player, ISHUH) && rnd(100) > 80)
	do {
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}

/*
 * is_wearing: is the hero wearing a particular ring
 */

bool
is_wearing(type)
int type;
{
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)

    return (ISRING(LEFT_1, type) || ISRING(LEFT_2, type) ||
	ISRING(LEFT_3, type) || ISRING(LEFT_4, type) ||
	ISRING(LEFT_5, type) ||
	ISRING(RIGHT_1, type) || ISRING(RIGHT_2, type) ||
	ISRING(RIGHT_3, type) || ISRING(RIGHT_4, type) ||
	ISRING(RIGHT_5, type));
}

/*
 * Maze_view: Returns true if the player can see the specified location
 * within the confines of a maze (within one column or row)
 */

bool
maze_view(y, x)
int y, x;
{
    int start, goal, delta, ycheck, xcheck, absy, absx;
    bool    row;

    /* Get the absolute value of y and x differences */
    absy = hero.y - y;
    absx = hero.x - x;
    if (absy < 0)
	absy = -absy;
    if (absx < 0)
	absx = -absx;

    /* Must be within one row or column */
    if (absy > 1 && absx > 1)
	return (FALSE);

    if (absy <= 1) {    /* Go along row */
	start = hero.x;
	goal = x;
	row = TRUE;
	ycheck = hero.y;
    }
    else {          /* Go along column */
	start = hero.y;
	goal = y;
	row = FALSE;
	xcheck = hero.x;
    }
    if (start <= goal)
	delta = 1;
    else
	delta = -1;

    while (start != goal) {
	if (row)
	    xcheck = start;
	else
	    ycheck = start;
	switch (winat(ycheck, xcheck)) {
	    case '|':
	    case '-':
	    case WALL:
	    case DOOR:
	    case SECRETDOOR:
		return (FALSE);
	    default:
		break;
	}
	start += delta;
    }
    return (TRUE);
}

/*
 * listen: listen for monsters less than 5 units away
 */
listen()
{
    struct linked_list  *item;
    struct thing    *tp;
    int thief_bonus = -50;
    int mcount = 0;

    if (player.t_ctype == C_THIEF)
	thief_bonus = 10;

    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	if (DISTANCE(hero.y, hero.x, tp->t_pos.y, tp->t_pos.x) < 81
	    && rnd(70) < (thief_bonus + 4 * pstats.s_dext +
	    6 * pstats.s_lvl)) {
	    msg("You hear a%s %s nearby.",
		vowelstr(monsters[tp->t_index].m_name),
		monsters[tp->t_index].m_name);
	    mcount++;
	}
    }
    if (mcount == 0)
	msg("You hear nothing.");
}

/*
 * nothing_message - print out "Nothing <adverb> happens."
 */
static char *nothings[] = {
    "",
    "unusual ",
    "seems to ",
    "at all ",
    "really ",
    "noticeable ",
    "different ",
    "strange ",
    "wierd ",
    "bizzare ",
    "wonky ",
    ""
};

nothing_message(flags)
int flags;
{
    int adverb = rnd(sizeof(nothings) / sizeof(char *));
    bool    blessed = flags & ISBLESSED;
    bool    cursed = flags & ISCURSED;

    msg("Nothing %s%shappens.", (blessed ? "good " :
	(cursed ? "bad " : "")), nothings[adverb]);
}

/*
 * feel_message - print out "You feel <description>."
 */
feel_message()
{
    char    *charp;

    switch (rnd(25)) {
	when 1:
	    charp = "bad";
	when 2:
	    charp = "hurt";
	when 3:
	    charp = "sick";
	when 4:
	    charp = "faint";
	when 5:
	    charp = "yucky";
	when 6:
	    charp = "wonky";
	when 7:
	    charp = "wierd";
	when 8:
	    charp = "queasy";
	when 9:
	    charp = "wounded";
	when 11:
	    charp = "unusual";
	when 12:
	    charp = "no pain";
	when 13:
	    charp = "strange";
	when 14:
	    charp = "noticable";
	when 15:
	    charp = "bizzare";
	when 16:
	    charp = "distressed";
	when 17:
	    charp = "different";
	when 18:
	    charp = "a touch of ague";
	when 19:
	    charp = "a migrane coming on";
	when 20:
	    charp = "Excedrin headache #666";
	when 21:
	    charp = "a disturbance in the force";
	when 22:
	    charp = "like someone dropped a house on you";
	when 23:
	    charp = "as if every nerve in your body is on fire";
	when 24:
	    charp = "like thousands of red-hot army ants are crawling under your skin";
	otherwise:
	    charp = "ill";
    }
    msg("You feel %s.", charp);
}

/*
 * const_bonus -    Hit point adjustment for changing levels
 */
const_bonus()
{
    int ret_val = -2;

    if (pstats.s_const > 12)
	ret_val = pstats.s_const - 12;
    else if (pstats.s_const > 6)
	ret_val = 0;
    else if (pstats.s_const > 3)
	ret_val = -1;

    return (ret_val);
}

/*
 * int_wis_bonus -  Spell point adjustment for changing levels
 */
int_wis_bonus()
{
    int ret_val = -2;
    int casters_stat;

    switch (player.t_ctype) {
	when    C_PALADIN:
	case C_CLERIC:
	    casters_stat = pstats.s_wisdom;
	when    C_RANGER:
	case C_DRUID:
	    casters_stat = pstats.s_wisdom;
	when    C_MAGICIAN:
	    casters_stat = pstats.s_intel;
	when    C_ILLUSION:
	    casters_stat = pstats.s_intel;
	otherwise:
	    if (is_wearing(R_WIZARD))
		casters_stat = pstats.s_intel;
	    else if (is_wearing(R_PIETY))
		casters_stat = pstats.s_wisdom;
	    else
		casters_stat = (rnd(2) ? pstats.s_wisdom :
		    pstats.s_intel);
    }

    if (casters_stat > 12)
	ret_val = casters_stat - 12;
    else if (casters_stat > 6)
	ret_val = 0;
    else if (casters_stat > 3)
	ret_val = -1;

    return (ret_val);
}

electrificate()
{
    int affect_dist = 4 + player.t_stats.s_lvl / 4;
    struct linked_list  *item, *nitem;

    for (item = mlist; item != NULL; item = nitem) {
	struct thing    *tp = THINGPTR(item);
	char    *mname = monsters[tp->t_index].m_name;

	nitem = next(item);
	if (DISTANCE(tp->t_pos.y, tp->t_pos.x, hero.y, hero.x) <
	    affect_dist) {
	    int damage = roll(2, player.t_stats.s_lvl);

	    debug("Charge does %d (%d)", damage,
		tp->t_stats.s_hpt - damage);
	    if (on(*tp, NOBOLT))
		continue;
	    if ((tp->t_stats.s_hpt -= damage) <= 0) {
		msg("The %s is killed by an electric shock.",
		    mname);
		killed(&player, item, NOMESSAGE, POINTS);
		continue;
	    }
	    if (rnd(tp->t_stats.s_intel / 5) == 0) {
		turn_on(*tp, ISFLEE);
		msg("The %s is shocked by electricity.", mname);
	    }
	    else
		msg("The %s is zapped by your electricity.",
		    mname);

	    summon_help(tp, NOFORCE);
	    turn_off(*tp, ISFRIENDLY);
	    turn_off(*tp, ISCHARMED);
	    turn_on(*tp, ISRUN);
	    turn_off(*tp, ISDISGUISE);
	    runto(&tp->t_pos, &hero);
	    fighting = after = running = FALSE;
	}
    }
}

/*
 * feed_me -    Print out interesting messages about food consumption
 */
static char *f_hungry[] = {
    "want a cookie",
    "feel like a snack",
    "feel like some fruit",
    "start having the munchies",
    "are starting to get hungry"
};
static char *f_weak[] = {
    "are really hungry",
    "could eat a horse",
    "want some food - now",
    "are starting to feel weak",
    "feel a gnawing in your stomach",
    "are even willing to eat up cram",
    "feel lightheaded from not eating"
};
static char *f_faint[] = {
    "get dizzy from not eating",
    "are starving for nutrients",
    "feel too weak from lack of food",
    "see a mirage of an incredible banquet",
    "have incredible cramps in your stomach"
};
static char *f_plop[] = {
    "faint",
    "pass out",
    "keel over",
    "black out"
};

feed_me(hungry_state)
{
    char    *charp, *charp2;

    switch (hungry_state) {
	when    F_OK:
	default:
	    debug("feed_me(%d) called.", hungry_state);
	when    F_HUNGRY:
	    charp = f_hungry[rnd(sizeof(f_hungry) /
		sizeof(char *))];
	when    F_WEAK:
	    charp = f_weak[rnd(sizeof(f_weak) / sizeof(char *))];
	when    F_FAINT:
	    charp = f_faint[rnd(sizeof(f_faint) / sizeof(char *))];
	    charp2 = f_plop[rnd(sizeof(f_plop) / sizeof(char *))];
    }

    msg("You %s.", charp);
    if (hungry_state == F_FAINT)
	msg("You %s.", charp2);
}


/*
 * get_monster_number - prompt player for a monster on list returns 0 if none
 * selected
 */
get_monster_number(message)
char    *message;
{
    int i;
    int pres_monst = 1;
    int ret_val = -1;
    char    buf[2 * LINELEN];
    char    monst_name[2 * LINELEN];

    while (ret_val == -1) {
	msg("Which monster do you wish to %s? (* for list)", message);
	if ((get_str(buf, cw)) != NORM)
	    return;

	if ((i = atoi(buf)) != 0)
	    ret_val = i;

	else if (buf[0] != '*') {
	    for (i = 1; i < nummonst; i++)
		if ((strcmp(monsters[i].m_name, buf) == 0))
		    ret_val = i;
	}

	/* The following hack should be redone by the windowing code */
	else
	    while (pres_monst < nummonst) { /* Print out the
			     * monsters */
		int num_lines = LINES - 3;

		wclear(hw);
		touchwin(hw);

		wmove(hw, 2, 0);
		for (i = 0; i < num_lines && pres_monst <
			nummonst; i++) {
		    sprintf(monst_name, "[%d] %s\n",
			pres_monst,
			monsters[pres_monst].m_name);
		    waddstr(hw, monst_name);
		    pres_monst++;
		}

		if (pres_monst < nummonst) {
		    mvwaddstr(hw, LINES - 1, 0, morestr);
		    wrefresh(hw);
		    wait_for(' ');
		}

		else {
		    mvwaddstr(hw, 0, 0, "Which monster");
		    waddstr(hw, "? ");
		    wrefresh(hw);
		}
	    }

get_monst:
	get_str(monst_name, hw);
	ret_val = atoi(monst_name);
	if ((ret_val < 1 || ret_val > nummonst - 1)) {
	    mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
	    wrefresh(hw);
	    goto get_monst;
	}

	/* Set up for redraw */
	clearok(cw, TRUE);
	touchwin(cw);
    }

    return (ret_val);
}
#if 0
#define FLAGSHIFT   28L
#define FLAGINDEX   0x0000000fL
#define FLAGMASK    0x0fffffffL
/*
 * on - check if a monster flag is on
 */
on(th, flag)
struct thing    th;
long    flag;
{
    return (th.t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag << 4));
}

/*
 * off - check if a monster flag is off
 */
off(th, flag)
struct thing    th;
long    flag;
{
    return !(th.t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag << 4));
}

/*
 * turn_on - turn on a monster flag
 */
turn_on(th, flag)
struct thing    th;
long    flag;
{
    th.t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag << 4);
}

/*
 * turn_off - turn off a monster flag
 */
turn_off(th, flag)
struct thing    th;
long    flag;
{
    th.t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~(flag << 4);
}
#endif
