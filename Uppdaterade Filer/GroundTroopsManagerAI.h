#include "BWTA.h"
#include"BWAPI.h"
#include<vector>

using namespace BWAPI;
using namespace std;

enum ArmyEvents
{
	GuardBase,
	ConquerEnemy
};

class GroundTroopsManagerAI
{
private:
	vector<Unit> marines;
	vector<Unit> medics;
	vector<Unit> tanks;
	ArmyEvents currentEvent;
	Position GatheringPoint;
	Unit enemy;
	Position primaryTarget;

public:
	GroundTroopsManagerAI();
	~GroundTroopsManagerAI() {};
	void addGroundTroop(Unit &unit);
	void update();
	void setGatheringPoint(); //initializing a spot to gather around, this spot can be changed
	void enemySpotted(Unit enemy); //both answer if enemies are around and sets the target
	void enemyKilled(Unit enemy);
	void killEnemies(); //set target to null or attack
	void setPrimaryTarget(Position primaryTarget);
	void sendArmyToEnemyBase();
	size_t marineSize() { return this->marines.size(); }
	size_t tanksSize() { return this->tanks.size(); }
};
