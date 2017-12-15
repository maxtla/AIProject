#include "MarineAI.h"

void MarineAI::findChokePoint()
{
	//Get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints = home->getChokepoints();
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

	bool inMotion = true;
	this->marinePtr->rightClick(choke->getCenter());
	setState(MarineStates::OscarMike);
}

void MarineAI::lookForEnemies()
{
	Unit tempTarget = NULL;
	//Find possible target, then decides which target is most appropriate to strika at
	for (auto u : Broodwar->enemy()->getUnits())
	{
		//getDistance() gives an integer of pixels between edge of this unit's tile to edge of parameter units edge, ignores obstacles
		if (this->marinePtr->getDistance(u) < 150)
		{
			Broodwar->printf("Enemy in Sight!");
			//algorithm to find a target for the Marine goes like this :
				//priority 1: find enemy with lowest health
				//priority 2: find enemy as biggest threat
			if (Broodwar->enemy()->getRace() == Races::Protoss)
			{

				if (tempTarget != NULL)
					if (u->getHitPoints() < tempTarget->getHitPoints())
						tempTarget = u;
					else
						tempTarget = u;


				//As you can see we can also implement one enemy type at a time, to prioritize which of the enemytypes is the biggest threat, 
				//or which is easier to kill off for a Marinem. Thats also why I kept the ifCase that ask which Race we are encountering atm.
				//But for now, lets keep it simple and let them decide the target based on which enemy has lowest health. 

				
				/*if (u->getType() == UnitTypes::Protoss_Zealot)
				{
					//Zealot are melee fighters, kill before they get chance to reach you
				}*/

			}
			else
			{
				//This is when the Enemy-race is Zerg read description above as why Zerg and Protoss cases look just the same
				
				if (tempTarget != NULL)
					if (u->getHitPoints() < tempTarget->getHitPoints())
						tempTarget = u;
					else
						tempTarget = u;
			}


			
		}
	}
	//See if any enemies were detected
	if (tempTarget != NULL)
	{
		this->target = tempTarget;
		setState(MarineStates::Combat);
		this->marinePtr->attack(this->target);
		Broodwar->printf("Bring them Down Sir!");
	}

}

void MarineAI::attack()
{
	//Look at health, will retreat when health is below 20%
	if (float(this->marinePtr->getHitPoints() / this->marinePtr->getInitialHitPoints()) < 0.2)
	{
		this->inCombat = false;
		setState(MarineStates::CoverAndMedic);
		//Marine wil keep target so no nullification
	}

	//Keep attacking until enemy is dead, which is according to BWAPI when HitPoints reaches 0...
	if (this->target->getHitPoints() == 0)
	{
		this->target = NULL;
		setState(MarineStates::KeepWatch);
	}

}

void MarineAI::takeCover()
{
	if (float(this->marinePtr->getHitPoints() / this->marinePtr->getInitialHitPoints()) < 0.2)
	{
		int xDir[] = { 1, -1, 1, -1 };  //these two are used two facilitate the "move away from target" function below
		int yDir[] = { 1, 1, -1, -1 };

		//4 loops for each direction , though this algorithm will only make Marine avoid the target other enemies may strike him down.
		for (int i = 0; i < 4; i++)
		{
			Position marinePos = this->marinePtr->getPosition();
			marinePos.x += 3 * xDir[i]; //this will move the temporary MarinePosition either 5 tiles east or west depending on xDir subscript
			marinePos.y += 3 * yDir[i];	//Same as above description
			if (this->target->getDistance(marinePos) > this->target->getDistance(this->marinePtr))
			{
				//walking the Marine here wil increase the distance between it and the enemy.
				this->marinePtr->move(marinePos);
			}
		}
	}
	else
		setState(MarineStates::FindChokePoint);


	
}

int MarineAI::getPreviousState() const
{
	return this->previousState;
}

void MarineAI::setState(int state)
{
	this->previousState = this->currentState;
	this->currentState = state;
}

MarineAI::MarineAI(Unit unit, BWTA::Region* home, BWTA::Region* enemy_base)
{
	this->target = NULL;
	this->home = home;
	this->enemy_base = enemy_base;
	this->marinePtr = unit;
	this->currentState = MarineStates::FindChokePoint;
	this->activeEvent = false;
	this->inCombat = false;
	this->currentEvent = INT_MAX;
	this->previousState = this->currentState;
}

void MarineAI::setEvent(int event)
{
	if (!inCombat) //bara f�r att efterlikna WorkerAI, kanske inte beh�ver if-sats alls
	{
		this->currentEvent = event;
		this->activeEvent = true;
	}

}

void MarineAI::update()
{
	switch (this->currentState)
	{
	case MarineStates::FindChokePoint:
		//Discover a choke point, then assign the Marine to stroll over there to a strategic position
		findChokePoint();
		break;
	case MarineStates::OscarMike:
		//Discover a choke point, then assign the Marine to stroll over there to a strategic position
		if (this->marinePtr->getPosition().x < this->chokePoint.x + 2 && this->marinePtr->getPosition().x < this->chokePoint.x - 2)
			if (this->marinePtr->getPosition().y < this->chokePoint.y + 2 && this->marinePtr->getPosition().y < this->chokePoint.y - 2)
			{
				setState(MarineStates::KeepWatch);
				this->inMotion = false;
			}
				
		lookForEnemies();
		break;
	case MarineStates::KeepWatch:
		//Marine is not moving, just detecting enemies, needs constant updating, so possible detection of enemies
		//get shot at immediatly, maybe to prioritize closerange enemies, to minimize damage done to troops.
		lookForEnemies();
		break;
	case MarineStates::Combat:
		//find targets, marines will take down enemies one by one, looking for lowest health enemy and take aim.
		attack();
		break;
	case MarineStates::CoverAndMedic:
		//step behind squad to move out of reach and call for medic
		takeCover();
		break;
	case MarineStates::StrikeAtBase:
		//left unimplemented for the moment
		
		break;
	default:
		break;
	}
}

int MarineAI::getCurrentState() const
{
	return this->currentState;
}
