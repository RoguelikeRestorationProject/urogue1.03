/*
    ident.c  -  This file contains routines to associate an identifier with an object.

    Last Modified: Dec 29, 1990   

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
 * ident
 *
 * This file contains routines to associate an identifier with an object. The
 * identifiers are organized by type. Once an identifier is attached to an
 * object, it remains with that object until the object is removed from the
 * game. The identifiers are small integers, and they are assigned merely by
 * counting objects of the same type. Allocation picks the next available
 * integer.
 *
 * It is required that the linked list be sorted within types so that gaps can
 * easily be detected.
 */

#include "rogue.h"

/*
 * Index of 0 is invalid (unused state)
 */

char    print_letters[] = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

linked_list *ident_list = NULL; /* master list of all items */

/*
 * get_ident()
 *
 * Gets the identifier for the given object. If an identifier exists, it is
 * returned. If not, one is allocated and returned to the user. The
 * identifier remains constant as long as the object is in the game.
 */
get_ident(obj_p)
struct object   *obj_p;
{
    int obj_type = obj_p->o_type;
    linked_list *list_p;    /* pointer into ident_list */
    int new_id = 1;     /* in case we have to allocate */
    struct object   *tmp_obj_p;
    struct linked_list  *new_place_p = NULL;


    if (identifier(obj_p) != 0)
	return (identifier(obj_p));

    /*
     * no identifier - must allocate one
     */

    for (list_p = ident_list; list_p != NULL; list_p = next(list_p)) {
	tmp_obj_p = OBJPTR(list_p);
	if (tmp_obj_p->o_type == obj_type) {
	    if (identifier(tmp_obj_p) == new_id) {
		/* if this id is taken, try next */
		new_place_p = list_p;
		new_id++;
	    }
	}
    }

    /*
     * If we get here, the object is not in the list, and we need to add
     * it. The proper id is in new_id, and the place to put it is right
     * after new_place_p.
     */
    list_p = (struct linked_list *) new(sizeof(*list_p));
    _attach_after(&ident_list, new_place_p, list_p);
    identifier(obj_p) = new_id;
    list_p->l_data = (char *) obj_p;
    return (new_id);
}

/*
 * get_obj()
 *
 * get the object pointer, given the type and identifier.
 *
 * Returns NULL if no match was found
 */

object  *
get_obj(type, ident)
int type, ident;
{
    linked_list *list_p;
    object  *obj_p;

    for (list_p = ident_list; list_p != NULL; list_p = next(list_p)) {
	obj_p = OBJPTR(list_p);
	if (obj_p->o_type == type && identifier(obj_p) == ident)
	    return (obj_p);
    }
    return (NULL);
}

/*
 * free_ident()
 *
 * Frees up an identifier by removing the list entry that contains that item. If
 * the item isn't found, nothing is done.
 */
free_ident(obj_p)
struct object   *obj_p;
{
    linked_list *list_p;


    for (list_p = ident_list; list_p != NULL; list_p = next(list_p)) {
	if (obj_p == OBJPTR(list_p)) {
	    _detach(&ident_list, list_p);   /* unlink it from the
			     * list */
	    ur_free(list_p);    /* release link structure */
	}
    }
}

/*
 * unprint_id
 *
 * Converts a printable id from print_letters to the real thing by getting the
 * index.
 */
unprint_id(print_id)
char    print_id;
{
    char    *id_p;

    for (id_p = print_letters; id_p != NULL; id_p++)
	if (*id_p == print_id)
	    break;
    return (id_p - print_letters);
}

/*
 * max_print
 *
 * returns the size of the print list
 */

max_print()
{
    return (sizeof(print_letters) - 2); /* 1 for blank and 1 for EOS
			 * string */
}
