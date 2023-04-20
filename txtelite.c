/* TXTELITE.C  1.5 */
/*
Textual version of Elite trading (C implementation)
Note that this program is a "quick-hack" text parser-driven version
of Elite with no combat or missions.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include "txtelite.h"


void
tweakseed(Seed *s)
{
	uint16_t temp;
	temp = (s->w0) + (s->w1) + (s->w2); /* 2-byte arithmetic */
	s->w0 = s->w1;
	s->w1 = s->w2;
	s->w2 = temp;
}

/**-String functions for text interface **/

void
stripout(char *s, const char c) /* Remove every 'c' from string 's' */
{
	char *r = s, *w = s;
	while (*r) {
		*w = *r++;
		w += (*w != c);
	}
	*w = '\0';
}

bool
stringbegins(char *s, char *t)
/* Return true iff string 't' begins with non-empty string 's' */
{
	size_t i = 0, l = strlen(s);
	while (i < l && toupper(s[i]) == toupper(t[i]))
		i++;
	return i == l;
}

unsigned int
stringmatch(char *s, char a[][MAXLEN], unsigned int n)
/* Check string 's' against 'n' options in string array 'a'
	If ith element is matched return i + 1 else return 0 */
{
	unsigned int i = 0;
	while (i < n) {
		if (stringbegins(s, a[i]))
			return i + 1;
		i++;
	}
	return 0;
}

void
spacesplit(char *s, char *t)
/* Split string 's' at first space,
	returning first word in 't' and shortening 's' */
{
	size_t i = 0, j = 0;
	size_t l = strlen(s);
	while (i < l && s[i] == ' ')
		i++; /* Strip leading spaces */

	if (i == l) {
		s[0] = 0; t[0] = 0;
		return;
	}

	while (i < l && s[i] != ' ')
		t[j++] = s[i++];

	t[j] = 0; i++; j = 0;

	while (i < l)
		s[j++] = s[i++];

	s[j] = 0;
}

/**-Functions for stock market **/

unsigned int
gamebuy(unsigned int i, unsigned int a)
/* Try to buy amount 'a' of good 'i' and return amount bought
	Can't buy more than is available, can afford, or will fit in hold */
{
	unsigned int t;
	if (cash < 0) {
		t = 0;
	} else {
		t = MIN(localmarket.quantity[i], a);
		if (commodities[i].unit == TONNE)
			t = MIN(holdspace, t);
		t = MIN(t, cash / localmarket.price[i]);
	}
	shipshold[i] += t;
	localmarket.quantity[i] -= t;
	cash -= t * localmarket.price[i];
	if (commodities[i].unit == TONNE)
		holdspace -= t;
	return t;
}

unsigned int
gamesell(unsigned int i, unsigned int a)
{
	unsigned int t = MIN(shipshold[i], a);
	shipshold[i] -= t;
	localmarket.quantity[i] += t;
	if (commodities[i].unit == TONNE)
		holdspace += t;
	cash += t * localmarket.price[i];
	return t;
}

Market
genmarket(uint8_t fluct, Planet p)
{
	Market market;
	for (int i = 0; i <= LAST_TRADE; i++) {
		signed int q; 
		signed int product = p.economy * commodities[i].gradient;
		signed int changing = fluct & commodities[i].maskbyte;
		q = commodities[i].basequant + changing - product;
		q = q & 0xFF;
		if (q & 0x80)
			q = 0; /* Clip to positive 8-bit */

		market.quantity[i] = (uint16_t)(q & 0x3F); /* Mask to 6 bits */

		q = commodities[i].baseprice + changing + product;
		q = q & 0xFF;
		market.price[i] = (uint16_t)(q * 4);
	}
	market.quantity[LAST_TRADE] = 0; /* Override to force non-availability */
	return market;
}

