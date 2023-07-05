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
	/* 2-byte arithmetic */
	temp = s->w0 + s->w1 + s->w2;
	s->w0 = s->w1;
	s->w1 = s->w2;
	s->w2 = temp;
}

/*
 * String functions for text interface
 */

void
stripout(char *s, char c) /* Remove every 'c' from string 's' */
{
	char *r = s, *w = s;
	while (*r) {
		*w = *r++;
		w += (*w != c);
	}
	*w = '\0';
}

bool
stringbegins(const char *s, const char *t)
/* Return true iff string 't' begins with non-empty string 's' */
{
	size_t i = 0, l = strlen(s);
	while (i < l && toupper(s[i]) == toupper(t[i]))
		i++;
	return i == l;
}

unsigned int
stringmatch(const char *s, const char *a[], unsigned int n)
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
split(char *s, char *t, char c)
/* Split string 's' at first 'c',
	returning first word in 't' and shortening 's' */
{
	size_t i = 0, j = 0, l = strlen(s);
	while (i < l && isspace(s[i]))
		i++;	/* Strip leading spaces */

	if (i == l) {
		s[0] = 0; t[0] = 0;
		return;
	}

	while (i < l && s[i] != c)
		t[j++] = s[i++];

	t[j] = 0; i++; j = 0;

	while (i < l)
		s[j++] = s[i++];

	s[j] = 0;
}

/*
 * Functions for stock market
 */

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
makemarket(uint8_t fluct, const Planet *p)
{
	Market market;
	signed int q, product, changing;
	unsigned int i;
	for (i = 0; i < N_ITEMS; i++) {
		product = p->eco * commodities[i].gradient;
		changing = fluct & commodities[i].mask;
		q = commodities[i].basequant + changing - product;
		q &= 0xFF;
		if (q & 0x80)
			q = 0; /* Clip to positive 8-bit */

		market.quantity[i] = (uint16_t)(q & 0x3F); /* Mask to 6 bits */

		q = commodities[i].baseprice + changing + product;
		q &= 0xFF;
		market.price[i] = (uint16_t)(q * 4);
	}
	/* Override to force non-availability */
	market.quantity[ALIEN_ITEMS_INDEX] = 0;
	return market;
}

Planet
makeplanet(Seed *s) /* Generate planet info from seed */
{
	Planet p;
	unsigned int pair1, pair2, pair3, pair4;
	bool longnameflag = s->w0 & 0x40;

	p.x = s->w1 >> 8;
	p.y = s->w0 >> 8;

	p.gov = (s->w1 >> 3) & 7;	/* bits 3, 4 and 5 of w1 */
	p.eco = (s->w0 >> 8) & 7;	/* bits 8, 9 and A of w0 */

	if (p.gov <= 1)
		p.eco |= 2;

	p.tech = ((s->w1 >> 8) & 3) + (p.eco ^ 7) + (p.gov >> 1);
	if (p.gov & 1)
		p.tech++;
	/* C simulation of 6502's LSR then ADC */

	p.pop = 4 * p.tech + p.eco + p.gov + 1;
	p.prod = ((p.eco ^ 7) + 3) * (p.gov + 4) * p.pop * 8;
	p.radius = 0x100 * (((s->w2 >> 8) & 0x0F) + 11) + p.x;

	p.goatsoupseed.a = s->w1 & 0xFF;
	p.goatsoupseed.b = s->w1 >> 8;
	p.goatsoupseed.c = s->w2 & 0xFF;
	p.goatsoupseed.d = s->w2 >> 8;

	/* Always 4 iterations of random number */
	pair1 = 2 * ((s->w2 >> 8) & 0x1F); tweakseed(s);
	pair2 = 2 * ((s->w2 >> 8) & 0x1F); tweakseed(s);
	pair3 = 2 * ((s->w2 >> 8) & 0x1F); tweakseed(s);
	pair4 = 2 * ((s->w2 >> 8) & 0x1F); tweakseed(s);

	p.name[0] = pairs[pair1];
	p.name[1] = pairs[pair1 + 1];
	p.name[2] = pairs[pair2];
	p.name[3] = pairs[pair2 + 1];
	p.name[4] = pairs[pair3];
	p.name[5] = pairs[pair3 + 1];

	if (longnameflag) {
		p.name[6] = pairs[pair4];
		p.name[7] = pairs[pair4 + 1];
		p.name[8] = 0;
	} else p.name[6] = 0;

	stripout(p.name, '.');
	return p;
}


