#include "WorkerAI.h"

//this function finds the nearest mineral source from unit point
void WorkerAI::findNearestMineral()
{
	Unit nearestMineral = nullptr;
	for (auto u : Broodwar->getMinerals())
	{
		if (nearestMineral == nullptr || this->workerPtr->getDistance(u) < this->workerPtr->getDistance(nearestMineral))
		{
			nearestMineral = u;
		}
	}
	if (nearestMineral != nullptr)
	{
		this->workerPtr->rightClick(nearestMineral);
		Broodwar->printf("Send worker %d to mineral %d", this->workerPtr->getID(), nearestMineral->getID());
		this->setState(WorkerStates::GatheringMinerals);
	}
}

void WorkerAI::build()
{
	isBuilding = true;
	
	switch (this->currentEvent)
	{
	case BuildEvents::buildSupplyDepot:
		//we need to find a suitable location for the building, let's say near the command center?
		_buildDepot();
		break;
	case BuildEvents::buildRefinery:
		_buildRefinery();
		break;
	default:
		break;
	}
}

void WorkerAI::_buildDepot()
{
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			TilePosition t_pos = u->getTilePosition();
			//check where we can build around it
			UnitType type = UnitTypes::Terran_Supply_Depot;

			const int up = 0;
			const int down = 1;
			const int left = 2;
			const int right = 3;
			int value = 0;
			while (!Broodwar->canBuildHere(t_pos, type))
			{
				//use a walker approach to find a random suitable position
				//this isn't smart AI just lazy programming
				value = rand() % 4;
				if (value == up)
					t_pos.y -= 1;
				if (value == down)
					t_pos.y += 1;
				if (value == left)
					t_pos.x -= 1;
				if (value == right)
					t_pos.x += 1;
			}
			this->workerPtr->build(type, t_pos);
			break;
		}
	}
}

void WorkerAI::_buildRefinery()
{
	//find nearest gas geysers
	Unit nearestGas= nullptr;
	for (auto u : Broodwar->getGeysers())
	{
		if (nearestGas == nullptr || this->workerPtr->getDistance(u) < this->workerPtr->getDistance(nearestGas))
		{
			nearestGas = u;
		}
	}
	Broodwar->printf("Nearest geyeser at: X: %d Y: %d", nearestGas->getTilePosition().x, nearestGas->getTilePosition().y);
	UnitType type = UnitTypes::Terran_Refinery;
	this->workerPtr->build(type, nearestGas->getTilePosition());

}

void WorkerAI::setState(int state)
{
	this->currentState = state;
	this->stateStack.push(state);
}

WorkerAI::WorkerAI(Unit unit)
{
	this->workerPtr = unit;
	this->currentState = WorkerStates::LookForMinerals;
	this->stateStack.push(this->currentState);
	this->activeEvent = false;
	this->currentEvent = INT_MAX;
	this->isBuilding = false;
	this->buildingDone = false;
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
		findNearestMineral();
		break;
	case WorkerStates::LookForGas:
		//implement a function that finds the nearest refinery from the command center
		//then command the unit to gather gas from there and change state to GatheringGas
		break;
	case WorkerStates::GatheringMinerals:
		//this state is a check for if a building was created we must go back to the previous state to relocate the mineral source
		//if no building was created then do nothing since the Startcraft AI does the gathering already
		if (buildingDone)
		{
			this->stateStack.pop();
			this->currentState = this->stateStack.top();
			buildingDone = false;
		}
		break;
	case WorkerStates::GatheringGas:
		//same reason here as above, relocate the source.
		if (buildingDone)
		{
			this->stateStack.pop();
			this->currentState = this->stateStack.top();
			buildingDone = false;
		}
		break;
	case WorkerStates::Building:
		//when this state is being set we need to check what buildevent is active
		//then run the right function for corresponding building
		//check if unit is in building mode, once the unit changes the building mode to false
		//we need to use the stack and go back to the previous state
		if (activeEvent)
		{
			if (!isBuilding)
			{
				Broodwar->printf("Building");
				build();

			}
			if (buildingDone)
			{
				this->currentEvent = INT_MAX;
				this->activeEvent = false;
				this->stateStack.pop();
				this->currentState = this->stateStack.top();
			}
		}
		break;
	default:
		break;
	}
}

int WorkerAI::getCurrentState() const
{
	return this->currentState;
}
