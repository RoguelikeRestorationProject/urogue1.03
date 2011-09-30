/*
    stepok.h  -  definitions for function step_ok
   
    Last Modified: Dec 30, 1990

    UltraRogue
    Copyright (C) 1986, 1987, 1990 Herb Chong
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
 * definitions for function step_ok: MONSTOK indicates it is OK to step on a
 * monster -- it is only OK when stepping diagonally AROUND a monster
 */
#define MONSTOK 1
#define NOMONST 2