/*
 * Generate galaxy
 * Functions for galactic hyperspace
 */

uint16_t
lrotate(uint16_t x) /* rotate 8-bit(?) number leftwards */
{
	return (2 * (x & 0x7F)) + ((x & 0x80) >> 7);
}

uint16_t
twist(uint16_t x)
{
	return 0x100 * lrotate(x >> 8) + lrotate(x & 0xFF);
}

void
nextgalaxy(Seed *s) /* Apply to base seed; once for galaxy 2 */
{
	s->w0 = twist(s->w0);	/* Twice for galaxy 3, etc. */
	s->w1 = twist(s->w1);	/* 8th application gives galaxy 1 again */
	s->w2 = twist(s->w2);
}

/* Original game generated from scratch each time info needed */
void
buildgalaxy(uint8_t galaxy_index)
{
	unsigned int i;
	/* Initialise seed for galaxy 1 */
	seed.w0 = base0; seed.w1 = base1; seed.w2 = base2;
	for (i = 1; i < galaxy_index; ++i)
		nextgalaxy(&seed);
	/* Put galaxy data into array of structs */  
	for (i = 0; i < GALAXY_SIZE; ++i)
		galaxy[i] = makeplanet(&seed);
}

/*
 * Functions for navigation
 */

void
gamejump(unsigned int i) /* Move to planet index 'i' */
{
	curr_planet = i;
	localmarket = makemarket(rand() % 0x100, &galaxy[i]);
}

unsigned int
distance(Planet *a, Planet *b)
{
	int xdelta = a->x - b->x, ydelta = a->y - b->y;
	return 4 * sqrt(xdelta * xdelta + ydelta * ydelta / 4);
}

unsigned int
matchplanet(char *s)
/* Return id of the planet whose name matches passed string
	closest to 'curr_planet' - if none return 'curr_planet' */
{
	unsigned int p = curr_planet, max_dist = -1, curr_dist, i;
	for (i = 0; i < GALAXY_SIZE; ++i) {
		if (stringbegins(s, galaxy[i].name)) {
			curr_dist = distance(&galaxy[i], &galaxy[curr_planet]);
		 	if (curr_dist < max_dist) {
		 		max_dist = curr_dist;
				p = i;
			}
		}
	}
	return p;
}

void
printplanet(const Planet *p, bool brief)
{
	if (brief) {
		printf("%12s TL: %2i %12s %16s", 
			p->name, p->tech + 1, 
			econames[p->eco], govnames[p->gov]);
	} else {
		printf(
			"Data on %s (%i, %i)"
			"\nEconomy: %s"
			"\nGovernment: %s"
			"\nTech Level: %2i"
			"\nPopulation: %.1f Billion"
//			"\n(%s)"	// Species
			"\nTurnover: %u Mcr"
			"\nRadius: %u km\n", 
			p->name, p->x, p->y, econames[p->eco], 
			govnames[p->gov], p->tech + 1, (float)p->pop / 10, 
			p->prod, p->radius
		);
		rnd_seed = p->goatsoupseed;
		goat_soup(GS_FMT, p);
	}
}

/*
 * Various command functions
 */

bool
dolocal(char *s)
{
	unsigned int d, i;
	(void)(&s);
	printf("Galaxy #%i\n", galaxy_index);
	for (i = 0; i < GALAXY_SIZE; ++i) {
		d = distance(&galaxy[i], &galaxy[curr_planet]);
		if (d <= maxfuel) {
			if (d <= fuel)
				putchar('*');
			else
				putchar('-');
			printplanet(&galaxy[i], true);
			printf("  (%.1fLY)\n", (float)d / 10);
		}
	}
	return true;
}

bool
dojump(char *s) /* Jump to planet name 's' */
{
	unsigned int d, dest = matchplanet(s);
	if (dest == curr_planet) {
		puts("Bad jump");
		return false;
	}
	d = distance(&galaxy[dest], &galaxy[curr_planet]);
	if (d > fuel) {
		puts("Jump too far");
		return false;
	}
	fuel -= d;
	gamejump(dest);
	printplanet(&galaxy[curr_planet], false);
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
	if (++galaxy_index > 8)
		galaxy_index = 1;
	buildgalaxy(galaxy_index);
	return true;
}

