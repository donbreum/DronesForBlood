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
    void setCurrentPosition(std::pair<std::size_t,std::size_t> position);
    void startSolver();
    void generateTestMap();
    void updatePenaltyOfNode(std::size_t row, std::size_t col, double penalty);
    bool getMapStable() {return  mapIsStable;}
    void makePathToDestination(std::size_t row, std::size_t col, std::vector<std::pair<std::size_t, std::size_t> > &path);
    void printMapStatus(std::size_t row, std::size_t col);
    void printCostMap();
    void printPathMap(std::size_t row, std::size_t col);

private:
    [[ noreturn ]] void mapStatusChecker();
    void waitForMapUnstable();
    void waitForMapStable();
    bool checkMapStable();

    void calculateNeighbors(std::size_t row, std::size_t col, std::vector<std::shared_ptr<Node>> &neighborNodes);
    void divideIntoCollections();
    void getSlice(std::size_t row, std::size_t col, std::size_t rowCount, std::size_t colCount, NodeCollection &collection);

    void printMap();


private:
    std::shared_ptr<std::thread> mapStatusThread;
    std::vector<std::vector<std::shared_ptr<Node>>> map;
    std::vector<NodeCollection> nodeCollections;
    std::shared_ptr<std::mutex> collectionControlMutex;
    std::chrono::steady_clock::time_point timeMeasureBegin;
    std::chrono::steady_clock::time_point timeMeasureEnd;
    bool mapIsStable = false;

    std::pair<std::size_t,std::size_t> currentPosition;

    // Note to self: DO NOT MAKE SUPER LARGER MAPS.
    const std::size_t mapSize = 2000;
    std::size_t gridRows = 50;
    std::size_t gridCols = 50;
};

#endif // PATHFINDER_H
