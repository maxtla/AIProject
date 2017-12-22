#include "GroundTroopsManagerAI.h"

GroundTroopsManagerAI::GroundTroopsManagerAI()
{
	this->setGatheringPoint();
	this->currentEvent = ArmyEvents::GuardBase;
	this->enemy = nullptr;
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
	/*if (enemy != nullptr)
		Broodwar->printf("Enemiy ID: %d", this->enemy->getID());*/
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
	case ArmyEvents::ConquerEnemy:
		if (true)
		{
			//if all enemies that we had encountered were killed move towards the primary target
			sendArmyToEnemyBase();
		}
		break;
	
	}

	//this->killEnemies(); // if enemies are around all movements orders will be overwritten in this function
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



}

void GroundTroopsManagerAI::enemySpotted(Unit enemy)
{
	if (this->enemy == nullptr)
		this->enemy = enemy;
}

void GroundTroopsManagerAI::enemyKilled(Unit enemy)
{
	if (enemy != nullptr)
		if (this->enemy->getID() == enemy->getID())
			this->enemy = nullptr;
}

void GroundTroopsManagerAI::killEnemies()
{
	if (enemy != nullptr)
	{
		for (size_t i = 0; i < this->marines.size(); i++)
		{
			if (marines[i]->canAttack() && !marines[i]->isAttacking())
			{
				marines[i]->attack(enemy);
			}
		}

		for (size_t k = 0; this->tanks.size(); k++)
		{
			if (tanks[k]->canAttack() && !tanks[k]->isAttacking())
			{
				tanks[k]->attack(enemy);
			}
		}		
	}
			
}

void GroundTroopsManagerAI::setPrimaryTarget(Position primaryTarget)
{
	this->primaryTarget = primaryTarget;
	this->currentEvent = ArmyEvents::ConquerEnemy;
}

void GroundTroopsManagerAI::sendArmyToEnemyBase()
{

	for (size_t i = 0; i < marines.size(); i++)
	{
		marines[i]->rightClick(primaryTarget);
	}

	for (size_t i = 0; i < medics.size(); i++)
	{
		medics[i]->follow(marines[i]);
	}

	for (size_t i = 0; i < tanks.size(); i++)
	{
		tanks[i]->rightClick(primaryTarget);
	}
}

