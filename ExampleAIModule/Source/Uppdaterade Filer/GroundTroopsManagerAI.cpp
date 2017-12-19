#include "GroundTroopsManagerAI.h"

GroundTroopsManagerAI::GroundTroopsManagerAI()
{
	this->setGatheringPoint();
	this->currentEvent = ArmyEvents::GuardBase;
}

void GroundTroopsManagerAI::addGroundTroop(Unit & unit)
{
	if (unit->getType() == UnitTypes::Terran_Marine)
		this->marines.push_back(unit);
	if (unit->getType() == UnitTypes::Terran_Medic)
		this->medics.push_back(unit);
	if (unit->getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode)
		this->tanks.push_back(unit);
}

void GroundTroopsManagerAI::update()
{

	switch (currentEvent)
	{
	case ArmyEvents::GuardBase:
		for (size_t i = 0; i < this->marines.size(); i++)
		{
			if (this->marines.at(i)->getDistance(this->GatheringPoint) > 100)
			{
				this->marines.at(i)->move(this->GatheringPoint);
			}
		}
		for (size_t i = 0; i < this->medics.size(); i++)
		{
			if (this->medics.at(i)->getDistance(this->GatheringPoint) > 100)
			{
				this->medics.at(i)->move(this->GatheringPoint);
			}
		}
		for (size_t i = 0; i < this->tanks.size(); i++)
		{
			if (this->tanks.at(i)->getDistance(this->GatheringPoint) > 100)
			{
				if (this->tanks.at(i)->isSieged())
					this->tanks.at(i)->unsiege();
				this->tanks.at(i)->move(this->GatheringPoint);
			}
			else if (!this->tanks.at(i)->isSieged() && this->tanks.at(i)->canSiege())
			{
				this->tanks.at(i)->siege();
			}
		}

		break;
	
	}

	this->lookForEnemies(); // if enemies are around all movements orders will be overwritten in this function
		//problem is, if gatheringpoint is far from the whole troop, its probably becomes messy
	

}

void GroundTroopsManagerAI::setGatheringPoint()
{
	std::set<BWTA::Chokepoint*> chokepoints = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion()->getChokepoints();
	double min_length = 10000;
	BWTA::Chokepoint* choke = NULL;

	//Iterate through all chokepoints and look for the one with the smallest gap (least width)
	for (std::set<BWTA::Chokepoint*>::iterator c = chokepoints.begin(); c != chokepoints.end(); c++)
	{
		double length = (*c)->getWidth();
		if (length < min_length || choke == NULL)
		{
			min_length = length;
			choke = *c;
		}
	}
	//found chokePoint, lets not go straight into it, where the whole army can't fit, so a bit
	//close to base is fine.


	if (Broodwar->self()->getStartLocation().x < Broodwar->mapWidth() / 2) //upperleft Corner
	{
		Position chokePosition = choke->getCenter();
		chokePosition.x -= 65;
		chokePosition.y += 65;
		this->GatheringPoint = chokePosition;
	}
	else //down-right corner
	{
		Position chokePosition = choke->getCenter();
		chokePosition.x += 65;
		chokePosition.y += 65;
		this->GatheringPoint = chokePosition;
	}




	//not using this yet
	//switch (currentEvent)
	//{ 
	//	case ArmyEvents::GuardBase: //just set a guardingPoint at closeest ChokePoint

	//		
	//		break;
	//	case ArmyEvents::ConquerEnemy:
	//		//find EnemyBase point
	//		break;
	//}
}

void GroundTroopsManagerAI::lookForEnemies()
{
	if (target != NULL) //if there is another target to kill ignore else
	{
		for (auto u : Broodwar->enemy()->getUnits())
		{
			if (u->getDistance(this->GatheringPoint) < 200)
			{
				this->target = u;
				break;
			}
		}
	}
	else
	{
		killTarget();
	}
}

void GroundTroopsManagerAI::killTarget()
{
	if (this->target->getHitPoints() < 0)
	{
		target = NULL;
	}
	else if (this->target->getDistance(this->GatheringPoint) > 100)
	{
		target = NULL;
	}
	else
	{
		for (size_t i = 0; i < marines.size(); i++)
		{
			marines.at(i)->attack(target);
		}
		for (size_t i = 0; i < marines.size(); i++)
		{
			if(tanks.at(i)->canAttack(target)) //extra check bcus they are in siegeMode
				tanks.at(i)->attack(target);
		}
	}
}

