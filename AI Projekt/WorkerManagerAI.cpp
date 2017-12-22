#include "WorkerManagerAI.h"

WorkerManagerAI::WorkerManagerAI()
{
	this->depots_num = 0;
}

WorkerManagerAI::~WorkerManagerAI()
{
}

void WorkerManagerAI::addWorker(Unit &unit)
{
	//initially tell worker to gather minerals from nearest source
	Unit nearestMineral = nullptr;
	for (auto u : Broodwar->getMinerals())
	{
		if (nearestMineral == nullptr || unit->getDistance(u) < unit->getDistance(nearestMineral))
		{
			nearestMineral = u;
		}
	}

	if (nearestMineral != nullptr)
	{
		unit->rightClick(nearestMineral);
		Broodwar->printf("Send worker %d to mineral %d", unit->getID(), nearestMineral->getID());
		workers.push_back(unit);
	}
}

//this function resets any workers that finished constructing and tells them to gather again
void WorkerManagerAI::update()
{
	//give idle workers work
	for (size_t i = 0; i < workers.size(); i++)
	{
		if (workers[i]->isIdle() && !workers[i]->isConstructing())
		{
			if (workers[i]->isCarryingGas())
			{
				//find nearest refinery
				Unit nearestRefinery = findNearestRefinery(workers[i]);
				if (nearestRefinery != nullptr)
				{
					workers[i]->rightClick(nearestRefinery);
					Broodwar->printf("Send worker %d to Refinery %d", workers[i]->getID(), nearestRefinery->getID());
				}
			}
			else 
			{
				//find nearest mineral
				Unit nearestMineral = findNearestMineral(workers[i]);
				if (nearestMineral != nullptr)
				{
					workers[i]->rightClick(nearestMineral);
					Broodwar->printf("Send worker %d to Mineral %d", workers[i]->getID(), nearestMineral->getID());
				}
			}
		}
	}
	//check how many workers we have, if we have more than 6 workers tell new workers to gather gas
	//int delta = workers.size() - 6;
	//for (auto u : Broodwar->self()->getUnits())
	//{
	//	if (u->getType() == UnitTypes::Terran_Refinery)
	//	{
	//		for (size_t i = 0; i < delta; i++)
	//		{
	//			if (!workers[i]->isGatheringGas())
	//			{
	//				Unit nearestRefinery = findNearestRefinery(workers[i]);
	//				if (nearestRefinery != nullptr)
	//				{
	//					workers[i]->rightClick(nearestRefinery);
	//					Broodwar->printf("Send worker %d to Refinery %d", workers[i]->getID(), nearestRefinery->getID());
	//				}
	//			}
	//		}
	//		break;
	//	}
	//}
}

bool WorkerManagerAI::build(int event)
{
	bool constructing = false;
	for (size_t i = 0; i < workers.size(); i++)
	{
		if (!workers[i]->isConstructing()) //if one of our workers is not busy constructing, tell him to build at suitable location
		{
			switch (event)
			{
			case BuildEvents::buildSupplyDepot:
				//we need to find a suitable location for the building, let's say near the command center?
				constructing = _buildDepot(i);
				break;
			case BuildEvents::buildRefinery:
				constructing = _buildRefinery(i);
				break;
			case BuildEvents::buildBarracks:
				constructing = _buildBarracks(i);
				break;
			case BuildEvents::buildAcademy:
				constructing = _buildAcademy(i);
				break;
			case BuildEvents::buildFactory:
				constructing = _buildFactory(i);
			default:
				break;
			}
			break;
		}
	}
	return constructing;
}

TilePosition WorkerManagerAI::findBuildLocation(UnitType type, TilePosition t_pos)
{
	//I also added to the algorithm to check if there are any mobile units standing in the location where
	//structure could be built, since this online tutorial had it for their structure location finder http://www.sscaitournament.com/index.php?action=tutorial

	bool foundLocation = false;

	int offset = 1;
	const int up = 0;
	const int down = 1;
	const int left = 2;
	const int right = 3;
	int value = 0;

	while (!foundLocation)
	{
		while (!Broodwar->canBuildHere(t_pos, type))
		{
			//use a walker approach to find a random suitable position
			//this isn't smart AI just lazy programming
			value = rand() % 4;
			if (value == up)
				t_pos.y -= offset;
			if (value == down)
				t_pos.y += offset;
			if (value == left)
				t_pos.x -= offset;
			if (value == right)
				t_pos.x += offset;
		}

		//See that no workers isn't in the way for construction
		bool workerBlockingSite = false;
		for (auto u : Broodwar->self()->getUnits())
		{
			if (u->getType().isWorker())
				if (u->getTilePosition().x + 2 < t_pos.x && u->getTilePosition().x - 2 > t_pos.x)
					if (u->getTilePosition().y + 2 < t_pos.y && u->getTilePosition().y - 2 > t_pos.y)
						workerBlockingSite = true;
		}
		if (!workerBlockingSite)
		{
			foundLocation = true;
		}
	}

	return t_pos;
}

Unit WorkerManagerAI::findNearestRefinery(Unit worker)
{
	Unit nearestRefinery = nullptr;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Refinery)
		{
			if (nearestRefinery == nullptr || worker->getDistance(u) < worker->getDistance(nearestRefinery))
			{
				nearestRefinery = u;
			}
		}
	}
	return nearestRefinery;
}

