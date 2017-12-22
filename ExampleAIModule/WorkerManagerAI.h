#include"BWAPI.h"
#include<vector>

using namespace BWAPI;
using namespace std;

enum BuildEvents
{
	buildSupplyDepot,
	buildBarracks,
	buildRefinery,
	buildAcademy,
	buildFactory,
	buildCommandCenter
};

class WorkerManagerAI
{
private:
	vector<Unit> workers;

public:
	WorkerManagerAI();
	~WorkerManagerAI();
	void addWorker(Unit &unit);
	void update();
	bool build(int event);
	TilePosition findBuildLocation(UnitType type, TilePosition t_pos);
	Unit findNearestRefinery(Unit worker);
	Unit findNearestMineral(Unit worker);
	bool _buildDepot(int index);
	bool _buildRefinery(int index);
	bool _buildBarracks(int index);
	bool _buildAcademy(int index);
	bool _buildFactory(int index);
	size_t size() { return this->workers.size(); }
};
