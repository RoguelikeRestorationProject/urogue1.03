/*
    command.c  -  Read and execute the user commands

    Last Modified: Dec 28, 1990

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
    
    See the file LICENSE.TXT for full copyright and licensing information.*/
*/

#include <ctype.h>
#include <signal.h>
#include "rogue.h"
#include "score.h"
#include "death.h"

/*
 * command: Process the user commands
 */

command()
{
    static char fight_ch;
    char    ch;
    int ntimes = 1; /* Number of player moves */
    static char countch, newcount;
    static bool an_after;
    static bool summoned;
    static coord    dta;

    an_after = FALSE;
    newcount = FALSE;
    if (on(player, CANFLY) && rnd(2))
	ntimes++;
    if (on(player, ISHASTE))
	ntimes++;
    if (fighting && att_bonus())
	ntimes *= 2;
    if (on(player, ISSLOW)) {
	if (player.t_turn != TRUE)
	    ntimes--;
	player.t_turn ^= TRUE;
    }

    /*
     * Let the daemons start up
     *
     * NOTE: summoned is not initialized deliberately so that i don't have
     * to make it static external and initialize it in funny ways. this
     * means that it is possible to have a bad value for summoned the
     * very first time that command is entered.
     */
    if (!summoned) {
	do_daemons(BEFORE);
	do_fuses(BEFORE);
    }
    summoned = FALSE;

    while (ntimes--) {
	moving = FALSE;
	/* If player is infested, take off a hit point */
	if (on(player, HASINFEST) && !is_wearing(R_HEALTH)) {
	    if ((pstats.s_hpt -= infest_dam) <= 0) {
		death(D_INFESTATION);
		return;
	    }
	}

		look(after);
	if (!running)
	    door_stop = FALSE;
	status(FALSE);
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump))
	    wrefresh(cw);   /* Draw screen */
	take = 0;
	after = TRUE;

	/*
	 * Read command or continue run
	 */
	if (!no_command) {
	    if (fighting) {
		ch = fight_ch;
	    }
	    else if (running) {

		/*
		 * If in a corridor, if we are at a turn with
		 * only one way to go, turn that way.
		 */
		if ((winat(hero.y, hero.x) == PASSAGE) && off(player, ISHUH) &&
		    (off(player, ISBLIND)))
		    switch (runch) {
			when 'h': corr_move(0, -1);
			when 'j': corr_move(1, 0);
			when 'k': corr_move(-1, 0);
			when 'l': corr_move(0, 1);
		    }
		ch = runch;
	    }
	    else if (count)
		ch = countch;
	    else {
		ch = readchar();
		if (mpos != 0 && !running)
		    msg("");    /* Erase message if its
			     * there */
	    }
	}
	else {
	    ch = '.';
	    fighting = moving = FALSE;
	}
	if (no_command) {
	    if (--no_command == 0)
		msg("You can move again.");
	}
	else {

	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch)) {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch)) {
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;

		/*
		 * Preserve count for commands which can be
		 * repeated.
		 */
		switch (ch) {
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'y':
		case 'u':
		case 'b':
		case 'n':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'Y':
		case 'U':
		case 'B':
		case 'N':
		case 'q':
		case 'r':
		case 's':
		case 'm':
		case 't':
		case 'C':
		case 'I':
		case '.':
		case 'z':
		case 'p':
		    break;
		default:
		    count = 0;
		}
	    }

	    /* Save current direction */
	    if (!running)   /* If running, it is already saved */
		switch (ch) {
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'y':
		case 'u':
		case 'b':
		case 'n':
		    runch = ch;
		    break;
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'Y':
		case 'U':
		case 'B':
		case 'N':
		    runch = tolower(ch);
		    break;
		}

	    newcount = FALSE;

	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch) {

		/*
		 * Movement and combat commands
		 */
		when 'h': do_move(0, -1);
		when 'j': do_move(1, 0);
		when 'k': do_move(-1, 0);
		when 'l': do_move(0, 1);
		when 'y': do_move(-1, -1);
		when 'u': do_move(-1, 1);
		when 'b': do_move(1, -1);
		when 'n': do_move(1, 1);
		when 'H': do_run('h');
		when 'J': do_run('j');
		when 'K': do_run('k');
		when 'L': do_run('l');
		when 'Y': do_run('y');
		when 'U': do_run('u');
		when 'B': do_run('b');
		when 'N': do_run('n');
		when 'm':
		    moving = TRUE;
		    if (!get_dir()) {
			after = FALSE;
			break;
		    }
		    do_move(delta.y, delta.x);
		when 'F':
		case 'f':
		    if (!fighting) {
			fight_ch = ch;
			if (get_dir()) {
			    dta.y = delta.y;
			    dta.x = delta.x;
			    beast = NULL;
			}
			else {
			    after = FALSE;
			    break;
			}
		    }
		    do_fight(dta.y, dta.x, (ch == 'F') ? TRUE : FALSE);
		when 't':
		    if (get_dir())
			missile(delta.y, delta.x, get_item("throw", NULL), &player);
		    else
			after = FALSE;

		    /*
		     * Informational commands - Do not do
		     * after daemons
		     */
		when ' ': after = FALSE;    /* do nothing */
		when 'Q':
		    after = FALSE;
		    quit();
		when 'i':
		    after = FALSE;
		    inventory(pack, '*');
		when 'I':
		    after = FALSE;
		    inventory(pack, 0);
		when '~':
		    after = FALSE;
		    next_exp_level(MESSAGE);
		when '>':
		    after = FALSE;
		    d_level();
		when '<':
		    after = FALSE;
		    u_level();
		when '?':
		    after = FALSE;
		    help();
		when '/':
		    after = FALSE;
		    identify();
		when 'v':
		    after = FALSE;
		    msg("UltraRogue Version %s.", release);
		when 'o':
		    after = FALSE;
		    option();
		    strcpy(fd_data[1].mi_name, fruit);
		when 17:    /* ctrl-r */
		    after = FALSE;
		    clearok(curscr, TRUE);
		when 15:{   /* ctrl-p */
		    bool    decrement = FALSE;

		    after = FALSE;
		    if (mpos == 0)
			decrement = TRUE;
		    msg_index = (msg_index + 9) % 10;
		    msg(msgbuf[msg_index]);
		    if (decrement)
			msg_index = (msg_index + 9) % 10;
		}
		when 'S':
		    after = FALSE;
		    if (save_game()) {
			wclear(cw);
			wrefresh(cw);
			endwin();
			exit(0);
		    }

		/*
		 * Other misc commands
		 */
		when '.':   ;   /* rest */
		when ',': add_pack(NULL, NOMESSAGE);
		when 'q': quaff(&player, -1, ISNORMAL);
		when 'r': read_scroll(&player, -1, ISNORMAL);
		when 'd': drop((struct linked_list *) NULL);
		when '^': set_trap(&player, hero.y, hero.x);
		when 'c': incant(&player, NULL);
		when 'D': dip_it();
		when 'e': eat();
		when '=': listen();
		when 'A': apply();
		when 'w': wield();
		when 'W': wear();
		when 'T': take_off();
		when 'P': ring_on();
		when 'R': ring_off();
		when 'p': prayer();
		when 'C': call(FALSE);
		when 'M': call(TRUE);
		when 's': search(FALSE);

		/*
		 * Directional commands - get_dir sets delta
		 */
		when 19:    /* ctrl-t */
		    if (get_dir())
			steal();
		    else
			after = FALSE;
		when 'z':
		    if (get_dir())
			do_zap(&player, -1, ISNORMAL);
		    else
			after = FALSE;
		when 'a':
		    if (get_dir())
			affect();
		    else
			after = FALSE;
		    touchwin(cw);

		/*
		 * wizard commands
		 */
		when 22:    /* ctrl-w */
		    after = FALSE;
		    if (!wizard) {
			if (canwizard)
			    if (passwd()) {
				msg("Welcome, oh mighty wizard.");
				wizard = waswizard = TRUE;
				(void) signal(SIGQUIT, SIG_DFL);
			    }
			    else
				msg("Incorrect password.");
			else
			    msg("Illegal command '^W'.");
		    }
		    else {
			object  *get_object();
			object  *obj;

			msg("Wizard command: ");
			mpos = 0;
			ch = readchar();
			switch (ch) {
			    when 'v':
				wiz_verbose = !wiz_verbose;
			    when 'e':   {
				wizard = FALSE;
				msg("Not wizard any more.");
			    }
			    when 's': activity();
			    when 't': teleport();
			    when 'm': overlay(mw, cw);
			    when 'f': overlay(stdscr, cw);
			    when 'i': inventory(lvl_obj, 0);
			    when 'c': buy_it('\0', ISNORMAL);
			    when 'I': whatis((struct linked_list *) NULL);
			    when 'F': msg("food left: %d\tfood level: %d", food_left, foodlev);
			    when 'M': creat_mons(&player, get_monster_number("create"), MESSAGE);
			    when 'r': msg("rnd(4)%d, rnd(40)%d, rnd(100)%d", rnd(4), rnd(40), rnd(100));
			    when 'C':
				if ((obj = get_object(pack, "charge", STICK, NULL)) != NULL)
				    obj->o_charges = 10000;
			    when 'w':
				if ((obj = get_object(pack, "price", NULL, NULL)) != NULL)
				    msg("Worth %d.", get_worth(obj));
			    when 'g': {
				int tlev;

				prbuf[0] = '\0';
				msg("Which level? ");
				if (get_str(prbuf, cw) == NORM) {
				    msg("");
				if ((tlev = atoi(prbuf)) < 1)
				    msg("Illegal level.");
				else if (tlev > 3000) {
				    levtype = THRONE;
				    level = tlev - 3000;
				}
				else if (tlev > 2000) {
				    levtype = MAZELEV;
				    level = tlev - 2000;
				}
				else if (tlev > 1000) {
				    levtype = POSTLEV;
				    level = tlev - 1000;
				}
				else {
				    levtype = NORMLEV;
				    level = tlev;
				}
				new_level(levtype);
			    }
			}
			when 'o':   {
			    int i;
			    struct linked_list  *item;

			    for (i = 0; i < 20; i++)
				raise_level();

			    max_stats.s_hpt += 1000;
			    max_stats.s_power += 1000;
			    pstats.s_hpt = max_stats.s_hpt;
			    pstats.s_power = max_stats.s_power;
			    max_stats.s_str = pstats.s_str = 25;
			    max_stats.s_intel = pstats.s_intel = 25;
			    max_stats.s_wisdom = pstats.s_wisdom = 25;
			    max_stats.s_dext = pstats.s_dext = 25;
			    max_stats.s_const = pstats.s_const = 25;

			    if (cur_weapon == NULL || cur_weapon->o_which != CLAYMORE) {
				item = spec_item(WEAPON, CLAYMORE, 10, 10);
				add_pack(item, NOMESSAGE);
				cur_weapon = OBJPTR(item);
				cur_weapon->o_flags |= ISKNOW;
			    }

			    /*
			     * and a kill-o-zap
			     * stick
			     */
			    item = spec_item(STICK, WS_DISINTEGRATE, 10000, 0);
			    obj = OBJPTR(item);
			    obj->o_flags |= ISKNOW;
			    know_items[TYP_STICK][WS_DISINTEGRATE] = TRUE;
			    if (guess_items[TYP_STICK][WS_DISINTEGRATE]) {
				free(guess_items[TYP_STICK][WS_DISINTEGRATE]);
				guess_items[TYP_STICK][WS_DISINTEGRATE] = NULL;
			    }
			    add_pack(item, NOMESSAGE);

			    /*
			     * and his suit of
			     * armor
			     */
			    if (cur_armor == NULL ||
				!(cur_armor->o_which == CRYSTAL_ARMOR ||
				  cur_armor->o_which == MITHRIL)) {
				item = spec_item(ARMOR, CRYSTAL_ARMOR, 15, 0);
				obj = OBJPTR(item);
				obj->o_flags |= ISKNOW;
				obj->o_weight =
				    armors[CRYSTAL_ARMOR].a_wght * 0.2;
				cur_armor = obj;
				add_pack(item, NOMESSAGE);
			    }

			    /*
			     * and some rings
			     * (have to put them
			     * on, for now)
			     */
			    if (!is_wearing(R_SEARCH)) {
				item = spec_item(RING, R_SEARCH, 0, 0);
				obj = OBJPTR(item);
				obj->o_flags |= ISKNOW;
				know_items[TYP_RING][R_SEARCH] = TRUE;
				if (guess_items[TYP_RING][R_SEARCH]) {
				    free(guess_items[TYP_RING][R_SEARCH]);
				    guess_items[TYP_RING][R_SEARCH] = NULL;
				}
				add_pack(item, NOMESSAGE);
			    }
			    if (!is_wearing(R_PIETY)) {
				item = spec_item(RING, R_PIETY, 0, 0);
				obj = OBJPTR(item);
				obj->o_flags |= ISKNOW;
				know_items[TYP_RING][R_PIETY] = TRUE;
				if (guess_items[TYP_RING][R_PIETY]) {
				    free(guess_items[TYP_RING][R_PIETY]);
				    guess_items[TYP_RING][R_PIETY] = NULL;
				}
				add_pack(item, NOMESSAGE);
			    }
			    item = spec_item(SCROLL, S_ELECTRIFY, 0, 0);
			    obj = OBJPTR(item);
			    obj->o_flags |= ISKNOW;
			    know_items[TYP_SCROLL][S_ELECTRIFY] = TRUE;
			    if (guess_items[TYP_SCROLL][S_ELECTRIFY]) {
				free(guess_items[TYP_SCROLL][S_ELECTRIFY]);
				guess_items[TYP_SCROLL][S_ELECTRIFY] = NULL;
			    }
			    add_pack(item, NOMESSAGE);

			    /* Spiff him up a bit */
			    quaff(&player, P_SHERO, ISBLESSED);
			    quaff(&player, P_CLEAR, ISBLESSED);
			    quaff(&player, P_FIRERESIST, ISBLESSED);
			    quaff(&player, P_TRUESEE, ISBLESSED);
			    quaff(&player, P_PHASE, ISBLESSED);
			    purse += 50000L;
			    updpack();
			}
			when    ESCAPE: /* Escape */
			    door_stop = FALSE;

			    count = 0;
			    after = FALSE;
			otherwise:
			    msg("Illegal wizard command '%s'.", unctrl(ch));
			    count = 0;
		    }
		}
		when    ESCAPE: /* Escape */
		    after = doescape();
		otherwise:
		    msg("Illegal command '%s'.",
			unctrl(ch));
		    count = 0;
		    after = FALSE;
	    }

	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}

	/*
	 * If he ran into something to take, let him pick it up.
	 */
	if (take != 0)
	    if (!moving)
		pick_up(take);
	    else
		show_floor();
	if (!running)
	    door_stop = FALSE;

	/*
	 * If after is true, mark an_after as true so that if we are
	 * hasted, the first "after" will be noted.
	 */
	if (after)
	    an_after = TRUE;
    }

    /*
     * Kick off the rest of the daemons and fuses
     */
    if (an_after) {
	int i;

	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);

	/* Special abilities */
	if ((player.t_ctype == C_THIEF || player.t_ctype == C_ASSASIN ||
	 player.t_ctype == C_NINJA || player.t_ctype == C_RANGER) &&
	    (rnd(100) < (2 * pstats.s_dext + 5 * pstats.s_lvl)))
	    search(TRUE);
	for (i = 0; i <= ring_value(R_SEARCH); i++)
	    search(FALSE);
	if (is_wearing(R_TELEPORT) && rnd(100) < 2) {
	    teleport();
	    if (off(player, ISCLEAR)) {
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8) +
			HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8) +
			HUHDURATION, AFTER);
		turn_on(player, ISHUH);
	    }
	    else
		msg("You feel dizzy for a moment, but it quickly passes.");
	}

	/* accidents and general clumsiness */
	if (fighting && rnd(50) == 0) {
	    msg("You become tired of this nonsense.");
	    fighting = after = FALSE;
	}
	if (on(player, ISELECTRIC))
	    electrificate();

	if (!fighting && (no_command == 0) && cur_weapon != NULL
	    && rnd(on(player, STUMBLER) ? 399 : 9999) == 0
	    && rnd(pstats.s_dext) <
	    2 - hitweight() + (on(player, STUMBLER) ? 4 : 0)) {
	    msg("You trip and stumble over your weapon.");
	    running = after = FALSE;
	    if (rnd(8) == 0 && (pstats.s_hpt -= roll(1, 10)) <= 0) {
		msg("You break your neck and die.");
		death(D_FALL);
		return;
	    }
	    else if (cur_weapon->o_flags & ISPOISON && rnd(4) == 0) {
		msg("You are cut by your %s!",
		    inv_name(cur_weapon, LOWERCASE));
		if (player.t_ctype != C_PALADIN
		    && !(player.t_ctype == C_NINJA &&
		    pstats.s_lvl > 12)
		    && !save(VS_POISON)) {
		    if (pstats.s_hpt == 1) {
			msg("You die from the poison in the cut.");
			death(D_POISON);
			return;
		    }
		    else {
			msg("You feel very sick now.");
			pstats.s_hpt /= 2;
			chg_str(-2, FALSE, FALSE);
		    }
		}
	    }
	}

	/* Time to enforce weapon and armor restrictions */
	if (rnd(9999) == 0)
	    if (((cur_weapon == NULL) ||
		(wield_ok(&player, cur_weapon, NOMESSAGE)))
		&& ((cur_armor == NULL) ||
		(wear_ok(&player, cur_armor, NOMESSAGE)))) {
		switch (player.t_ctype) {
		    when    C_CLERIC:
		    case C_DRUID:
		    case C_RANGER:
		    case C_PALADIN:
			if (rnd(luck) != 0)
			    /* You better have done
			     * little wrong */
			    goto bad_cleric;

			msg("You are enraptured by the renewed power of your god.");
		    when    C_MAGICIAN:
		    case C_ILLUSION:
			msg("You become in tune with the universe.");
		    when    C_THIEF:
		    case C_NINJA:
		    case C_ASSASIN:
			msg("You become supernaly sensitive to your surroundings.");
		    when    C_FIGHTER:
			msg("You catch your second wind.");

		    otherwise:
			msg("What a strange type you are!");
		}
		pstats.s_hpt = max_stats.s_hpt += rnd(pstats.s_lvl) + 1;
		pstats.s_power = max_stats.s_power += rnd(pstats.s_lvl) + 1;
	    }
	    else {  /* he blew it - make him pay */
		int death_cause;

		switch (player.t_ctype) {
		    when    C_CLERIC:
		    case C_DRUID:
		    case C_RANGER:
		    case C_PALADIN:
		bad_cleric:
			msg("Your god scourges you for your misdeeds.");
			death_cause = D_GODWRATH;
		    when    C_MAGICIAN:
		    case C_ILLUSION:
			msg("You short out your manna on the unfamiliar %s.",
			    (cur_armor != NULL ? "armor" : "weapon"));

			death_cause = D_SPELLFUMBLE;
		    when    C_THIEF:
		    case C_NINJA:
		    case C_ASSASIN:
			msg("You trip and fall because of the unfamiliar %s.",
			    (cur_armor != NULL ? "armor" : "weapon"));
			death_cause = D_CLUMSY;
		    when    C_FIGHTER:
			debug("Fighter getting raw deal?");
		    otherwise:
			msg("What a strange type you are!");
		}
		aggravate();
		pstats.s_power /= 2;
		pstats.s_hpt /= 2;
		player.t_no_move++;
		if ((pstats.s_hpt -= rnd(pstats.s_lvl)) <= 0) {
		    death(death_cause);
		}
	    }

	if (rnd(9999) == 0) {
	    new_level(THRONE);
	    fighting = running = after = FALSE;
	    summoned = TRUE;
	}
    }
}

