#ifndef GRID_H_
#define GRID_H_

#include <cmath>
#include <queue>
#include <vector>
#include <unordered_set>

#include "Square.h"
#include "Location.h"
#include "Move.h"
#include "Matrix.h"

const int N = 0, E = 1, S = 2, W = 3;
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      //{N, E, S, W}

struct Grid
{
    /*
        Variables
    */
    int rows, cols,
        attackradius, viewradius;

    std::vector<std::vector<Square> > grid;

    std::vector<Location> attackTranslations, viewTranslations,
                          attackBorderTranslations, viewBorderTranslations;

    std::unordered_set<Location, LocationIndex> updateableBattleLocations, updateablePossibleLocations;

    /*
        Functions
    */
    Grid();
    Grid(int Rows, int Cols, int AttackRadius, int ViewRadius);

    void reset();

    Location getLocation(const Location &loc, int direction);
    Location getAbsLocation(const Location &loc, int direction);
    Location getTranslateLocation(const Location &loc, const Location &translateAmount);

    void setupLandNeighbourInformation();
    void setupTranslationValues();
    void setupBattleNeighbours();
    void setupPossibleNeighbours();

    void updateBattleNeighbours();
    void calculateBattleNeighbours(const Location &sLoc);
    void updatePossibleNeighbours();
    void calculatePossibleNeighbours(const Location &sLoc);

    //marks the location as water and removes relevant neighbour information
    void addWaterInformation(const Location &wLoc);

    void updateVisionInformation();


    //distance functions on the surface of a torus
    int edist(const Location &loc1, const Location &loc2);                  //Euclidean
    int mdist(const Location &loc1, const Location &loc2);                  //manhatten
    int hdist(const Location &source, std::unordered_set<Location, LocationIndex> &targets);   //min manhatten

    //index functions
    inline std::vector<Square>& operator[](int r) const
    {
        return (std::vector<Square>&) grid[r];
    };

    inline Square& operator[](const Location &loc) const
    {
        return (Square&) grid[loc.row][loc.col];
    };
};

#endif //GRID_H_
