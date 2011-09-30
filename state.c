/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005, 2007 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_MSTATS       0xABCD0002
#define RSID_THING        0xABCD0003
#define RSID_OBJECT       0xABCD0004
#define RSID_MAGICITEMS   0xABCD0005
#define RSID_KNOWS        0xABCD0006
#define RSID_GUESSES      0xABCD0007
#define RSID_OBJECTLIST   0xABCD0008
#define RSID_BAGOBJECT    0xABCD0009
#define RSID_MONSTERLIST  0xABCD000A
#define RSID_MONSTERSTATS 0xABCD000B
#define RSID_MONSTERS     0xABCD000C
#define RSID_TRAP         0xABCD000D
#define RSID_WINDOW       0xABCD000E
#define RSID_DAEMONS      0xABCD000F
#define RSID_STICKS       0xABCD0010
#define RSID_IARMOR       0xABCD0011
#define RSID_SPELLS       0xABCD0012
#define RSID_ILIST        0xABCD0013
#define RSID_HLIST        0xABCD0014
#define RSID_DEATHTYPE    0xABCD0015
#define RSID_CTYPES       0XABCD0016
#define RSID_COORDLIST    0XABCD0017
#define RSID_ROOMS        0XABCD0018
#define RSID_ARTIFACT     0xABCD0019

#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "rogue.h"

void rs_write_object_list(EFILE *efp, struct linked_list *l);
void rs_read_object_list(EFILE *efp, struct linked_list **list);

/* we keep these to get some type checking enforced */

void 
rs_write_char(EFILE *efp, char c)           
{ 
	efwriten(&c, 1, efp); 
}

void rs_read_char(EFILE *efp, char *c)             
{ 
	efreadn(c, 1, efp); 
}

void
rs_write_int(EFILE *efp, int c)
{
	efwriten(&c, 4, efp);
}

void 
rs_read_int(EFILE *efp, int *i)
{ 
	efreadn(i, 4, efp); 
}

void
rs_write_uint(EFILE *efp, unsigned int c)
{
	efwriten(&c, 4, efp);
}

void 
rs_read_uint(EFILE *efp, unsigned int *i)               
{ 
	efreadn(i, 4, efp); 
}

void 
rs_write_chars(EFILE *efp, char *c, int nitems) 
{ 
	efwriten(&nitems, 4, efp);
	efwrite(c,1,nitems,efp); 
}

void 
rs_read_chars(EFILE *efp, char *i, int nitems)
{ 
	int count;

	efreadn(&count, 4, efp);

	if (!eferror(efp) && nitems != count)
		efseterr(efp, EILSEQ);

	efread(i,1,count,efp); 
}

void
rs_write_marker(EFILE *efp, int id)
{
    rs_write_int(efp, id);
}

void
rs_read_marker(EFILE *efp, int id)
{
    int nid;

    rs_read_int(efp, &nid);

	if (!eferror(efp) && (id != nid))
		efseterr(efp, EILSEQ);
}

/******************************************************************************/

void *
get_list_item(struct linked_list *l, int i)
{
    int count;

    for(count = 0; l != NULL; count++, l = l->l_next)
        if (count == i)
		    return(l->l_data);
    
    return(NULL);
}

int
find_list_ptr(struct linked_list *l, void *ptr)
{
    int count;

    for(count = 0; l != NULL; count++, l = l->l_next)
        if (l->l_data == ptr)
            return(count);
    
    return(-1);
}

int
list_size(struct linked_list *l)
{
    int count;
    
    for(count = 0; l != NULL; count++, l = l->l_next)
        ;
    
    return(count);
}

/******************************************************************************/

void
rs_write_string(EFILE *efp, char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : (int) strlen(s) + 1;

	efwriten(&len,4,efp);
	efwrite(s,1,len,efp);
}

void
rs_read_string(EFILE *efp, char *s, int max)
{
    int len = 0;

    efreadn(&len, 4, efp);

	if (!eferror(efp) && (len > max))
		efseterr(efp, EILSEQ);
	else
		efread(s,1,len,efp);
}

void
rs_read_new_string(EFILE *efp, char **s)
{
    int len=0;
    char *buf=0;

    efreadn(&len, 4, efp);

	if (eferror(efp))
		return;

    if (len == 0)
        buf = NULL;
    else
    { 
        buf = malloc(len);

        if (buf == NULL)            
            efseterr(efp, ENOMEM);
    }

    efread(buf,1,len,efp);

    *s = buf;
}

void
rs_write_string_index(EFILE *efp, char *master[], int max, const char *str)
{
    int i;

    for(i = 0; i < max; i++)
        if (str == master[i])
		{
            rs_write_int(efp, i);
			return;
		}

    rs_write_int(efp,-1);
}

void
rs_read_string_index(EFILE *efp, char *master[], int maxindex, char **str)
{
    int i;

    rs_read_int(efp, &i);

	if (!eferror(efp) && (i > maxindex))
		efseterr(efp, EILSEQ);
    else if (i >= 0)
        *str = master[i];
    else
        *str = NULL;
}

void
rs_write_string_cindex(EFILE *efp, char *src, char *dst)
{
	if ((src == NULL) || (dst == NULL) || (dst <= src))
		efseterr(efp, EINVAL);

	rs_write_int(efp, (int)(dst - src));
}