Unit WorkerManagerAI::findNearestMineral(Unit worker)
{
	Unit nearestMineral = nullptr;
	for (auto u : Broodwar->getMinerals())
	{
		if (nearestMineral == nullptr || worker->getDistance(u) < worker->getDistance(nearestMineral))
		{
			nearestMineral = u;
		}
	}
	return nearestMineral;
}

bool WorkerManagerAI::_buildDepot(int index)
{
	bool constructing = false;

	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			TilePosition center_pos = u->getTilePosition();
			UnitType type = UnitTypes::Terran_Supply_Depot;

			//check where we can build around it
			TilePosition build_pos = center_pos;

			//Base can only be in upper lfeft or down east corner. which make the building algorithm for base similar to each other,
			//just mirrored to each other.
			if (center_pos.x < Broodwar->mapWidth()/2) //upper left Corner if true
			{
				if (this->depots_num == 1)
				{
					build_pos.y += 3;
				}
				if (this->depots_num == 2)
				{
					build_pos.y += 3;
					build_pos.x += 3;
				}
				if (this->depots_num == 3)
				{
					build_pos.y += 5;
				}
				if (this->depots_num == 4)
				{
					build_pos.y += 5;
					build_pos.x += 3;
				}
			}
			else
			{
				if (this->depots_num == 1)
				{
					build_pos.y -= 2;
				}
				if (this->depots_num == 2)
				{
					build_pos.y -= 2;
					build_pos.x += 3;
				}
				if (this->depots_num == 3)
				{
					build_pos.y -= 4;
				}
				if (this->depots_num == 4)
				{
					build_pos.y -= 4;
					build_pos.x += 3;
				}
			
			}
			
			//this->findBuildLocation(type, build_pos);

			if (workers[index]->build(type, build_pos))
				constructing = true;
			break;
		}
	}
	return constructing;
}

bool WorkerManagerAI::_buildRefinery(int index)
{
	bool constructing = false;
	//find nearest gas geysers
	Unit nearestGas = nullptr;
	for (auto u : Broodwar->getGeysers())
	{
		if (nearestGas == nullptr || workers[index]->getDistance(u) < workers[index]->getDistance(nearestGas))
		{
			nearestGas = u;
		}
	}
	Broodwar->printf("Nearest geyeser at: X: %d Y: %d", nearestGas->getTilePosition().x, nearestGas->getTilePosition().y);
	UnitType type = UnitTypes::Terran_Refinery;
	if (workers[index]->build(type, nearestGas->getTilePosition()))
		constructing = true;

	return constructing;
}

bool WorkerManagerAI::_buildBarracks(int index)
{
	bool constructing = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			TilePosition center_pos = u->getTilePosition();
			UnitType type = UnitTypes::Terran_Barracks;

			//check where we can build around it
			TilePosition build_pos = center_pos;

			//Base can only be in upper lfeft or down east corner. which make the building algorithm for base similar to each other,
			//just mirrored to each other.
			if (center_pos.x < Broodwar->mapWidth() / 2) //upper left Corner if true
			{
				build_pos.x += 4;
			}
			else
			{
				build_pos.x -= 4;
			}
			if (workers[index]->build(type, build_pos))
				constructing = true;
			break;
		}
	}
	return constructing;
}

bool WorkerManagerAI::_buildAcademy(int index)
{
	bool constructing = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			TilePosition center_pos = u->getTilePosition();
			UnitType type = UnitTypes::Terran_Academy;

			//check where we can build around it
			TilePosition build_pos = center_pos;

			//Base can only be in upper lfeft or down east corner. which make the building algorithm for base similar to each other,
			//just mirrored to each other.
			if (center_pos.x < Broodwar->mapWidth() / 2) //upper left Corner if true
			{
				build_pos.y += 7;
			}
			else
			{
				build_pos.y -= 6;
			}
			if (workers[index]->build(type, build_pos))
				constructing = true;
			break;
		}
	}
	return constructing;
}

bool WorkerManagerAI::_buildFactory(int index)
{
	bool constructing = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Command_Center)
		{
			TilePosition center_pos = u->getTilePosition();
			UnitType type = UnitTypes::Terran_Factory;

			//check where we can build around it
			TilePosition build_pos = center_pos;

			//Base can only be in upper lfeft or down east corner. which make the building algorithm for base similar to each other,
			//just mirrored to each other.
			if (center_pos.x < Broodwar->mapWidth() / 2) //upper left Corner if true
			{
				build_pos.y += 7;
				build_pos.x += 4;
			}
			else
			{
				build_pos.y -= 6;
				build_pos.x -= 6;

			}
			if (workers[index]->build(type, build_pos))
				constructing = true;
			break;
		}
	}
	return constructing;
}

void WorkerManagerAI::reAssignResourceGathering()
{
	for (size_t i = 0; i < this->workers.size(); i++)
	{
		if (i % 2 == 1)
			this->workers.at(i)->rightClick(this->findNearestRefinery(workers.at(i)));
		else
			this->workers.at(i)->rightClick(this->findNearestMineral(workers.at(i)));
	}
}

void WorkerManagerAI::increaseNumDepots()
{
	this->depots_num++;
}
