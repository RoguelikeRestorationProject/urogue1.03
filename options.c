/*
    options.c  -  This file has all the code for the option command.
   
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

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include "rogue.h"

#define NUM_OPTS    (sizeof optlist / sizeof (OPTION))
#define EQSTR(a, b, c)  (strncmp(a, b, c) == 0)

/*
 * description of an option and what to do with it
 */
struct optstruct {
    char    *o_name;    /* option name */
    char    *o_prompt;  /* prompt for interactive entry */
    bool    *o_opt;     /* pointer to thing to set */
    int     (*o_putfunc) ();/* function to print value */
    int     (*o_getfunc) ();/* function to get value interactively */
};

typedef struct optstruct    OPTION;

int put_bool(), get_bool(), put_str(), get_str(), put_abil(), get_abil(),
put_inv(), get_inv();

OPTION  optlist[] = {
    {"terse", "Terse output (terse): ",
	&terse, put_bool, get_bool},
    {"flush", "Flush typeahead during battle (flush): ",
	&fight_flush, put_bool, get_bool},
    {"jump", "Show position only at end of run (jump): ",
	&jump, put_bool, get_bool},
    {"inven", "Style of inventories (inven): ",
	&inv_type, put_inv, get_inv},
    {"askme", "Ask me about unidentified things (askme): ",
	&askme, put_bool, get_bool},
    {"doorstop", "Stop running when adjacent (doorstop): ",
	&doorstop, put_bool, get_bool},
    {"name", "Name (name): ",
	whoami, put_str, get_str},
    {"fruit", "Fruit (fruit): ",
	fruit, put_str, get_str},
    {"file", "Save file (file): ",
	file_name, put_str, get_str},
    {"score", "Score file (score): ",
	score_file, put_str, get_str},
    {"class", "Character class (class): ",
	&char_type, put_abil, get_abil}
};

/*
 * print and then set options from the terminal
 */
option()
{
    OPTION  *op;
    int retval;

    wclear(hw);
    touchwin(hw);

    /*
     * Display current values of options
     */
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	waddstr(hw, op->o_prompt);
	(*op->o_putfunc) (op->o_opt, hw);
	waddch(hw, '\n');
    }

    /*
     * Set values
     */
    wmove(hw, 0, 0);
    for (op = optlist; op < &optlist[NUM_OPTS]; op++) {
	waddstr(hw, op->o_prompt);
	if ((retval = (*op->o_getfunc) (op->o_opt, hw)))
	    if (retval == QUIT)
		break;
	    else if (op > optlist) {    /* MINUS */
		wmove(hw, (op - optlist) - 1, 0);
		op -= 2;
	    }
	    else {  /* trying to back up beyond the top */
		putchar('\007');
		wmove(hw, 0, 0);
		op--;
	    }
    }

    /*
     * Switch back to original screen
     */
    mvwaddstr(hw, LINES - 1, 0, spacemsg);
    wrefresh(hw);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
 * put out a boolean
 */
put_bool(b, win)
bool    *b;
WINDOW  *win;
{
    waddstr(win, *b ? "True" : "False");
}

/*
 * put out a string
 */
put_str(str, win)
char    *str;
WINDOW  *win;
{
    waddstr(win, str);
}

/*
 * print the character type
 */
put_abil(ability, win)
bool    *ability;
WINDOW  *win;
{
    char    *abil;

    switch (*ability) {
	case C_FIGHTER:
	    abil = "Fighter";
	    break;
	case C_MAGICIAN:
	    abil = "Magic User";
	    break;
	case C_CLERIC:
	    abil = "Cleric";
	    break;
	case C_THIEF:
	    abil = "Thief";
	    break;
	case C_PALADIN:
	    abil = "Paladin";
	    break;
	case C_RANGER:
	    abil = "Ranger";
	    break;
	case C_ILLUSION:
	    abil = "Illusionist";
	    break;
	case C_ASSASIN:
	    abil = "Assasin";
	    break;
	case C_NINJA:
	    abil = "Ninja";
	    break;
	case C_DRUID:
	    abil = "Druid";
	    break;
	default:
	    abil = "(unknown)";
    }
    waddstr(win, abil);
}


