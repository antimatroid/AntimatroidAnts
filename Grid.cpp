#include "Grid.h"

using namespace std;

//constructers
Grid::Grid()
{
    rows = cols = 0;
};

Grid::Grid(int Rows, int Cols, int AttackRadius, int ViewRadius)
{
    rows = Rows;
    cols = Cols;
    attackradius = AttackRadius;
    viewradius = ViewRadius;

    grid = std::vector<std::vector<Square> >(rows, std::vector<Square>(cols, Square()));

    setupLandNeighbourInformation();
    setupTranslationValues();
    setupBattleNeighbours();
    setupPossibleNeighbours();
};

//reset function
void Grid::reset()
{
    for(int r=0; r<rows; r++)
        for(int c=0; c<cols; c++)
            //if(!grid[r][c].isWater)
                grid[r][c].reset();
};


//returns the new location from moving in a given direction with the edges wrapped
Location Grid::getLocation(const Location &loc, int direction)
{
    return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};

//returns the new location from moving in a given direction without the edges wrapped
Location Grid::getAbsLocation(const Location &loc, int direction)
{
    return Location(loc.row + DIRECTIONS[direction][0],
                    loc.col + DIRECTIONS[direction][1]);
};

//returns the new location from moving in a given direction with the edges wrapped
Location Grid::getTranslateLocation(const Location &loc, const Location &translateAmount)
{
    return Location( (loc.row + translateAmount.row + rows) % rows,
                     (loc.col + translateAmount.col + cols) % cols );
};


//adds the location of each of the neighbours to each square
void Grid::setupLandNeighbourInformation()
{
    Location cLoc, nLoc;

    //adds land neighbours to each square
    for(int r=0; r<rows; r++)
        for(int c=0; c<cols; c++)
        {
            cLoc = Location(r, c);

            for(int d=0; d<TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);
                (*this)[cLoc].landNeighbours.push_back(nLoc);
            }
        }
};

void Grid::setupTranslationValues()
{
    //works out attack and vision translation amounts
    Location sLoc, cLoc, nLoc;
    std::queue<Location> searchQueue;
    std::unordered_set<Location, LocationIndex> attackBorder, viewBorder;
    Matrix<uint8_t> searched(rows, cols, 0);

    //works out the attack translation amounts
    sLoc = Location(ceil(sqrt(viewradius)), ceil(sqrt(viewradius)));

    viewTranslations.push_back(sLoc - sLoc);

    searchQueue.push(sLoc);
    searched[sLoc] = 1;

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)(*this)[cLoc].landNeighbours.size(); d++)
        {
            nLoc = (*this)[cLoc].landNeighbours[d];

            if(!searched[nLoc])
            {
                searched[nLoc] = 1;

                if(edist(sLoc, nLoc) <= viewradius)
                {
                    viewTranslations.push_back(nLoc-sLoc);
                    searchQueue.push(nLoc);

                    if(edist(sLoc, nLoc) <= attackradius)
                        attackTranslations.push_back(nLoc-sLoc);
                    else if(edist(sLoc, cLoc) <= attackradius && !attackBorder.count(cLoc))
                    {
                        attackBorder.insert(cLoc);
                        attackBorderTranslations.push_back(cLoc-sLoc);
                    }
                }
                else if(!viewBorder.count(cLoc))
                {
                    viewBorder.insert(cLoc);
                    viewBorderTranslations.push_back(cLoc-sLoc);
                }
            }
        }
    }
};

void Grid::setupBattleNeighbours()
{
    Location sLoc, cLoc, tLoc, nLoc;
    vector<Location> battleTranslations;
    Matrix<uint8_t> added(rows, cols, 0);

    sLoc = Location(ceil(sqrt(viewradius)), ceil(sqrt(viewradius)));

    vector<Location> moveLocs = (*this)[sLoc].landNeighbours;
    moveLocs.push_back(sLoc);

    for(int d=0; d<(int)moveLocs.size(); d++)
    {
        nLoc = moveLocs[d];
        if(d > 0) //don't want to add itself
            battleTranslations.push_back(nLoc-sLoc);
        added[nLoc] = 1;
    }

    //for each land neighbour adds the attack border translations and any land neighbours of the translated location
    for(int m=0; m<(int)moveLocs.size(); m++)
    {
        cLoc = moveLocs[m];

        for(int a=0; a<(int)attackBorderTranslations.size(); a++)
        {
            tLoc = getTranslateLocation(cLoc, attackBorderTranslations[a]);

            if(!(*this)[tLoc].isWater)
            {
                if(!added[tLoc])
                {
                    battleTranslations.push_back(tLoc-sLoc);
                    added[tLoc] = 1;
                }

                for(int d=0; d<(int)(*this)[tLoc].landNeighbours.size(); d++)
                {
                    nLoc = (*this)[tLoc].landNeighbours[d];
                    if(!added[nLoc])
                    {
                        battleTranslations.push_back(nLoc-sLoc);
                        added[nLoc] = 1;
                    }
                }
            }
        }
    }

    int nonBorderTranslations = attackTranslations.size() - attackBorderTranslations.size();
    for(int m=0; m<(int)moveLocs.size(); m++)
    {
        cLoc = moveLocs[m];

        for(int a=0; a<nonBorderTranslations; a++)
        {
            tLoc = getTranslateLocation(cLoc, attackTranslations[a]);

            if(!(*this)[tLoc].isWater && !added[tLoc])
            {
                battleTranslations.push_back(tLoc-sLoc);
                added[tLoc] = 1;
            }
        }
    }

    //adds the battle neighbours to each square
    for(int r=0; r<rows; r++)
    {
        for(int c=0; c<cols; c++)
        {
            cLoc = Location(r, c);

            for(int b=0; b<(int)battleTranslations.size(); b++)
                (*this)[cLoc].battleNeighbours.push_back(getTranslateLocation(cLoc, battleTranslations[b]));
        }
    }
};

