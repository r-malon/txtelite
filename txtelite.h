#ifndef TXTELITE_H
#define TXTELITE_H

#define MAXLEN (20) /* Length of strings */
#define GALAXY_SIZE (256)
#define LAST_TRADE (16)

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

enum {
	Lave_INDEX=7,	/* Lave is 7th generated planet in galaxy one */
	Zaonce_INDEX=129,
	Diso_INDEX=147,
	Ried_INDEX=46
};

enum Unit { TONNE, KILO, GRAM };

typedef struct {
	uint8_t a, b, c, d;
} FastSeed;  /* four byte random number used for planet description */

typedef struct {
	uint16_t w0, w1, w2;
} Seed;  /* six byte random number used as seed for planets */

typedef struct {
	unsigned int x;
	unsigned int y;				/* One byte unsigned */
	unsigned int economy;		/* These two are actually only 0-7 */
	unsigned int govtype;
	unsigned int techlev;		/* 0-16 I think */
	unsigned int population;	/* One byte */
	unsigned int productivity;	/* Two byte */
	unsigned int radius;		/* Two byte (not used at all) */
	FastSeed	goatsoupseed;
	char name[12];
} Planet;

typedef struct {
	unsigned int baseprice;
	int16_t gradient;
	unsigned int basequant;
	uint8_t maskbyte;
	enum Unit unit;
	char name[MAXLEN]; /* longest="Radioactives" */
} Item;

typedef struct {
	unsigned int quantity[LAST_TRADE + 1];
	unsigned int price[LAST_TRADE + 1];
} Market;

Planet galaxies[GALAXY_SIZE]; /* Need 0 to GALAXY_SIZE-1 inclusive */
Seed seed;
FastSeed rnd_seed;

/* Player workspace */
unsigned int shipshold[LAST_TRADE + 1];	/* Contents of cargo bay */
int curr_planet;						/* Current planet */
uint8_t galaxy_index;					/* Galaxy number (1-8) */
int32_t cash;
unsigned int fuel;
unsigned int holdspace;
Market localmarket; /* If declared further up, 'cash' bugs */

unsigned int fuelcost = 2; /* 0.2 CR/LY */
unsigned int maxfuel = 70; /* 7.0 LY tank */

const uint16_t base0 = 0x5A4A;
const uint16_t base1 = 0x0248;
const uint16_t base2 = 0xB753;	/* Base seed for galaxy 1 */

char unitnames[][3] = {"t", "kg", "g"};

char pairs0[] = 
"ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSES"
"ARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

char pairs[] = 
"..LEXEGEZACEBISO"
"USESARMAINDIREA."
"ERATENBERALAVETI"
"EDORQUANTEISRION"; /* Dots should be nullprint characters */

char govnames[][MAXLEN] = {
	"Anarchy",
	"Feudal",
	"Multi-Gov",
	"Dictatorship",
	"Communist",
	"Confederacy",
	"Democracy",
	"Corporate State"
};
char econnames[][MAXLEN] = {
	"Rich Ind",
	"Average Ind",
	"Poor Ind",
	"Mainly Ind",
	"Mainly Agri",
	"Rich Agri",
	"Average Agri",
	"Poor Agri"
};

/* Data for DB's price/availability generation system */
/* Base  Grad Base Mask Un   Name
	price ient quant     it */
Item commodities[] = {
	{0x13, -0x02, 0x06, 0x01, TONNE, "Food"}, 
	{0x14, -0x01, 0x0A, 0x03, TONNE, "Textiles"}, 
	{0x41, -0x03, 0x02, 0x07, TONNE, "Radioactives"}, 
	{0x28, -0x05, 0xE2, 0x1F, TONNE, "Slaves"}, 
	{0x53, -0x05, 0xFB, 0x0F, TONNE, "Liquor/Wines"}, 
	{0xC4, +0x08, 0x36, 0x03, TONNE, "Luxuries"}, 
	{0xEB, +0x1D, 0x08, 0x78, TONNE, "Narcotics"}, 
	{0x9A, +0x0E, 0x38, 0x03, TONNE, "Computers"}, 
	{0x75, +0x06, 0x28, 0x07, TONNE, "Machinery"}, 
	{0x4E, +0x01, 0x11, 0x1F, TONNE, "Alloys"}, 
	{0x7C, +0x0d, 0x1D, 0x07, TONNE, "Firearms"}, 
	{0xB0, -0x09, 0xDC, 0x3F, TONNE, "Furs"}, 
	{0x20, -0x01, 0x35, 0x03, TONNE, "Minerals"}, 
	{0x61, -0x01, 0x42, 0x07, KILO, "Gold"}, 
	{0xAB, -0x02, 0x37, 0x1F, KILO, "Platinum"}, 
	{0x2D, -0x01, 0xFA, 0x0F, GRAM, "Gem stones"}, 
	{0x35, +0x0F, 0xC0, 0x07, TONNE, "Alien items"}, 
};

