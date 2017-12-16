#include "ExampleAIModule.h" 
	using namespace BWAPI;

bool eventProcessing;
bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

//This is the startup method. It is called once
//when a new game has been started with the bot.
void ExampleAIModule::onStart()
{
	Broodwar->setLocalSpeed(10);
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

	//Create a WorkerAI instance for each worker, set correct state and push back to vector
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->getType().isWorker())
		{
			WorkerAI wrkAI = WorkerAI(u);
			wrkAI.setState(WorkerStates::LookForMinerals);
			this->workerAIContainer.push_back(wrkAI);
		}
	}

	//fill the queue
	for (int i = GameStrategy::createFirstDepot; i != GameStrategy::Last; i++)
	{
		this->GameStates.push(i);
	}

	marineCounter = 0;
	eventProcessing = false;
}

//Called when a game is ended.
//No need to change this.
void ExampleAIModule::onEnd(bool isWinner)
{
	if (isWinner)
	{
		Broodwar->sendText("I won!");
	}
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

void ExampleAIModule::buildDepot(size_t unit)
{
	this->workerAIContainer.at(unit).setState(WorkerStates::Building);
	this->workerAIContainer.at(unit).setEvent(BuildEvents::buildSupplyDepot);
	eventProcessing = true;
}

void ExampleAIModule::createWorker()
{
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_SCV))
		{
			u->train(UnitTypes::Terran_SCV);
			break;
		}
	}
}

void ExampleAIModule::createMarine()
{
	for (auto u : Broodwar->self()->getUnits())
	{
		if (u->canTrain(UnitTypes::Terran_Marine))
		{
			u->train(UnitTypes::Terran_Marine);
			break;
		}
	}
}

void ExampleAIModule::buildRefinery(size_t unit)
{
	//just pick a worker we know is not busy building
	this->workerAIContainer.at(unit).setState(WorkerStates::Building);
	this->workerAIContainer.at(unit).setEvent(BuildEvents::buildRefinery);
	eventProcessing = true;
}

void ExampleAIModule::buildBarracks(size_t unit)
{
	this->workerAIContainer.at(unit).setState(WorkerStates::Building);
	this->workerAIContainer.at(unit).setEvent(BuildEvents::buildBarracks);
	eventProcessing = true;
}

void ExampleAIModule::update()
{
	//this the Games strategy bot logic
	switch (this->GameStates.front())
	{
	case GameStrategy::createFirstDepot:
		//check if the current event is being processed
		//if false, pick an available worker to build a depot
		if (!eventProcessing && Broodwar->self()->minerals() >= 100)
		{
			size_t unit = 0;
			buildDepot(unit);
		}
		break;
	case GameStrategy::createWorker:
		//simply create a worker and then move onto the next stage, we can start building the refinery while the worker is being completed
		createWorker();
		this->GameStates.pop();
		break;
	case GameStrategy::createRefinery:
		//tell a worker to build a refinery
		if (!eventProcessing && Broodwar->self()->minerals() >= 100)
		{
			Broodwar->printf("I'm calling build refinery function");
			size_t unit = 1;
			buildRefinery(unit);
		}
		break;
	case GameStrategy::createSecondDepot:
		if (!eventProcessing && Broodwar->self()->minerals() >= 100)
		{
			Broodwar->printf("I'm calling build depot function");
			//use a different unit then Unit 0 here since unit 0 needs 3 update iterations to be able to build again
			//to avoid this behavior we could use booleans as return values when we call a worker for help
			//but since we know which unit is available at this moment we use a simpler approach even though it's lazy and bad programming behaviour
			size_t unit = 2;
			buildDepot(unit);
		}
		break;
	case GameStrategy::createFourWorkers:
		if (!eventProcessing && Broodwar->self()->minerals() >= 50 && Broodwar->self()->allUnitCount() < 13) //make sure we have enough minerals for 4 workers
		{
			eventProcessing = true;
			createWorker();
		}
		else if (Broodwar->self()->allUnitCount() == 13)
		{
			this->GameStates.pop();
		}
		break;
	case GameStrategy::createBarracks:
		if (!eventProcessing && Broodwar->self()->minerals() >= 150)
		{
			Broodwar->printf("I'm calling build barracks function");
			size_t unit = 3;
			buildBarracks(unit);
		}
		break;
	case GameStrategy::createTenMarines:
		if (!eventProcessing && Broodwar->self()->minerals() >= 250 && marineCounter < 10)
		{
			eventProcessing = true;
			for (int i = 0; i < 5; i++)
			{
				createMarine();
			}
		}
		else if (marineCounter == 10)
			this->GameStates.pop();

		break;
	default:
		break;
	}
}

//This is the method called each frame. This is where the bot's logic
//shall be called.
void ExampleAIModule::onFrame()
{
	//Call every 100:th frame
	if (Broodwar->getFrameCount() % 100 == 0)
	{
		//each 100th frame call update on our workerAI
		for (size_t i = 0; i < this->workerAIContainer.size(); i++)
		{
			this->workerAIContainer.at(i).update();
		}
		//Broodwar->printf("%d", GameStates.front());
		for (size_t i = 0; i < this->marineAIContainer.size(); i++)
		{
			this->marineAIContainer.at(i).update();
		}
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
	if (unit->getPlayer() == Broodwar->self())
	{
		Broodwar->sendText("A %s [%x] has been created at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
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
	if (unit->getType().isBuilding() && eventProcessing)
	{
		eventProcessing = false;
		GameStates.pop();
		for (size_t i = 0; i < this->workerAIContainer.size(); i++)
		{
			if (workerAIContainer.at(i).getCurrentState() == WorkerStates::Building)
			{
				if (workerAIContainer.at(i).getPreviousState() != WorkerStates::Building)
				{
					workerAIContainer.at(i).setState(workerAIContainer.at(i).getPreviousState());
				}
				else
				{
					workerAIContainer.at(i).setState(WorkerStates::LookForMinerals);
				}
				i = this->workerAIContainer.size();
			}
		}
	}
	//when a worker is completed, push him back into the vector
	//when we have more than 5 workers, tell the new workers to gather gas instead
	if (unit->getType() == UnitTypes::Terran_SCV)
	{
		eventProcessing = false;
		//push the new worker into our container
		WorkerAI wrkAi = WorkerAI(unit);
		wrkAi.setState(WorkerStates::LookForMinerals);
		this->workerAIContainer.push_back(wrkAi);
		//check if we have more than 5 workers, if so we tell X amount workers to gather gas
		if (this->workerAIContainer.size() > 5)
		{
			//make sure the player has at least one refinery
			for (auto u : Broodwar->self()->getUnits())
			{
				if (u->getType() == UnitTypes::Terran_Refinery)
				{
					size_t delta = this->workerAIContainer.size() - 5;
					for (size_t i = 0; i < delta; i++)
					{
						if (this->workerAIContainer.at(i).getCurrentState() != WorkerStates::Building)
							this->workerAIContainer.at(i).setState(WorkerStates::LookForGas);
					}
					break;
				}
			}
		}
	}

	if (unit->getType() == UnitTypes::Terran_Marine)
	{
		MarineAI mrnAi = MarineAI(unit, home, enemy_base);
		this->marineAIContainer.push_back(mrnAi);
		marineCounter++;
		if (marineCounter == 5)
			eventProcessing = false;
		if (marineCounter == 10)
			eventProcessing = false;
		if (marineCounter == 15)
			eventProcessing = false;
		if (marineCounter == 20)
			eventProcessing = false;
	}
}