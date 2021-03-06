#include "dbg.h"
#include "constructors.h"

struct StrictlyProperBlock * block_new(struct StrictlyProperBlock const stackb)
{
	struct StrictlyProperBlock * heapb = malloc(sizeof(struct StrictlyProperBlock));
	if (heapb)
		*heapb = stackb;
	return heapb;
}

void block_free(struct StrictlyProperBlock * heapb)
{
	free(heapb);
}

struct BlockSystemStorage * blockSystemStorage_new
(
	size_t const numBlocks,
	struct StrictlyProperBlock const * const blocks,
	CalcBlockInputsFunction const calcBlockInputs,
	BlockSystemOutputFunction const systemOutputs,
	BlockSystemUtilityFunction const systemUtility,
	void * const systemStorage
)
{
	size_t totalBlockInputs = 0;
	size_t totalBlockOutputs = 0;
	for (size_t i = 0; i < numBlocks; i++)
	{
		totalBlockInputs += blocks[i].numInputs;
		totalBlockOutputs += blocks[i].numOutputs;
	}

	double * storage = NULL;
	double ** input_storage = NULL;
	double ** output_storage = NULL;
	struct BlockSystemStorage * bheap = NULL;

	storage = malloc((totalBlockInputs + totalBlockOutputs) * sizeof(double));
	check_mem(storage);
	input_storage = malloc(numBlocks * sizeof(double*));
	check_mem(input_storage);
	output_storage = malloc(numBlocks * sizeof(double*));
	check_mem(output_storage);
	bheap = malloc(sizeof(struct BlockSystemStorage));
	check_mem(bheap);

	size_t inputi = 0;
	size_t outputi = totalBlockInputs;
	for (size_t i = 0; i < numBlocks; i++)
	{
		input_storage[i] = &storage[inputi];
		output_storage[i] = &storage[outputi];
		inputi += blocks[i].numInputs;
		outputi += blocks[i].numOutputs;
	}

	struct BlockSystemStorage bstack;
	bstack.numBlocks = numBlocks;
	bstack.blocks = blocks;
	bstack.blockInputs = input_storage;
	bstack.blockOutputs = output_storage;
	bstack.calcBlockInputs = calcBlockInputs;
	bstack.systemOutput = systemOutputs;
	bstack.systemUtility = systemUtility;
	bstack.systemStorage = systemStorage;

	*bheap = bstack;
	return bheap;
error:
	free(storage);
	free(input_storage);
	free(output_storage);
	free(bheap);
	return NULL;
}

void blockSystemStorage_free(struct BlockSystemStorage * storage)
{
	free(storage->blockInputs[0]);
	free((void*)storage->blockInputs); //cast avoids a warning
	free((void*)storage->blockOutputs); //cast avoids a warning
	free(storage);
}