void
rs_read_string_cindex(EFILE *efp, char *src, char **dst)
{
	int ofs;

	rs_read_int(efp, &ofs);

	if (eferror(efp))
		return;

	*dst = src + ofs;

	return;
}

void
rs_write_coord(EFILE *efp, coord c)
{
    rs_write_int(efp, c.x);
    rs_write_int(efp, c.y);
}

void
rs_write_coords(EFILE *efp, coord *c, int count)
{
	int n = 0;

    rs_write_int(efp, count);

	for(n = 0; n < count; n++)
		rs_write_coord(efp, c[n]);
}

void
rs_read_coord(EFILE *efp, coord *c)
{
    coord in;

    rs_read_int(efp,&in.x);
    rs_read_int(efp,&in.y);

    if (!eferror(efp)) 
    {
        c->x = in.x;
        c->y = in.y;
    }
}

void
rs_read_coords(EFILE *efp, coord *c, int count)
{
	int n = 0, value = 0;

	rs_read_int(efp, &value);

	for(n = 0; n < count; n++)
		rs_read_coord(efp, &c[n]);
}

void
rs_write_window(EFILE *efp, WINDOW *win)
{
    int row,col,height,width;

	width  = getmaxx(win);
    height = getmaxy(win);

    rs_write_marker(efp,RSID_WINDOW);
    rs_write_int(efp,height);
    rs_write_int(efp,width);

    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            rs_write_int(efp, mvwinch(win,row,col));
}

void
rs_read_window(EFILE *efp, WINDOW *win)
{
    int row,col,maxlines,maxcols,value,width,height;
    
    width  = getmaxx(win);
    height = getmaxy(win);

    rs_read_marker(efp, RSID_WINDOW);

    rs_read_int(efp, &maxlines);
    rs_read_int(efp, &maxcols);

	if (eferror(efp))
		return;

    for(row = 0; row < maxlines; row++)
        for(col = 0; col < maxcols; col++)
        {
            rs_read_int(efp, &value);

            if ((row < height) && (col < width))
                mvwaddch(win,row,col,value);
        }
}

void
rs_write_levtype(EFILE *efp, LEVTYPE c)
{
    int lt;
    
    switch(c)
    {
        case NORMLEV: lt = 1; break;
        case POSTLEV: lt = 2; break;
        case MAZELEV: lt = 3; break;
        case THRONE: lt = 4; break;
        default: lt = -1; break;
    }
    
    rs_write_int(efp,lt);
}

void
rs_read_levtype(EFILE *efp, LEVTYPE *l)
{
    int lt;
    
    rs_read_int(efp, &lt);

	if (eferror(efp))
		return;

    switch(lt)
    {
        case 1: *l = NORMLEV; break;
        case 2: *l = POSTLEV; break;
        case 3: *l = MAZELEV; break;
        case 4: *l = THRONE; break;
        default: *l = NORMLEV; break;
    }
}

void
rs_write_stats(EFILE *efp, struct stats *s)
{
    rs_write_marker(efp, RSID_STATS);
    rs_write_int(efp, s->s_str);
    rs_write_int(efp, s->s_intel);
    rs_write_int(efp, s->s_wisdom);
    rs_write_int(efp, s->s_dext);
    rs_write_int(efp, s->s_const);
    rs_write_int(efp, s->s_charisma);
    rs_write_uint(efp, s->s_exp);
    rs_write_int(efp, s->s_lvl);
    rs_write_int(efp, s->s_arm);
    rs_write_int(efp, s->s_hpt);
    rs_write_int(efp, s->s_pack);
    rs_write_int(efp, s->s_carry);
    efwrite(s->s_dmg, 1, sizeof(s->s_dmg), efp);
}

void
rs_read_stats(EFILE *efp, struct stats *s)
{
    rs_read_marker(efp, RSID_STATS);
    rs_read_int(efp,&s->s_str);
    rs_read_int(efp,&s->s_intel);
    rs_read_int(efp,&s->s_wisdom);
    rs_read_int(efp,&s->s_dext);
    rs_read_int(efp,&s->s_const);
    rs_read_int(efp,&s->s_charisma);
    rs_read_int(efp,&s->s_exp);
    rs_read_int(efp,&s->s_lvl);
    rs_read_int(efp,&s->s_arm);
    rs_read_int(efp,&s->s_hpt);
    rs_read_int(efp,&s->s_pack);
    rs_read_int(efp,&s->s_carry);
    efread(s->s_dmg,1,sizeof(s->s_dmg),efp);
}

void
rs_write_magic_items(EFILE *efp, struct magic_item *i, int count)
{
    int n;
    
    rs_write_marker(efp, RSID_MAGICITEMS);
    rs_write_int(efp, count);

    for(n = 0; n < count; n++)
        rs_write_int(efp,i[n].mi_prob);
}

void
rs_read_magic_items(EFILE *efp, struct magic_item *mi, int count)
{
    int n;
    int value;

    rs_read_marker(efp, RSID_MAGICITEMS);
    rs_read_int(efp, &value);

	if (!eferror(efp) && (value != count))
		efseterr(efp, EILSEQ);
    else
		for(n = 0; n < value; n++)
		    rs_read_int(efp,&mi[n].mi_prob);
}

