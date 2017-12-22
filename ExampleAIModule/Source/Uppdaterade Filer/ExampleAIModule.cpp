#include "ExampleAIModule.h" 
using namespace BWAPI;

int frameUpdateTime;
bool buildProcess;
bool eventProcessing;
bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

//This is the startup method. It is called once
//when a new game has been started with the bot.
void ExampleAIModule::onStart()
{
	frameUpdateTime = 5;
	buildProcess = false;

	Broodwar->setLocalSpeed(frameUpdateTime);
	Broodwar->sendText("Hello world!");

	//Only for debugging purpose, it could be beneficial to know which Race you are up against
	if (Broodwar->enemy()->getRace() == Races::Protoss)
	{
		Broodwar->printf("Enemy Race is Protoss");
	}
	else
	{
		Broodwar->printf("Enemy Race is Zerg");
	}
	//Enable flags
	Broodwar->enableFlag(Flag::UserInput);
	//Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Start analyzing map data
	BWTA::readMap();
	analyzed = false;
	analysis_just_finished = false;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL); //Threaded version
	AnalyzeThread();

	this->wManager = new WorkerManagerAI();
	this->aManager = new GroundTroopsManagerAI();
	counter = 0;
	currentState = GameStrategy::createSupplyDepot;
}

//Called when a game is ended.
//No need to change this.
void ExampleAIModule::onEnd(bool isWinner)
{
	if (isWinner)
	{
		Broodwar->sendText("I won!");
	}
	delete this->wManager;
	delete this->aManager;
}

//Finds a guard point around the home base.
//A guard point is the center of a chokepoint surrounding
//the region containing the home base.
Position ExampleAIModule::findGuardPoint()
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

	return choke->getCenter();
}


bool ExampleAIModule::createWorker()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_SCV))
		{
			creating = u->train(UnitTypes::Terran_SCV);
			
			break;
		}
	}
	return creating;
}

bool ExampleAIModule::createMarine()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_Marine))
		{
			
			creating = u->train(UnitTypes::Terran_Marine);
			break;
		}
	}
	return creating;
}

bool ExampleAIModule::createMedic()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_Medic))
		{
			creating = u->train(UnitTypes::Terran_Medic);
			break;
		}
	}
	return creating;
}

bool ExampleAIModule::createMachineShop()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Factory)
		{
			creating = u->buildAddon(UnitTypes::Terran_Machine_Shop);
			break;
		}
	}
	return creating;
}

bool ExampleAIModule::researchSiegeMode()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType() == UnitTypes::Terran_Machine_Shop)
		{
			creating = u->research(TechTypes::Tank_Siege_Mode);
			break;
		}
	}
	return creating;
}

bool ExampleAIModule::createSiegeTank()
{
	bool creating = false;
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_Siege_Tank_Tank_Mode))
		{
			creating = u->train(UnitTypes::Terran_Siege_Tank_Tank_Mode);
			break;
		}
	}
	return creating;
}


