#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <utility>
#include <vector>
#include <iostream>
#include <math.h>
#include <memory>
#include <unistd.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "headers/global/defines.h"
#include "headers/global/geofunctions.h"
#include "headers/pathfinder.h"
#include "headers/mapgenerator.h"
#include "headers/watch/watchzone.h"
#include "headers/pathshortener.h"

class MapController
{
public:
    MapController();
    ~MapController();
    bool getMapReady() { return mapReady;}
    void generateMap(std::pair<double, double> startCoord, std::pair<double, double> endCoord, double distanceBetweenNodes, double width, double padLength);
    std::pair<std::size_t, std::size_t> getMapSize();
    void setGoalPosition(std::pair<double,double> goalCoord) {goalPosition = getClosestNodeIndex(goalCoord);}
    void startSolver(std::pair<double, double> worldCoord);
    void setCurrentHeading(std::pair<double, double> headingCoord);
    void setCurrentPosition(std::pair<double, double> currentCoord);
    void updatePenaltyOfArea(std::pair<double,double> position, double radius, double penalty);
    void updatePenaltyOfNode(std::size_t row, std::size_t col, double penalty);
    bool getPathToDestination(std::vector<std::pair<double, double> > &path);

    void printCurrentPositionImage(std::pair<double,double> newPosition);
    void printPathImage(std::vector<std::pair<std::size_t, std::size_t> > &path);
    void printShortPathImage(std::vector<std::pair<std::size_t, std::size_t> > &shortPath);
    std::pair<double,double> getWorldCoordAtIndex(std::size_t row, std::size_t col) {return map->at(row).at(col)->getWorldCoordinate();}


private:
    std::pair<std::size_t, std::size_t> getClosestNodeIndex(std::pair<double, double> worldCoord);
    bool makePathToDestination(std::pair<std::size_t,std::size_t> pos, std::vector<std::pair<std::size_t, std::size_t> > &path);
    bool isInsideMap(int row, int col);

private:
    cv::Mat pathImage;
    std::shared_ptr<std::vector<std::vector<std::shared_ptr<Node>>>> map;
    std::vector<NodeCollection> nodeCollections;
    std::pair<std::size_t,std::size_t> goalPosition;
    std::pair<std::size_t,std::size_t> initPosition;
    std::shared_ptr<std::pair<std::size_t,std::size_t>> currentHeading;
    std::pair<double, double> currentPosition;
    std::vector<std::pair<std::size_t, std::size_t>> currentPath;
    std::vector<std::pair<std::size_t, std::size_t>> currentShortPath;
    Pathfinder solver;
    bool mapReady = false;

    int timeCounter = 0;
    int iterationCounter = 0;
};

#endif // MAPCONTROLLER_H