Planet
makeplanet(Seed *s) /* Generate planet info from seed */
{
	Planet thissys;
	unsigned int pair1, pair2, pair3, pair4;
	uint16_t longnameflag = (s->w0) & 0x40;

	thissys.x = (s->w1) >> 8;
	thissys.y = (s->w0) >> 8;

	thissys.govtype = ((s->w1) >> 3) & 7; /* bits 3, 4 and 5 of w1 */

	thissys.economy = ((s->w0) >> 8) & 7; /* bits 8, 9 and A of w0 */
	if (thissys.govtype <= 1)
		thissys.economy = thissys.economy | 2;

	thissys.techlev = (((s->w1) >> 8) & 3) + (thissys.economy ^ 7);
	thissys.techlev += thissys.govtype >> 1;
	if ((thissys.govtype & 1) == 1)
		thissys.techlev++;
	/* C simulation of 6502's LSR then ADC */

	thissys.population = 4 * thissys.techlev + thissys.economy;
	thissys.population += thissys.govtype + 1;

	thissys.productivity = ((thissys.economy ^ 7) + 3) * (thissys.govtype + 4);
	thissys.productivity *= thissys.population * 8;

	thissys.radius = 0x100 * ((((s->w2) >> 8) & 0x0F) + 11) + thissys.x;

	thissys.goatsoupseed.a = s->w1 & 0xFF;
	thissys.goatsoupseed.b = s->w1 >> 8;
	thissys.goatsoupseed.c = s->w2 & 0xFF;
	thissys.goatsoupseed.d = s->w2 >> 8;

	/* Always four iterations of random number */
	pair1 = 2 * (((s->w2) >> 8) & 0x1F); tweakseed(s);
	pair2 = 2 * (((s->w2) >> 8) & 0x1F); tweakseed(s);
	pair3 = 2 * (((s->w2) >> 8) & 0x1F); tweakseed(s);
	pair4 = 2 * (((s->w2) >> 8) & 0x1F); tweakseed(s);

	(thissys.name)[0] = pairs[pair1];
	(thissys.name)[1] = pairs[pair1 + 1];
	(thissys.name)[2] = pairs[pair2];
	(thissys.name)[3] = pairs[pair2 + 1];
	(thissys.name)[4] = pairs[pair3];
	(thissys.name)[5] = pairs[pair3 + 1];

	if (longnameflag) {
		(thissys.name)[6] = pairs[pair4];
		(thissys.name)[7] = pairs[pair4 + 1];
		(thissys.name)[8] = 0;
	} else (thissys.name)[6] = 0;

	stripout(thissys.name, '.');
	return thissys;
}


/**+Generate galaxy **/
/* Functions for galactic hyperspace */

uint16_t
lrotate(uint16_t x) /* rotate 8-bit(?) number leftwards */
{
	uint16_t temp = x & 0x80;
	return (2 * (x & 0x7F)) + (temp >> 7);
}

uint16_t
twist(uint16_t x)
{
	return (uint16_t)((0x100 * lrotate(x >> 8)) + lrotate(x & 0xFF));
}

void
nextgalaxy(Seed *s) /* Apply to base seed; once for galaxy 2 */
{
	s->w0 = twist(s->w0); /* Twice for galaxy 3, etc. */
	s->w1 = twist(s->w1); /* Eighth application gives galaxy 1 again */
	s->w2 = twist(s->w2);
}

/* Original game generated from scratch each time info needed */
void
buildgalaxy(uint8_t galaxy_index)
{
	/* Initialise seed for galaxy 1 */
	seed.w0 = base0; seed.w1 = base1; seed.w2 = base2;
	for (unsigned int galcount = 1; galcount < galaxy_index; ++galcount)
		nextgalaxy(&seed);
	/* Put galaxy data into array of structures */  
	for (int pcount = 0; pcount < GALAXY_SIZE; ++pcount)
		galaxies[pcount] = makeplanet(&seed);
}

/**-Functions for navigation **/

void
gamejump(int i) /* Move to planet index 'i' */
{
	curr_planet = i;
	localmarket = genmarket(rand() % 0x100, galaxies[i]);
}

unsigned int
distance(Planet a, Planet b)
{
	int xdelta = a.x - b.x, ydelta = a.y - b.y;
	return 4 * sqrt(xdelta * xdelta + ydelta * ydelta / 4);
}


int
matchplanet(char *s)
/* Return id of the planet whose name matches passed string
	closest to 'curr_planet' - if none return 'curr_planet' */
{
	int p = curr_planet;
	unsigned int d = 9999;
	for (int pcount = 0; pcount < GALAXY_SIZE; ++pcount) {
		if (stringbegins(s, galaxies[pcount].name)) {
		 	if (distance(galaxies[pcount], galaxies[curr_planet]) < d) {
		 		d = distance(galaxies[pcount], galaxies[curr_planet]);
				p = pcount;
			}
		}
	}
	return p;
}

void
printplanet(Planet p, bool brief)
{
	if (brief) {
		printf("%10s TL: %2i %12s %15s", 
			p.name, p.techlev + 1, 
			econnames[p.economy], govnames[p.govtype]);
	} else {
		printf(
			"Planet: %s"
			"\nPosition (%i), %i"
			"\nEconomy: (%i) %s"
			"\nGovernment: (%i) %s"
			"\nTech Level: %2i"
			"\nTurnover: %u"
			"\nRadius: %u"
			"\nPopulation: %u billion\n", 
			p.name, p.x, p.y, p.economy, econnames[p.economy], 
			p.govtype, govnames[p.govtype], p.techlev + 1, p.productivity, 
			p.radius, p.population >> 3
		);
		rnd_seed = p.goatsoupseed;
		goat_soup("\x8F is \x97.", &p);
	}
}

