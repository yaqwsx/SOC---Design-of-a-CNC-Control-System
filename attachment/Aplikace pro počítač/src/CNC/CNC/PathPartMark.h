#pragma once
#include "pathpartunproc.h"


class PathPartMark :
	public PathPartUnproc
{
	public:
		PathPartMark(PathPartType type, string text, size_t line);
		~PathPartMark(void);
		PathPartType GetType(){return type;};
	private:
		PathPartType type;
};