/**-Required data for text interface **/
char item_names[LAST_TRADE][MAXLEN];
/* Item names used in text commands, set using commodities array */


#define N_COMMANDS (13)

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
void goat_soup(const char *, Planet *);

char commands[N_COMMANDS][MAXLEN] = {
	"buy",		"sell",		"fuel",		"jump",
	"cash",		"market",	"help",		"hold",
	"sneak",	"local",	"info",		"galhyp",	"quit"
};

bool (*comfuncs[N_COMMANDS])(char *) = {
	dobuy,		dosell,		dofuel,		dojump,
	docash,		domarket,	dohelp,		dohold,
	dosneak,	dolocal,	doinfo,		dogalhyp,	doquit
};

/*
"Goat Soup" planetary description string code
adapted from Christian Pinder's reverse engineered sources.
*/
struct desc_choice {
	const char *option[5];
};

static struct desc_choice desc_list[] = {
/* 81 */	{"fabled", "notable", "well-known", "famous", "noted"},
/* 82 */	{"very", "mildly", "most", "reasonably", ""},
/* 83 */	{"ancient", "\x95", "great", "vast", "pink"},
/* 84 */	{"\x9E \x9D plantations", "mountains", "\x9C", "\x94 forests", "oceans"},
/* 85 */	{"shyness", "silliness", "mating traditions", "loathing of \x86", "love for \x86"},
/* 86 */	{"food blenders", "tourists", "poetry", "discos", "\x8E"},
/* 87 */	{"talking tree", "crab", "bat", "lobst", "\xB2"},
/* 88 */	{"beset", "plagued", "ravaged", "cursed", "scourged"},
/* 89 */	{"\x96 civil war", "\x9B \x98 \x99s", "a \x9B disease", "\x96 earthquakes", "\x96 solar activity"},
/* 8A */	{"its \x83 \x84", "the \xB1 \x98 \x99", "its inhabitants' \x9A \x85", "\xA1", "its \x8D \x8E"},
/* 8B */	{"juice", "brandy", "water", "brew", "gargle blasters"},
/* 8C */	{"\xB2", "\xB1 \x99", "\xB1 \xB2", "\xB1 \x9B", "\x9B \xB2"},
/* 8D */	{"fabulous", "exotic", "hoopy", "unusual", "exciting"},
/* 8E */	{"cuisine", "night life", "casinos", "sitcoms", " \xA1 "},
/* 8F */	{"\xB0", "The planet \xB0", "The world \xB0", "This planet", "This world"},
/* 90 */	{"n unremarkable", " boring", " dull", " tedious", " revolting"},
/* 91 */	{"planet", "world", "place", "little planet", "dump"},
/* 92 */	{"wasp", "moth", "grub", "ant", "\xB2"},
/* 93 */	{"poet", "arts graduate", "yak", "snail", "slug"},
/* 94 */	{"tropical", "dense", "rain", "impenetrable", "exuberant"},
/* 95 */	{"funny", "weird", "unusual", "strange", "peculiar"},
/* 96 */	{"frequent", "occasional", "unpredictable", "dreadful", "deadly"},
/* 97 */	{"\x82 \x81 for \x8A", "\x82 \x81 for \x8A and \x8A", "\x88 by \x89", "\x82 \x81 for \x8A but \x88 by \x89", "a\x90 \x91"},
/* 98 */	{"\x9B", "mountain", "edible", "tree", "spotted"},
/* 99 */	{"\x9F", "\xA0", "\x87oid", "\x93", "\x92"},
/* 9A */	{"ancient", "exceptional", "eccentric", "ingrained", "\x95"},
/* 9B */	{"killer", "deadly", "evil", "lethal", "vicious"},
/* 9C */	{"parking meters", "dust clouds", "icebergs", "rock formations", "volcanoes"},
/* 9D */	{"plant", "tulip", "banana", "corn", "\xB2weed"},
/* 9E */	{"\xB2", "\xB1 \xB2", "\xB1 \x9B", "inhabitant", "\xB1 \xB2"},
/* 9F */	{"shrew", "beast", "bison", "snake", "wolf"},
/* A0 */	{"leopard", "cat", "monkey", "goat", "fish"},
/* A1 */	{"\x8C \x8B", "\xB1 \x9F \xA2", "its \x8D \xA0 \xA2", "\xA3 \xA4", "\x8C \x8B"},
/* A2 */	{"meat", "cutlet", "steak", "burgers", "soup"},
/* A3 */	{"ice", "mud", "Zero-G", "vacuum", "\xB1 ultra"},
/* A4 */	{"hockey", "cricket", "karate", "polo", "tennis"}
};
/*
0xB0 = <planet name>
0xB1 = <planet name>ian
0xB2 = <random name>
*/
#endif /* TXTELITE_H */