/*
 * allow changing a boolean option and print it out
 */

get_bool(bp, win)
bool    *bp;
WINDOW  *win;
{
    int oy, ox;
    bool    op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *bp ? "True" : "False");
    while (op_bad) {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (readchar()) {
	case 't':
	case 'T':
	    *bp = TRUE;
	    op_bad = FALSE;
	    break;
	case 'f':
	case 'F':
	    *bp = FALSE;
	    op_bad = FALSE;
	    break;
	case '\n':
	case '\r':
	    op_bad = FALSE;
	    break;
	case '\033':
	case '\007':
	    return QUIT;
	case '-':
	    return MINUS;
	default:
	    mvwaddstr(win, oy, ox + 10, "(T or F)");
	}
    }
    wmove(win, oy, ox);
    wclrtoeol(win);
    waddstr(win, *bp ? "True" : "False");
    waddch(win, '\n');
    return NORM;
}

/*
 * set a string option
 */
get_str(opt, win)
char    *opt;
WINDOW  *win;
{
    char    *sp;
    int c, oy, ox;
    char    buf[2 * LINELEN];

    wrefresh(win);
    getyx(win, oy, ox);

    /*
     * loop reading in the string, and put it in a temporary buffer
     */
    for (sp = buf;
	 (c = readchar()) != '\n' &&
	 c != '\r' &&
	 c != '\033' &&
	 c != '\007' &&
	 sp < &buf[LINELEN - 1];
	 wclrtoeol(win), wrefresh(win)) {
	if (c == -1)
	    continue;
	else if (c == '\b') {
	    if (sp > buf) {
		int i;

		sp--;
		for (i = strlen(unctrl(*sp)); i; i--)
		    waddch(win, '\b');
	    }
	    continue;
	}
	else if (c == '\0') {
	    sp = buf;
	    wmove(win, oy, ox);
	    continue;
	}
	else if (sp == buf && c == '-' && win == hw)
	    break;
	*sp++ = c;
	waddstr(win, unctrl(c));
    }
    *sp = '\0';
    if (sp > buf)       /* only change option if something has been
		 * typed */
	strcpy(opt, buf, strlen(buf));
    wmove(win, oy, ox);
    waddstr(win, opt);
    waddch(win, '\n');
    wrefresh(win);
    if (win == cw)
	mpos += sp - buf;
    if (c == '-')
	return MINUS;
    else if (c == '\033' || c == '\007')
	return QUIT;
    else
	return NORM;
}

/*
 * The ability field is read-only
 */
get_abil(abil, win)
bool    *abil;
WINDOW  *win;
{
    int oy, ox, ny, nx;
    bool    op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    put_abil(abil, win);
    getyx(win, ny, nx);
    while (op_bad) {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (readchar()) {
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    default:
		mvwaddstr(win, ny, nx + 5,
		    "(no change allowed)");
	}
    }
    wmove(win, ny, nx + 5);
    wclrtoeol(win);
    wmove(win, ny, nx);
    waddch(win, '\n');
    return NORM;
}


/*
 * parse options from string, usually taken from the environment. the string
 * is a series of comma seperated values, with booleans being stated as
 * "name" (true) or "noname" (false), and strings being "name=....", with the
 * string being defined up to a comma or the end of the entire option string.
 */

