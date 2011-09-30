/*
    trader.c  -  Anything to do with trading posts
   
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

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include "rogue.h"

static int	effective_purse;/* Paladins have a 10% tithe */
static int	num_transactions;
static int	cur_worth;  /* How much the shop item costs */

/*
 * do_post: Buy and sell things in a trading post
 */
do_post()
{
	bool	bad_letter = FALSE;

	effective_purse = ((player.t_ctype == C_PALADIN) ?
	    (9 * purse / 10) : purse);
	num_transactions = 0;
	cur_worth = 0;

	for (;;) {
	    wclear(hw);
	    if (!open_market())
		return;

	    wstandout(hw);
	    mvwaddstr(hw, 0, COLS / 2 - 30, "Welcome to Friendly Fiend's Flea Market");
	    wstandend(hw);
	    wclrtoeol(hw);
	    trans_line();   /* hw,LINES-2,0,"You have xxx transactions
		     * left"); */
	    if (bad_letter) {
		bad_letter = FALSE;
		wstandout(hw);
		mvwaddstr(hw, 7, 0, "Type 'i' or 'I' to inventory, 'l' to leave, 'b' to buy, or 's' to sell");
		wstandend(hw);
	    }
	    mvwaddstr(hw, 6, 0, "Do you wish to buy, sell, inventory, or leave?");
	    touchwin(hw);
	    wrefresh(hw);

	    switch (readchar()) {
		when 'b':
		    mvwaddstr(hw, 7, 0, "Lets go into the buying section of the store...");
		    touchwin(hw);
		    wrefresh(hw);
		    buy_it('\0', ISNORMAL);
		when 's':
		    mvwaddstr(hw, 7, 0, "Lets go into the selling section of the store...");
		    touchwin(hw);
		    wrefresh(hw);
		    sell_it();
		when 'i':
		    inventory(pack, '*');
		when 'I':
		    inventory(pack, 0);
		when 'l':
		    wclear(hw);
		    wrefresh(hw);
		    return;
		otherwise:
		    bad_letter = TRUE;
	    }
	}
}

