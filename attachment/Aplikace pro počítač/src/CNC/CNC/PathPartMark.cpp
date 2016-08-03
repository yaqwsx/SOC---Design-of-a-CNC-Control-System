#include "PathPartMark.h"


PathPartMark::PathPartMark(PathPartType t, string text, size_t line):
	PathPartUnproc(text, line), type(t)
{
}


PathPartMark::~PathPartMark(void)
{
}