/**-Various command functions **/

bool
dolocal(char *s)
{
	unsigned int d;
	(void)(&s);
	printf("Galaxy #%i\n", galaxy_index);
	for (int pcount = 0; pcount < GALAXY_SIZE; ++pcount) {
		d = distance(galaxies[pcount], galaxies[curr_planet]);
		if (d <= maxfuel) {
			if (d <= fuel)
				putchar('*');
			else
				putchar('-');
			printplanet(galaxies[pcount], true);
			printf("  (%.1fLY)\n", (float)d/10);
		}
	}
	return true;
}

bool
dojump(char *s) /* Jump to planet name 's' */
{
	unsigned int d;
	int dest = matchplanet(s);
	if (dest == curr_planet) {
		puts("Bad jump");
		return false;
	}
	d = distance(galaxies[dest], galaxies[curr_planet]);
	if (d > fuel) {
		puts("Jump too far");
		return false;
	}
	fuel -= d;
	gamejump(dest);
	printplanet(galaxies[curr_planet], false);
	return true;
}

bool
dosneak(char *s) /* Like 'dojump' but no fuel cost */
{
	bool sneaked;
	unsigned int fuelkeep = fuel;
	fuel = -1;
	sneaked = dojump(s);
	fuel = fuelkeep;
	return sneaked;
}

bool
dogalhyp(char *s) /* Jump to next galaxy */
/* Preserve int (e.g. if leave 7th planet arrive at 7th planet)
	Classic Elite always jumped to nearest planet (0x60, 0x60) */
{
	(void)(&s);
	if (++galaxy_index == 9)
		galaxy_index = 1;
	buildgalaxy(galaxy_index);
	return true;
}

bool
doinfo(char *s)
{
	int dest = matchplanet(s);
	printplanet(galaxies[dest], false);
	return true;
}

bool
dohold(char *s)
{
	unsigned int a = atoi(s), t = 0;
	for (int i = 0; i <= LAST_TRADE; i++) {
		if (commodities[i].unit == TONNE)
			t += shipshold[i];
	}
	if (t > a) {
		puts("Hold too full");
		return false;
	}
	holdspace = a - t;
	return true;
}

bool
dosell(char *s) /* Sell amount S(2) of good S(1) */
{
	unsigned int i, a, t;
	char s2[MAXLEN];
	spacesplit(s, s2);
	a = atoi(s);
	if (a == 0)
		a = 1;
	i = stringmatch(s2, item_names, LAST_TRADE + 1);
	if (i == 0) {
		puts("Unknown item");
		return false;
	}
	i -= 1;
	t = gamesell(i, a);

	if (t == 0)
		fputs("Can't sell any ", stdout);
	else
		printf("Selling %i%s of ", t, unitnames[commodities[i].unit]);
	fputs(item_names[i], stdout);
	return true;
}

bool
dobuy(char *s) /* Buy amount S(2) of good S(1) */
{
	unsigned int i, a, t;
	char s2[MAXLEN];
	spacesplit(s, s2);
	a = atoi(s);
	if (a == 0)
		a = 1;
	i = stringmatch(s2, item_names, LAST_TRADE + 1);
	if (i == 0) {
		puts("Unknown item");
		return false;
	}
	i -= 1;
	t = gamebuy(i, a);

	if (t == 0)
		fputs("Can't buy any ", stdout);
	else
		printf("Buying %i%s of ", t, unitnames[commodities[i].unit]);
	fputs(item_names[i], stdout);
	return true;
}

unsigned int
gamefuel(unsigned int f) /* Attempt to buy 'f' tons of fuel */
{
	if ((f + fuel) > maxfuel)
		f = maxfuel - fuel;
	if (fuelcost > 0) {
		if ((int)(f * fuelcost) > cash)
			f = (unsigned int)(cash / fuelcost);
	}
	fuel += f; cash -= fuelcost * f;
	return f;
}

bool
dofuel(char *s)
{
	unsigned int f = gamefuel(10 * atoi(s));
	if (f == 0) {
		fputs("Can't buy any fuel", stdout);
		return false;
	}
	printf("Buying %.1fLY fuel", (float)f/10);
	return true;
}

bool
docash(char *s) /* Cheat alter cash by 's' */
{
	int32_t a = 10 * atof(s);
	if (a == 0) {
		puts("Invalid number");
		return false;
	}
	cash += a;
	return true;
}

