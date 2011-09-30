# Last Modified: Dec 30, 1990
#
#   UltraRogue
#   Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
#   All rights reserved.
#    
#   Based on "Advanced Rogue"
#   Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
#   All rights reserved.
#
#   Based on "Super-Rogue"
#   Copyright (C) 1982, 1983 Robert D. Kindelberger
#   All rights reserved.
#
#   Based on "Rogue: Exploring the Dungeons of Doom"
#   Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
#   All rights reserved.
#    
#   See the file LICENSE.TXT for full copyright and licensing information.


HDRS=	rogue.h 
SRCS=	armor.c artifact.c bag.c chase.c command.c daemon.c daemons.c encumb.c\
	fight.c getplay.c ident.c init.c io.c list.c magic.c magicitm.c main.c\
	maze.c misc.c monsdata.c monsters.c move.c newlvl.c options.c pack.c\
	passages.c player.c potions.c properti.c random.c rings.c rip.c rogue.c\
	rooms.c save.c scrolls.c status.c sticks.c things.c trader.c vers.c\
	weapons.c wizard.c
OBJS=	armor.o artifact.o bag.o chase.o command.o daemon.o daemons.o encumb.o\
	fight.o getplay.o ident.o init.o io.o list.o magic.o magicitm.o main.o\
	maze.o misc.o monsdata.o monsters.o move.o newlvl.o options.o pack.o\
	passages.o player.o potions.o properti.o random.o rings.o rip.o rogue.o\
	rooms.o	save.o scrolls.o status.o sticks.o things.o trader.o vers.o\
	weapons.o wizard.o
CFLAGS=	-g

urogue: $(OBJS)
	cc -g -o urogue $(OBJS) -lcurses

urprint: urprint.o magicitm.o monsdata.o
	cc -o urprint urprint.o magicitm.o monsdata.o

armor.o: armor.c rogue.h
artifact.o: artifact.c  rogue.h
bag.o: bag.c rogue.h
chase.o: chase.c rogue.h 
command.o: command.c rogue.h
daemon.o: daemon.c rogue.h
daemons.o: daemons.c rogue.h 
encumb.o: encumb.c rogue.h
fight.o: fight.c rogue.h
getplay.o: getplay.c rogue.h
ident.o: ident.c rogue.h
init.o: init.c rogue.h
io.o: io.c rogue.h 
list.o: list.c rogue.h
magic.o: magic.c rogue.h
magicitm.o: magicitm.c rogue.h
main.o: main.c rogue.h 
maze.o: maze.c rogue.h
misc.o: misc.c rogue.h
monsdata.o: monsdata.c rogue.h
monsters.o: monsters.c rogue.h 
move.o: move.c rogue.h 
newlvl.o: newlvl.c rogue.h
options.o: options.c rogue.h
pack.o: pack.c rogue.h
passages.o: passages.c rogue.h
player.o: player.c rogue.h
potions.o: potions.c rogue.h
properti.o: properti.c rogue.h
random.o: random.c
rings.o: rings.c rogue.h
rip.o: rip.c rogue.h 
rogue.o: rogue.c rogue.h
rooms.o: rooms.c rogue.h 
save.o: save.c rogue.h
scrolls.o: scrolls.c rogue.h 
status.o: status.c rogue.h
sticks.o: sticks.c rogue.h
things.o: things.c rogue.h
trader.o: trader.c rogue.h
urprint.o: urprint.c rogue.h
	cc $(CFLAGS) -c urprint.c
vers.o: vers.c
weapons.o: weapons.c rogue.h
wizard.o: wizard.c rogue.h
