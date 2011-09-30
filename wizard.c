/*
    wizard.c  -  Special wizard commands
   
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
 * Special wizard commands (some of which are also non-wizard commands under
 * strange circumstances)
 */

#include <ctype.h>
#include "rogue.h"

/*
 * whatis: What a certain object is
 */

int
whatis(struct linked_list *what)
{
    struct object *obj;
    int kludge;
    int print_message = FALSE;

    if (what == NULL)
    {
	print_message = TRUE;

	while ((what = get_item("identify", 0)) == NULL)
	    ;
    }

    obj = OBJPTR(what);
    obj->o_flags |= ISKNOW;

    switch (obj->o_type)
    {
	case SCROLL:
	    kludge = TYP_SCROLL;
	    break;
	case POTION:
	    kludge = TYP_POTION;
	when STICK:
	    kludge = TYP_STICK;
	when RING:
	    kludge = TYP_RING;
	when WEAPON:
	case ARMOR:
	default:
	    kludge = -1;
    }
    if (kludge != -1) {
	know_items[kludge][obj->o_which] = TRUE;
	if (guess_items[kludge][obj->o_which]) {
	    free(guess_items[kludge][obj->o_which]);
	    guess_items[kludge][obj->o_which] = NULL;
	}
    }
    if (print_message)
	msg(inv_name(obj, UPPERCASE));
}

/*
 * teleport: Bamf the hero someplace else
 */

teleport()
{
    struct room *new_rp, *old_rp = roomin(&hero);
    int rm, which;
    coord   c;
    bool    is_lit = FALSE; /* For saving room light state */
    bool    rand_position = TRUE;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    if (is_wearing(R_TELCONTROL)) {
	msg("Where do you wish to teleport to? (* for help)");
	wmove(cw, hero.y, hero.x);
	wrefresh(cw);
	which = (short) (getchar() & 0177);
	while (which != (short) ESCAPE && which != (short) LINEFEED
	    && which != (short) CARRIAGE_RETURN) {
	    switch (which) {
		when 'h':   c.x--;
		when 'j':   c.y++;
		when 'k':   c.y--;
		when 'l':   c.x++;
		when 'y':   c.x--;
			c.y--;
		when 'u':   c.x++;
			c.y--;
		when 'b':   c.x--;
			c.y++;
		when 'n':   c.x++;
			c.y++;
		when '*':
		    msg("Use h,j,k,l,y,u,b,n to position cursor, then hit return.");
	    }
	    c.y = max(c.y, 1);
	    c.y = min(c.y, LINES - 3);
	    c.x = max(c.x, 1);
	    c.x = min(c.x, COLS - 1);
	    wmove(cw, c.y, c.x);
	    wrefresh(cw);
	    which = (short) (getchar() & 0177);
	}
	which = winat(c.y, c.x);
	if ((which == FLOOR || which == PASSAGE || which == DOOR) &&
	    ((ring_value(R_TELCONTROL) == 0 && rnd(10) < 6)
	     || (ring_value(R_TELCONTROL) > 0 && rnd(10) < 9))) {
	    rand_position = FALSE;
	    msg("You attempt succeeds.");
	    hero = c;
	    new_rp = roomin(&hero);
	}
	else
	    msg("Your attempt fails.");
    }
    if (rand_position) {
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &hero);
	} while (winat(hero.y, hero.x) != FLOOR);
	new_rp = &rooms[rm];
    }

    /* If hero gets moved, darken old room */
    if (old_rp && old_rp != new_rp) {
	if (!(old_rp->r_flags & ISDARK))
	    is_lit = TRUE;
	old_rp->r_flags |= ISDARK;  /* Fake darkness */
	light(&c);
	if (is_lit)
	    old_rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);

    /*
     * turn off ISHELD in case teleportation was done while fighting
     */
    if (on(player, ISHELD)) {
	struct linked_list  *ip, *nip;
	struct thing    *mp;

	turn_off(player, ISHELD);
	hold_count = 0;
	for (ip = mlist; ip; ip = nip) {
	    mp = THINGPTR(ip);
	    nip = next(ip);
	    if (on(*mp, DIDHOLD)) {
		turn_off(*mp, DIDHOLD);
		turn_on(*mp, CANHOLD);
	    }
	    turn_off(*mp, DIDSUFFOCATE);
	}
    }

    extinguish(suffocate);
    player.t_no_move = 0;   /* not trapped anymore */

    extinguish(suffocate);
    player.t_no_move = 0;   /* not trapped anymore */

    count = 0;
    running = FALSE;
    flushout();
    return rm;
}

/*
 * passwd: see if user knows password
 */

passwd()
{
    char    *sp, c;
    char    buf[2 * LINELEN], *crypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = (getchar() & 0177)) != '\n' && c != '\r' && c != '\033')
	if (c == '\0')
	    sp = buf;
	else if (c == '\b' && sp > buf)
	    sp--;
	else
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return (strcmp("pswd", crypt(buf, 123)) == 0);
}
