/*
    getplay.c  -  Procedures for saving and retrieving a characters

    Last Modified: 09/16/86

    UltraRogue
    Copyright (C) 1984, 1985, 1986 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.*/
*/

/*
 * Procedures for saving and retrieving a characters
 * starting attributes, armour, and weapon.
 * New addition to version 1.3 by S. A. Hester  11/8/83
*/

#include <stdio.h>
#include <ctype.h>
#include "rogue.h"

#define I_STR	0
#define I_INTEL	1
#define I_WISDOM	2
#define I_DEXT	3
#define I_CONST	4
#define I_CHARISMA	5
#define I_HPT	6
#define I_POWER	7
#define I_CTYPE	8
#define MAXPATT	9   /* Total Number of above defines. */
#define MAXPDEF	10  /* Maximum number of pre-defined chars */

static int	def_array[MAXPDEF][MAXPATT];	/* Pre-def'd chars */

int
geta_player()
{
	int  i;
	int  fd;
	char pbuf[2 * LINELEN];

	if ((fd = open("urogue.chr", 0)) < 0)
	    return FALSE;

	encread((char *) def_array, sizeof(def_array), fd);
	close(fd);

	wclear(hw);
	touchwin(hw);

	print_stored();
	mvwaddstr(hw, 0, 0, "Do you wish to select a character? ");
	wrefresh(hw);

	if ((getchar() & 0177) != 'y')
	    return FALSE;

	do
	{
	    wmove(hw, LINES - 1, 0);
	    wclrtoeol(hw);
	    mvwaddstr(hw, 0, 0, "Enter the number of a pre-defined character: ");
	    wclrtoeol(hw);
	    wrefresh(hw);
	    get_str(pbuf, hw);
	    i = atoi(pbuf) - 1;

	    if (i < 0 || i > MAXPDEF - 1)
	    {
		wstandout(hw);
		mvwaddstr(hw, 1, 0, "Please use the range 1 to");
		wprintw(hw, " %d.", MAXPDEF);
		wstandend(hw);
		wclrtoeol(hw);
		wrefresh(hw);
	    }
	    else if (def_array[i][I_STR] == 0)
	    {
		wstandout(hw);
		mvwaddstr(hw,1,0,"Please enter the number of a known character: ");
		wstandend(hw);
		wclrtoeol(hw);
	    }
	    else
	    {
		mvwaddstr(hw, 1, 0, "");
		wclrtoeol(hw);
	    }

	}
	while (i < 0 || i > MAXPDEF - 1 || (def_array[i][I_STR] == 0));

	pstats.s_str	  = def_array[i][I_STR];
	pstats.s_intel	  = def_array[i][I_INTEL];
	pstats.s_wisdom   = def_array[i][I_WISDOM];
	pstats.s_dext	  = def_array[i][I_DEXT];
	pstats.s_const	  = def_array[i][I_CONST];
	pstats.s_charisma = def_array[i][I_CHARISMA];
	pstats.s_hpt	  = def_array[i][I_HPT];
	pstats.s_power	  = def_array[i][I_POWER];
	player.t_ctype	  = char_type = def_array[i][I_CTYPE];
	max_stats	  = pstats;

	return(TRUE);
}

