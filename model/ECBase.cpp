#include "ECBase.h"

char const * players_ec[] =
{
	"", "GAIA", "Player 1", "Player 2", "Player 3", "Player 4", "Player 5", "Player 6", "Player 7", "Player 8"
};

ECBase::ECBase(enum ECType c, long t, long s)
:	type(t), ectype(c), size(s)
{}

AOKRECT::AOKRECT(long t, long r, long b, long l)
: top(t), right(r), bottom(b), left(l)
{
}
