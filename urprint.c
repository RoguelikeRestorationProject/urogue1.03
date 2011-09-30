/*
    urprint.c  -

    Last Modified: 11/03/86

    UltraRogue
    Copyright (C) 1986 Herb Chong
    All rights reserved.    

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <stdio.h>
#include "rogue.h"

static char *progname;

int multi = FALSE;

main(argc, argv)
int argc;
char	*argv[];
{
	register int	x;

	if (argc > 2)
		multi = TRUE;

	progname = *argv;

	for (x = 1; x < argc; x++) {
		if (argv[x][0] != '-')
			break;
		switch (argv[x][1]) {
		case 'a':
			pr_armour();
			break;
		case 'm':
			pr_monsters();
			break;
		case 'i':
		case 'A':
			multi = TRUE;
			pr_armour();
			pr_scrolls();
			pr_rings();
			pr_potions();
			pr_food();
			pr_sticks();
			pr_weapons();
			break;
		case 'w':
			pr_weapons();
			break;
		case 's':
			pr_scrolls();
			break;
		case 'r':
			pr_rings();
			break;
		case 'p':
			pr_potions();
			break;
		case 'f':
			pr_food();
			break;
		case 'S':
			pr_sticks();
			break;
		default:
			usage();
		}
	}

	if (argc <= 1) {
		usage();
		exit(1);
	}
}

usage()
{
	fprintf(stderr, "usage: %s -afsSprmw [ -A ]\n", progname);
	fprintf(stderr, "\t-a\tArmor\n");
	fprintf(stderr, "\t-f\tFood\n");
	fprintf(stderr, "\t-s\tScrolls\n");
	fprintf(stderr, "\t-S\tSticks\n");
	fprintf(stderr, "\t-p\tPotions\n");
	fprintf(stderr, "\t-r\tRings\n");
	fprintf(stderr, "\t-m\tMonsters\n");
	fprintf(stderr, "\t-w\tWeapons\n");
	fprintf(stderr, "\t-A\tPrint all of the above (except monsters).\n");
}

extern struct init_armor	armors[];

pr_armour()
{
	register int	i;

	printf("%25s   %%  AC Weight Worth\n", "Armor");

	for (i = 0; i < maxarmors; i++) {
		printf("%25.25s ", armors[i].a_name);
		printf("%3d ", armors[i].a_prob);
		printf(" %2d ", armors[i].a_class);
		printf(" %3d ", armors[i].a_wght);
		printf(" %5ld ", armors[i].a_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

pr_scrolls()
{
	register int	i;
	float	q;

	printf("%25s   %%	Curse Bless Worth\n", "Scrolls");
	for (i = 0; i < maxscrolls; i++) {

		printf("%25.25s  ", s_magic[i].mi_name);
		q = s_magic[i].mi_prob / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", s_magic[i].mi_curse);
		printf(" %3d ", s_magic[i].mi_bless);
		printf(" %5ld ", s_magic[i].mi_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

pr_potions()
{
	register int	i;
	float	q;

	printf("%25s   %%	Curse Bless Worth\n", "Potions");
	for (i = 0; i < maxpotions; i++) {

		printf("%25.25s  ", p_magic[i].mi_name);
		q = p_magic[i].mi_prob / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", p_magic[i].mi_curse);
		printf(" %3d ", p_magic[i].mi_bless);
		printf(" %5ld ", p_magic[i].mi_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

pr_rings()
{
	register int	i;
	float	q;

	printf("%25s   %%	Curse Bless Worth\n", "Rings");
	for (i = 0; i < maxrings; i++) {

		printf("%25.25s  ", r_magic[i].mi_name);
		q = r_magic[i].mi_prob / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", r_magic[i].mi_curse);
		printf(" %3d ", r_magic[i].mi_bless);
		printf(" %5ld ", r_magic[i].mi_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

pr_food()
{
	register int	i;
	float	q;

	printf("%25s   %%	Curse Bless Worth\n", "Food");
	for (i = 0; i < maxfoods; i++) {

		printf("%25.25s  ", fd_data[i].mi_name);
		q = fd_data[i].mi_prob / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", fd_data[i].mi_curse);
		printf(" %3d ", fd_data[i].mi_bless);
		printf(" %5ld ", fd_data[i].mi_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

pr_sticks()
{
	register int	i;
	float	q;

	printf("%25s   %%	Curse Bless Worth\n", "Sticks");
	for (i = 0; i < maxsticks; i++) {

		printf("%25.25s  ", ws_magic[i].mi_name);
		q = ws_magic[i].mi_prob / 10.0;
		printf("%-3.1f	", q);
		printf(" %3d ", ws_magic[i].mi_curse);
		printf(" %3d ", ws_magic[i].mi_bless);
		printf(" %5ld ", ws_magic[i].mi_worth);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

extern struct monster	monsters[];

pr_monsters()
{

	/*
	 * CARRY_PROB,	NORMAL, WANDER, SYMBOL, INTELLIGENCE,
	 * {ATTRIBUTE_FLAGS}, CREATURE_SUMMONED, NUMBER_SUMMONED,
	 * ADDED_EXPERIENCE/HIT_POINT, {str,	exp_pts, exp_level,
	 * armor_class, hit_points, "damage"}},
	 */

	register int	i;

	printf(
		   "Number%30s Letter  Int Str	Lvl    Exp(X) Tres	AC Flags\n%64s%12s\n",
		   "Monster", "HP", "Damage");

	for (i = 1; i < nummonst + 2; i++) {
		printf("%4d) %30.30s   '%c' %5s %2d %c",
			   i,
			   monsters[i].m_name,
			   monsters[i].m_appear,
			   monsters[i].m_intel,
			   monsters[i].m_stats.s_str,
			   (monsters[i].m_wander ? 'W' : ' '));

		printf(" %2d ", monsters[i].m_stats.s_lvl);
		printf(" %5ld", monsters[i].m_stats.s_exp);

		if (monsters[i].m_add_exp != 0)
			printf("(%2d)", monsters[i].m_add_exp);
		else
			printf("	");

		if (monsters[i].m_carry != 0)
			printf("%3d%%", monsters[i].m_carry);
		else
			printf("	");

		printf("   %2d ", monsters[i].m_stats.s_arm);

		find_flags(i);

		/* Second line */
		printf("\n%64.5s %10s",
			   monsters[i].m_stats.s_hpt,
			   monsters[i].m_stats.s_dmg
			);

		if (monsters[i].m_typesum != 0)
			printf("%-20d(%s)",
				   monsters[i].m_numsum,
				   monsters[i].m_typesum);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

find_flags(i)
register int	i;
{
	register int	k;
	register long	flag;

	for (k = 0; k < 16; k++) {
		if ((flag = monsters[i].m_flags[k]) != 0) {
			if (flag == ISBLIND) { printf(" BLIND"); continue; }
			if (flag == ISINWALL) { printf(" INWALL"); continue; }
			if (flag == ISRUN) { printf(" RUN"); continue; }
			if (flag == ISFLEE) { printf(" FLEE"); continue; }
			if (flag == ISINVIS) { printf(" INVIS"); continue; }
			if (flag == ISMEAN) { printf(" M"); continue; }
			if (flag == ISGREED) { printf(" GREEDY"); continue; }
			if (flag == CANSHOOT) { printf(" SHOOT"); continue; }
			if (flag == ISHELD) { printf(" HELD"); continue; }
			if (flag == ISHUH) { printf(" HUH"); continue; }
			if (flag == ISREGEN) { printf(" REGEN"); continue; }
			if (flag == CANHUH) { printf(" HUH"); continue; }
			if (flag == CANSEE) { printf(" SEE"); continue; }
			if (flag == HASFIRE) { printf(" FIRE"); continue; }
			if (flag == ISSLOW) { printf(" SLOW"); continue; }
			if (flag == ISHASTE) { printf(" HASTE"); continue; }
			if (flag == ISCLEAR) { printf(" CLEAR"); continue; }
			if (flag == CANINWALL) { printf(" INWALL"); continue; }
			if (flag == ISDISGUISE) { printf(" DISGUISE"); continue; }
			if (flag == CANBLINK) { printf(" BLINK"); continue; }
			if (flag == CANSNORE) { printf(" SNORE"); continue; }
			if (flag == HALFDAMAGE) { printf(" HALFDAMAGE"); continue; }
			if (flag == CANSUCK) { printf(" SUCK"); continue; }
			if (flag == CANRUST) { printf(" RUST"); continue; }
			if (flag == CANPOISON) { printf(" POISON"); continue; }
			if (flag == CANDRAIN) { printf(" DRAIN"); continue; }
			if (flag == ISUNIQUE) { printf(" UNIQUE"); continue; }
			if (flag == STEALGOLD) { printf(" STEALGOLD"); continue; }
			if (flag == STEALMAGIC) { printf(" STEALMAGIC"); continue; }
			if (flag == CANDISEASE) { printf(" DISEASE"); continue; }
			if (flag == HASDISEASE) { printf(" DISEASE"); continue; }
			if (flag == CANSUFFOCATE) { printf(" SUFFOCATE"); continue; }
			if (flag == DIDSUFFOCATE) { printf(" DIDSUFFOCATE"); continue; }
			if (flag == BOLTDIVIDE) { printf(" BOLTDIVIDE"); continue; }
			if (flag == BLOWDIVIDE) { printf(" BLOWDIVIDE"); continue; }
			if (flag == NOCOLD) { printf(" NOCOLD"); continue; }
			if (flag == TOUCHFEAR) { printf(" TOUCHFEAR"); continue; }
			if (flag == BMAGICHIT) { printf(" M_+_HIT"); continue; }
			if (flag == NOFIRE) { printf(" NOFIRE"); continue; }
			if (flag == NOBOLT) { printf(" NOBOLT"); continue; }
			if (flag == CARRYGOLD) { printf(" CARRYGOLD"); continue; }
			if (flag == CANITCH) { printf(" ITCH"); continue; }
			if (flag == HASITCH) { printf(" ITCH"); continue; }
			if (flag == DIDDRAIN) { printf(" DIDDRAIN"); continue; }
			if (flag == WASTURNED) { printf(" WASTURNED"); continue; }
			if (flag == CANSELL) { printf(" SELL"); continue; }
			if (flag == CANBLIND) { printf(" BLIND"); continue; }
			if (flag == CANBBURN) { printf(" BBURN"); continue; }
			if (flag == ISCHARMED) { printf(" CHARMED"); continue; }
			if (flag == CANSPEAK) { printf(" SPEAK"); continue; }
			if (flag == CANFLY) { printf(" FLY"); continue; }
			if (flag == ISFRIENDLY) { printf(" FRIENDLY"); continue; }
			if (flag == CANHEAR) { printf(" HEAR"); continue; }
			if (flag == ISDEAF) { printf(" DEAF"); continue; }
			if (flag == CANSCENT) { printf(" SCENT"); continue; }
			if (flag == ISUNSMELL) { printf(" UNSMELL"); continue; }
			if (flag == WILLRUST) { printf(" RUST"); continue; }
			if (flag == WILLROT) { printf(" ROT"); continue; }
			if (flag == SUPEREAT) { printf(" SUPEREAT"); continue; }
			if (flag == PERMBLIND) { printf(" PERMBLIND"); continue; }
			if (flag == MAGICHIT) { printf(" M_HIT"); continue; }
			if (flag == CANINFEST) { printf(" INFEST"); continue; }
			if (flag == HASINFEST) { printf(" INFEST"); continue; }
			if (flag == NOMOVE) { printf(" NOMOVE"); continue; }
			if (flag == CANSHRIEK) { printf(" SHRIEK"); continue; }
			if (flag == CANDRAW) { printf(" DRAW"); continue; }
			if (flag == CANSMELL) { printf(" SMELL"); continue; }
			if (flag == CANPARALYZE) { printf(" PARALYZE"); continue; }
			if (flag == CANROT) { printf(" ROT"); continue; }
			if (flag == ISSCAVENGE) { printf(" SCAVENGE"); continue; }
			if (flag == DOROT) { printf(" ROT"); continue; }
			if (flag == CANSTINK) { printf(" STINK"); continue; }
			if (flag == HASSTINK) { printf(" STINK"); continue; }
			if (flag == ISSHADOW) { printf(" SHADOW"); continue; }
			if (flag == CANCHILL) { printf(" CHILL"); continue; }
			if (flag == CANHUG) { printf(" HUG"); continue; }
			if (flag == CANSURPRISE) { printf(" SURPRISE"); continue; }
			if (flag == CANFRIGHTEN) { printf(" FRIGHTEN"); continue; }
			if (flag == CANSUMMON) { printf(" SUMMON"); continue; }
			if (flag == TOUCHSTONE) { printf(" TOUCHSTONE"); continue; }
			if (flag == LOOKSTONE) { printf(" LOOKSTONE"); continue; }
			if (flag == CANHOLD) { printf(" HOLD"); continue; }
			if (flag == DIDHOLD) { printf(" DIDHOLD"); continue; }
			if (flag == DOUBLEDRAIN) { printf(" DOUBLEDRAIN"); continue; }
			if (flag == ISUNDEAD) { printf(" UNDEAD"); continue; }
			if (flag == BLESSMAP) { printf(" B_MAP"); continue; }
			if (flag == BLESSGOLD) { printf(" B_GOLD"); continue; }
			if (flag == BLESSMONS) { printf(" B_MONS"); continue; }
			if (flag == BLESSMAGIC) { printf(" B_MAGIC"); continue; }
			if (flag == BLESSFOOD) { printf(" B_FOOD"); continue; }
			if (flag == CANBRANDOM) { printf(" B_RANDOM"); continue; }
			if (flag == CANBACID) { printf(" B_ACID"); continue; }
			if (flag == CANBFIRE) { printf(" B_FIRE"); continue; }
			if (flag == CANBBOLT) { printf(" B_BOLT"); continue; }
			if (flag == CANBGAS) { printf(" B_GAS"); continue; }
			if (flag == CANBICE) { printf(" B_ICE"); continue; }
			if (flag == CANBPGAS) { printf(" B_PGAS"); continue; }
			if (flag == CANBSGAS) { printf(" B_SGAS"); continue; }
			if (flag == CANBSLGAS) { printf(" B_SLGAS"); continue; }
			if (flag == CANBFGAS) { printf(" B_FGAS"); continue; }
			if (flag == CANBREATHE) { printf(" BREATHE"); continue; }
			if (flag == STUMBLER) { printf(" STUMBLER"); continue; }
			if (flag == POWEREAT) { printf(" P_EAT"); continue; }
			if (flag == ISELECTRIC) { printf(" ELECTRIC"); continue; }
			if (flag == HASOXYGEN) { printf(" OXYGEN"); continue; }
			if (flag == POWERDEXT) { printf(" P_DEXT"); continue; }
			if (flag == POWERSTR) { printf(" P_STR"); continue; }
			if (flag == POWERWISDOM) { printf(" P_WISDOM"); continue; }
			if (flag == POWERINTEL) { printf(" P_INTEL"); continue; }
			if (flag == POWERCONST) { printf(" P_CONST"); continue; }
			if (flag == SUPERHERO) { printf(" SUPERHERO"); continue; }
			if (flag == ISUNHERO) { printf(" UNHERO"); continue; }
			if (flag == CANCAST) { printf(" CAST"); continue; }
			if (flag == CANTRAMPLE) { printf(" TRAMPLE"); continue; }
			if (flag == CANSWIM) { printf(" SWIM"); continue; }
			if (flag == LOOKSLOW) { printf(" LOOKSLOW"); continue; }
			if (flag == CANWIELD) { printf(" WIELD"); continue; }
			if (flag == CANDARKEN) { printf(" DARKEN"); continue; }
			if (flag == ISFAST) { printf(" FAST"); continue; }
			if (flag == CANBARGAIN) { printf(" BARGAIN"); continue; }
			if (flag == NOMETAL) { printf(" NOMETAL"); continue; }
			if (flag == CANSPORE) { printf(" SPORE"); continue; }
			if (flag == NOSHARP) { printf(" NOSHARP"); continue; }
			if (flag == DRAINWISDOM) { printf(" D_WISDOM"); continue; }
			if (flag == DRAINBRAIN) { printf(" D_BRAIN"); continue; }
			if (flag == ISLARGE) { printf(" LARGE"); continue; }
			if (flag == ISSMALL) { printf(" SMALL"); continue; }
			if (flag == CANSTAB) { printf(" STAB"); continue; }
			if (flag == ISFLOCK) { printf(" FLOCK"); continue; }
			if (flag == ISSWARM) { printf(" SWARM"); continue; }
			if (flag == CANSTICK) { printf(" STICK"); continue; }
			if (flag == CANTANGLE) { printf(" TANGLE"); continue; }
			if (flag == DRAINMAGIC) { printf(" D_ENCHANT"); continue; }
			if (flag == SHOOTNEEDLE) { printf(" NEEDLE"); continue; }
			if (flag == CANZAP) { printf(" ELECTRIC_ZAP"); continue; }
			if (flag == HASARMOR) { printf(" HASARMOR"); continue; }
			if (flag == CANTELEPORT) { printf(" TELEPORT"); continue; }
			if (flag == ISBERSERK) { printf(" BERSERK"); continue; }
			if (flag == ISFAMILIAR) { printf(" FAMILIAR"); continue; }
			if (flag == HASFAMILIAR) { printf(" HASFAMILIAR"); continue; }
			if (flag == SUMMONING) { printf(" SUMMONING"); continue; }
			if (flag == CANREFLECT) { printf(" REFLECT"); continue; }
			if (flag == LOWFRIENDLY) { printf(" L_FRIEND"); continue; }
			if (flag == MEDFRIENDLY) { printf(" M_FRIEND"); continue; }
			if (flag == HIGHFRIENDLY) { printf(" H_FRIEND"); continue; }
			if (flag == MAGICATTRACT) { printf(" M_ATTRACT"); continue; }
			if (flag == ISGOD) { printf(" *GOD*"); continue; }
			if (flag == CANLIGHT) { printf(" LIGHT"); continue; }
			if (flag == LOWCAST) { printf(" L_CAST"); continue; }
			if (flag == MEDCAST) { printf(" M_CAST"); continue; }
			if (flag == HIGHCAST) { printf(" H_CAST"); continue; }
			if (flag == WASSUMMONED) { printf(" SUMMONER"); continue; }
			if (flag == HASSUMMONED) { printf(" HASSUMMONED"); continue; }
			if (flag == CANTRUESEE) { printf(" TRUESEE"); continue; }
			printf(" ?????");
		}
	}
}

extern struct init_weps weaps[];

pr_weapons()
{
	register int	i;

	printf("%17s  Damage	Throw  Weight Worth Flags\n", "Weapon");

	for (i = 1; i < maxweapons; i++) {

		printf("%17.17s ", weaps[i].w_name);
		printf("%5s ", weaps[i].w_dam);
		printf(" %8s ", weaps[i].w_hrl);
		printf(" %5d ", weaps[i].w_wght);
		printf(" %5ld ", weaps[i].w_worth);

		wfind_flags(i);

		if (weaps[i].w_launch != NONE)
			printf(" by %s", weaps[weaps[i].w_launch].w_name);

		printf("\n");
	}
	if (multi)
		putchar('\n');
}

wfind_flags(i)
register int	i;
{
	register long	flag = weaps[i].w_flags;
	register long	mask;

	for (i = 0; i < 32; i++, mask = flag & (1L << i)) {
		if (mask & ISCURSED) printf(" CURSED");
		if (mask & ISKNOW) printf(" KNOW");
		if (mask & ISPOST) printf(" POST");
		if (mask & ISMETAL) printf(" METAL");
		if (mask & ISPROT) printf(" PROT");
		if (mask & ISBLESSED) printf(" BLESS");
		if (mask & ISZAPPED) printf(" ZAP");
		if (mask & ISVORPED) printf(" VORPAL");
		if (mask & ISSILVER) printf(" SILVER");
		if (mask & ISPOISON) printf(" POISON");
		if (mask & CANRETURN) printf(" MISS_RET");
		if (mask & ISOWNED) printf(" ALWAYS_RET");
		if (mask & ISLOST) printf(" DISSAPEARS");
		if (mask & ISMISL) printf(" MISSLE");
		if (mask & ISMANY) printf(" MANY");
		if (mask & CANBURN) printf(" BURN");
		if (mask & ISSHARP) printf(" SHARP");
		if (mask & ISTWOH) printf(" TWOH");
		if (mask & ISLITTLE) printf(" LITTLE");
	}
}
