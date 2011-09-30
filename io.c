/*
    io.c  -  Various input/output functions
   
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
#include <varargs.h>
#include "rogue.h"
#include "stepok.h"

static char mbuf[2 * LINELEN];
static int  newpos = 0;

/*
 * msg: Display a message at the top of the screen.
 */

msg(fmt, args)
char    *fmt;
int args;
{

    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0') {
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	mpos = 0;
	return;
    }

    /*
     * otherwise add to the message and flush it out
     */
    doadd(fmt, &args);
    endmsg();
}

/*
 * add things to the current message
 */
addmsg(fmt, args)
char    *fmt;
int args;
{
    doadd(fmt,&args);
}

/*
 * Display a new msg (giving him a chance to see the previous one if it is up
 * there with the --More--)
 */
endmsg()
{
    strcpy(msgbuf[msg_index], mbuf);
    msg_index = ++msg_index % 10;
    if (mpos) {
	wmove(cw, 0, mpos);
	waddstr(cw, morestr);
	wrefresh(cw);
	wait_for(' ');
    }
    mvwaddstr(cw, 0, 0, mbuf);
    wclrtoeol(cw);
    mpos = newpos;
    newpos = 0;
	/* draw(cw); CHECK */

    wrefresh(cw); /* CHECK */
}

doadd(fmt, args)
char    *fmt;
int **args;
{
    vsprintf(&mbuf[newpos], fmt, args);
    newpos = strlen(mbuf);
}

/*
 * step_ok: returns true if it is ok for type to step on ch flgptr will be
 * NULL if we don't know what the monster is yet!
 */

step_ok(y, x, can_on_monst, flgptr)
int y, x, can_on_monst;
struct thing    *flgptr;
{
    struct linked_list  *item;
    char    ch;

    /* What is here?  Don't check monster window if MONSTOK is set */
    if (can_on_monst == MONSTOK)
	ch = mvinch(y, x);
    else
	ch = winat(y, x);

    switch (ch) {
	when ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    if (flgptr && on(*flgptr, CANINWALL))
		return (TRUE);
	    return FALSE;
	when    SCROLL:

	    /*
	     * If it is a scroll, it might be a scare monster scroll so
	     * we need to look it up to see what type it is.
	     */
	    if (flgptr && flgptr->t_ctype == C_MONSTER) {
		item = find_obj(y, x, TRUE);
		if (item != NULL && (OBJPTR(item))->o_type ==
		    SCROLL && (OBJPTR(item))->o_which ==
		    S_SCARE &&
		    rnd(flgptr->t_stats.s_intel) < 12)
		return (FALSE); /* All but smart ones are
			 * scared */
	    }
	    return (TRUE);
	otherwise:
	    return (!isalpha(ch));
    }
}

/*
 * shoot_ok: returns true if it is ok for type to shoot over ch
 */

shoot_ok(ch)
{
    switch (ch) {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    return FALSE;
	default:
	    return (!isalpha(ch));
    }
}

/*
 * readchar - get an ASCII character
 */

readchar()
{
    return (ttgetc() & 0x7f);
}

/*
 * status: Display the important stats line.  Keep the cursor where it was.
 */
