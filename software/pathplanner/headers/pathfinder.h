#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <utility>
#include <vector>
#include <iostream>
#include <math.h>
#include <memory>
#include <unistd.h>

#include "nodecollection.h"

class Pathfinder
{
public:
    Pathfinder();
    void setMap(std::shared_ptr<std::vector<std::vector<std::shared_ptr<Node>>>> aMap) {map = aMap;}
    void setNodeCollections(std::vector<NodeCollection> collections) {nodeCollections = collections;}
    void setCurrentPosition(std::shared_ptr<std::pair<std::size_t,std::size_t>> position) {currentPosition = position;}
    void startSolver();
    //void generateTestMap();
    void setCurrentHeading(std::shared_ptr<std::pair<std::size_t,std::size_t>> heading);
    void updatePenaltyOfNode(std::size_t row, std::size_t col, double penalty);
    bool getMapStable() {return  mapIsStable;}
    long getCurrentComputationTime();

private:
    [[ noreturn ]] void mapStatusChecker();
    void waitForMapUnstable();
    void waitForMapStable();
    bool checkMapStable();
    void pauseSolver();
    void resumeSolver();

private:
    std::shared_ptr<std::thread> mapStatusThread;
    std::shared_ptr<std::vector<std::vector<std::shared_ptr<Node>>>> map;
    std::vector<NodeCollection> nodeCollections;
    std::shared_ptr<std::mutex> collectionControlMutex;
    std::chrono::steady_clock::time_point timeMeasureBegin;
    std::chrono::steady_clock::time_point timeMeasureEnd;
    bool mapIsStable = false;

    std::shared_ptr<std::pair<std::size_t,std::size_t>> currentPosition;
    std::shared_ptr<std::pair<std::size_t,std::size_t>> currentHeading;
};

#endif // PATHFINDER_H