buy_it(itemtype, flags)
char    itemtype;
int flags;
{
	int i;
	bool	blessed = flags & ISBLESSED;
	bool	cursed = flags & ISCURSED;
	bool	is_spell = flags & SCR_MAGIC;
	int array_size; /* # of items within type */
	int which_type; /* Which type to buy */
	int which_one;	/* Which one within type */
	int plus_or_minus = 0;	/* for magic items */
	struct magic_item   *magic_array = NULL;
	struct linked_list  *item;
	struct object	*obj;
	char	buf[2 * LINELEN];

buy_more:
	do {
	    array_size = 0;
	    if (itemtype == '\0') {
		mpos = 0;
		mvwaddstr(hw, 11, 0, "WHAT\tTYPE\n! Potion\n? Scroll\n= Ring\n/ Stick\n] Armor\n) Weapon\n: Food");
		if (wizard)
		    mvwaddstr(hw, 19, 0, ", Artifact");
		mvwaddstr(hw, 9, 0, "What type of item do you want? ");
		touchwin(hw);
		wrefresh(hw);
		itemtype = readchar();
	    }
	    switch (itemtype) {
		when	POTION:
		    which_type = TYP_POTION;
		    array_size = maxpotions;
		    magic_array = p_magic;
		when	SCROLL:
		    which_type = TYP_SCROLL;
		    array_size = maxscrolls;
		    magic_array = s_magic;
		when	FOOD:
		    which_type = TYP_FOOD;
		    array_size = maxfoods;
		    magic_array = fd_data;
		when	WEAPON:
		    which_type = TYP_WEAPON;
		    array_size = maxweapons;
		when	ARMOR:
		    which_type = TYP_ARMOR;
		    array_size = maxarmors;
		when	RING:
		    which_type = TYP_RING;
		    array_size = maxrings;
		    magic_array = r_magic;
		when	STICK:
		    which_type = TYP_STICK;
		    array_size = maxsticks;
		    magic_array = ws_magic;
		when	ARTIFACT:
		    if (!wizard) {
			itemtype = '\0';
			continue;
		    }
		    which_type = TYP_ARTIFACT;
		    array_size = maxartifact;
		when	ESCAPE:
		    return;
		otherwise:
		    wstandout(hw);
		    mvwaddstr(hw, 10, 0, "We don't stock any of those.");
		    wstandend(hw);
		    itemtype = '\0';
		    continue;
	    }
	} while (array_size == 0);

	which_one = array_size;
	do {
	    struct magic_item	*m_item;

	    mpos = 0;
	    sprintf(buf, "Which kind of %s do you wish to have (* for list)? ",
		things[which_type].mi_name);
	    mvwaddstr(hw, 9, 0, buf);
	    touchwin(hw);
	    wrefresh(hw);
	    buf[0] = '\0';
	    switch (get_str(buf, hw)) {
		when	QUIT:
		case ESCAPE:
		    itemtype = '\0';
		    goto buy_more;
	    }
	    if (buf[0] == '*') {    /* print list */
		add_line(" ID  BASECOST NAME");
		switch (which_type) {
		    when    TYP_RING:
		    case TYP_POTION:
		    case TYP_STICK:
		    case TYP_SCROLL:
		    case TYP_FOOD:
			for (i = 0, m_item = magic_array; i < array_size; i++, m_item++)
			    if (!is_spell && m_item->mi_worth > 0) {
				sprintf(buf, "%3d) %8d %s", i, m_item->mi_worth, m_item->mi_name);
				add_line(buf);
			    }
		    when    TYP_ARMOR:
			for (i = 0; i < array_size; i++)
			    if (!is_spell && armors[i].a_worth > 0) {
				sprintf(buf, "%3d) %8d %s", i, armors[i].a_worth, armors[i].a_name);
				add_line(buf);
			    }
		    when    TYP_WEAPON:
			for (i = 0; i < array_size; i++)
			    if (!is_spell && weaps[i].w_worth > 0) {
				sprintf(buf, "%3d) %8d %s", i, weaps[i].w_worth, weaps[i].w_name);
				add_line(buf);
			    }
		    when    TYP_ARTIFACT:
			for (i = 0; i < array_size; i++) {
			    sprintf(buf, "%3d) %8d %s", i, arts[i].ar_worth, arts[i].ar_name);
			    add_line(buf);
			}
		    otherwise:
			add_line("What a strange type.");
		}
		end_line();
		touchwin(hw);
		wrefresh(hw);
		continue;
	    }
	    if (isdigit(buf[0]))
		which_one = atoi(buf);
	    else
		switch (which_type) {
		    when    TYP_RING:
		    case TYP_POTION:
		    case TYP_STICK:
		    case TYP_SCROLL:
		    case TYP_FOOD:
			for (i = 0, m_item = magic_array; i < array_size; i++, m_item++)
			    if (strcmp(buf, m_item->mi_name) == 0)
				which_one = i;
		    when    TYP_ARMOR:
			for (i = 0; i < array_size; i++)
			    if (strcmp(buf, armors[i].a_name) == 0)
				which_one = i;
		    when    TYP_WEAPON:
			for (i = 0; i < array_size; i++)
			    if (strcmp(buf, weaps[i].w_name) == 0)
				which_one = i;
		    when    TYP_ARTIFACT:
			for (i = 0; i < array_size; i++)
			    if (strcmp(buf, arts[i].ar_name) == 0)
				which_one = i;
		    otherwise:
			msg("What a strange type.");
		}
	    if (which_one < 0 || which_one >= array_size) {
		wstandout(hw);
		mvwaddstr(hw, 10, 0, "Type the name or an ID number.");
		wstandend(hw);
	    }

	} while (which_one < 0 || which_one >= array_size);

	item = new_item(sizeof *obj);
	obj = OBJPTR(item);
	if (which_type == TYP_ARTIFACT) {
	    new_artifact(which_one, obj);
	    add_pack(item, NOMESSAGE);
	    itemtype = '\0';
	    goto buy_more;
	}
	obj->o_type = itemtype;
	obj->o_which = which_one;
	obj->o_mark[0] = '\0';
	obj->o_group = 0;
	obj->o_count = 1;
	obj->o_weight = 0;
	obj->o_dplus = obj->o_hplus = 0;

	if (!is_spell) {
	    plus_or_minus = -100;
	    do {
		mvwaddstr(hw, 10, 0, "Do you want the cursed, blessed, or normal version? (c, b, n) [n]");
		touchwin(hw);
		wrefresh(hw);
		switch (readchar()) {
		    when    ESCAPE:
			discard(item);
			itemtype = '\0';
			goto buy_more;
		    when 'c':
			cursed = TRUE;
			plus_or_minus = 0;
		    when 'b':
			blessed = TRUE;
			plus_or_minus = 0;
		    when 'n':
		    case ' ':
			plus_or_minus = 0;
		    otherwise:
			wstandout(hw);
			mvwaddstr(hw, 11, 0, "Type 'c' for cursed, 'b' for blessed, or 'n' for normal");
			wstandend(hw);
		}

	    } while (plus_or_minus == -100);
	}
	/* else used blessed, cursed from flags parameter */

	if (which_type == TYP_WEAPON)
	    init_weapon(obj, which_one);

	obj->o_flags |= ISKNOW;
	if (cursed) {
	    plus_or_minus = -(rnd(2) + 1);
	    obj->o_flags |= ISCURSED;
	}
	else if (blessed) {
	    plus_or_minus = (rnd(3) + 1);
	    obj->o_flags |= ISBLESSED;
	}
	else {
	    plus_or_minus = 0;
	    obj->o_flags |= ISNORMAL;
	}

	switch (which_type) {
	    when    TYP_WEAPON:
		obj->o_hplus += plus_or_minus;
		obj->o_dplus += plus_or_minus;
	    when    TYP_ARMOR:
		obj->o_weight = armors[which_one].a_wght;
		obj->o_ac = armors[which_one].a_class - plus_or_minus;
	    when    TYP_STICK:
		fix_stick(obj);
	    when    TYP_RING:
		obj->o_ac = plus_or_minus;
	    case TYP_SCROLL:
	    case TYP_POTION:
		obj->o_weight = things[which_type].mi_wght;
	    when    TYP_FOOD:
		otherwise:
		msg("That's a strange thing to try to own.");
		itemtype = '\0';
		goto buy_more;
	}

	cur_worth = get_worth(obj) * (luck + level / 15 + 1);
	describe_it(obj);
	if (cur_worth > effective_purse) {
	    wstandout(hw);
	    mvwaddstr(hw, 12, 0, "Unfortunately, you can't afford it.");
	    wstandend(hw);
	    wclrtoeol(hw);
	    touchwin(hw);
	    wrefresh(hw);
	    itemtype = '\0';
	    goto buy_more;
	}

	mvwaddstr(hw, 12, 0, "Do you want it? [y] ");
	wclrtoeol(hw);
	touchwin(hw);
	wrefresh(hw);
	switch (readchar()) {
	    when    ESCAPE:
	    case 'n':
		msg("");
		itemtype = '\0';
		goto buy_more;
	}

	/*
	 * The hero bought the item here
	 */
	mpos = 0;
	if (add_pack(item, NOMESSAGE) && !is_spell) {
	    purse -= cur_worth; /* take his money */
	    effective_purse -= cur_worth;
	    ++num_transactions;
	    trans_line();   /* show remaining deals */
	    switch (which_type) {
		when	TYP_RING:
		case TYP_STICK:
		case TYP_SCROLL:
		case TYP_POTION:
		    know_items[which_type][which_one] = TRUE;
	    }
	}
}