void
rs_write_scrolls(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_write_string(efp, s_names[i]);
		rs_write_int(efp,s_know[i]);
        rs_write_string(efp,s_guess[i]);
    }
}

void
rs_read_scrolls(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_read_new_string(efp,&s_names[i]);
        rs_read_int(efp,&s_know[i]);
        rs_read_new_string(efp,&s_guess[i]);
    }
}

void
rs_write_potions(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
		rs_write_string_index(efp,rainbow,NCOLORS,p_colors[i]);
        rs_write_int(efp,p_know[i]);
        rs_write_string(efp,p_guess[i]);
    }
}

void
rs_read_potions(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_read_string_index(efp,rainbow,NCOLORS,&p_colors[i]);
		rs_read_int(efp,&p_know[i]);
        rs_read_new_string(efp,&p_guess[i]);
    }
}

void
rs_write_rings(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
		rs_write_string_index(efp,stones,NSTONES,r_stones[i]);
        rs_write_int(efp,r_know[i]);
        rs_write_string(efp,r_guess[i]);
    }
}

void
rs_read_rings(EFILE *efp)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(efp,stones,NSTONES,&r_stones[i]);
		rs_read_int(efp,&r_know[i]);
        rs_read_new_string(efp,&r_guess[i]);
    }
}

void
rs_write_sticks(EFILE *efp)
{
    int i;

    rs_write_marker(efp, RSID_STICKS);

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_type[i],"staff") == 0)
        {
            rs_write_int(efp,0);
			rs_write_string_index(efp,wood,NWOOD,ws_made[i]);
        }
        else
        {
            rs_write_int(efp,1);
			rs_write_string_index(efp,metal,NMETAL,ws_made[i]);
        }

		rs_write_int(efp, ws_know[i]);
        rs_write_string(efp, ws_guess[i]);
    }
}
        
void
rs_read_sticks(EFILE *efp)
{
    int i = 0, list = 0;

    rs_read_marker(efp, RSID_STICKS);

	if (eferror(efp))
		return;

    for(i = 0; i < MAXSTICKS; i++)
    { 
        rs_read_int(efp,&list);

		if (eferror(efp))
			return;

        ws_made[i] = NULL;

        if (list == 0)
        {
			rs_read_string_index(efp,wood,NWOOD,&ws_made[i]);
            ws_type[i] = "staff";
        }
        else 
        {
			rs_read_string_index(efp,metal,NMETAL,&ws_made[i]);
			ws_type[i] = "wand";
        }

		rs_read_int(efp, &ws_know[i]);
        rs_read_new_string(efp, &ws_guess[i]);
    }
}

void
rs_write_daemons(EFILE *efp, struct delayed_action *d_list, int count)
{
    int i = 0, j = 0;
    int func = 0;
        
    rs_write_marker(efp, RSID_DAEMONS);
    rs_write_int(efp, count);
        
    for(i = 0; i < count; i++)
    {
        if ( d_list[i].d_func == rollwand)
            func = 1;
        else if ( d_list[i].d_func == doctor)
            func = 2;
        else if ( d_list[i].d_func == stomach)
            func = 3;
        else if ( d_list[i].d_func == runners)
            func = 4;
        else if ( d_list[i].d_func == swander)
            func = 5;
        else if ( d_list[i].d_func == unscent)
            func = 6;
        else if ( d_list[i].d_func == unelectrify)
            func = 7;
        else if ( d_list[i].d_func == unshero)
            func = 8;
        else if ( d_list[i].d_func == unbhero)
            func = 9;
        else if ( d_list[i].d_func == unxray)
            func = 10;
        else if ( d_list[i].d_func == wghtchk)
            func = 11;
        else if ( d_list[i].d_func == unstink)
            func = 12;
        else if ( d_list[i].d_func == res_strength)
            func = 13;
        else if ( d_list[i].d_func == un_itch)
            func = 14;
        else if ( d_list[i].d_func == cure_disease)
            func = 15;
        else if ( d_list[i].d_func == unconfuse)
            func = 16;
        else if ( d_list[i].d_func == suffocate)
            func = 17;
        else if ( d_list[i].d_func == undisguise)
            func = 18;
        else if ( d_list[i].d_func == shero)
            func = 19;
        else if ( d_list[i].d_func == hear)
            func = 20;
        else if ( d_list[i].d_func == unhear)
            func = 21;
        else if ( d_list[i].d_func == sight)
            func = 22;
        else if ( d_list[i].d_func == scent)
            func = 23;
        else if ( d_list[i].d_func == nohaste)
            func = 24;
        else if ( d_list[i].d_func == unclrhead)
            func = 25;
        else if ( d_list[i].d_func == unsee)
            func = 26;
        else if ( d_list[i].d_func == unphase)
            func = 27;
        else if (d_list[i].d_func == NULL)
            func = 0;
        else
            func = -1;

        rs_write_int(efp, d_list[i].d_type);
        rs_write_int(efp, func);

		if ( d_list[i].d_arg == 0 )
		{
			rs_write_int(efp, 0);
			rs_write_int(efp, 0);
		}
		else if (d_list[i].d_arg == &player)
		{
			rs_write_int(efp, 0);
			rs_write_int(efp, 1);
		}
		else
		{
			struct object *obj = d_list[i].d_arg;

			j = find_list_ptr(player.t_pack, obj);

		    if (j >= 0)
		    {
			    rs_write_int(efp,1);
				rs_write_int(efp,j);
			}
			else
			{
				j = find_list_ptr(lvl_obj, obj);

				if (j >= 0)
				{
					rs_write_int(efp,2);
					rs_write_int(efp,j);
				}
				else /* not necessarily an error condition, could
						be that player/level no longer has the object
						so this reference would get discarded anyway */
				{
					rs_write_int(efp,0);
					rs_write_int(efp,0);
				}
			}
		}

        rs_write_int(efp, d_list[i].d_time);
    } 
}       

