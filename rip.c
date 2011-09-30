/*
    rip.c  -  File for the fun ends Death or a total win
   
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
#include <time.h>
#include "rogue.h"
#include "score.h"
#include "death.h"

static char *rip[] = {
    "                       __________",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |",
    "                  |                  |",
    "                  |    killed by     |",
    "                  |                  |",
    "                  |       1980       |",
    "                 *|     *  *  *      | *",
    "       ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

char    *killname();

/*
 * death: Do something really fun when he dies
 */

death(monst)
short   monst;
{
    char    **dp = rip, *killer;
    struct tm   *lt;
    time_t  date;
    char    buf[80];
    struct tm   *localtime();

    if (is_wearing(R_RESURRECT) || rnd(wizard ? 3 : 67) == 0) {
	int die = TRUE;

	if (resurrect-- == 0)
	    msg("You've run out of lives.");
	else if (!save_resurrect(ring_value(R_RESURRECT)))
	    msg("Your attempt to return from the grave fails.");
	else {
	    struct linked_list  *item;
	    struct linked_list  *next_item;
	    struct object   *obj;
	    int rm, flags;
	    coord   pos;

	    die = FALSE;
	    msg("You feel sudden warmth and then nothingness.");
	    teleport();
	    if (ring_value(R_RESURRECT) > 1 && rnd(10)) {
		pstats.s_hpt = 2 * pstats.s_const;
		pstats.s_const = max(pstats.s_const - 1, 3);
	    }
	    else {
		for (item = pack; item != NULL;
		    item = next_item) {
		    obj = OBJPTR(item);
		    if (obj->o_flags & ISOWNED ||
			obj->o_flags & ISPROT) {
			next_item = next(item);
			continue;
		    }
		    flags = obj->o_flags;
		    obj->o_flags &= ~ISCURSED;
		    dropcheck(obj);
		    obj->o_flags = flags;
		    next_item = next(item);
		    rem_pack(obj);
		    if (obj->o_type == ARTIFACT)
			has_artifact &= ~(1 <<
			    obj->o_which);
		    do {
			rm = rnd_room();
			rnd_pos(&rooms[rm], &pos);
		    } until(winat(pos.y, pos.x) == FLOOR);
		    obj->o_pos = pos;
		    add_obj(item, obj->o_pos.y,
			obj->o_pos.x);
		}
		pstats.s_hpt = pstats.s_const;
		pstats.s_const = max(pstats.s_const -
		    roll(2, 2), 3);
	    }
	    chg_str(roll(1, 4), TRUE, FALSE);
	    pstats.s_lvl = max(pstats.s_lvl, 1);
	    no_command += 2 + rnd(4);
	    if (on(player, ISHUH))
		lengthen(unconfuse, rnd(8) + HUHDURATION);
	    else
		fuse(unconfuse, 0, rnd(8) + HUHDURATION, AFTER);
	    turn_on(player, ISHUH);
	    light(&hero);
	}
	if (die) {
	    wmove(cw, mpos, 0);
	    waddstr(cw, morestr);
	    wrefresh(cw);
	    wait_for(' ');
	}
	else
	    return;
    }
    time(&date);
    lt = localtime(&date);
    clear();
    wclear(cw);
    move(8, 0);
    while (*dp)
	printw("%s\n", *dp++);
    mvaddstr(14, 28 - ((strlen(whoami) + 1) / 2), whoami);
    sprintf(buf, "%d+%ld Points", pstats.s_lvl, pstats.s_exp);
    mvaddstr(15, 28 - ((strlen(buf) + 1) / 2), buf);
    killer = killname(monst);
    mvaddstr(17, 28 - ((strlen(killer) + 1) / 2), killer);
    mvaddstr(18, 28, (sprintf(prbuf, "%2d", lt->tm_year), prbuf));
    move(LINES - 1, 0);
    idenpack();
    wrefresh(cw);
    refresh();
    score((long) pstats.s_exp, pstats.s_lvl, KILLED, monst);
    exit(0);
}

/*
 * score -- figure score and post it.
 */