/*
 * sell_it: Sell an item to the trading post
 */
sell_it()
{
	struct object	*obj;
	struct linked_list  *item;
	char	buf[2 * LINELEN];

	if ((item = get_item("sell", 0)) == NULL)
	    return;
	obj = OBJPTR(item);
	msg("");
	touchwin(hw);
	wrefresh(hw);

	if ((obj->o_type == ARTIFACT) || (cur_worth = get_worth(obj)) == 0) {
	    mpos = 0;
	    msg("We don't buy those.");
	    if (is_wearing(R_ADORNMENT) && rnd(10) < 4)
		msg("How about that %s ring instead?", r_stones[R_ADORNMENT]);
	    return;
	}
	describe_it(obj);
	mvwaddstr(hw, 12, 0, "Do you want to sell it? [n] ");
	touchwin(hw);
	wrefresh(hw);
	switch (readchar()) {
	    case 'y':
		break;
	    otherwise:
		msg("");
		if (is_wearing(R_ADORNMENT))
		    msg("How about that %s ring instead?",
			r_stones[R_ADORNMENT]);
		return;
	}

	rem_pack(obj);
	discard(item);
	purse += cur_worth; /* give him his money */
	++num_transactions;
	effective_purse += cur_worth;
	sprintf(buf, "Sold %s.	Hit space to continue.",
	    inv_name(obj, LOWERCASE));
	mvwaddstr(hw, 13, 0, buf);
	touchwin(hw);
	wrefresh(hw);
	wait_for(' ');
}

/*
 * describe_it: Laud or condemn the object
 */
describe_it(obj)
struct object   *obj;
{
	static char *cursed_d[] = {
	    "worthless hunk of junk",
	    "shoddy piece of trash",
	    "piece of rusty garbage",
	    "example of terrible workmanship",
	    "cheap hack"
	};
	static char *normal_d[] = {
	    "journeyman's piece",
	    "fine deal",
	    "great bargain",
	    "good find",
	    "real value",
	    "piece of honest workmanship",
	    "steal",
	    "purchase worth making",
	    "inexpensive product"
	};
	static char *blessed_d[] = {
	    "magnificant masterpiece",
	    "quality product",
	    "exceptional find",
	    "unbeatable value",
	    "rare beauty",
	    "superior product",
	    "well-crafted item"
	};
	char	*charp;
	char	buf[2 * LINELEN];