void
rs_read_daemons(EFILE *efp, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    
    rs_read_marker(efp, RSID_DAEMONS);
    rs_read_int(efp, &value);

    if (!eferror(efp) && (value > count))
	{
		efseterr(efp, EILSEQ);
		return;
	}

    for(i=0; i < count; i++)
    {
		func = 0;
        rs_read_int(efp, &d_list[i].d_type);
        rs_read_int(efp, &func);

		if (eferror(efp))
			return;
                    
        switch(func)
        {
			case  1: d_list[i].d_func = rollwand;    break;
            case  2: d_list[i].d_func = doctor;      break;
            case  3: d_list[i].d_func = stomach;     break;
            case  4: d_list[i].d_func = runners;     break;
            case  5: d_list[i].d_func = swander;     break;
            case  6: d_list[i].d_func = unscent;     break;
            case  7: d_list[i].d_func = unelectrify; break;
            case  8: d_list[i].d_func = unshero;     break;
            case  9: d_list[i].d_func = unbhero;     break;
            case 10: d_list[i].d_func = unxray;      break;
            case 11: d_list[i].d_func = wghtchk;     break;
            case 12: d_list[i].d_func = unstink;     break;
            case 13: d_list[i].d_func = res_strength;break;
            case 14: d_list[i].d_func = un_itch;     break;
            case 15: d_list[i].d_func = cure_disease;break;
            case 16: d_list[i].d_func = unconfuse;   break;
            case 17: d_list[i].d_func = suffocate;   break;
            case 18: d_list[i].d_func = undisguise;  break;
            case 19: d_list[i].d_func = shero;       break;
            case 20: d_list[i].d_func = hear;        break;
            case 21: d_list[i].d_func = unhear;      break;
            case 22: d_list[i].d_func = sight;       break;
            case 23: d_list[i].d_func = scent;       break;
            case 24: d_list[i].d_func = nohaste;     break;
            case 25: d_list[i].d_func = unclrhead;   break;
            case 26: d_list[i].d_func = unsee;       break;
            case 27: d_list[i].d_func = unphase;     break;
			case  0:
            case -1:
            default: d_list[i].d_func = NULL;
                     break;
        }   

        rs_read_int(efp, &value);

		if (eferror(efp))
			return;

		if (value == 0) 
		{
			rs_read_int(efp, &value);

			if (eferror(efp))
				return;

			if (value == 1)
				d_list[i].d_arg = &player;
			else
				d_list[i].d_arg = NULL;
		}
		else if (value == 1)
		{
			rs_read_int(efp, &value);

			if (eferror(efp))
				return;

			d_list[i].d_arg = get_list_item(player.t_pack, value);
		}
		else if (value == 2)
		{
			rs_read_int(efp, &value);

			if (eferror(efp))
				return;

			d_list[i].d_arg = get_list_item(lvl_obj, value);
		}
		else
		{
			rs_read_int(efp, &value);
			d_list[i].d_arg = NULL;
        }

        rs_read_int(efp, &d_list[i].d_time);

		if (d_list[i].d_func == NULL) 
		{
			d_list[i].d_time = 0;
			d_list[i].d_arg = 0;
			d_list[i].d_type = 0;
		}
    }
}        

void
rs_write_room(EFILE *efp, struct room *r)
{
    rs_write_coord(efp, r->r_pos);
    rs_write_coord(efp, r->r_max);
    rs_write_uint(efp, r->r_flags);
    rs_write_int(efp, r->r_fires); 
    rs_write_coords(efp, r->r_exit, MAXDOORS);
}

void
rs_read_room(EFILE *efp, struct room *r)
{  
    rs_read_coord(efp,&r->r_pos);
    rs_read_coord(efp,&r->r_max);
    rs_read_uint(efp,&r->r_flags);
    rs_read_int(efp, &r->r_fires);
    rs_read_coords(efp, r->r_exit, MAXDOORS);
}

void
rs_write_rooms(EFILE *efp, struct room r[], int count)
{
    int n = 0;

    rs_write_int(efp, count);
    
    for(n = 0; n < count; n++)
        rs_write_room(efp, &r[n]);
}

void
rs_read_rooms(EFILE *efp, struct room *r, int count)
{
    int value = 0, n = 0;
    
    rs_read_int(efp,&value);

    if (!eferror(efp) && (value > count))
        efseterr(efp, EILSEQ);
	else
		for(n = 0; n < value; n++)
			rs_read_room(efp,&r[n]);
}

void
rs_write_room_reference(EFILE *efp, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(efp, room);
}

void
rs_read_room_reference(EFILE *efp, struct room **rp)
{
    int i;
    
    rs_read_int(efp, &i);

	if (eferror(efp))
		return;

    *rp = &rooms[i];
}