parse_opts(str)
char    *str;
{
    char    *sp;
    OPTION  *op;
    int len;

    while (*str) {
	for (sp = str; isalpha(*sp); sp++)
	    continue;
	len = sp - str;

	/*
	 * Look it up and deal with it
	 */
	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
	    if (EQSTR(str, op->o_name, len)) {
		if (op->o_putfunc == put_bool)
		    *(bool *) op->o_opt = TRUE;
		else {  /* string option */
		    char    *start;
		    char    value[80];

		    /*
		     * Skip to start of string value
		     */
		    for (str = sp + 1; *str == '='; str++)
			continue;
		    start = (char *) value;

		    /*
		     * Skip to end of string value
		     */
		    for (sp = str + 1; *sp && *sp != ',';
			sp++)
			continue;
		    strcpy(start, str, sp - str);

		    /*
		     * Put the value into the option
		     * field
		     */
		    if (op->o_putfunc != put_abil &&
			op->o_putfunc != put_inv)
			strcpy(op->o_opt, value);

		    if (op->o_putfunc == put_inv) {
			int len = strlen(value);

			if (isupper(value[0]))
			    value[0] = tolower(value[0]);
			if (EQSTR(value, "overwrite",
			    len))
			    *op->o_opt = INV_OVER;
			if (EQSTR(value, "slow", len))
			    *op->o_opt = INV_SLOW;
			if (EQSTR(value, "clear", len))
			    *op->o_opt = INV_CLEAR;
		    }
		    else if (*op->o_opt == -1) {
			int len = strlen(value);

			if (isupper(value[0]))
			    value[0] = tolower(value[0]);
			if (EQSTR(value, "fighter",
			    len))
			    *op->o_opt = C_FIGHTER;
			else if (EQSTR(value, "magic",
			    min(len, 5)))
			    *op->o_opt = C_MAGICIAN;
			else if (EQSTR(value, "illus",
			    min(len, 5)))
			    *op->o_opt = C_ILLUSION;
			else if (EQSTR(value, "cleric",
			    len))
			    *op->o_opt = C_CLERIC;
			else if (EQSTR(value, "thief",
			    len))
			    *op->o_opt = C_THIEF;
			else if (EQSTR(value, "paladin",
			    len))
			    *op->o_opt = C_PALADIN;
			else if (EQSTR(value, "ranger",
			    len))
			    *op->o_opt = C_RANGER;
			else if (EQSTR(value, "assasin",
			    len))
			    *op->o_opt = C_ASSASIN;
			else if (EQSTR(value, "druid",
			    len))
			    *op->o_opt = C_DRUID;
			else if (EQSTR(value, "ninja",
			    len))
			    *op->o_opt = C_NINJA;
		    }
		}
		break;
	    }
	    else if (op->o_putfunc == put_bool
		 && EQSTR(str, "no", 2) &&
		EQSTR(str + 2, op->o_name, len - 2)) {
		*(bool *) op->o_opt = FALSE;
		break;
	    }

	/*
	 * skip to start of next option name
	 */
	while (*sp && !isalpha(*sp))
	    sp++;
	str = sp;
    }
}

/*
 * print the inventory type
 */
put_inv(inv, win)
bool    *inv;
WINDOW  *win;
{
    char    *style;

    switch (*inv) {
	case INV_OVER:
	    style = "Overwrite";
	    break;
	case INV_SLOW:
	    style = "Slow";
	    break;
	case INV_CLEAR:
	    style = "Clear Screen";
	    break;
	default:
	    style = "(unknown)";
    }
    waddstr(win, style);
}

/*
 * The inventory field.
 */
get_inv(inv, win)
bool    *inv;
WINDOW  *win;
{
    int oy, ox, ny, nx;
    bool    op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    put_inv(inv, win);
    getyx(win, ny, nx);
    while (op_bad) {
	wmove(win, oy, ox);
	wrefresh(win);
	switch (readchar()) {
	    case '\n':
	    case '\r':
		op_bad = FALSE;
		break;
	    case '\033':
	    case '\007':
		return QUIT;
	    case '-':
		return MINUS;
	    case 'O':
	    case 'o':
		*inv = INV_OVER;
		op_bad = FALSE;
		break;
	    case 'S':
	    case 's':
		*inv = INV_SLOW;
		op_bad = FALSE;
		break;
	    case 'C':
	    case 'c':
		*inv = INV_CLEAR;
		op_bad = FALSE;
		break;
	    default:
		mvwaddstr(win, ny, nx + 5, "(Use: o, s, or c)");
	}
    }
    wmove(win, oy, ox);
    wclrtoeol(win);
    switch (*inv) {
	case INV_SLOW:
	    waddstr(win, "Slow\n");
	    break;
	case INV_CLEAR:
	    waddstr(win, "Clear Screen\n");
	    break;
	case INV_OVER:
	    waddstr(win, "Overwrite\n");
	    break;
	default:
	    waddstr(win, "Unknown\n");
	    break;
    }
    return NORM;
}
