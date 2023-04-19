# Text Elite
[Ian Bell's Text Elite](http://www.iancgbell.clara.net/elite/text/)\
source code fixed, cleaned, and ported to C99.

## Comments
The nature of basic mechanisms used to generate the Elite socio-economic
universe are now widely known. A competent games programmer should be able to
produce equivalent functionality. A competent hacker should be able to lift 
the exact system from the object code base of official conversions.

This file may be regarded as defining the Classic Elite universe.

It contains a C implementation of the precise 6502 algorithms used in the
original BBC Micro version of Acornsoft Elite (apart from Galactic Hyperspace
target systems) together with a parsed textual command testbed.

Note that this is not the universe of David Braben's 'Frontier' series.

6502 Elite fires up at Lave with fluctuation=00
and these prices tally with the NES ones.
However, the availabilities reside in the saved game data.
Availabilities are calculated (and fluctuation randomised) on hyperspacing
I have checked this code for Zaonce with fluctaution &AB against the 
SuperVision 6502 code and both prices and availabilities tally.

Prices and availabilities are influenced by the planet's economy type
(0-7) and a random "fluctuation" byte that was kept within the saved
commander position to keep the market prices constant over gamesaves.
Availabilities must be saved with the game since the player alters them
by buying (and selling(?))

Almost all operations are one byte only and overflow "errors" are
extremely frequent and exploited.

Item prices are held internally in a single byte=true value/4.
The decimal point in prices is introduced only when printing them.
Internally, all prices are integers. The player's cash is held in four bytes.

**See also**:\
[Go implementation](https://github.com/andrewsjg/GoElite/)