void
rs_write_traps(EFILE *efp, struct trap *trap,int count)
{
    int n;

	rs_write_marker(efp, RSID_DAEMONS);
    rs_write_int(efp, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_int(efp, trap[n].tr_type);
        rs_write_int(efp, trap[n].tr_show);
        rs_write_coord(efp, trap[n].tr_pos);
        rs_write_uint(efp, trap[n].tr_flags);
    }
}

void
rs_read_traps(EFILE *efp, struct trap *trap, int count)
{
    int value = 0, n = 0;

	rs_read_marker(efp, RSID_DAEMONS);
    rs_read_int(efp,&value);

	if (!eferror(efp) && (value != count))
		efseterr(efp, EILSEQ);
    else
		for(n=0;n<value;n++)
        {   
			rs_read_int(efp,&trap[n].tr_type);
            rs_read_int(efp,&trap[n].tr_show);
            rs_read_coord(efp,&trap[n].tr_pos);
            rs_read_uint(efp,&trap[n].tr_flags);
		}  
}

void
rs_write_monsters(EFILE *efp, struct monster *m, int count)
{
    int n;
    
    rs_write_marker(efp, RSID_MONSTERS);
    rs_write_int(efp, count);

    for(n=0;n<count;n++)
    {
        rs_write_int(efp, m[n].m_normal);
        rs_write_int(efp, m[n].m_wander);
        rs_write_int(efp, m[n].m_numsum);
    }
}

void
rs_read_monsters(EFILE *efp, struct monster *m, int count)
{
    int value = 0, n = 0;


    rs_read_marker(efp, RSID_MONSTERS);
    rs_read_int(efp, &value);

    if (!eferror(efp) && value != count)
        efseterr(efp, EILSEQ);

    for(n = 0; n < count; n++)
    {
        rs_read_int(efp, &m[n].m_normal);
        rs_read_int(efp, &m[n].m_wander);
		rs_read_int(efp, &m[n].m_numsum);
    }
}

void
rs_write_artifact(EFILE *efp, struct artifact *a)
{
    rs_write_marker(efp, RSID_ARTIFACT);

	if (a == NULL)
        rs_write_int(efp, 0);
	else
	{
		rs_write_int(efp, 1);
		rs_write_uint(efp, a->ar_flags);
		rs_write_uint(efp, a->ar_rings);
		rs_write_uint(efp, a->ar_potions);
		rs_write_uint(efp, a->ar_scrolls);
		rs_write_uint(efp, a->ar_wands);
		rs_write_object_list(efp, a->t_art);
	}
}

void
rs_read_artifact(EFILE *efp, struct artifact *a)
{
    int index = -1;

    if (a == NULL)
		efseterr(efp, EINVAL);

    rs_read_marker(efp, RSID_ARTIFACT);
    rs_read_int(efp, &index);

    if (index == 0)
        return;

	rs_read_uint(efp, &a->ar_flags);
	rs_read_uint(efp, &a->ar_rings);
	rs_read_uint(efp, &a->ar_potions);
	rs_read_uint(efp, &a->ar_scrolls);
	rs_read_uint(efp, &a->ar_wands);
	rs_read_object_list(efp, &a->t_art);
}

void
rs_write_object(EFILE *efp, struct object *o)
{
    rs_write_marker(efp, RSID_OBJECT);
    rs_write_int(efp, o->o_type);
    rs_write_coord(efp, o->o_pos);
	rs_write_string(efp, o->o_text);
    rs_write_int(efp, o->o_launch);
    efwrite(o->o_damage, 1, sizeof(o->o_damage), efp);
    efwrite(o->o_hurldmg, 1, sizeof(o->o_hurldmg), efp);
    rs_write_int(efp, o->o_count);
    rs_write_int(efp, o->o_which);
    rs_write_int(efp, o->o_hplus);
    rs_write_int(efp, o->o_dplus);
    rs_write_int(efp, o->o_ac);
    rs_write_uint(efp, o->o_flags);
    rs_write_int(efp, o->o_group);
    rs_write_int(efp, o->o_weight);
    efwrite(o->o_mark, 1, MARKLEN, efp);
	rs_write_artifact(efp, &o->art_stats);
}

void
rs_read_object(EFILE *efp, struct object *o)
{
    rs_read_marker(efp, RSID_OBJECT);
    rs_read_int(efp, &o->o_type);
    rs_read_coord(efp, &o->o_pos);
	rs_read_new_string(efp, &o->o_text);
    rs_read_int(efp, &o->o_launch);
    efread(o->o_damage, 1, sizeof(o->o_damage), efp);
    efread(o->o_hurldmg, 1, sizeof(o->o_hurldmg), efp);
    rs_read_int(efp, &o->o_count);
    rs_read_int(efp, &o->o_which);
    rs_read_int(efp, &o->o_hplus);
    rs_read_int(efp, &o->o_dplus);
    rs_read_int(efp, &o->o_ac);
    rs_read_uint(efp,&o->o_flags);
    rs_read_int(efp, &o->o_group);
    rs_read_int(efp, &o->o_weight);
    efread(o->o_mark, 1, MARKLEN, efp);
	rs_read_artifact(efp, &o->art_stats);
}

