/*
    encumb.c  -  Stuff to do with encumberence

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

    See the file LICENSE.TXT for full copyright and licensing information.*/
*/

#include "rogue.h"

/*
	updpack()
	    Update his pack weight and adjust fooduse accordingly
*/

void
updpack()
{
	int pack_level; /* 0 empty, 5 full */
	int curcarry = packweight();

	pstats.s_carry = totalenc();	/* update max encumb */
	if (is_carrying(TR_PURSE))
	    pstats.s_carry += 1000;

	foodlev = 0;
	pack_level = ((curcarry + rnd(5) * 5) / pstats.s_carry) - 1;
	switch ((curcarry * 5) / pstats.s_carry) {  /* % of total capacity */
	    case 5:	/* 100 % */
		foodlev++;
	    case 4:	/* 80 % */
		if (rnd(100) < 80)
		    foodlev++;
	    case 3:	/* 60 % */
		if (rnd(100) < 60)
		    foodlev++;
	    case 2:	/* 40 % */
		if (rnd(100) < 40)
		    foodlev++;
	    case 1:	/* 20 % */
		if (rnd(100) < 20)
		    foodlev++;
	    case 0:	/* 0 % */
		foodlev++;
	}
	pstats.s_pack = curcarry;   /* update pack weight */

	if (is_carrying(TR_PURSE))  /* makes pack lighter */
	    foodlev--;
}


/*
 * packweight: Get the total weight of the hero's pack
 */
packweight()
{
	struct linked_list  *pc;
	int weight = 0;

	for (pc = pack; pc != NULL; pc = next(pc)) {
	    struct object   *obj = OBJPTR(pc);

	    weight += itemweight(obj) * obj->o_count;
	}
	if (weight < 0)     /* caused by artifacts or blessed items */
	    weight = 0;

	return (weight);
}


/*
 * itemweight: Get the weight of an object
 */
itemweight(wh)
struct object   *wh;
{
	int weight = wh->o_weight;  /* get base weight */
	int ac;

	switch (wh->o_type) {
	    when    ARMOR:  /* -20% for each plus */
		ac = armors[wh->o_which].a_class - wh->o_ac;
		weight = ((weight * 5) - (weight * ac)) / 5;
	    when    WEAPON:
		if ((wh->o_hplus + wh->o_dplus) > 0)
		    weight /= 2;
	}
	if (wh->o_flags & ISCURSED)
	    weight += weight / 5;   /* +20% for cursed */
	else if (wh->o_flags & ISBLESSED)
	    weight -= weight / 5;   /* -20% for blessed */

	if (weight < 0)
	    weight = 0;
	return (weight);
}


/*
 * playenc: Get hero's carrying ability above norm 50 units per point of STR
 * over 8 300 units per plus on R_CARRYING 1000 units for TR_PURSE
 */
playenc()
{
	int ret_val = (pstats.s_str - 8) * 50;

	if (is_wearing(R_CARRYING))
	    ret_val += ring_value(R_CARRYING) * 300;
	return (ret_val);
}


/*
 * totalenc: Get total weight that the hero can carry
 */
totalenc()
{
	int wtotal = 1500 + playenc();

	switch (hungry_state) {
	    case F_OK:
	    case F_HUNGRY:; /* no change */
	    when F_WEAK:
		wtotal -= wtotal / 10;	/* 10% off weak */
	    when F_FAINT:
		wtotal /= 2;	/* 50% off faint */
	}
	return (wtotal);
}



/*
 * whgtchk: See if the hero can carry his pack
 */

wghtchk()
{
	int dropchk, err = TRUE;
	char	ch;
	int wghtchk();

	inwhgt = TRUE;
	if (pstats.s_pack > pstats.s_carry) {
	    ch = mvwinch(stdscr, hero.y, hero.x);
	    if ((ch != FLOOR && ch != PASSAGE)) {
		extinguish(wghtchk);
		fuse(wghtchk, TRUE, 1, AFTER);
		inwhgt = FALSE;
		return;
	    }
	    extinguish(wghtchk);
	    msg("Your pack is too heavy for you.");
	    do {
		dropchk = drop((struct linked_list *) NULL);
		if (dropchk == FALSE) {
		    mpos = 0;
		    msg("You must drop something.");
		}
		if (dropchk == TRUE)
		    err = FALSE;
	    } while (err);
	}
	inwhgt = FALSE;
}


/*
 * hitweight: Gets the fighting ability according to current weight This
 * returns a  +2 hit for very light pack weight This returns a  +1 hit for
 * light pack weight 0 hit for medium pack weight -1 hit for heavy pack
 * weight -2 hit for very heavy pack weight
 */

hitweight()
{
	return (3 - foodlev);
}