score(amount, lvl, flags, monst)
long    amount;
short   monst;
int flags;
int lvl;
{
    static struct sc_ent {
	int sc_lvl;
	long    sc_score;
	char    sc_name[76];
	long    sc_gold;
	int sc_flags;
	int sc_level;
	short   sc_artifacts;
	short   sc_monster;
    }   top_ten[10];
    struct sc_ent   *scp;
    int i;
    char    *killer;
    static char *reason[] = {
	"killed",
	"quit",
	"a winner",
	"a total winner"
    };
    int endit();
    char    *packend;
    extern  int fd_score;

    if (flags != WINNER && flags != TOTAL && flags != SCOREIT) {
	if (flags == CHICKEN)
	    packend = "when you quit";
	else
	    packend = "at your untimely demise";
	noecho();
	nl();
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	gets(prbuf);
	showpack(packend);
    }

    /*
     * Open file and read list
     */

    if (fd_score < 0)
	return;

    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	scp->sc_lvl = 0L;
	scp->sc_score = 0L;
	for (i = 0; i < 76; i++)
	    scp->sc_name[i] = rnd(255);
	scp->sc_gold = 0L;
	scp->sc_flags = rnd(10);
	scp->sc_level = rnd(10);
	scp->sc_monster = rnd(10);
	scp->sc_artifacts = 0;
    }

    if (flags != SCOREIT) {
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	fflush(stdout);
	gets(prbuf);
    }
    encread((char *) top_ten, sizeof top_ten, fd_score);

    /*
     * Insert player in list if need be
     */
    if (!waswizard) {
	for (scp = top_ten; scp < &top_ten[10]; scp++) {
	    if (lvl > scp->sc_lvl)
		break;
	    if (lvl == scp->sc_lvl && amount > scp->sc_score)
		break;
	}
    }
    if (flags != SCOREIT) {
	clear();
	refresh();
	endwin();
    }

    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank\tScore\tGold\t\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++) {

	if (scp->sc_score) {
	    printf("%d\t%ld+%ld\t%ld\t%s:\n", scp - top_ten + 1,
		   scp->sc_lvl,
		   scp->sc_score,
		   scp->sc_gold,
		   scp->sc_name);
	    if (scp->sc_artifacts) {
		char    things[60];
		int i;
		bool    first = TRUE;

		things[0] = '\0';
		for (i = 0; i <= maxartifact; i++) {
		    if (scp->sc_artifacts & (1 << i)) {
			if (strlen(things))
			    strcat(things, ", ");
			if (first) {
			    strcat(things,
				"retrieved ");
			    first = FALSE;
			}
			if (55 - strlen(things) <
			    strlen(arts[i].ar_name)) {
			    printf("\t\t\t%s\n",
				things);
			    things[0] = '\0';
			}
			strcat(things, arts[i].ar_name);
		    }
		}
		if (strlen(things))
		    printf("\t\t\t%s,", things);
		putchar('\n');
	    }
	    printf("\t\t\t%s on level %d",
		   reason[scp->sc_flags], scp->sc_level);
	    if (scp->sc_flags == 0) {
		printf(" by");
		killer = killname(scp->sc_monster);
		printf(" %s", killer);
	    }
	    putchar('\n');
	}
    }
    lseek(fd_score, 0L, 0);

    /*
     * Update the list file
     */
    encwrite((char *) top_ten, sizeof top_ten, fd_score);
    close(fd_score);
    if (!prscore) {
	putchar('\n');
	printf(retstr);
	fflush(stdout);
	gets(prbuf);
    }
}

total_winner()
{
    struct linked_list  *item;
    struct object   *obj;
    int worth, oldpurse;
    char    c;
    struct linked_list  *bag = NULL;

    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have \n");
    addstr("escaped the Dungeons of Doom alive.  You journey home \n");
    addstr("and sell all your loot at a great profit.\n");
    addstr("The White Council approves the recommendation of\n");
    if (player.t_ctype == C_FIGHTER)
	addstr("the fighters guild and appoints you Lord Protector\n");
    else if (player.t_ctype == C_ASSASIN)
	addstr("the assassins guild and appoints you Master Murderer\n");
    else if (player.t_ctype == C_NINJA)
	addstr("the ninja guild and appoints you Shogun\n");
    else if (player.t_ctype == C_ILLUSION)
	addstr("the illusionists guild and appoints you Master Wizard\n");
    else if (player.t_ctype == C_MAGICIAN)
	addstr("the magicians guild and appoints you Master Wizard\n");
    else if (player.t_ctype == C_CLERIC)
	addstr("the temple priests and appoints you Master of the Flowers\n");
    else if (player.t_ctype == C_DRUID)
	addstr("the temple priests and appoints you Master of the Flowers\n");
    else if (player.t_ctype == C_RANGER)
	addstr("the rangers guild and appoints you Master Ranger\n");
    else if (player.t_ctype == C_PALADIN)
	addstr("the paladins guild and appoints you Master Paladin\n");
    else if (player.t_ctype == C_THIEF) {
	addstr("the thieves guild under protest and appoints you\n");
	addstr("Master of the Highways\n");
    }
    addstr("of the Land Between the Mountains.\n");
    mvaddstr(LINES - 1, 0, spacemsg);
    refresh();
    wait_for(' ');
    clear();
    idenpack();
    oldpurse = purse;
    mvaddstr(0, 0, "   Worth  Item");
    for (c = 'a', item = pack; item != NULL; c++, item = next(item)) {
	obj = OBJPTR(item);
	worth = get_worth(obj);
	purse += worth;
	if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
	    bag = obj->o_bag;
	mvprintw(c - 'a' + 1, 0, "%c) %8d  %s", c,
	     worth, inv_name(obj, UPPERCASE));
    }
    if (bag != NULL) {
	mvaddstr(LINES - 1, 0, morestr);
	refresh();
	wait_for(' ');
	clear();
	mvprintw(0, 0, "Contents of the Magic Purse of Yendor:\n");
	for (c = 'a', item = bag; item != NULL;
	    c++, item = next(item)) {
	    obj = OBJPTR(item);
	    worth = get_worth(obj);
	    whatis(item);
	    purse += worth;
	    mvprintw(c - 'a' + 1, 0, "%c) %8d %s\n", c,
		 worth, inv_name(obj, UPPERCASE));
	}
    }
    mvprintw(c - 'a' + 1, 0, "   %6d  Gold Pieces          ", oldpurse);
    refresh();
    if (has_artifact == 255)
	score((long) pstats.s_exp, pstats.s_lvl, TOTAL, 0);
    else
	score((long) pstats.s_exp, pstats.s_lvl, WINNER, 0);
    exit(0);
}

