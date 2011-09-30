/*
    bag.c  -  new bag functions

    Last Modified: Dec 27, 1990   

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
 * new bag functions
 *
 * This is a simple version of bag.c that uses linked lists to perform the bag
 * functions. The bag is just a linked list of objects (struct object) to be
 * specific, but most of that is supposed to be hidden from the user, who
 * should access the bag only through the functions presented here.
 */

#include "rogue.h"

/*
 * apply_to_bag
 *
 * This is the general bag manipulation routine. The bag is subjected to
 * selection criteria and those objects which pass are processed by an action
 * routine. The two criteria are type and filter function. The filter
 * function returns TRUE if the object passes and FALSE otherwise. The filter
 * function is passed the object and the user-supplied argument. This gives
 * the user plenty of flexibility in determining which items will be
 * processed. The action routine is passed the object, the id, and the
 * user-supplied argument given to apply_to_bag. Specifying NULL for either
 * the type or filter function means that criterion always selects. A NULL
 * action routine means no processing is done and the first object which
 * passes the filter is returned to the user. The action routine returns TRUE
 * if processing should continue or FALSE if the current item should be
 * returned to the caller.
 *
 * Returns NULL if the bag is empty or if nothing qualified.
 */

struct object   *
apply_to_bag(bag_p, type, bff_p, baf_p, user_arg)
linked_list *bag_p;     /* linked list of objects */
int type;           /* what is its type (ARMOR, ...) */
int     (*bff_p) ();        /* bag filter function */
int     (*baf_p) ();        /* bag action routine */
long    user_arg;       /* user argument for filter, action */
{
    struct object   *bag_obj_p = NULL;  /* qualifying object */
    struct object   *cur_obj_p; /* current object */

    if (bag_p == NULL)
	return (NULL);
    for (; bag_p != NULL; bag_p = next(bag_p)) {
	cur_obj_p = OBJPTR(bag_p);
	if (type != 0 && type != cur_obj_p->o_type)
	    continue;
	if (bff_p != NULL && !(*bff_p)(cur_obj_p, user_arg))
	    continue;

	/*
	 * At this point, we have an object which qualifies for
	 * processing
	 */
	bag_obj_p = cur_obj_p;  /* in case the user wants it */
	if (baf_p != NULL && (*baf_p)(cur_obj_p, user_arg,
	    identifier(bag_obj_p)))
	    continue;

	/*
	 * We have an object which qualifies, quit now!
	 */
	break;
    }
    if (bag_p == NULL)
	return (NULL);
    return (bag_obj_p);
}

/*
 * count_bag
 *
 * Counts up all bag items which meet the selection criteria
 */
count_bag(bag_p, type, bff_p)
linked_list *bag_p;
int type;
int     (*bff_p) ();
{
    int count = 0;
    int baf_increment();

    apply_to_bag(bag_p, type, bff_p, baf_increment, &count);
    return (count);
}

/*
 * del_bag
 *
 * Removes an object from a bag and throws it away.
 */
del_bag(bag_p, obj_p)
linked_list *bag_p;
object  *obj_p;
{
    object  *pop_bag();

    pop_bag(bag_p, obj_p);  /* get the thing from the bag */
    ur_free(obj_p);     /* release the memory */
}

/*
 * pop_bag
 *
 * Removes an item from a bag and returns it to the user. If the item is not in
 * the bag, return NULL.
 */
struct object   *
pop_bag(bag_pp, obj_p)
linked_list **bag_pp;
object  *obj_p;
{
    linked_list *item_p;

    for (item_p = *bag_pp; item_p != NULL && OBJPTR(item_p) != obj_p;
	 item_p = next(item_p));
    if (item_p == NULL)
	return (NULL);
    _detach(bag_pp, item_p);
    return (obj_p);
}

/*
 * push_bag
 *
 * stuff another item into the bag
 */
push_bag(bag_pp, obj_p)
linked_list **bag_pp;
object  *obj_p;
{
    linked_list *item_p;
    linked_list *new_p;
    linked_list *best_p = NULL;

    new_p = (linked_list *) new(sizeof(*new_p));
    new_p->l_data = (char *) obj_p; /* attach our object */
    identifier(obj_p) = get_ident(obj_p);   /* tag this object for
			 * inventory */

    /*
     * Find a place in the bag - try to match the type, then sort by
     * identifier
     */
    for (item_p = *bag_pp; item_p != NULL; item_p = next(item_p)) {
	if ((OBJPTR(item_p))->o_type == obj_p->o_type) {
	    if (best_p == NULL)
		best_p = item_p;
	    else if (identifier((OBJPTR(item_p))) >
		identifier((OBJPTR(best_p))) &&
		identifier((OBJPTR(item_p))) <
		identifier(obj_p))
		best_p = item_p;
	}
    }
    _attach_after(bag_pp, best_p, new_p);   /* stuff it in the list */
}

/*
 * scan_bag
 *
 * Gets the object from the bag that matches the type and id. The object is not
 * removed from the bag.
 */
struct object   *
scan_bag(bag_p, type, id)
linked_list *bag_p;
int type;           /* type: armor, scroll, ... */
char    id;         /* integer identifier */
{
    object  *obj_p;

    for (; bag_p != NULL; bag_p = next(bag_p)) {
	obj_p = OBJPTR(bag_p);
	if (obj_p->o_type == type && identifier(obj_p) == id)
	    break;
    }
    if (bag_p == NULL)
	return (NULL);
    return (obj_p);
}

/*
 * select_bag
 *
 * Selects the Nth occurrance of an item from a bag that matches the type and
 * filter test. Returns NULL if the item doesn't exist.
 */
struct object   *
select_bag(bag_p, type, bff_p, index)
linked_list *bag_p;
int type;
int     (*bff_p) ();        /* bag filter function */
int index;          /* which item to grab */

{
    int baf_decrement_test();

    return (apply_to_bag(bag_p, type, bff_p, baf_decrement_test, &index));
}