void Grid::setupPossibleNeighbours()
{
    Location sLoc, cLoc, tLoc, nLoc;
    vector<Location> battleTranslations;
    Matrix<uint8_t> added(rows, cols, 0);

    sLoc = Location(ceil(sqrt(viewradius)), ceil(sqrt(viewradius)));

    //for each land neighbour adds the attack border translations and any land neighbours of the translated location
    for(int a=0; a<(int)attackBorderTranslations.size(); a++)
    {
        tLoc = getTranslateLocation(sLoc, attackBorderTranslations[a]);

        if(!(*this)[tLoc].isWater)
        {
            if(!added[tLoc])
            {
                battleTranslations.push_back(tLoc-sLoc);
                added[tLoc] = 1;
            }

            for(int d=0; d<(int)(*this)[tLoc].landNeighbours.size(); d++)
            {
                nLoc = (*this)[tLoc].landNeighbours[d];
                if(!added[nLoc])
                {
                    battleTranslations.push_back(nLoc-sLoc);
                    added[nLoc] = 1;
                }
            }
        }
    }

    int nonBorderTranslations = attackTranslations.size() - attackBorderTranslations.size();
    for(int a=0; a<nonBorderTranslations; a++)
    {
        tLoc = getTranslateLocation(sLoc, attackTranslations[a]);

        if(!(*this)[tLoc].isWater && !added[tLoc])
        {
            battleTranslations.push_back(tLoc-sLoc);
            added[tLoc] = 1;
        }
    }

    //adds the battle neighbours to each square
    for(int r=0; r<rows; r++)
    {
        for(int c=0; c<cols; c++)
        {
            cLoc = Location(r, c);

            for(int b=0; b<(int)battleTranslations.size(); b++)
                (*this)[cLoc].possibleNeighbours.push_back(getTranslateLocation(cLoc, battleTranslations[b]));
        }
    }
};



void Grid::updateBattleNeighbours()
{
    for(auto bLoc=updateableBattleLocations.begin(); bLoc != updateableBattleLocations.end(); bLoc++)
        if(!(*this)[*bLoc].isWater)
            calculateBattleNeighbours(*bLoc);
    updateableBattleLocations.clear();
};

void Grid::calculateBattleNeighbours(const Location &sLoc)
{
    Location cLoc, tLoc, nLoc;
    Matrix<uint8_t> added(rows, cols, 0);

    (*this)[sLoc].battleNeighbours.clear();

    vector<Location> moveLocs = (*this)[sLoc].landNeighbours;
    moveLocs.push_back(sLoc);

    for(int d=0; d<(int)moveLocs.size(); d++)
    {
        nLoc = moveLocs[d];
        (*this)[sLoc].battleNeighbours.push_back(nLoc);
        added[nLoc] = 1;
    }

    //for each land neighbour adds the attack border translations and any land neighbours of the translated location
    for(int m=0; m<(int)moveLocs.size(); m++)
    {
        cLoc = moveLocs[m];

        for(int a=0; a<(int)attackBorderTranslations.size(); a++)
        {
            tLoc = getTranslateLocation(cLoc, attackBorderTranslations[a]);

            if(!(*this)[tLoc].isWater)
            {
                if(!added[tLoc])
                {
                    (*this)[sLoc].battleNeighbours.push_back(tLoc);
                    added[tLoc] = 1;
                }

                for(int d=0; d<(int)(*this)[tLoc].landNeighbours.size(); d++)
                {
                    nLoc = (*this)[tLoc].landNeighbours[d];
                    if(!added[nLoc])
                    {
                        (*this)[sLoc].battleNeighbours.push_back(nLoc);
                        added[nLoc] = 1;
                    }
                }
            }
        }
    }

    int nonBorderTranslations = attackTranslations.size() - attackBorderTranslations.size();
    for(int m=0; m<(int)moveLocs.size(); m++)
    {
        cLoc = moveLocs[m];

        for(int a=0; a<nonBorderTranslations; a++)
        {
            tLoc = getTranslateLocation(cLoc, attackTranslations[a]);

            if(!(*this)[tLoc].isWater && !added[tLoc])
            {
                (*this)[sLoc].battleNeighbours.push_back(tLoc);
                added[tLoc] = 1;
            }
        }
    }

    //adds the battle neighbours to each square
    /*if(sLoc == Location(10, 10))
        for(int r=0; r<rows; r++)
        {
            for(int c=0; c<cols; c++)
                cout << added(r,c);
            cout << endl;
        }*/
};