/*
 * quit: Have player make certain, then exit.
 */
quit()
{

    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    wrefresh(cw);
    if (readchar() == 'y') {
	clear();
	wclear(cw);
	wrefresh(cw);
	move(LINES - 1, 0);
	wrefresh(stdscr);
	refresh();
	score((long) pstats.s_exp, pstats.s_lvl, CHICKEN, 0);
	byebye();
    }
    else {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status(FALSE);
	wrefresh(cw);
	mpos = 0;
	count = 0;
	fighting = running = 0;
    }
}

/*
 * search: Player gropes about him to find hidden things.
 */
search(is_thief)
bool    is_thief;
{
    int x, y;
    char    ch;

    /*
     * Look all around the hero, if there is something hidden there, give
     * him a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++) {
	    ch = winat(y, x);
	    if (isatrap(ch)) {
		struct trap *tp;
		struct room *rp;

		if (isatrap(mvwinch(cw, y, x)))
		    continue;
		tp = trap_at(y, x);
		if ((tp->tr_flags & ISTHIEFSET) ||
		    (rnd(100) > 50 && !is_thief))
		    break;
		rp = roomin(&hero);
		if (tp->tr_type == FIRETRAP && rp != NULL) {
		    rp->r_flags &= ~ISDARK;
		    light(&hero);
		}
		tp->tr_flags |= ISFOUND;
		mvwaddch(cw, y, x, ch);
		count = 0;
		running = FALSE;
		msg(tr_name(tp->tr_type));
	    }
	    else if (ch == SECRETDOOR) {
		if (rnd(100) < 20 && !is_thief) {
		    mvaddch(y, x, DOOR);
		    count = 0;
		}
	    }
	}
}

/*
 * help: Give single character help, or the whole mess if he wants it
 */
help()
{
    struct h_list   *strp = helpstr;
    char    helpch;
    int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;

    /*
     * If its not a *, print the right help string or an error if he
     * typed a funny character.
     */
    if (helpch != '*') {
	wmove(cw, 0, 0);
	while (strp->h_ch) {
	    if (strp->h_desc == 0)
		if (!wizard) {
		    break;
		}
		else {
		    strp++;
		    continue;
		}

	    if (strp->h_ch == helpch) {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'.", unctrl(helpch));
	return;
    }

    /*
     * Here we print help for everything. Then wait before we return to
     * command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
	if (strp->h_desc == 0)
	    if (!wizard) {
		break;
	    }
	    else {
		strp++;
		continue;
	    }

	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	strp++;

	if (++cnt >= 46 && strp->h_ch && (strp->h_desc != NULL
	    || wizard)) {
	    wmove(hw, LINES - 1, 0);
	    wprintw(hw, morestr);
	    wrefresh(hw);
	    wait_for(' ');
	    wclear(hw);
	    cnt = 0;
	}
    }
    wmove(hw, LINES - 1, 0);
    wprintw(hw, morestr);
    wrefresh(hw);
    wait_for(' ');
    wclear(hw);
    wrefresh(hw);

    /*
     * Print info on keypad
     */
    keypadhelp();
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status(FALSE);
    touchwin(cw);
}

/*
 * identify: Tell the player what a certain thing is.
 */

identify()
{
    char    ch, *str;

    msg("What do you want identified? ");
    mpos = 0;
    if ((ch = readchar()) == ESCAPE) {
	msg("");
	return;
    }
    if (isalpha(ch)) {
	id_monst(ch);
	return;
    }
    switch (ch) {
	when '|':
	case '-':
	    str = "wall of a room";
	when GOLD:str = "gold";
	when STAIRS:str = "passage leading down";
	when DOOR:str = "door";
	when FLOOR:str = "room floor";
	when VPLAYER:str = "The hero of the game ---> you";
	when IPLAYER:str = "you (but invisible)";
	when PASSAGE:str = "passage";
	when POST:str = "trading post";
	when POOL:str = "a shimmering pool";
	when TRAPDOOR:str = "trapdoor";
	when ARROWTRAP:str = "arrow trap";
	when SLEEPTRAP:str = "sleeping gas trap";
	when BEARTRAP:str = "bear trap";
	when TELTRAP:str = "teleport trap";
	when DARTTRAP:str = "dart trap";
	when MAZETRAP:str = "entrance to a maze";
	when FIRETRAP:str = "fire trap";
	when POISONTRAP:str = "poison pool trap";
	when LAIR:str = "monster lair entrance";
	when RUSTTRAP:str = "rust trap";
	when POTION:str = "potion";
	when SCROLL:str = "scroll";
	when FOOD:str = "food";
	when WEAPON:str = "weapon";
	when ' ': str = "solid rock";
	when ARMOR:str = "armor";
	when ARTIFACT:str = "an artifact from bygone ages";
	when RING:str = "ring";
	when STICK:str = "wand or staff";
	otherwise: str = "unknown character";
    }
    msg("'%s'; %s", unctrl(ch), str);
}

/*
 * d_level: He wants to go down a level
 */

d_level()
{
    bool    no_phase = FALSE;

    if (mvinch(hero.y, hero.x) != STAIRS) {
	if (off(player, CANINWALL)) {   /* Must use stairs if can't
			 * phase */
	    msg("I see no way down.");
	    return;
	}
	extinguish(unphase);    /* Using phase to go down gets rid of
		     * it */
	no_phase = TRUE;
    }
    if (is_wearing(R_LEVITATION) || on(player, CANFLY)) {
	msg("You can't!  You're floating in the air.");
	return;
    }
    if (rnd(pstats.s_dext) < 3 * (2 - hitweight() +
	(on(player, STUMBLER) ? 4 : 0))) {
	msg("You trip and fall down the stairs.");
	if ((pstats.s_hpt -= roll(1, 10)) <= 0) {
	    msg("You break your neck and die.");
	    death(D_FALL);
	    return;
	}
    }
    level++;
    new_level(NORMLEV);
    if (no_phase)
	unphase();
}

/*
 * u_level: He wants to go up a level
 */

u_level()
{
    char    ch;

    if (has_artifact && ((ch = mvinch(hero.y, hero.x)) == STAIRS ||
		 (on(player, CANINWALL)
	    && (is_wearing(R_LEVITATION) || on(player, CANFLY))))) {
	if (--level == 0)
	    total_winner();
	else if (rnd(wizard ? 3 : 15) == 0)
	    new_level(THRONE);
	else {
	    new_level(NORMLEV);
	    msg("You feel a wrenching sensation in your gut.");
	}
	if (on(player, CANINWALL) && ch != STAIRS) {
	    extinguish(unphase);
	    unphase();
	}
	return;
    }
    else if (ch != STAIRS &&
	 !(on(player, CANINWALL) && (is_wearing(R_LEVITATION)
			 || on(player, CANFLY))))
	msg("I see no way up.");
    else
	msg("Your way is magically blocked.");
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
call(mark)
bool    mark;
{
    struct object   *obj;
    char    *elsewise;
    int item_type = numthings;
    char    **item_color;

    int bff_callable(), bff_markable();
    object  *get_object();

    if (mark)
	obj = get_object(pack, "mark", NULL, bff_markable);
    else
	obj = get_object(pack, "call", NULL, bff_callable);
    if (obj == NULL)
	return;

    switch (obj->o_type) {
	when    RING:
	    item_type = TYP_RING;
	    item_color = r_stones;
	when    POTION:
	    item_type = TYP_POTION;
	    item_color = p_colors;
	when    SCROLL:
	    item_type = TYP_SCROLL;
	    item_color = s_names;
	when    STICK:
	    item_type = TYP_STICK;
	    item_color = ws_made;
	otherwise:
	    if (!mark) {
		msg("You can't call that anything.");
		return;
	    }
    }
    elsewise = (guess_items[item_type][obj->o_which] != NULL ?
       guess_items[item_type][obj->o_which] : item_color[obj->o_which]);

    if (know_items[item_type][obj->o_which] && !mark) {
	msg("That has already been identified.");
	return;
    }
    if (mark) {
	if (obj->o_mark[0]) {
	    addmsg(terse ? "M" : "Was m");
	    msg("arked \"%s\".", obj->o_mark);
	}
	msg(terse ? "Mark it: " : "What do you want to mark it? ");
	prbuf[0] = '\0';
    }
    else {
	addmsg(terse ? "C" : "Was c");
	msg("alled \"%s\".", elsewise);
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (guess_items[item_type][obj->o_which] != NULL)
	    free(guess_items[item_type][obj->o_which]);
	strcpy(prbuf, elsewise);
    }
    if (get_str(prbuf, cw) == NORM) {
	if (mark) {
	    strncpy(obj->o_mark, prbuf, MARKLEN - 1);
	    obj->o_mark[MARKLEN - 1] = '\0';
	}
	else {
	    guess_items[item_type][obj->o_which] = new((unsigned int) strlen(prbuf) + 1);
	    strcpy(guess_items[item_type][obj->o_which], prbuf);
	}
    }
}

/*
 * att_bonus: bonus attacks for certain player classes
 */

att_bonus()
{
    int bonus = FALSE;

    if ((player.t_ctype == C_FIGHTER || player.t_ctype == C_PALADIN)
	&& (pstats.s_lvl > 12 ||
	(pstats.s_lvl > 6 && pstats.s_lvl < 13 && rnd(2))))
	bonus = TRUE;
    else if ((player.t_ctype == C_RANGER)
	 && (pstats.s_lvl > 14 ||
	     (pstats.s_lvl > 7 && pstats.s_lvl < 15 && rnd(2))))
	bonus = TRUE;
    else if ((player.t_ctype == C_NINJA)
	 && (pstats.s_lvl > 8 ||
	     (pstats.s_lvl > 4 && pstats.s_lvl < 9 && rnd(2))))
	bonus = TRUE;
    return bonus;
}
