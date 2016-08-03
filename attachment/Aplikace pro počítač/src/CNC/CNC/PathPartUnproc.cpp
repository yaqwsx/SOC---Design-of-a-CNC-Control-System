#include "PathPartUnproc.h"


PathPartUnproc::PathPartUnproc(string t, size_t l): text(t)
{
	SetLine(l);
}


PathPartUnproc::~PathPartUnproc(void)
{
}

PathPartSimple PathPartUnproc::ToSimple()
{
	PathPartSimple ret;
	ret.type = UNPROC;
	ret.typeEx = PPEx_TEXT;
	ret.text = text;
	ret.line = GetLine();
	return ret;
};
