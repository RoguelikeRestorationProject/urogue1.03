/*
    status.c  -  This file contains functions for complex status determination 

    Last Modified: Jan 7, 1991

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990, 1991 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
 * This file contains functions for complex status determination of monsters
 * and objects. Mark Williams C for the ST blows up on code that is too
 * complex, and moving things here is a good idea anyway because it simplifies
 * understanding the code.
 */

#include "rogue.h"

/*
 * has_defensive_spell() - has monster cast a defensive spell
 * Any flags added here must also be in player_powers[]
 */
has_defensive_spell(th)
struct thing    th;
{
    if (on(th, HASOXYGEN))
        return (TRUE);
    if (on(th, CANFLY))
        return (TRUE);
    if (on(th, CANINWALL))
        return (TRUE);
    if (on(th, CANREFLECT))
        return (TRUE);
    if (on(th, CANSEE))
        return (TRUE);
    if (on(th, HASMSHIELD))
        return (TRUE);
    if (on(th, HASSHIELD))
        return (TRUE);
    if (on(th, ISHASTE))
        return (TRUE);
    if (on(th, ISREGEN))
        return (TRUE);
    if (on(th, ISDISGUISE))
        return (TRUE);
    if (on(th, ISINVIS))
        return (TRUE);
    if (on(th, NOCOLD))
        return (TRUE);
    if (on(th, NOFIRE))
        return (TRUE);
    if (on(th, ISELECTRIC))
        return (TRUE);
    return(FALSE);
}