int
puta_player()
{
	int fd;
	char	pbuf[2 * LINELEN];
	short	i;
	char	*class = which_class(player.t_ctype);

	sprintf(pbuf, "You have a %s with the following attributes:", class);
	mvwaddstr(hw, 2, 0, pbuf);
	wclrtoeol(hw);

	sprintf(pbuf,
	    "Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d Pow: %d Hpt: %d",
	    pstats.s_intel,
	    pstats.s_str,
	    pstats.s_wisdom,
	    pstats.s_dext,
	    pstats.s_const,
	    pstats.s_charisma,
	    pstats.s_power,
	    pstats.s_hpt );

	mvwaddstr(hw, 3, 0, "");
	wclrtoeol(hw);
	mvwaddstr(hw, 4, 0, pbuf);
	wclrtoeol(hw);
	mvwaddstr(hw, 5, 0, "");
	wclrtoeol(hw);
	mvwaddstr(hw, 0, 0, "Would you like to save this character?");
	wclrtoeol(hw);
	wrefresh(hw);

	if ((getchar() & 0177) != 'y')
	    return(TRUE);

	do
	{
	    mvwaddstr(hw, 0, 0, "Overwrite which number? ");
	    wclrtoeol(hw);
	    wrefresh(hw);
	    get_str(pbuf, hw);
	    i = atoi(pbuf) - 1;

	    if (i < 0 || i > MAXPDEF - 1)
	    {
		wstandout(hw);
		mvwaddstr(hw, 1, 0, "Use the range 1 to");
		wprintw(hw, " %d!", MAXPDEF);
		wstandend(hw);
		wclrtoeol(hw);
		wrefresh(hw);
	    }
	}
	while (i < 0 || i > MAXPDEF - 1);

	/* Set some global stuff */

	def_array[i][I_STR]	 = pstats.s_str;
	def_array[i][I_INTEL]	 = pstats.s_intel;
	def_array[i][I_WISDOM]	 = pstats.s_wisdom;
	def_array[i][I_DEXT]	 = pstats.s_dext;
	def_array[i][I_CONST]	 = pstats.s_const;
	def_array[i][I_CHARISMA] = pstats.s_charisma;
	def_array[i][I_HPT]	 = pstats.s_hpt;
	def_array[i][I_POWER]	 = pstats.s_power;
	def_array[i][I_CTYPE]	 = player.t_ctype;

	/* OK. Now let's write this stuff out! */

	if ((fd = creat("urogue.chr", 0644)) < 0)
	{
	    sprintf(pbuf, "I can't seem to open/create urogue.chr.");
	    mvwaddstr(hw, 5, 5, pbuf);
	    mvwaddstr(hw, 6, 5, "However I'll let you play it anyway!");
	    mvwaddstr(hw, LINES - 1, 0, spacemsg);
	    wrefresh(hw);
	    wait_for(' ');

	    return(TRUE);
	}

	encwrite((char *) def_array, sizeof(def_array), fd);
	close(fd);
	return(TRUE);
}

void
do_getplayer()
{
	if (char_type == C_NOTSET)
	    do
	    {
		/* See what type character will be */

		mvwaddstr(hw, 3, 0, "[a] Fighter\t"
				    "[b] Paladin\t"
				    "[c] Ranger\n"
				    "[d] Cleric\t"
				    "[e] Druid\t"
				    "[f] Magician\n"
				    "[g] Illusionist\t"
				    "[h] Thief\t"
				    "[i] Assasin\t"
				    "[j] Ninja");

		mvwaddstr(hw, 0, 0, "What character class do you desire? ");
		wrefresh(hw);
		char_type = readchar() - 'a';

		if (char_type < C_FIGHTER || char_type >= C_MONSTER)
		{
		    wstandout(hw);
		    mvwaddstr(hw, 1, 0, "Please enter a letter from a - j");
		    wstandend(hw);
		    wclrtoeol(hw);
		    wrefresh(hw);
		}
		else
		{
		    mvwaddstr(hw, 1, 0, "");
		    wclrtoeol(hw);
		}
	    }
	    while (char_type < C_FIGHTER || char_type >= C_MONSTER);

   player.t_ctype = char_type;
}

void
print_stored()
{
	int i;
	char	*class;
	char	pbuf[2 * LINELEN];

	wstandout(hw);
	mvwaddstr(hw, 9, 0, "YOUR CURRENT CHARACTERS:");
	wstandend(hw);
	wclrtoeol(hw);

	for (i = 0; i < MAXPDEF; i++)
	{
	    if (def_array[i][I_STR])
	    {
		class = which_class(def_array[i][I_CTYPE]);

		sprintf(pbuf,
		    "%d. (%s): Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d"
		    " Pow: %d Hpt: %d",
		    i + 1,
		    class,
		    def_array[i][I_INTEL],
		    def_array[i][I_STR],
		    def_array[i][I_WISDOM],
		    def_array[i][I_DEXT],
		    def_array[i][I_CONST],
		    def_array[i][I_CHARISMA],
		    def_array[i][I_POWER],
		    def_array[i][I_HPT]);

		mvwaddstr(hw, 11 + i, 0, pbuf);

	    }
	    else
	    {
		sprintf(pbuf, "%d.  ### NONE ###", i + 1);
		mvwaddstr(hw, 11 + i, 0, pbuf);
	    }
	}
}

char *
which_class(int c_class)
{
	char	*class;

	switch (c_class)
	{
	    case C_FIGHTER:   class = "fighter";
	    case C_MAGICIAN:  class = "magician";
	    case C_CLERIC:    class = "cleric";
	    case C_THIEF:     class = "thief";
	    case C_PALADIN:   class = "paladin";
	    case C_RANGER:    class = "ranger";
	    case C_DRUID:     class = "druid";
	    case C_ILLUSION:  class = "illusionist";
	    case C_ASSASIN:   class = "assasin";
	    case C_NINJA:     class = "ninja";
	    default:	      class = "monster";
	}

	return (class);
}
