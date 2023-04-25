#ifndef TXTELITE_H
#define TXTELITE_H

#define MAXLEN 20 /* Length of strings */
#define GALAXY_SIZE 256

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

enum {
	Lave_INDEX=7,	/* Lave is 7th generated planet in galaxy one */
	Zaonce_INDEX=129,
	Diso_INDEX=147,
	Ried_INDEX=46,
};

enum Unit { TONNE, KILO, GRAM };

typedef struct {
	uint8_t baseprice;
	int16_t gradient;
	uint8_t basequant;
	uint8_t mask;
	enum Unit unit;
	char *name; /* longest="Radioactives" */
} Item;

#define I(index, name, baseprice, grad, basequant, mask, unit) index, 
enum ItemIndex {
	#include "items.tbl"
};
#undef I

#define I(index, name, baseprice, grad, basequant, mask, unit) name, 
static const char *item_names[] = {
	#include "items.tbl"
};
#undef I

#define I(index, name, baseprice, grad, basequant, mask, unit) \
	{baseprice, grad, basequant, mask, unit, name}, 
static const Item commodities[] = {
	#include "items.tbl"
};
#undef I

#define N_ITEMS (sizeof commodities / sizeof (Item))

typedef struct {
	uint8_t a, b, c, d;
} FastSeed;	/* four byte random number used for planet description */

typedef struct {
	uint16_t w0, w1, w2;
} Seed;	/* six byte random number used as seed for planets */

typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t eco;	/* These two are actually only 0-7 */
	uint8_t gov;
	uint8_t tech;	/* 0-16 */
	uint8_t pop;
	uint16_t prod;
	uint16_t radius;	/* Not used at all */
	FastSeed	goatsoupseed;
	char name[12];
} Planet;

typedef struct {
	unsigned int quantity[N_ITEMS];
	unsigned int price[N_ITEMS];
} Market;

Planet galaxies[GALAXY_SIZE];
Seed seed;
FastSeed rnd_seed;

/* Player workspace */
unsigned int shipshold[N_ITEMS];	/* Contents of cargo bay */
unsigned int curr_planet;			/* Current planet */
uint8_t galaxy_index;				/* Galaxy number (1-8) */
int32_t cash;
unsigned int fuel;
unsigned int holdspace;
Market localmarket;			/* If declared further up, 'cash' bugs */

unsigned int fuelcost = 2; /* 0.2 CR/LY */
unsigned int maxfuel = 70; /* 7.0 LY tank */

static const uint16_t base0 = 0x5A4A;
static const uint16_t base1 = 0x0248;
static const uint16_t base2 = 0xB753;	/* Base seed for galaxy 1 */

static const char *unitnames[] = {"t", "kg", "g"};

static const char *pairs0 = 
"ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSES"
"ARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

static const char *pairs = 
"..LEXEGEZACEBISO"
"USESARMAINDIREA."
"ERATENBERALAVETI"
"EDORQUANTEISRION"; /* Dots should be nullprint characters */

/* From most dangerous to safest */
static const char *govnames[] = {
	"Anarchy",
	"Feudal",
	"Multi-Government",
	"Dictatorship",
	"Communist",
	"Confederacy",
	"Democracy",
	"Corporate State"
};
static const char *econnames[] = {
	"Rich Ind",
	"Average Ind",
	"Poor Ind",
	"Mainly Ind",
	"Mainly Agri",
	"Rich Agri",
	"Average Agri",
	"Poor Agri"
};

bool dobuy(char *);
bool dosell(char *);
bool dofuel(char *);
bool dojump(char *);
bool docash(char *);
bool domarket(char *);
bool dohelp(char *);
bool dohold(char *);
bool dosneak(char *);
bool dolocal(char *);
bool doinfo(char *);
bool dogalhyp(char *);
bool doquit(char *);

static const char *commands[] = {
	"buy",		"sell",		"fuel",		"jump",
	"cash",		"market",	"help",		"hold",
	"sneak",	"local",	"info",		"galhyp",	"quit",
};

#define N_COMMANDS (sizeof commands / sizeof commands[0])

bool (*comfuncs[N_COMMANDS])(char *) = {
	dobuy,		dosell,		dofuel,		dojump,
	docash,		domarket,	dohelp,		dohold,
	dosneak,	dolocal,	doinfo,		dogalhyp,	doquit,
};