	if (obj->o_flags & ISBLESSED)
	    charp = blessed_d[rnd(sizeof(blessed_d) / sizeof(char *))];
	else if (obj->o_flags & ISCURSED)
	    charp = cursed_d[rnd(sizeof(cursed_d) / sizeof(char *))];
	else
	    charp = normal_d[rnd(sizeof(normal_d) / sizeof(char *))];

	sprintf(buf, "It's a%s %s worth %d pieces of gold.",
	    vowelstr(charp), charp, cur_worth);
	mvwaddstr(hw, 11, 0, buf);
	wclrtoeol(hw);
}

/*
 * open_market: Retruns TRUE when ok do to transacting
 */
open_market()
{
	int maxtrans = is_wearing(R_ADORNMENT) ? MAXPURCH + 4 : MAXPURCH;

	if (wizard || num_transactions < maxtrans || (level == 0))
	    return TRUE;
	else {
	    msg("The market is closed. The stairs are that-a-way.");
	    return FALSE;
	}
}

/*
 * get_worth: Calculate an objects worth in gold
 */
get_worth(obj)
struct object   *obj;
{
	long	worth = 0;
	int wh = obj->o_which;
	int blessed = obj->o_flags & ISBLESSED;
	int cursed = obj->o_flags & ISCURSED;

	switch (obj->o_type) {
	    when    FOOD:
		if (wh < maxfoods) {
		    worth = obj->o_count * fd_data[wh].mi_worth;
		    if (blessed)
			worth *= 2;
		}
	    when    WEAPON:
		if (wh < maxweapons) {
		    worth = weaps[wh].w_worth;
		    worth *= obj->o_count * (2 +
			    (4 * obj->o_hplus +
			    4 * obj->o_dplus));
		    if (obj->o_flags & ISSILVER)
			worth *= 2;
		    if (obj->o_flags & ISPOISON)
			worth *= 2;
		    if (obj->o_flags & ISZAPPED)
			worth += 20 * obj->o_charges;
		}
	    when    ARMOR:
		if (wh < maxarmors) {
		    int plusses = armors[wh].a_class -
			obj->o_ac;

		    worth = armors[wh].a_worth;
		    if (plusses > 0)
			worth *= (1 + (10 *
			    (armors[wh].a_class -
			    obj->o_ac)));
		}
	    when    SCROLL:
		if (wh < maxscrolls)
		worth = s_magic[wh].mi_worth;
	    when    POTION:
		if (wh < maxpotions)
		    worth = p_magic[wh].mi_worth;
	    when    RING:
		if (wh < maxrings) {
		    worth = r_magic[wh].mi_worth;
		    worth += obj->o_ac * 40;
		}
	    when    STICK:
		if (wh < maxsticks) {
		    worth = ws_magic[wh].mi_worth;
		    worth += 20 * obj->o_charges;
		}
	    when    ARTIFACT:
		if (wh < maxartifact)
		    worth = arts[wh].ar_worth;
	    otherwise:
		worth = 0;
	}

	if (obj->o_flags & ISPROT)  /* 300% more for protected */
	    worth *= 3;

	if (blessed)	    /* 50% more for blessed */
	    worth = 3 * worth / 2;
	else if (cursed)    /* half for cursed */
	    worth /= 2;

	if (obj->o_flags & (CANRETURN | ISOWNED))
	    worth *= 4;
	else if (obj->o_flags & CANRETURN)
	    worth *= 2;
	else if (obj->o_flags & ISLOST)
	    worth /= 3;

	return (max(0, worth));
}

/*
 * trans_line: Show how many transactions the hero has left
 */
trans_line()
{
	char	buf[2 * LINELEN];
	int adorned = is_wearing(R_ADORNMENT);

	if (level == 0 && purse > 0)
	    sprintf(buf, "You still have %d pieces of gold left.", purse);
	else if (purse == 0)
	    sprintf(buf, "You have no money left.");
	else if (!wizard)
	    sprintf(buf, "You have %d transactions and %d gold pieces remaining.",
		max(0, (adorned ? MAXPURCH + 4 : MAXPURCH) - num_transactions), effective_purse);
	else
	    sprintf(buf, "You have infinite transactions remaining.");
	mvwaddstr(hw, LINES - 2, 0, buf);
}