status(display)
bool    display;            /* TRUE - force refresh */
{
    struct stats    *stat_ptr, *max_ptr;
    int oy, ox;
    static char buf[2 * LINELEN];

    stat_ptr = &pstats;
    max_ptr = &max_stats;

    getyx(cw, oy, ox);
    sprintf(buf, "Int:%d(%d) Str:%d(%d) Wis:%d(%d) Dex:%d(%d) Con:%d(%d) Carry:%d(%d)",
	stat_ptr->s_intel, max_ptr->s_intel,
	stat_ptr->s_str, max_ptr->s_str,
	stat_ptr->s_wisdom, max_ptr->s_wisdom,
	stat_ptr->s_dext, max_ptr->s_dext,
	stat_ptr->s_const, max_ptr->s_const,
	stat_ptr->s_pack / 10, stat_ptr->s_carry / 10
	);
    mvwaddstr(cw, LINES - 2, 0, buf);
    wclrtoeol(cw);

    sprintf(buf, "Lvl:%d Au:%d Hpt:%d(%d) Pow:%d(%d) Ac:%d Exp:%d+%ld %s",
	level,
	purse,
	stat_ptr->s_hpt, max_ptr->s_hpt,
	stat_ptr->s_power, max_ptr->s_power,
	(cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
	 : stat_ptr->s_arm) - ring_value(R_PROTECT),
	stat_ptr->s_lvl,
	stat_ptr->s_exp,
	cnames[player.t_ctype][min(stat_ptr->s_lvl - 1, 14)]);
    mvwaddstr(cw, LINES - 1, 0, buf);
    switch (hungry_state) {
	when    F_OK:;
	when    F_HUNGRY:
	    waddstr(cw, " Hungry");
	when    F_WEAK:
	    waddstr(cw, " Weak");
	when    F_FAINT:
	    waddstr(cw, " Fainting");
    }
    wclrtoeol(cw);
    wmove(cw, oy, ox);
    if (display)
	wrefresh(cw);
}

/*
 * wait_for Sit around until the guy types the right key
 */

wait_for(ch)
char    ch;
{
    char    c;

    if (ch == '\n')
	while ((c = readchar()) != '\n' && c != '\r')
	    continue;
    else
	while (readchar() != ch)
	    continue;
}

/*
 * show_win: function used to display a window and wait before returning
 */

show_win(scr, message)
WINDOW  *scr;
char    *message;
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    wrefresh(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * restscr: Restores the screen to the terminal
 */
restscr(scr)
WINDOW  *scr;
{
    clearok(scr, TRUE);
    touchwin(scr);
}

/*
 * add_line: Add a line to the list of discoveries
 */

add_line(fmt, arg)
char    *fmt, *arg;
{
    WINDOW  *tw;

    if (line_cnt == 0) {
	wclear(hw);
	if (inv_type == INV_SLOW)
	    mpos = 0;
    }
    if (inv_type == INV_SLOW) {
	if (*fmt != '\0')
	    msg(fmt, arg);
	line_cnt++;
    }
    else {
	/* chai: was if (line_cnt >= LINES - 1 || fmt == NULL) { */
	if (line_cnt >= LINES - 2 || fmt == NULL) {
	    if (fmt == NULL && !newpage && inv_type == INV_OVER) {
		tw = newwin(line_cnt + 2, COLS, 0, 0);
		overwrite(hw, tw);
		wstandout(tw);
		mvwaddstr(tw, line_cnt, 0, spacemsg);
		wstandend(tw);
		touchwin(tw);
		wrefresh(tw);
		wait_for(' ');
		delwin(tw);
		touchwin(cw);
	    }
	    else {
		wstandout(hw);
		mvwaddstr(hw, LINES - 1, 0, spacemsg);
		wstandend(hw);
		wrefresh(hw);
		wait_for(' ');
		clearok(curscr, TRUE);
		wclear(hw);
	    }
	    newpage = TRUE;
	    line_cnt = 0;
	}
	if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0')) {
	    mvwprintw(hw, line_cnt++, 0, fmt, arg);
	    lastfmt = fmt;
	    lastarg = arg;
	}
    }
}

/*
 * end_line: End the list of lines
 */
end_line()
{
    if (inv_type != INV_SLOW)
	if (line_cnt == 1 && !newpage) {
	    mpos = 0;
	    msg(lastfmt, lastarg);
	}
	else
	    add_line(NULL);
    line_cnt = 0;
    newpage = FALSE;
}


/*
 * hearmsg: Call msg() only if you are not deaf
 */
hearmsg(fmt, va_alist)
char    *fmt;
va_dcl
{
    if (off(player, ISDEAF))
	msg(fmt, va_alist);
    else if (wizard) {
	msg("Couldn't hear: ");
	msg(fmt, va_alist);
    }
}

/*
 * seemsg: Call msg() only if you are not blind
 */
seemsg(fmt, va_alist)
char    *fmt;
va_dcl
{
    if (off(player, ISBLIND))
	msg(fmt, va_alist);
    else if (wizard) {
	msg("Couldn't see: ");
	msg(fmt, va_alist);
    }
}