void ExampleAIModule::update()
{

	//this the Games strategy bot logic
	switch (this->currentState)
	{
	case GameStrategy::createSupplyDepot:
		//check if the current event is being processed
		//if false, pick an available worker to build a depot
		if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Supply_Depot) == 2 && Broodwar->self()->allUnitCount(UnitTypes::Terran_Barracks) == 0)
		{
			currentState = GameStrategy::createBarracks;
		}
		else if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Supply_Depot) == 4 && Broodwar->self()->allUnitCount(UnitTypes::Terran_Barracks) > 0)
		{
			if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Siege_Tank_Tank_Mode) > 0)
			{
				currentState = GameStrategy::createSiegeTank;
			}
			
		}
		
		else if (!buildProcess && Broodwar->self()->minerals() >= 100)
		{
			Broodwar->printf("I'm calling build depot function");
			buildProcess = wManager->build(BuildEvents::buildSupplyDepot);	
			wManager->increaseNumDepots();
		}
		break;
	case GameStrategy::createWorker:
		//simply create a worker and then move onto the next stage, we can start building the refinery while the worker is being completed
		if (Broodwar->self()->minerals() >= 50 && Broodwar->self()->allUnitCount(UnitTypes::Terran_SCV) < 9)
		{
			Broodwar->printf("Create worker called");
			createWorker();
		}
		else if (Broodwar->self()->allUnitCount(UnitTypes::Terran_SCV) == 9)
		{
			currentState = GameStrategy::createMarine;
		}
		break;
	case GameStrategy::createRefinery:
		//tell a worker to build a refinery
		if (!buildProcess && Broodwar->self()->minerals() >= 100)
		{
			Broodwar->printf("I'm calling build refinery function");
			buildProcess = wManager->build(BuildEvents::buildRefinery);
		}
		if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Refinery) == 1)
		{
			buildProcess = false;
			currentState = GameStrategy::createWorker;
		}
		break;
	case GameStrategy::createBarracks:
		if (!buildProcess && Broodwar->self()->minerals() >= 150)
		{
			Broodwar->printf("I'm calling build barracks function");
			buildProcess = wManager->build(BuildEvents::buildBarracks);		
		}
		if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Barracks) == 1)
		{
			currentState = GameStrategy::createRefinery;
		}
		break;
	case GameStrategy::createMarine:
		if (Broodwar->self()->minerals() >= 250 && counter < 10)
		{
			for (int i = 0; i < 5; i++)
			{
				createMarine();
				counter++;
			}
			if (counter <= 5 && this->aManager->marineSize() > 0)
			{
				currentState = GameStrategy::createFactory;
				counter = 0;
			}
			else if (counter <= 5)
			{
				//currentState = GameStrategy::createSupplyDepot; // inte nödvändig i början
				counter = 0;
			}
		}
		break;
	case GameStrategy::createAcademy:
		if (!buildProcess && Broodwar->self()->minerals() >= 150 && Broodwar->self()->allUnitCount(UnitTypes::Terran_Academy) < 1)
		{
			Broodwar->printf("I'm calling build academy function");
			buildProcess = wManager->build(BuildEvents::buildAcademy);
			currentState = GameStrategy::createMarine;
		}
		if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Academy) == 1)
		{
			currentState = GameStrategy::createFactory;
		}
		break;
	case GameStrategy::createFactory:
		if (!buildProcess && Broodwar->self()->minerals() >= 150)
		{
			Broodwar->printf("I'm calling build academy function");
			buildProcess = wManager->build(BuildEvents::buildFactory);
			currentState = createAcademy;
		}
		if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Factory) == 1)
		{
			currentState = GameStrategy::createMedic;
		}
		break;
	case GameStrategy::createMedic:
		if (Broodwar->self()->minerals() >= 150 && Broodwar->self()->gas() >= 75)
		{
			bool created = false;
			for (int i = 0; i < 3; i++)
			{
				created = createMedic();
			}
			if (created)
				currentState = GameStrategy::createMachineShop;
		}
		break;
	case GameStrategy::createMachineShop:
		if (Broodwar->self()->minerals() >= 50 && Broodwar->self()->gas() >= 50)
		{
			//this is when both gas and mineral becomes equal important
			//So i assign all workers in WManger
			
			if (createMachineShop())
			{ 
				this->wManager->reAssignResourceGathering();  //here new resassignment to resourceGathering
				currentState = GameStrategy::researchSiegeMode;
			}
				
		}
		break;
	case GameStrategy::researchSiegeMode:
		if (Broodwar->self()->minerals() >= 100 && Broodwar->self()->gas() >= 100)
		{
			if (researchSiegeMode())
				currentState = GameStrategy::createSiegeTank;
		}
		break;
	case GameStrategy::createSiegeTank:
		if (Broodwar->self()->minerals() >= 150 && Broodwar->self()->gas() >= 100)
		{
			if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Machine_Shop) > 0)
			{
				Broodwar->printf("Make Siege tank!");
				createSiegeTank();
				if(this->aManager->tanksSize() == 2 && Broodwar->self()->allUnitCount(UnitTypes::Terran_Supply_Depot) < 4)
					currentState = GameStrategy::createSupplyDepot; // flyttad hit eftersom det är inte förr'ns nu den behövs
			}
			/*if (Broodwar->self()->allUnitCount(UnitTypes::Terran_Siege_Tank_Siege_Mode) == 3)
			{
				currentState = GameStrategy::attack;
			}*/
		}
		break;
	default:
		break;
	}
}

//This is the method called each frame. This is where the bot's logic
//shall be called.
void ExampleAIModule::onFrame()
{
	frameUpdateTime += frameUpdateTime;
	//Call every 100:th frame
	if (Broodwar->getFrameCount() % 100 == 0)
	{
		
		//Update the GroundTroops
		aManager->update();
		wManager->update();

		Broodwar->printf("State: %d", currentState);
		/*Broodwar->printf("%d", Broodwar->self()->allUnitCount(UnitTypes::Terran_SCV));
		Broodwar->printf("%d", wManager->size());*/
	}

	//update the game state machine every frame? 
	update();
	//Draw lines around regions, chokepoints etc.
	if (analyzed)
	{
		drawTerrainData();
	}
}

//Is called when text is written in the console window.
//Can be used to toggle stuff on and off.
void ExampleAIModule::onSendText(std::string text)
{
	if (text == "/show players")
	{
		showPlayers();
	}
	else if (text == "/show forces")
	{
		showForces();
	}
	else
	{
		Broodwar->printf("You typed '%s'!", text.c_str());
		Broodwar->sendText("%s", text.c_str());
	}
}

//Called when the opponent sends text messages.
//No need to change this.
void ExampleAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

//Called when a player leaves the game.
//No need to change this.
void ExampleAIModule::onPlayerLeft(BWAPI::Player player)
{
	Broodwar->sendText("%s left the game.", player->getName().c_str());
}

