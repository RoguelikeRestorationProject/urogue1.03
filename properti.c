/*
    properti.c  -  This file contains all the bag related filters and action routines

    Last Modified: Dec 29, 1990   

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

/*
 * baf_decrement_test
 *
 * Assumes the argument is a pointer to int and it just decrements it. Returns
 * TRUE, except when the count goes to zero.
 */
baf_decrement_test(obj_p, count_p)
struct object   *obj_p;
int *count_p;
{
    if (*--count_p > 0)
	return (TRUE);
    return (FALSE);
}

/*
 * baf_identify
 *
 * Bag action function to identify an object. This is needed to conform to bag
 * action routine calling conventions and to put the linked list structure on
 * top of the object before calling whatis()
 */

baf_identify(obj_p, junk)
struct object   *obj_p;
int junk;           /* unwanted arguments */
{
    linked_list l;
    linked_list *lp = &l;

    lp->l_data = (char *) obj_p;    /* stuff object in the right place */
    whatis(lp);
    return (TRUE);
}

/*
 * baf_increment
 *
 * Assumes the argument is a pointer to int and it just increments it and
 * returns TRUE
 */
baf_increment(obj_p, count_p)
object  *obj_p;
int *count_p;
{
    (*count_p)++;
    return (TRUE);
}

/*
 * baf_print_item
 *
 * Bag action function to print a single item, inventory style.
 */
baf_print_item(obj_p, type, id)
struct object   *obj_p;     /* thingy getting listed */
char    type;           /* character representing item (or NULL) */
char    id;         /* small number identifying item */
{
    char    inv_temp[3 * LINELEN];  /* plenty of space for paranoid
		     * programmers */

    extern char print_letters[];

    if (type == 0)
	sprintf(inv_temp, "%c%c) %s", obj_p->o_type,
	      print_letters[id], inv_name(obj_p, LOWERCASE), FALSE);
    else
	sprintf(inv_temp, "%c) %s", print_letters[id],
	    inv_name(obj_p, LOWERCASE), FALSE);
    add_line(inv_temp);
    return (TRUE);
}

/*
 * bff_group
 *
 * This bag filter function checks to see if two items can be combined by
 * adjusting the count. Grouped items can be combined if the group numbers
 * match. The only other item that is allowed to have a count is food, and
 * there an exact match is required.
 */
bff_group(obj_p, new_obj_p)
struct object   *obj_p;     /* object in pack */
struct object   *new_obj_p; /* new object being added */
{
    if (new_obj_p->o_group > 0 && new_obj_p->o_group == obj_p->o_group)
	return (TRUE);
    if (new_obj_p->o_type == FOOD &&
	obj_p->o_type == new_obj_p->o_type &&
	obj_p->o_which == new_obj_p->o_which)
	return (TRUE);
    return (FALSE);
}

/*
 * bff_callable
 *
 * Figures out which items can be callable: current rules are: potions, scrolls,
 * staffs, and rings.
 */

bff_callable(obj_p, junk)
struct object   *obj_p;
int junk;
{
    if (obj_p->o_type == POTION || obj_p->o_type == RING ||
	obj_p->o_type == STICK || obj_p->o_type == SCROLL)
	return (TRUE);
    return (FALSE);
}

/*
 * bff_markable
 *
 * Selects which items can be marked. Current rules exclude only gold.
 */

bff_markable(obj_p, junk)
struct object   *obj_p;
int junk;
{
    if (obj_p->o_type == GOLD)
	return (FALSE);
    return (TRUE);
}

/*
 * bffron
 *
 * returns TRUE if hero is wearing this ring
 */
bffron(obj_p, junk)
object  *obj_p;
int junk;
{
    return (cur_ring[LEFT_1] == obj_p || cur_ring[LEFT_2] == obj_p ||
	cur_ring[LEFT_3] == obj_p || cur_ring[LEFT_4] == obj_p ||
	cur_ring[LEFT_5] ||
	cur_ring[RIGHT_1] == obj_p || cur_ring[RIGHT_2] == obj_p ||
	cur_ring[RIGHT_3] == obj_p || cur_ring[RIGHT_4] == obj_p ||
	cur_ring[RIGHT_5]);
}

/*
 * bffroff
 *
 * returns TRUE if hero isn't wearing this ring
 */

bffroff(obj_p, junk)
object  *obj_p;
int junk;
{
    return (!bffron(obj_p, junk));
}

/*
 * bff_zappable
 *
 * Selects which items can be zapped. This includes both sticks and magically
 * enhanced weapons with lightning ability.
 */
bff_zappable(obj_p, junk)
struct object   *obj_p;
int junk;
{
    if (obj_p->o_type == STICK)
	return (TRUE);
    if (obj_p->o_type == WEAPON && obj_p->o_flags & ISZAPPED)
	return (TRUE);
    return (FALSE);
}

/*
 * baf_curse
 *
 * Curse all non-artifact items in the player's pack
 */

int
baf_curse(struct object *obj_p, int junk)
{
    if (obj_p->o_type != ARTIFACT && rnd(8) == 0) {
	obj_p->o_flags |= ISCURSED;
	obj_p->o_flags &= ~ISBLESSED;
    }
    return (TRUE);
}

/*
 * bag action routine to fetch the current weapon
 */

int
bafcweapon(struct object *obj_p, int junk)
{
    if (obj_p == cur_weapon)
	return (FALSE); /* found what we wanted - stop and return it */
    return (TRUE);
}

/*
 * bag action routine to fetch the current armor
 */

int
bafcarmor(struct object *obj_p, int junk)
{
    if (obj_p == cur_armor)
	return (FALSE); /* found what we wanted - stop and return it */
    return (TRUE);
}
