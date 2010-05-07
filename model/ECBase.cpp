#include "ECBase.h"

ECBase::ECBase(enum TType c, long t)
:	type(t), ttype(c)
{}

AOKRECT::AOKRECT(long t, long r, long b, long l)
: top(t), right(r), bottom(b), left(l)
{
}
