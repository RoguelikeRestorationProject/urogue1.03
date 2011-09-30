/*
    main.c  -  Rogue Exploring the dungeons of doom
   
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

/*
 * Rogue Exploring the dungeons of doom Copyright (C) 1980 by Michael Toy and
 * Glenn Wichman All rights reserved
 *
 */

#include <stdio.h>
#include <signal.h>
#include "rogue.h"
#include "score.h"

int fd_score = -1;

/* daemon.c globals  */
int  demoncnt = 0;   /* number of active daemons */



/* command.c globals */
char    fight_ch;
char countch, newcount;
int an_after;
int summoned;
coord    dta;

/* chase.c globals */
coord   ch_ret;         /* Where chasing takes you */
coord   shoot_dir;
struct linked_list   *arrow, *bolt, *rock, *silverarrow, *fbbolt;
struct linked_list   *bullet, *firearrow, *dart, *dagger, *shuriken;
struct linked_list   *oil, *grenade;

main(argc, argv, envp)
int argc;
char    **argv;
char    **envp;
{
    int x;
    char    *env;
    char    *crypt(), *strrchr();
    int lowtime;
    long    now;
    int rflag;

    for (x = 1; x < argc; x++) {
	if (argv[x][0] != '-')
	    break;
	switch (argv[x][1]) {
	    when 's':
		prscore = TRUE;
	    when 'v':
		prversion = TRUE;
	    when 'r':
		rflag = TRUE;
	    otherwise:
		fprintf(stderr, "%s: Unknown option '%c'.\n",
		    argv[0], argv[x][1]);
		exit(1);
	}
    }
    if (!rflag) {
	argc -= (x - 1);
	argv += (x - 1);
    }

    /* Get default score file */
    strcpy(score_file, "urogue.scr");

    fd_score = open(score_file, 2);

    if ((env = getenv("OPTIONS")) != NULL)
	parse_opts(env);
    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
	    "candleberry", "caprifig", "dewberry", "elderberry",
	    "gooseberry", "guanabana", "hagberry", "ilama", "imbu",
	    "jaboticaba", "jujube", "litchi", "mombin", "pitanga",
	    "prickly pear", "rambutan", "sapodilla", "soursop",
	    "sweetsop", "whortleberry"
	};

	srandom(1234);
	strcpy(fruit, funfruit[rnd(sizeof(funfruit) /
	    sizeof(funfruit[0]))]);
    }

    /* put a copy of fruit in the right place */
    strcpy(fd_data[1].mi_name, fruit);

    /*
     * print scores
     */
    if (prscore) {
	waswizard = TRUE;
	score(0L, 0, SCOREIT, 0);
	exit(0);
    }

    /*
     * check for version option
     */
    if (prversion) {
	printf("UltraRogue Version %s.\n", release);
	exit(0);
    }

    if (argc == 2 && argv[1][0] != '\0' &&
	!restore(argv[1], envp))    /* Note: restore returns on error
		     * only */
	exit(1);
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) : lowtime);
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    srandom(dnum);

    init_things();      /* Set up probabilities of things */
    init_fd();      /* Set up food probabilities */
    init_colors();      /* Set up colors of potions */
    init_stones();      /* Set up stone settings of rings */
    init_materials();   /* Set up materials of wands */
    initscr();      /* Start up cursor package */
    init_names();       /* Set up names of scrolls */
    cbreak();
    crmode();       /* Cbreak mode */
    noecho();       /* Echo off */
    nonl();

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    waswizard = wizard; /* set wizard flags */

    init_player();      /* look up things and outfit pack */

    resurrect = pstats.s_const;
    init_exp();     /* set first experience level change */
    init_flags();       /* set initial flags */
    wclear(hw);
    wrefresh(hw);
    new_level(POSTLEV); /* Draw current level */

    /*
     * Start up daemons and fuses
     */
    daemon(doctor, (int) &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    char_type = player.t_ctype;
    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();  /* Command execution */
    fatal("");
}

/*
 * fatal: Exit the program, printing a message.
 */

fatal(s)
char    *s;
{
    clear();
    move(LINES - 2, 0);
    printw("%s", s);
    wrefresh(stdscr);
    endwin();
    printf("\n");       /* So the cursor doesn't stop at the end of
		 * the line */
    exit(0);
}

/*
 * rnd: Pick a very random number.
 */

rnd(range)
int range;
{
    return (range <= 0 ? 0 : (random() & 0x7fffffffL) % range);
}

/*
 * roll: roll a number of dice
 */

roll(number, sides)
int number, sides;
{
    int dtotal = 0;

    while (number--)
	dtotal += rnd(sides) + 1;
    return dtotal;
}