bool
domarket(char *s)
{
	(void)(&s);
	printf("%-14s%-7s%-8s%8s\n", "Name", "Price", "Quantity", "Loaded");
	for (int i = 0; i <= LAST_TRADE; i++)
		printf("%-14s%6.1f%8u%-4s%5u\n", 
			commodities[i].name, 
			(float)(localmarket.price[i])/10, 
			localmarket.quantity[i], 
			unitnames[commodities[i].unit], 
			shipshold[i]
		);
	printf("\nFuel: %-17.1fHoldspace: %it", (float)fuel/10, holdspace);
	return true;
}

bool
parser(char *s) /* Parse and execute command 's' */
{
	unsigned int i;
	char c[MAXLEN];
	spacesplit(s, c);
	i = stringmatch(c, commands, N_COMMANDS);
	if (i)
		return (*comfuncs[i - 1])(s);
	printf("Bad command: %s", c);
	return false;
}

bool
doquit(char *s)
{
	(void)(&s);
	exit(EXIT_SUCCESS);
	return false;
}

bool
dohelp(char *s)
{
	(void)(&s);
	puts(
	"Commands:"
	"\nbuy   <amount>"
	"\nsell  <amount>"
	"\nfuel  <amount>     (buy amount light years of fuel)"
	"\njump  <planetname> (limited by fuel)"
	"\nsneak <planetname> (any distance - no fuel cost)"
	"\ngalhyp             (jumps to next galaxy)"
	"\ninfo  <planetname> (prints info on planet)"
	"\nmarket             (shows market prices)"
	"\nlocal              (lists planets within 7 light years)"
	"\ncash  <number>     (alters cash - cheating!)"
	"\nhold  <number>     (change cargo bay)"
	"\nhelp               (display this text)"
	"\nquit or ^C         (exit)"
	"\n\nAbbreviations allowed e.g. b fo 5 = buy food 5"
	);
	return true;
}

int
gen_rnd_number(void)
{
	int a, x;
	x = (rnd_seed.a * 2) & 0xFF;
	a = x + rnd_seed.c;
	if (rnd_seed.a > 0x7F)
		a++;
	rnd_seed.a = a & 0xFF;
	rnd_seed.c = x;

	a /= 0x100;	/* a = any carry left from above */
	x = rnd_seed.b;
	a = (a + x + rnd_seed.d) & 0xFF;
	rnd_seed.b = a;
	rnd_seed.d = x;
	return a;
}

void
goat_soup(const char *source, Planet *p)
{
	for (uint8_t c = *source; c != '\0'; c = *++source) {
		if (c < 0x80) {
			putchar(c);
		} else {
			if (c <= 0xA4) {
				int rnd = gen_rnd_number();
				goat_soup(desc_list[c - 0x81].option[(rnd * 5) / 0xFF], p);
			} else switch(c) {
				case 0xB0: /* planet name */ {
					int i = 1;
					putchar(p->name[0]);
					while (p->name[i] != '\0')
						putchar(tolower(p->name[i++]));
				}	break;
				case 0xB1: /* <planet name>ian */ {
					int i = 1;
					putchar(p->name[0]);
					while (p->name[i] != '\0') {
						if (p->name[i + 1] != '\0' 
						  || (p->name[i] != 'E' 
						  && p->name[i] != 'I'))
							putchar(tolower(p->name[i]));
						i++;
					}
					fputs("ian", stdout);
				}	break;
				case 0xB2: /* random name */ {
					int len = gen_rnd_number() & 3;
					for (int i = 0; i <= len; i++) {
						int x = gen_rnd_number() & 0x3E;
						if (i == 0)
							putchar(pairs0[x]);
						else
							putchar(tolower(pairs0[x]));
						putchar(tolower(pairs0[x + 1]));
					}
				}	break;
				default:
					printf("<bad char in data [%X]>", c);
					return;
			}
		}
	}
}


int
main(void)
{
	char com[MAXLEN], buf[16];

	srand(time(NULL));

	for (int i = 0; i <= LAST_TRADE; i++)
		strcpy(item_names[i], commodities[i].name);

	galaxy_index = 1;
	buildgalaxy(galaxy_index);

	curr_planet = Lave_INDEX;	/* Don't use jump */
	localmarket = genmarket(0x00, galaxies[Lave_INDEX]);
	fuel = maxfuel;

	puts("Welcome to Text Elite 1.5.\n");

#define PARSER(S) { strcpy(buf, S); parser(buf); }
	PARSER("hold 20");		/* Small cargo bay */
	PARSER("cash +100");	/* 100 CR */
	PARSER("help");
#undef PARSER

	for (;;) {
		printf("\n$%.1f> ", (float)cash/10);
		fgets(com, MAXLEN, stdin);
		com[strcspn(com, "\n")] = '\0';
		if (*com)
			parser(com);
	}
	/* Unreachable */
	return 0;
}
