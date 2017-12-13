#include "WorkerAI.h"

void WorkerAI::setState(int state)
{
	this->currentState = state;

}

WorkerAI::WorkerAI(Unit unit)
{
	this->workerPtr = unit;
	this->currentState = WorkerStates::LookForMinerals;
	this->stateStack.push(this->currentState);
	this->activeEvent = false;
	this->currentEvent = INT_MAX;
	this->isBuilding = false;
}

void WorkerAI::setEvent(int event)
{
	this->currentEvent = event;
	this->activeEvent = true;
}

void WorkerAI::update()
{
	switch (this->currentState)
	{
	case WorkerStates::LookForMinerals:
		//Implement a function that finds the nearest mineral from the command center
		//then command the unit to gather minerals from there and change state to GatheringMinerals
		break;
	case WorkerStates::LookForGas:
		//implement a function that finds the nearest refinery from the command center
		//then command the unit to gather gas from there and change state to GatheringGas
		break;
	case WorkerStates::GatheringMinerals:
		//not sure if this state is even nessessary
		break;
	case WorkerStates::GatheringGas:
		//not sure if needed
		break;
	case WorkerStates::Building:
		//when this state is being set we need to check what buildevent is active
		//then run the right function for corresponding building
		//check if unit is in building mode, once the unit changes the building mode to false
		//we need to use the stack and go back to the previous state
		break;
	default:
		break;
	}
}