void Grid::updatePossibleNeighbours()
{
    for(auto bLoc=updateablePossibleLocations.begin(); bLoc != updateablePossibleLocations.end(); bLoc++)
        if(!(*this)[*bLoc].isWater)
            calculatePossibleNeighbours(*bLoc);
    updateablePossibleLocations.clear();
};

void Grid::calculatePossibleNeighbours(const Location &sLoc)
{
    Location cLoc, tLoc, nLoc;
    Matrix<uint8_t> added(rows, cols, 0);

    (*this)[sLoc].possibleNeighbours.clear();

    for(int a=0; a<(int)attackBorderTranslations.size(); a++)
    {
        tLoc = getTranslateLocation(sLoc, attackBorderTranslations[a]);

        if(!(*this)[tLoc].isWater)
        {
            if(!added[tLoc])
            {
                (*this)[sLoc].possibleNeighbours.push_back(tLoc);
                added[tLoc] = 1;
            }

            for(int d=0; d<(int)(*this)[tLoc].landNeighbours.size(); d++)
            {
                nLoc = (*this)[tLoc].landNeighbours[d];
                if(!added[nLoc])
                {
                    (*this)[sLoc].possibleNeighbours.push_back(nLoc);
                    added[nLoc] = 1;
                }
            }
        }
    }

    int nonBorderTranslations = attackTranslations.size() - attackBorderTranslations.size();
    for(int a=0; a<nonBorderTranslations; a++)
    {
        tLoc = getTranslateLocation(sLoc, attackTranslations[a]);

        if(!(*this)[tLoc].isWater && !added[tLoc])
        {
            (*this)[sLoc].possibleNeighbours.push_back(tLoc);
            added[tLoc] = 1;
        }
    }

    //adds the battle neighbours to each square
    /*if(sLoc == Location(10, 10))
        for(int r=0; r<rows; r++)
        {
            for(int c=0; c<cols; c++)
                cout << added(r,c);
            cout << endl;
        }*/
};

//marks the location as water and removes relevant neighbour information
void Grid::addWaterInformation(const Location &wLoc)
{
    //don't need to do this twice
    if((*this)[wLoc].isWater)
        return;

    Location cLoc, nLoc;
    for(int d=0; d<(int)(*this)[wLoc].landNeighbours.size(); d++)
    {
        cLoc = (*this)[wLoc].landNeighbours[d];

        //updates the land neighbour information
        for(int d=0; d<(int)(*this)[cLoc].landNeighbours.size(); d++)
        {
            nLoc = (*this)[cLoc].landNeighbours[d];
            if(nLoc == wLoc)
            {
                (*this)[cLoc].landNeighbours.erase((*this)[cLoc].landNeighbours.begin()+d);

                (*this)[cLoc].landDirections.erase((*this)[cLoc].landDirections.begin()+d);
                (*this)[cLoc].oppositeLandDirections.erase((*this)[cLoc].oppositeLandDirections.begin()+d);
            }
        }
    }

    //updates the battle neighbour information
    for(int d=0; d<(int)(*this)[wLoc].battleNeighbours.size(); d++)
        updateableBattleLocations.insert((*this)[wLoc].battleNeighbours[d]);

    for(int d=0; d<(int)(*this)[wLoc].possibleNeighbours.size(); d++)
        updateablePossibleLocations.insert((*this)[wLoc].possibleNeighbours[d]);

    (*this)[wLoc].landNeighbours.clear();
    (*this)[wLoc].battleNeighbours.clear();
    (*this)[wLoc].possibleNeighbours.clear();

    (*this)[wLoc].landDirections.clear();
    (*this)[wLoc].oppositeLandDirections.clear();

    (*this)[wLoc].possibleEnemy = 0;
    (*this)[wLoc].isWater = 1;
};


//euclidean distance squared between two locations on the surface of a torus
int Grid::edist(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = std::min(d1, rows-d1),
        dc = std::min(d2, cols-d2);
    return dr*dr + dc*dc;
};

//manhatten distance between two locations on the surface of a torus
int Grid::mdist(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = std::min(d1, rows-d1),
        dc = std::min(d2, cols-d2);
    return dr + dc;
};

//minimum manhatten distance between a source location and multiple target locations on the surface of a torus
int Grid::hdist(const Location &source, std::unordered_set<Location, LocationIndex> &targets)
{
    int hdist = 9999;

    for(auto target=targets.begin(); target!=targets.end(); target++)
        hdist = std::min(hdist, mdist(source, *target));

    return hdist;
};