void
rs_write_object_list(EFILE *efp, struct linked_list *l)
{
    rs_write_marker(efp, RSID_OBJECTLIST);
    rs_write_int(efp, list_size(l));

    for( ;l != NULL; l = l->l_next)
	{
		rs_write_char(efp, l->l_letter);
        rs_write_object(efp, OBJPTR(l));
	}
}

void
rs_read_object_list(EFILE *efp, struct linked_list **list)
{
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    rs_read_marker(efp, RSID_OBJECTLIST);
    rs_read_int(efp, &cnt);

	if (eferror(efp))
		return;

    for (i = 0; i < cnt; i++) 
    {
        l = new_item(sizeof(struct object));

		l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;
		
		rs_read_char(efp, &l->l_letter);
        rs_read_object(efp,OBJPTR(l));

        if (previous == NULL)
            head = l;

        previous = l;
    }
            
    if (l != NULL)
        l->l_next = NULL;
    
    *list = head;
}

void
rs_write_object_reference(EFILE *efp, struct linked_list *list, struct object *item)
{
    int i;

    i = find_list_ptr(list, item);

    rs_write_int(efp, i);
}

void
rs_read_object_reference(EFILE *efp, struct linked_list *list, struct object **item)
{
    int i;

    rs_read_int(efp, &i);

	if (!eferror(efp))
	    *item = get_list_item(list,i);
	else
		*item = NULL;
}

int
find_thing_coord(struct linked_list *monlist, coord *c)
{
    struct linked_list *mitem;
    struct thing *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = THINGPTR(mitem);

        if (c == &tp->t_pos)
            return(i);

        i++;
    }

    return(-1);
}

int
find_object_coord(struct linked_list *objlist, coord *c)
{
    struct linked_list *oitem;
    struct object *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = OBJPTR(oitem);

        if (c == &obj->o_pos)
            return(i);

        i++;
    }

    return(-1);
}

void
rs_write_thing(EFILE *efp, struct thing *t)
{
    int i = -1;

    rs_write_marker(efp, RSID_THING);

    if (t == NULL)
	{
        rs_write_int(efp, 0);
		return;
    }
    
    rs_write_int(efp, 1);
    rs_write_int(efp, t->t_turn);
    rs_write_int(efp, t->t_wasshot);
    rs_write_int(efp, t->t_type);
    rs_write_int(efp, t->t_disguise);
    rs_write_int(efp, t->t_oldch);
    rs_write_int(efp, t->t_ctype);
    rs_write_int(efp, t->t_index);
    rs_write_int(efp, t->t_no_move);
    rs_write_int(efp, t->t_quiet);
    rs_write_int(efp, t->t_doorgoal);
    rs_write_coord(efp, t->t_pos);
    rs_write_coord(efp, t->t_oldpos);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
		0,2: location of shop_door
        0,3: location of shk_pos
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->t_dest == &hero)
    {
        rs_write_int(efp,0);
        rs_write_int(efp,1);
    }
    else if (t->t_dest != NULL)
    {
        i = find_thing_coord(mlist, t->t_dest);
            
        if (i >=0 )
        {
            rs_write_int(efp,1);
            rs_write_int(efp,i);
        }
        else
        {
            i = find_object_coord(lvl_obj, t->t_dest);
            
            if (i >= 0)
            {
                rs_write_int(efp,2);
                rs_write_int(efp,i);
            }
            else
            {
                rs_write_int(efp, 0);
                rs_write_int(efp,1); /* chase the hero anyway */
            }
        }
    }
    else
    {
        rs_write_int(efp,0);
        rs_write_int(efp,0);
    }
    
    rs_write_uint(efp, t->t_flags[0]);
    rs_write_uint(efp, t->t_flags[1]);
    rs_write_uint(efp, t->t_flags[2]);
    rs_write_uint(efp, t->t_flags[3]);
    rs_write_object_list(efp, t->t_pack);
    rs_write_stats(efp, &t->t_stats);
    rs_write_stats(efp, &t->maxstats);
}

void
rs_read_thing(EFILE *efp, struct thing *t)
{
    int listid = 0, index = -1;

    if (t == NULL)
		efseterr(efp, EINVAL);

    rs_read_marker(efp, RSID_THING);
    rs_read_int(efp, &index);

	if (eferror(efp))
		return;

	if (index == 0)
        return;

    rs_read_int(efp, &t->t_turn);
    rs_read_int(efp, &t->t_wasshot);
    rs_read_int(efp, &t->t_type);
    rs_read_int(efp, &t->t_disguise);
    rs_read_int(efp, &t->t_oldch);
    rs_read_int(efp, &t->t_ctype);
    rs_read_int(efp, &t->t_index);
    rs_read_int(efp, &t->t_no_move);
    rs_read_int(efp, &t->t_quiet);
    rs_read_int(efp, &t->t_doorgoal);
    rs_read_coord(efp, &t->t_pos);
    rs_read_coord(efp, &t->t_oldpos);

    /* 
	t_dest can be (listid,index):
	    0,0: NULL
            0,1: location of hero
	    0,2: location of shop_door
            0,3: location of shk_pos
            1,i: location of a thing (monster)
            2,i: location of an object
            3,i: location of gold in a room

		We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */
            
    rs_read_int(efp, &listid);
    rs_read_int(efp, &index);

	if (eferror(efp))
		return;

    t->t_reserved = -1;

    if (listid == 0) /* hero or NULL */
    {
		if (index == 1)
			t->t_dest = &hero;
		else
			t->t_dest = NULL;
    }
    else if (listid == 1) /* monster/thing */
    {
		t->t_dest     = NULL;
        t->t_reserved = index;
    }
    else if (listid == 2) /* object */
    {
		struct object *obj;

        obj = get_list_item(lvl_obj, index);

        if (obj != NULL)
        {
            t->t_dest = &obj->o_pos;
        }
    }
    else
		t->t_dest = NULL;

    rs_read_uint(efp, &t->t_flags[0]);
    rs_read_uint(efp, &t->t_flags[1]);
    rs_read_uint(efp, &t->t_flags[2]);
    rs_read_uint(efp, &t->t_flags[3]);
    rs_read_object_list(efp, &t->t_pack);
    rs_read_stats(efp, &t->t_stats);
    rs_read_stats(efp, &t->maxstats);
}