/*
"Goat Soup" planetary description string code
adapted from Christian Pinder's reverse engineered sources.
*/
void goat_soup(const char *, const Planet *);

#define N_OPTIONS 5
#define GS_FMT "\x8E is \x96."

static const char *descriptions[][N_OPTIONS] = {
/* 80 */	{"fabled", "notable", "well-known", "famous", "noted"},
/* 81 */	{"very", "mildly", "most", "reasonably", ""},
/* 82 */	{"ancient", "\x94", "great", "vast", "pink"},
/* 83 */	{"\x9D \x9C plantations", "mountains", "\x9B", "\x93 forests", "oceans"},
/* 84 */	{"shyness", "silliness", "mating traditions", "loathing of \x85", "love for \x85"},
/* 85 */	{"food blenders", "tourists", "poetry", "discos", "\x8D"},
/* 86 */	{"talking tree", "crab", "bat", "lobst", "\xF2"},
/* 87 */	{"beset", "plagued", "ravaged", "cursed", "scourged"},
/* 88 */	{"\x95 civil war", "\x9A \x97 \x98s", "a \x9A disease", "\x95 earthquakes", "\x95 solar activity"},
/* 89 */	{"its \x82 \x83", "the \xF1 \x97 \x98", "its inhabitants' \x99 \x84", "\xA0", "its \x8C \x8D"},
/* 8A */	{"juice", "brandy", "water", "brew", "gargle blasters"},
/* 8B */	{"\xF2", "\xF1 \x98", "\xF1 \xF2", "\xF1 \x9A", "\x9A \xF2"},
/* 8C */	{"fabulous", "exotic", "hoopy", "unusual", "exciting"},
/* 8D */	{"cuisine", "night life", "casinos", "sitcoms", " \xA0 "},
/* 8E */	{"\xF0", "The planet \xF0", "The world \xF0", "This planet", "This world"},
/* 8F */	{"n unremarkable", " boring", " dull", " tedious", " revolting"},
/* 90 */	{"planet", "world", "place", "little planet", "dump"},
/* 91 */	{"wasp", "moth", "grub", "ant", "\xF2"},
/* 92 */	{"poet", "arts graduate", "yak", "snail", "slug"},
/* 93 */	{"tropical", "dense", "rain", "impenetrable", "exuberant"},
/* 94 */	{"funny", "weird", "unusual", "strange", "peculiar"},
/* 95 */	{"frequent", "occasional", "unpredictable", "dreadful", "deadly"},
/* 96 */	{"\x81 \x80 for \x89", "\x81 \x80 for \x89 and \x89", "\x87 by \x88", "\x81 \x80 for \x89 but \x87 by \x88", "a\x8F \x90"},
/* 97 */	{"\x9A", "mountain", "edible", "tree", "spotted"},
/* 98 */	{"\x9E", "\x9F", "\x86oid", "\x92", "\x91"},
/* 99 */	{"ancient", "exceptional", "eccentric", "ingrained", "\x94"},
/* 9A */	{"killer", "deadly", "evil", "lethal", "vicious"},
/* 9B */	{"parking meters", "dust clouds", "icebergs", "rock formations", "volcanoes"},
/* 9C */	{"plant", "tulip", "banana", "corn", "\xF2weed"},
/* 9D */	{"\xF2", "\xF1 \xF2", "\xF1 \x9A", "inhabitant", "\xF1 \xF2"},
/* 9E */	{"shrew", "beast", "bison", "snake", "wolf"},
/* 9F */	{"leopard", "cat", "monkey", "goat", "fish"},
/* A0 */	{"\x8B \x8A", "\xF1 \x9E \xA1", "its \x8C \x9F \xA1", "\xA2 \xA3", "\x8B \x8A"},
/* A1 */	{"meat", "cutlet", "steak", "burgers", "soup"},
/* A2 */	{"ice", "mud", "Zero-G", "vacuum", "\xF1 ultra"},
/* A3 */	{"hockey", "cricket", "karate", "polo", "tennis"},
};

#define N_DESC (sizeof descriptions / sizeof descriptions[0])

/*
0xF0 = <planet name>
0xF1 = <planet name>ian
0xF2 = <random name>
*/
#endif /* TXTELITE_H */