char    *
killname(monst)
short   monst;
{
    static char mons_name[80];

    if (monst >= 0) {
	switch (monsters[monst].m_name[0]) {
	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
		sprintf(mons_name, "an %s",
		    monsters[monst].m_name);
		break;
	    default:
		sprintf(mons_name, "a %s",
		    monsters[monst].m_name);
	}
	return (mons_name);
    }
    else
	switch (monst) {
	    case D_ARROW:
		return "an arrow";
	    case D_DART:
		return "a dart";
	    case D_BOLT:
		return "a bolt";
	    case D_POISON:
		return "poison";
	    case D_POTION:
		return "a cursed potion";
	    case D_PETRIFY:
		return "petrification";
	    case D_SUFFOCATION:
		return "suffocation";
	    case D_INFESTATION:
		return "a parasite";
	    case D_DROWN:
		return "drowning";
	    case D_FALL:
		return "falling";
	    case D_FIRE:
		return "slow boiling in oil";
	    case D_SPELLFUMBLE:
		return "a botched spell";
	    case D_DRAINLIFE:
		return "a drain life spell";
	    case D_ARTIFACT:
		return "an artifact of the gods";
	    case D_GODWRATH:
		return "divine retribution";
	    case D_CLUMSY:
		return "excessive clumsyness";
	}
}

/*
 * showpack: Display the contents of the hero's pack
 */
showpack(howso)
char    *howso;
{
    char    *iname;
    int cnt, worth, ch, oldpurse;
    struct linked_list  *item;
    struct object   *obj;
    struct linked_list  *bag = NULL;

    cnt = 1;
    clear();
    mvprintw(0, 0, "Contents of your pack %s:\n", howso);
    ch = 0;
    oldpurse = purse;
    purse = 0;
    for (item = pack; item != NULL; item = next(item)) {
	obj = OBJPTR(item);
	worth = get_worth(obj);
	whatis(item);
	purse += worth;
	if (obj->o_type == ARTIFACT && obj->o_which == TR_PURSE)
	    bag = obj->o_bag;
	iname = inv_name(obj, UPPERCASE);
	mvprintw(cnt, 0, "%d) %s\n", ch, iname);
	ch += 1;
	if (++cnt > LINES - 5 && next(item) != NULL) {
	    cnt = 1;
	    mvaddstr(LINES - 1, 0, morestr);
	    refresh();
	    wait_for(' ');
	    clear();
	}
    }
    if (bag != NULL) {
	mvaddstr(LINES - 1, 0, morestr);
	refresh();
	wait_for(' ');
	clear();
	cnt = 1;
	ch = 0;
	mvprintw(0, 0, "Contents of the Magic Purse of Yendor %s:\n",
	     howso);
	for (item = bag; item != NULL; item = next(item)) {
	    obj = OBJPTR(item);
	    worth = get_worth(obj);
	    whatis(item);
	    purse += worth;
	    mvprintw(cnt, 0, "%d) %s\n", ch, inv_name(obj,
		UPPERCASE));
	    ch += 1;
	    if (++cnt > LINES - 5 && next(item) != NULL) {
		cnt = 1;
		mvaddstr(LINES - 1, 0, morestr);
		refresh();
		wait_for(' ');
		clear();
	    }
	}
    }
    mvprintw(cnt + 1, 0, "Carrying %d gold pieces", oldpurse);
    mvprintw(cnt + 2, 0, "Carrying objects worth %d gold pieces", purse);
    purse += oldpurse;
    refresh();
}

void
byebye()
{
    endwin();
    ttputc('\n');
    exit(0);
}

/*
 * save_resurrect: chance of resurrection according to modifed D&D
 * probabilities
 */
save_resurrect(bonus)
int bonus;
{
    register    need, adjust;

    adjust = pstats.s_const + bonus - luck;
    if (adjust > 17)
	return TRUE;
    else if (adjust < 14)
	need = 5 * (adjust + 5);
    else
	need = 90 + 2 * (adjust - 13);
    return (roll(1, 100) < need);
}