void
rs_read_new_thing(EFILE *efp, struct thing **t)
{
	struct thing *tp;

    tp = malloc( sizeof(struct thing) );

	if (tp == NULL) 
	{
		efseterr(efp, ENOMEM);
		return;
	}

	tp->t_index = -2;

    rs_read_thing(efp, tp);

	if (tp->t_index == -2) 
	{
		free(tp);
		*t = NULL;
	}
	else
		*t = tp;
}
    
void
rs_fix_thing(struct thing *t)
{
    struct thing *tp;

    if (t == NULL)
		return;

    if (t->t_reserved < 0)
        return;

    tp = get_list_item(mlist,t->t_reserved);

    if (tp != NULL)
        t->t_dest = &tp->t_pos;
}

void
rs_write_thing_list(EFILE *efp, struct linked_list *l)
{
    int cnt = 0;
    
    rs_write_marker(efp, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(efp, cnt);

    while (l != NULL) 
	{
        rs_write_thing(efp, (struct thing *)l->l_data);
		rs_write_char(efp, l->l_letter);
        l = l->l_next;
    }
}

void
rs_read_thing_list(EFILE *efp, struct linked_list **list)
{
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    rs_read_marker(efp, RSID_MONSTERLIST);
    rs_read_int(efp, &cnt);

	if (eferror(efp))
		return;

    for (i = 0; i < cnt; i++) 
    {
        l = creat_item();

        l->l_prev = previous;
            
        if (previous != NULL)
            previous->l_next = l;

        rs_read_new_thing(efp, (struct thing **) &l->l_data);
		rs_read_char(efp, &l->l_letter);
        if (previous == NULL)
            head = l;

        previous = l;
    }
        
    if (l != NULL)
        l->l_next = NULL;

    *list = head;  
}

void
rs_fix_thing_list(struct linked_list *list)
{
    struct linked_list *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(THINGPTR(item));
}

int
rs_save_file(const char *file_name)
{
    EFILE *efp = efopen(file_name, "w");

    if (efp == NULL)
        return(-1);

    rs_write_string(efp, version);
    rs_write_int(efp, LINES);
    rs_write_int(efp, COLS);
    rs_write_object_list(efp, lvl_obj);
    rs_write_thing(efp, &player);
    rs_write_thing_list(efp, mlist);
    rs_write_traps(efp, traps, MAXTRAPS);             
    rs_write_rooms(efp, rooms, MAXROOMS);
    rs_write_object_reference(efp, player.t_pack, cur_armor);
    rs_write_object_reference(efp, player.t_pack, cur_ring[0]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[1]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[2]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[3]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[4]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[5]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[6]);
    rs_write_object_reference(efp, player.t_pack, cur_ring[7]);
    rs_write_object_reference(efp, player.t_pack, cur_weapon); 
    rs_write_int(efp, foodlev);
    rs_write_int(efp, ntraps);
    rs_write_int(efp, trader);
    rs_write_int(efp, curprice);
    rs_write_int(efp, seed);
    rs_write_int(efp, max_level);
    rs_write_int(efp, lost_dext);
    rs_write_int(efp, level);
    rs_write_int(efp, monslevel);
    rs_write_int(efp, purse);
    rs_write_int(efp, inpack);
    rs_write_int(efp, no_food);
    rs_write_int(efp, food_left);
    rs_write_int(efp, group);
    rs_write_int(efp, hungry_state);
    rs_write_int(efp, infest_dam);
    rs_write_int(efp, lost_str);
    rs_write_int(efp, hold_count);
    rs_write_int(efp, trap_tries);
    rs_write_int(efp, spell_power);
    rs_write_chars(efp, curpurch, 15);
    rs_write_int(efp, PLAYER);
    rs_write_chars(efp, whoami, LINELEN);
    rs_write_chars(efp, fruit, LINELEN);
    rs_write_scrolls(efp);
    rs_write_potions(efp);
    rs_write_rings(efp);
    rs_write_sticks(efp);
    rs_write_window(efp, cw);
    rs_write_window(efp, mw);
    rs_write_window(efp, stdscr);
    rs_write_int(efp, pool_teleport);
    rs_write_int(efp, waswizard);
    rs_write_int(efp, wizard);
    rs_write_int(efp, fight_flush);
    rs_write_int(efp, terse);
    rs_write_int(efp, jump);
    rs_write_int(efp, slow_invent);
    rs_write_int(efp, askme);
    rs_write_coord(efp, delta);
    rs_write_levtype(efp, levtype);
    rs_write_monsters(efp, monsters, NUMMONST+1);
    rs_write_magic_items(efp, things, NUMTHINGS);
    rs_write_magic_items(efp, s_magic, MAXSCROLLS);
    rs_write_magic_items(efp, p_magic, MAXPOTIONS);
    rs_write_magic_items(efp, r_magic, MAXRINGS);
    rs_write_magic_items(efp, ws_magic, MAXSTICKS);
    rs_write_coord(efp, ch_ret);
    rs_write_int(efp, demoncnt);
    rs_write_daemons(efp, d_list, MAXDAEMONS);
    rs_write_int(efp, between);
    rs_write_int(efp, inbag);
    rs_write_string(efp, bag_letters);
    rs_write_string_cindex(efp, bag_letters, bag_index);
    rs_write_int(efp, has_artifact);
    rs_write_int(efp, luck);
    rs_write_int(efp, msg_index);
    rs_write_chars(efp,(char *)msgbuf,10*2*BUFSIZ);
    rs_write_string(efp, pack_letters);
    rs_write_string_cindex(efp, pack_letters, pack_index);
    rs_write_int(efp, picked_artifact);
    rs_write_int(efp, resurrect);

	return( efclose(efp) );
}

int
rs_restore_file(const char *file_name)
{
    EFILE *efp = efopen(file_name, "r");
    
    if (efp == NULL)
	return -1;

    rs_read_new_string(efp, &oversion);
    rs_read_int(efp, &oldline);
    rs_read_int(efp, &oldcol);  
    rs_read_object_list(efp, &lvl_obj);
    rs_read_thing(efp, &player);
    rs_read_thing_list(efp, &mlist);
    rs_fix_thing(&player);
    rs_fix_thing_list(mlist);
    rs_read_traps(efp, traps, MAXTRAPS);             
    rs_read_rooms(efp, rooms, MAXROOMS);
    rs_read_object_reference(efp, player.t_pack, &cur_armor);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[0]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[1]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[2]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[3]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[4]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[5]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[6]);
    rs_read_object_reference(efp, player.t_pack, &cur_ring[7]);
    rs_read_object_reference(efp, player.t_pack, &cur_weapon);
    rs_read_int(efp, &foodlev);
    rs_read_int(efp, &ntraps);
    rs_read_int(efp, &trader);
    rs_read_int(efp, &curprice);
    rs_read_int(efp, &seed);
    rs_read_int(efp, &max_level);
    rs_read_int(efp, &lost_dext);
    rs_read_int(efp, &level);
    rs_read_int(efp, &monslevel);
    rs_read_int(efp, &purse);
    rs_read_int(efp, &inpack);
    rs_read_int(efp, &no_food);
    rs_read_int(efp, &food_left);
    rs_read_int(efp, &group);
    rs_read_int(efp, &hungry_state);
    rs_read_int(efp, &infest_dam);
    rs_read_int(efp, &lost_str);
    rs_read_int(efp, &hold_count);
    rs_read_int(efp, &trap_tries);
    rs_read_int(efp, &spell_power);
    rs_read_chars(efp, curpurch, 15);
    rs_read_int(efp, &PLAYER);
    rs_read_chars(efp, whoami, LINELEN);
    rs_read_chars(efp, fruit, LINELEN);
    rs_read_scrolls(efp);
    rs_read_potions(efp);
    rs_read_rings(efp);
    rs_read_sticks(efp);
    rs_read_window(efp, cw);
    rs_read_window(efp, mw);
    rs_read_window(efp, stdscr);
    rs_read_int(efp, &pool_teleport);
    rs_read_int(efp, &waswizard);
    rs_read_int(efp, &wizard);
    rs_read_int(efp, &fight_flush);
    rs_read_int(efp, &terse);
    rs_read_int(efp, &jump);
    rs_read_int(efp, &slow_invent);
    rs_read_int(efp, &askme);
    rs_read_coord(efp, &delta);
    rs_read_levtype(efp, &levtype);
    rs_read_monsters(efp, monsters, NUMMONST+1);
    rs_read_magic_items(efp, things, NUMTHINGS);
    rs_read_magic_items(efp, s_magic, MAXSCROLLS);
    rs_read_magic_items(efp, p_magic, MAXPOTIONS);
    rs_read_magic_items(efp, r_magic, MAXRINGS);
    rs_read_magic_items(efp, ws_magic, MAXSTICKS);
    rs_read_coord(efp, &ch_ret);
    rs_read_int(efp, &demoncnt);
    rs_read_daemons(efp, d_list, MAXDAEMONS);
    rs_read_int(efp, &between);
    rs_read_int(efp, &inbag);
    rs_read_string(efp, bag_letters, 27);
    rs_read_string_cindex(efp, bag_letters, &bag_index);
    rs_read_int(efp, &has_artifact);
    rs_read_int(efp, &luck);
    rs_read_int(efp, &msg_index);
    rs_read_chars(efp,(char *)msgbuf,10*2*BUFSIZ);
    rs_read_string(efp, pack_letters, 27);
    rs_read_string_cindex(efp, pack_letters, &pack_index);
    rs_read_int(efp, &picked_artifact);
    rs_read_int(efp, &resurrect);

    return(efclose(efp));
}