//Called when a nuclear launch is detected.
//No need to change this.
void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
	if (target != Positions::Unknown)
	{
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)", target.x, target.y);
	}
	else
	{
		Broodwar->printf("Nuclear Launch Detected");
	}
}

//No need to change this.
void ExampleAIModule::onUnitDiscover(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitShow(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitHide(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//Called when a new unit has been created.
//Note: The event is called when the new unit is built, not when it
//has been finished.
void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	UnitType type = unit->getType();
	if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("A %s [%x] has been created at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
		
		if (type.isBuilding())
		{
			buildProcess = false;
		}
	}
	
}

//Called when a unit has been destroyed.
void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("My unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}
	else
	{
		Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}
}

//Only needed for Zerg units.
//No need to change this.
void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

//No need to change this.
void ExampleAIModule::onUnitRenegade(BWAPI::Unit unit)
{
	//Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

//No need to change this.
void ExampleAIModule::onSaveGame(std::string gameName)
{
	Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

//Analyzes the map.
//No need to change this.
DWORD WINAPI AnalyzeThread()
{
	BWTA::analyze();

	//Self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL)
	{
		home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//Enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy()) != NULL)
	{
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

//Prints some stats about the units the player has.
//No need to change this.
void ExampleAIModule::drawStats()
{
	BWAPI::Unitset myUnits = Broodwar->self()->getUnits();
	Broodwar->drawTextScreen(5, 0, "I have %d units:", myUnits.size());
	std::map<UnitType, int> unitTypeCounts;
	for (auto u : myUnits)
	{
		if (unitTypeCounts.find(u->getType()) == unitTypeCounts.end())
		{
			unitTypeCounts.insert(std::make_pair(u->getType(), 0));
		}
		unitTypeCounts.find(u->getType())->second++;
	}
	int line = 1;
	for (std::map<UnitType, int>::iterator i = unitTypeCounts.begin(); i != unitTypeCounts.end(); i++)
	{
		Broodwar->drawTextScreen(5, 16 * line, "- %d %ss", i->second, i->first.getName().c_str());
		line++;
	}
}

//Draws terrain data aroung regions and chokepoints.
//No need to change this.
void ExampleAIModule::drawTerrainData()
{
	//Iterate through all the base locations, and draw their outlines.
	for (auto bl : BWTA::getBaseLocations())
	{
		TilePosition p = bl->getTilePosition();
		Position c = bl->getPosition();
		//Draw outline of center location
		Broodwar->drawBox(CoordinateType::Map, p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, Colors::Blue, false);
		//Draw a circle at each mineral patch
		for (auto m : bl->getStaticMinerals())
		{
			Position q = m->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map, q.x, q.y, 30, Colors::Cyan, false);
		}
		//Draw the outlines of vespene geysers
		for (auto v : bl->getGeysers())
		{
			TilePosition q = v->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map, q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, Colors::Orange, false);
		}
		//If this is an island expansion, draw a yellow circle around the base location
		if (bl->isIsland())
		{
			Broodwar->drawCircle(CoordinateType::Map, c.x, c.y, 80, Colors::Yellow, false);
		}
	}
	//Iterate through all the regions and draw the polygon outline of it in green.
	for (auto r : BWTA::getRegions())
	{
		BWTA::Polygon p = r->getPolygon();
		for (int j = 0; j<(int)p.size(); j++)
		{
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Green);
		}
	}
	//Visualize the chokepoints with red lines
	for (auto r : BWTA::getRegions())
	{
		for (auto c : r->getChokepoints())
		{
			Position point1 = c->getSides().first;
			Position point2 = c->getSides().second;
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Red);
		}
	}
}

//Show player information.
//No need to change this.
void ExampleAIModule::showPlayers()
{
	for (auto p : Broodwar->getPlayers())
	{
		Broodwar->printf("Player [%d]: %s is in force: %s", p->getID(), p->getName().c_str(), p->getForce()->getName().c_str());
	}
}

//Show forces information.
//No need to change this.
void ExampleAIModule::showForces()
{
	for (auto f : Broodwar->getForces())
	{
		BWAPI::Playerset players = f->getPlayers();
		Broodwar->printf("Force %s has the following players:", f->getName().c_str());
		for (auto p : players)
		{
			Broodwar->printf("  - Player [%d]: %s", p->getID(), p->getName().c_str());
		}
	}
}

//Called when a unit has been completed, i.e. finished built.
void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	Broodwar->sendText("A %s [%x] has been completed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);

	//find the worker who was building and tell him that the building is finished so he can resume gathering
	//we have to do this because the workes does not directly get the event information
	UnitType type = unit->getType();
	//when a worker is completed, push him back into the vector
	//when we have more than 5 workers, tell the new workers to gather gas instead
	if (type == UnitTypes::Terran_SCV)
	{
		wManager->addWorker(unit);
	}
	else
	{
		aManager->addGroundTroop(unit); //all other units are now grouped into GroundTroopAI
	}