#include <math.h>
#include <string.h>
#include <assert.h>
#include "solvers.h"

void euler_f_step
(
	size_t const numStates,
	size_t const numInputs,
	double const dt, // time step
	double const currentTime,
	double * const nextState, // (1 x numStates)
	double * const currentdState, // (1 x numStates)
	double const * const currentState, // (1 x numStates) also initial conditions
	double const * const currentInput, // (1 x numInputs)
	PhysicsFunction const f,
	void * const storage
)
{
	size_t i;
	f(numStates, numInputs, currentdState, currentTime, currentState, currentInput, storage);
	for (i = 0; i < numStates; i++)
		nextState[i] = currentState[i] + currentdState[i] * dt;
}

void rk4_f_step
(
	size_t const numStates,
	size_t const numInputs,
	double const dt, // time step
	double const currentTime, // current time
	double * const nextState, // (1 x numStates)
	double * const currentdState, // (1 x numStates)
	double * const B, // (1 x numStates) solver storage/temp
	double * const C, // (1 x numStates) solver storage/temp
	double * const D, // (1 x numStates) solver storage/temp
	double const * const currentState, // (1 x numStates) also initial conditions
	double const * const currentInput, // (1 x numInputs)
	double const * const currentInput2, // (1 x numInputs) input at time ti + dt/2
	double const * const nextInput, // (1 x numInputs) input at time ti + dt
	PhysicsFunction const f,
	void * const storage
)
{
	size_t i;
	double const dt2 = dt / 2;
	double const currentTime2 = currentTime + dt2;
	double const nextTime = currentTime + dt;
	double * const A = currentdState;

	assert(currentState != nextState);
	assert(A != B && A != C && A != D && A != nextState && A != currentState);
	assert(B != C && B != D && B != nextState && B != currentState);
	assert(C != D && C != nextState && C != currentState);
	assert(D != nextState && D != currentState);

	f(numStates, numInputs, A, currentTime, currentState, currentInput, storage);
	for (i = 0; i < numStates; i++)
		nextState[i] = currentState[i] + A[i] * dt2;

	f(numStates, numInputs, B, currentTime2, nextState, currentInput2, storage);
	for (i = 0; i < numStates; i++)
		nextState[i] = currentState[i] + B[i] * dt2;

	f(numStates, numInputs, C, currentTime2, nextState, currentInput2, storage);
	for (i = 0; i < numStates; i++)
		nextState[i] = currentState[i] + C[i] * dt;

	f(numStates, numInputs, D, nextTime, nextState, nextInput, storage);
	for (i = 0; i < numStates; i++)
		nextState[i] = currentState[i] + dt * (A[i] + 2 * B[i] + 2 * C[i] + D[i]) / 6;
}

void euler
(
	struct StrictlyProperBlock block,
	double const dt, //time step
	size_t numSteps,
	double const * const time, // numSteps x 1 time vector
	size_t numStates,
	double const * const Xi, // numStates x 1 initial conditions vector (will be over-written)
	size_t numInputs,
	double const * const U, // numSteps x numInputs input values over time
	size_t numOutputs,
	double * const Y // numSteps x numOutputs output buffer
)
{
	double * const temp_memory = malloc(block.numStates * 2 * sizeof(double));
	if (!temp_memory)
		return;
	
	double * const currentState = &temp_memory[0 * block.numStates];
	double * const nextState = currentState; //in this case it's ok that these are the same block of memory
	double * const currentdState = &temp_memory[1 * block.numStates];

	
	double const * currentInput;
	double * currentOutput;

	memcpy(currentState, Xi, block.numStates * sizeof(double));
	size_t i;
	for (i = 0; i < (numSteps - 1); i++)
	{
		currentInput = &U[i*block.numInputs];
		currentOutput = &Y[i*block.numOutputs];

		block.h(block.numStates, block.numOutputs, currentOutput, time[i], currentState, block.storage);
		euler_f_step(block.numStates, block.numInputs, dt, time[i], nextState, currentdState, currentState, currentInput, block.f, block.storage);
	}

	//currentState = nextState;
	currentOutput = &Y[i*block.numOutputs];
	block.h(block.numStates, block.numOutputs, currentOutput, time[i], currentState, block.storage);

	free(temp_memory);
}

void rk4
(
	struct StrictlyProperBlock block,
	double const dt, //time step
	size_t numSteps, //size of time vector
	double const * const time, //time vector
	size_t numStates,
	double const * const Xi, // numStates x 1 initial conditions vector
	size_t numInputs,
	double const * const U1, // numSteps x numInputs inputs
	double const * const U2, // numSteps x numInputs inputs
	size_t numOutputs,
	double * const Y
)
{
	double * const temp_memory = malloc(block.numStates * 6 * sizeof(double));
	if (!temp_memory)
		return;

	double * const currentState = &temp_memory[0 * block.numStates];
	double * const nextState = &temp_memory[1 * block.numStates];
	double * const currentdState = &temp_memory[2 * block.numStates];
	double * const B = &temp_memory[3 * block.numStates];
	double * const C = &temp_memory[4 * block.numStates];
	double * const D = &temp_memory[5 * block.numStates];

	double const * currentInput;
	double const * currentInput2;
	double const * nextInput;
	double * currentOutput;

	memcpy(currentState, Xi, block.numStates * sizeof(double));
	size_t i;
	for (i = 0; i < (numSteps - 1); i++)
	{
		currentInput = &U1[i*block.numInputs];
		currentInput2 = &U2[i*block.numInputs];
		nextInput = &U1[(i + 1)*block.numInputs];
		currentOutput = &Y[i*block.numOutputs];

		block.h(block.numStates, block.numOutputs, currentOutput, time[i], currentState, block.storage);
		rk4_f_step(block.numStates, block.numInputs, dt, time[i], nextState, currentdState, B, C, D, currentState, currentInput, currentInput2, nextInput, block.f, block.storage);
		memcpy(currentState, nextState, block.numStates * sizeof(double));
	}

	currentOutput = &Y[i*block.numOutputs];
	block.h(block.numStates, block.numOutputs, currentOutput, time[i], currentState, block.storage);

	free(temp_memory);
}



size_t numTimeSteps
(
	double const dt,
	double const duration
)
{
	return (size_t)floor(duration / dt + 1);
}

void initializeTime
(
	size_t const numSteps,
	double * const time,
	double const dt,
	double const ti
)
{
	for (size_t i = 0; i < numSteps; i++)
		time[i] = i*dt + ti;
}