bool
doinfo(char *s)
{
	unsigned int dest = matchplanet(s);
	printplanet(&galaxy[dest], false);
	return true;
}

bool
dohold(char *s)
{
	unsigned int a = atoi(s), t = 0, i;
	for (i = 0; i < N_ITEMS; i++) {
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
	split(s, s2, ' ');
	a = atoi(s);
	if (a == 0)
		a = 1;
	i = stringmatch(s2, item_names, N_ITEMS);
	if (i == 0) {
		puts("Unknown item");
		return false;
	}
	t = gamesell(--i, a);
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
	split(s, s2, ' ');
	a = atoi(s);
	if (a == 0)
		a = 1;
	i = stringmatch(s2, item_names, N_ITEMS);
	if (i == 0) {
		puts("Unknown item");
		return false;
	}
	t = gamebuy(--i, a);
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
	printf("Buying %.1fLY fuel", (float)f / 10);
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
	unsigned int i;
	(void)(&s);
	printf("%-16s%-8s%-8s%8s\n", "Name", "Price", "Quantity", "Loaded");
	for (i = 0; i < N_ITEMS; i++)
		printf("%-16s%5.1f%10u%-4s%5u\n", 
			commodities[i].name, 
			(float)(localmarket.price[i]) / 10, 
			localmarket.quantity[i], 
			unitnames[commodities[i].unit], 
			shipshold[i]
		);
	printf("\nFuel: %-17.1fHoldspace: %it", (float)fuel / 10, holdspace);
	return true;
}

bool
parse(char *s) /* Parse and execute command 's' */
{
	unsigned int i; char com[MAXLEN];
	split(s, com, ' ');
	i = stringmatch(com, commands, N_COMMANDS);
	if (i)
		return (*comfuncs[i - 1])(s);
	printf("Bad command: %s", com);
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
	"\ngalhyp             (jump to next galaxy)"
	"\ninfo  <planetname> (print info on planet)"
	"\nmarket             (show market prices)"
	"\nlocal              (list planets within 7 light years)"
	"\ncash  <number>     (alter cash - cheat!)"
	"\nhold  <number>     (change cargo bay)"
	"\nhelp               (display this text)"
	"\nquit or ^C         (exit)"
	"\n\nAbbreviations allowed, e.g. b fo 5 = buy food 5"
	);
	return true;
}

uint8_t
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
goat_soup(const char *s, const Planet *p)
{
	uint8_t c;
	for (c = *s; c != '\0'; c = *++s) {
		if (c < 0x80) {
			putchar(c);
		} else {
			c ^= 0x80;
			if (c < N_DESC) {
				int r = gen_rnd_number();
				goat_soup(descriptions[c][(r * N_OPTIONS) / 0xFF], p);
			} else switch(c ^ 0x80) {
				case 0xF0: /* planet name */ {
					unsigned int i = 1;
					putchar(p->name[0]);
					while (p->name[i] != '\0')
						putchar(tolower(p->name[i++]));
				}	break;
				case 0xF1: /* <planet name>ian */ {
					unsigned int i = 1;
					putchar(p->name[0]);
					while (p->name[i] != '\0') {
						if (p->name[i + 1] != '\0' 
						  || (p->name[i] != 'E' && p->name[i] != 'I'))
							putchar(tolower(p->name[i]));
						i++;
					}
					fputs("ian", stdout);
				}	break;
				case 0xF2: /* random name */ {
					unsigned int i, x, len = gen_rnd_number() & 3;
					for (i = 0; i <= len; i++) {
						x = gen_rnd_number() & 0x3E;
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

	galaxy_index = 1;
	buildgalaxy(galaxy_index);

	curr_planet = Lave_INDEX;	/* Don't use jump */
	localmarket = makemarket(0x00, &galaxy[Lave_INDEX]);
	fuel = maxfuel;

	puts("Welcome to Text Elite 1.5.\n");

#define PARSE(S) { strcpy(buf, S); parse(buf); }
	PARSE("hold 20");		/* Small cargo bay */
	PARSE("cash +100");		/* 100 CR */
	PARSE("help");
#undef PARSE

	for (;;) {
		printf("\n$%.1f>", (float)cash / 10);
		fgets(com, MAXLEN, stdin);
		com[strcspn(com, "\n")] = '\0';
		if (*com)
			parse(com);
	}
	/* Unreachable */
	return 0;
}
