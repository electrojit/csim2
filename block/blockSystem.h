#ifndef _BLOCKSYSTEM_H_
#define _BLOCKSYSTEM_H_

#include "block.h"

struct blockSystem;

typedef void(*updateChildInputFunction)(
	struct blockSystem const * system,
	FLOAT_TYPE time,
	FLOAT_TYPE * childState[], // array of child block state arrays
	FLOAT_TYPE const systemInput[]
	);

struct blockSystem
{
	size_t numChildren; // number of child blocks
	size_t numInputs; // number of inputs
	struct block * child; // array of child blocks
	FLOAT_TYPE ** childInput; // array of child block input arrays
	void * storage; // put anything you want here
	updateChildInputFunction updateChildInput; // childInput = f(time, childStates, systemInput)
};

struct block * blockSystem( struct block * block, struct blockSystem * system );

#endif
