#include "State.h"

using namespace std;

//constructor
State::State()
{
};

//deconstructor
State::~State()
{
};

//sets the state up
void State::setup()
{
    cin >> *this;
    grid = Grid(rows, cols, attackradius, viewradius);

    closestFriendHillInfoChanged = closestEnemyHillInfoChanged = 1;
    closestHill = closestFriendHill = closestEnemyHill = Matrix<int>(rows, cols, 9999);
    adjUnmovedFriends = adjUnmovedEnemies = Matrix<uint8_t>(rows, cols, 0);

    cout << "go" << endl;
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    oldFood = food;
    oldMyHills = myHills;
    oldEnemyHills = enemyHills;

    moveableAnts.clear();
    unmovedAnts.clear();
    enemyAnts.clear();
    myHills.clear();
    enemyHills.clear();
    food.clear();
    uncollectedFood.clear();

    closestFriendHillInfoChanged = closestEnemyHillInfoChanged = 0;

    possibleEnemyBorder.clear();
    notVisibleBorder.clear();

    grid.reset();
};

bool State::checkTime(double maxTimeProportion)
{
    return timer.getTime() < maxTimeProportion*turntime;
};

//outputs move information to the engine
void State::makeMove(const Move &move)
{
    if(grid[move.aLoc].available == 0)
    {
        if(move.direction != 'X')
            cout << "o " << move.aLoc.row << " " << move.aLoc.col << " " << move.direction << endl;

        grid[move.mLoc].taken = 1;
    }

    moveableAnts.erase(move.aLoc);
    unmovedAnts.erase(move.aLoc);

    //reuses ant
    if(move.reuseAnt)
    {
        grid[move.cLoc].available = move.pathLength;
        moveableAnts.insert(move.cLoc);
    }

    grid[move.aLoc].ant = -1;
    if(!move.reuseAnt || move.aLoc != move.cLoc)
    {
        grid[move.aLoc].available  = -1;
    }
};

void State::makeMoves(const vector<Move> &moves)
{
    for(int m=0; m<(int)moves.size(); m++)
        makeMove(moves[m]);
};

vector<Move> State::getOurDefensiveMovesMinimisingBattleEnemies(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, less<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            //moves onto one of our hills
            if(grid[mLoc].hill == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill == 0)
                continue;

            //minimises possible battle enemies
            if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }


        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getOurSpecialOffensiveMoves(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));
    }

    Location aLoc, bLoc, mLoc, tLoc;
    char bDirection;
    int bPossibleEnemySlack, cPossibleEnemySlack, minBattleFriends;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        //works out bPossibleEnemySlack
        minBattleFriends = 9;
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(bLoc, grid.attackTranslations[t]);
            if(grid[tLoc].adjToEnemy && grid[tLoc].possibleBattleFriends < minBattleFriends)
                minBattleFriends = grid[tLoc].possibleBattleFriends;
        }
        bPossibleEnemySlack = minBattleFriends - grid[bLoc].possibleBattleEnemies;

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            //works out battle slack
            minBattleFriends = 9;
            for(int t=0; t<(int)grid.attackTranslations.size(); t++)
            {
                tLoc = grid.getTranslateLocation(mLoc, grid.attackTranslations[t]);
                if(grid[tLoc].adjToEnemy && grid[tLoc].possibleBattleFriends < minBattleFriends)
                    minBattleFriends = grid[tLoc].possibleBattleFriends;
            }
            cPossibleEnemySlack = minBattleFriends - grid[mLoc].possibleBattleEnemies;

            //tried to get positive battle slack
            if(cPossibleEnemySlack > 0 && bPossibleEnemySlack == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(cPossibleEnemySlack == 0 && bPossibleEnemySlack > 0)
                continue;

            //tries to get in battle range of positive number of enemies
            if(grid[mLoc].possibleBattleEnemies > 0 && grid[bLoc].possibleBattleEnemies == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies == 0 && grid[bLoc].possibleBattleEnemies > 0)
                continue;

            //minimises the number of possible battle enemies
            if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //maximises the number of possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
                continue;



            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                bPossibleEnemySlack = cPossibleEnemySlack;
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getOurOffensiveMovesMinimisingPositiveBattleEnemies(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));
    }

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            //tries to get in battle range of positive number of enemies
            if(grid[mLoc].possibleBattleEnemies > 0 && grid[bLoc].possibleBattleEnemies == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies == 0 && grid[bLoc].possibleBattleEnemies > 0)
                continue;

            //minimises the number of possible battle enemies
            if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //maximises the number of possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getOurOffensiveMovesMaximisingBattleEnemies(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));
    }

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            //minimises the number of possible battle enemies
            if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //maximises the number of possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getOurNeutralMovesMinimisingPositiveBattleEnemies(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));
    }

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            if(grid[mLoc].soonestEnemyBattle == 0) //neutral move
                continue;

            //tries to get in battle range of positive number of enemies
            if(grid[mLoc].possibleBattleEnemies > 0 && grid[bLoc].possibleBattleEnemies == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies == 0 && grid[bLoc].possibleBattleEnemies > 0)
                continue;

            //minimises the number of possible battle enemies
            if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //maximises the number of possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getOurNeutralMovesMaximisingBattleEnemies(const vector<Location> &ourAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedFriends;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)ourAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[ourAnts[a] ].closestEnemy, ourAnts[a]));
    }

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(grid[mLoc].ant > 0 || grid[mLoc].taken || taken[mLoc]) //location is already taken
                continue;

            if(grid[mLoc].soonestEnemyBattle == 0) //neutral move
                continue;

            //maximises the number of possible battle enemies
            if(grid[mLoc].possibleBattleEnemies > grid[bLoc].possibleBattleEnemies)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemies < grid[bLoc].possibleBattleEnemies)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0)
                continue;

            //minimises adjacent unmoved friends
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //maximises the number of possible battle locations
            if(grid[mLoc].possibleBattleEnemyLocations > grid[bLoc].possibleBattleEnemyLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleEnemyLocations < grid[bLoc].possibleBattleEnemyLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestEnemy < grid[bLoc].closestEnemy)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestEnemy > grid[bLoc].closestEnemy)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};


vector<Move> State::getTheirDefensiveMoves(const vector<Location> &theirAnts)
{
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, less<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedEnemies;

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)theirAnts.size(); a++)
    {
        orderedAnts.push(pair<int, Location>(grid[theirAnts[a] ].closestFriend, theirAnts[a]));
    }

    Location aLoc, bLoc, mLoc;
    char bDirection;

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(taken[mLoc]) //location is already taken
                continue;

            //moves onto one of their hills
            if(grid[mLoc].hill == grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill == grid[aLoc].ant)
                continue;

            //minimises possible battle enemies
            if(grid[mLoc].possibleBattleFriends < grid[bLoc].possibleBattleFriends)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriends > grid[bLoc].possibleBattleFriends)
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > -1 && grid[mLoc].hill != grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > -1 && grid[bLoc].hill != grid[aLoc].ant)
                continue;

            //minimises adjacent unmoved enemies
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises possible battle locations
            if(grid[mLoc].possibleBattleFriendLocations < grid[bLoc].possibleBattleFriendLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriendLocations > grid[bLoc].possibleBattleFriendLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestFriend < grid[bLoc].closestFriend)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestFriend > grid[bLoc].closestFriend)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getTheirNeutralMovesMinimisingPositiveBattleEnemies(const vector<Location> &theirAnts, const vector<Move> &ourMoves)
{
    Location aLoc, bLoc, mLoc, tLoc;
    char bDirection;
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> battleFriends(rows, cols, 0),
                    taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedEnemies;

    //sets up the battle friends information
    for(int m=0; m<(int)ourMoves.size(); m++)
    {
        aLoc = ourMoves[m].mLoc;
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(aLoc, grid.attackTranslations[t]);
            battleFriends[tLoc]++;
        }
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)theirAnts.size(); a++)
        orderedAnts.push(pair<int, Location>(grid[theirAnts[a] ].closestFriend, theirAnts[a]));

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(taken[mLoc]) //location is already taken
                continue;

            if(grid[mLoc].soonestFriendBattle == 0)
                continue;

            //tries to get in battle range of positive number of friends
            if(battleFriends[mLoc] > 0 && battleFriends[bLoc] == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] == 0 && battleFriends[bLoc] > 0)
                continue;

            //minimises the number of possible battle friends
            if(battleFriends[mLoc] < battleFriends[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] > battleFriends[bLoc])
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > -1 && grid[mLoc].hill != grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0 && grid[bLoc].hill != grid[aLoc].ant)
                continue;

            //minimises adjacent unmoved enemies
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises possible battle locations
            if(grid[mLoc].possibleBattleFriendLocations < grid[bLoc].possibleBattleFriendLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriendLocations > grid[bLoc].possibleBattleFriendLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestFriend < grid[bLoc].closestFriend)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestFriend > grid[bLoc].closestFriend)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getTheirNeutralMovesMaximisingBattleEnemies(const vector<Location> &theirAnts, const vector<Move> &ourMoves)
{
    Location aLoc, bLoc, mLoc, tLoc;
    char bDirection;
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> battleFriends(rows, cols, 0),
                    taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedEnemies;

    //sets up the battle friends information
    for(int m=0; m<(int)ourMoves.size(); m++)
    {
        aLoc = ourMoves[m].mLoc;
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(aLoc, grid.attackTranslations[t]);
            battleFriends[tLoc]++;
        }
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)theirAnts.size(); a++)
        orderedAnts.push(pair<int, Location>(grid[theirAnts[a] ].closestFriend, theirAnts[a]));

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(taken[mLoc]) //location is already taken
                continue;

            if(grid[mLoc].soonestFriendBattle == 0)
                continue;

            //minimises the number of possible battle friends
            if(battleFriends[mLoc] > battleFriends[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] < battleFriends[bLoc])
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > -1 && grid[mLoc].hill != grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0 && grid[bLoc].hill != grid[aLoc].ant)
                continue;

            //minimises adjacent unmoved enemies
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises possible battle locations
            if(grid[mLoc].possibleBattleFriendLocations < grid[bLoc].possibleBattleFriendLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriendLocations > grid[bLoc].possibleBattleFriendLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestFriend < grid[bLoc].closestFriend)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestFriend > grid[bLoc].closestFriend)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};


vector<Move> State::getTheirOffensiveMovesMinimisingPositiveBattleEnemies(const vector<Location> &theirAnts, const vector<Move> &ourMoves)
{
    Location aLoc, bLoc, mLoc, tLoc;
    char bDirection;
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> battleFriends(rows, cols, 0),
                    taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedEnemies;

    //sets up the battle friends information
    for(int m=0; m<(int)ourMoves.size(); m++)
    {
        aLoc = ourMoves[m].mLoc;
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(aLoc, grid.attackTranslations[t]);
            battleFriends[tLoc]++;
        }
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)theirAnts.size(); a++)
        orderedAnts.push(pair<int, Location>(grid[theirAnts[a] ].closestFriend, theirAnts[a]));

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(taken[mLoc]) //location is already taken
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > -1 && grid[mLoc].hill != grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0 && grid[bLoc].hill != grid[aLoc].ant)
                continue;

            //walks adjacent to food
            if(grid[mLoc].adjFood > grid[bLoc].adjFood)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].adjFood < grid[bLoc].adjFood)
                continue;

            //tries to get in battle range of positive number of friends
            if(battleFriends[mLoc] > 0 && battleFriends[bLoc] == 0)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] == 0 && battleFriends[bLoc] > 0)
                continue;

            //minimises the number of possible battle friends
            if(battleFriends[mLoc] < battleFriends[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] > battleFriends[bLoc])
                continue;

            //minimises adjacent unmoved enemies
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //minimises possible battle locations
            if(grid[mLoc].possibleBattleFriendLocations < grid[bLoc].possibleBattleFriendLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriendLocations > grid[bLoc].possibleBattleFriendLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestFriend < grid[bLoc].closestFriend)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestFriend > grid[bLoc].closestFriend)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};

vector<Move> State::getTheirOffensiveMovesMaximisingBattleEnemies(const vector<Location> &theirAnts, const vector<Move> &ourMoves)
{
    Location aLoc, bLoc, mLoc, tLoc;
    char bDirection;
    vector<Move> moves;
    priority_queue<pair<int, Location>, vector<pair<int, Location> >, greater<pair<int, Location> > > orderedAnts;

    Matrix<uint8_t> battleFriends(rows, cols, 0),
                    taken(rows, cols, 0),
                    adjUnmovedInfo = adjUnmovedEnemies;

    //sets up the battle friends information
    for(int m=0; m<(int)ourMoves.size(); m++)
    {
        aLoc = ourMoves[m].mLoc;
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(aLoc, grid.attackTranslations[t]);
            battleFriends[tLoc]++;
        }
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
        taken[*ant] = 1;

    for(int a=0; a<(int)theirAnts.size(); a++)
        orderedAnts.push(pair<int, Location>(grid[theirAnts[a] ].closestFriend, theirAnts[a]));

    while(!orderedAnts.empty())
    {
        bLoc = aLoc = orderedAnts.top().second;
        bDirection = 'X';
        orderedAnts.pop();

        adjUnmovedInfo[aLoc]--;
        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
            adjUnmovedInfo[grid[aLoc].landNeighbours[d] ]--;

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];

            if(taken[mLoc]) //location is already taken
                continue;

            //walks onto an enemy hill
            if(grid[mLoc].hill > -1 && grid[mLoc].hill != grid[aLoc].ant)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[bLoc].hill > 0 && grid[bLoc].hill != grid[aLoc].ant)
                continue;

            //walks adjacent to food
            if(grid[mLoc].adjFood > grid[bLoc].adjFood)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].adjFood < grid[bLoc].adjFood)
                continue;

            //maximises the number of possible battle friends
            if(battleFriends[mLoc] > battleFriends[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(battleFriends[mLoc] < battleFriends[bLoc])
                continue;

            //minimises adjacent unmoved enemies
            if(adjUnmovedInfo[mLoc] < adjUnmovedInfo[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(adjUnmovedInfo[mLoc] > adjUnmovedInfo[bLoc])
                continue;

            //maximises possible battle locations
            if(grid[mLoc].possibleBattleFriendLocations > grid[bLoc].possibleBattleFriendLocations)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].possibleBattleFriendLocations < grid[bLoc].possibleBattleFriendLocations)
                continue;

            //minimises closest enemy
            if(grid[mLoc].closestFriend < grid[bLoc].closestFriend)
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(grid[mLoc].closestFriend > grid[bLoc].closestFriend)
                continue;

            //minimises closest hill
            if(closestHill[mLoc] < closestHill[bLoc])
            {
                bLoc = mLoc;
                bDirection = grid[aLoc].landDirections[d];
                continue;
            }
            else if(closestHill[mLoc] > closestHill[bLoc])
                continue;
        }

        taken[aLoc] = 0;
        taken[bLoc] = 1;

        //adds the move
        moves.push_back(Move(aLoc, bLoc, bDirection));
    }

    return moves;
};



void State::updateVisionInformation()
{
    Location cLoc, tLoc, nLoc;
    noVisibleSquares = 0;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
    {
        cLoc = *ant;

        for(int t=0; t<(int)grid.viewTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(cLoc, grid.viewTranslations[t]);

            if(grid[tLoc].lastSeen < turn)
                noVisibleSquares++;

            if(grid[tLoc].lastSeen == -1)
            //if(closestFriendHill[tLoc] == 9999) //have to recalculate information
                closestFriendHillInfoChanged = closestEnemyHillInfoChanged = 1;

            grid[tLoc].lastSeen = turn;
            grid[tLoc].possibleEnemy = 0;
        }
    }

    if(noVisibleSquares > 0.3*rows*cols && unmovedAnts.size() > 5*enemyAnts.size())
        offensive = 1;
    else
        offensive = 0;
};

void State::bombVisionInformation(const Location &bLoc, unordered_set<Location, LocationIndex> &uBorder)
{
    Location tLoc;

    for(int t=0; t<(int)grid.viewTranslations.size(); t++)
    {
        tLoc = grid.getTranslateLocation(bLoc, grid.viewTranslations[t]);

        if(grid[tLoc].lastSeen == -1 && uBorder.count(tLoc))
            uBorder.erase(tLoc);
    }
};

void State::processOldInformation()
{
    Location cLoc;

    for(auto fLoc=oldFood.begin(); fLoc != oldFood.end(); fLoc++)
    {
        cLoc = *fLoc;
        if(grid[cLoc].lastSeen < turn)
        {
            grid[cLoc].isFood = 1;
            grid[cLoc].taken = 1;
            uncollectedFood.insert(cLoc);
            food.insert(cLoc);
        }
    }

    for(auto hLoc=oldMyHills.begin(); hLoc != oldMyHills.end(); hLoc++)
    {
        cLoc = *hLoc;
        if(grid[cLoc].lastSeen < turn)  //hill is out of view
            myHills.insert(cLoc);
        else if(!myHills.count(cLoc))   //hill has been razed
        {
            grid[cLoc].hill = -1;
            closestFriendHillInfoChanged = 1;
        }

    }

    for(auto hLoc=oldEnemyHills.begin(); hLoc != oldEnemyHills.end(); hLoc++)
    {
        cLoc = *hLoc;
        if(grid[cLoc].lastSeen < turn)  //hill is out of view
            enemyHills.insert(cLoc);
        else if(!enemyHills.count(cLoc))    //hill has been razed
        {
            grid[cLoc].hill = -1;
            closestEnemyHillInfoChanged = 1;
        }
    }
};

void State::collectFoodLocation(const Location &fLoc)
{
    grid[fLoc].isFood = 0;
    uncollectedFood.erase(fLoc);
};

void State::updateAdjFoodInformation(const Location &fLoc)
{
    for(int d=0; d<(int)grid[fLoc].landNeighbours.size(); d++)
        grid[grid[fLoc].landNeighbours[d] ].adjFood--;
};

void State::updateAdjFoodInformation()
{
    for(auto fLoc=food.begin(); fLoc!=food.end(); fLoc++)
        for(int d=0; d<(int)grid[*fLoc].landNeighbours.size(); d++)
            grid[grid[*fLoc].landNeighbours[d] ].adjFood++;
};

void State::updateClosestHillInformation()
{
    if(closestFriendHillInfoChanged || closestEnemyHillInfoChanged)
    {
        Location cLoc, nLoc;
        queue<Location> searchQueue;

        closestHill.reset(9999);

        for(auto ant=myHills.begin(); ant!=myHills.end(); ant++)
        {
            closestHill[*ant] = 0;
            searchQueue.push(*ant);
        }
        for(auto ant=enemyHills.begin(); ant!=enemyHills.end(); ant++)
        {
            closestHill[*ant] = 0;
            searchQueue.push(*ant);
        }

        while(!searchQueue.empty())
        {
            cLoc = searchQueue.front();
            searchQueue.pop();

            for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = grid[cLoc].landNeighbours[d];

                if(grid[nLoc].lastSeen > -1 && closestHill[nLoc] == 9999)
                {
                    closestHill[nLoc] = closestHill[cLoc] + 1;
                    searchQueue.push(nLoc);
                }
            }
        }


    }

    //closestFriendHillInfoChanged=1;
    if(closestFriendHillInfoChanged)
    {
        Location cLoc, nLoc;
        queue<Location> searchQueue;

        closestFriendHill.reset(9999);
        unseenBorder.clear();

        for(auto ant=myHills.begin(); ant!=myHills.end(); ant++)
        {
            closestFriendHill[*ant] = 0;
            searchQueue.push(*ant);
        }

        while(!searchQueue.empty())
        {
            cLoc = searchQueue.front();
            searchQueue.pop();

            for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = grid[cLoc].landNeighbours[d];

                if(grid[nLoc].lastSeen == -1)
                    unseenBorder.insert(nLoc);
                else if(closestFriendHill[nLoc] == 9999)
                {
                    closestFriendHill[nLoc] = closestFriendHill[cLoc] + 1;
                    searchQueue.push(nLoc);
                }
            }
        }
    }

    if(closestEnemyHillInfoChanged)
    {
        Location cLoc, nLoc;
        queue<Location> searchQueue;

        closestEnemyHill.reset(9999);

        for(auto ant=enemyHills.begin(); ant!=enemyHills.end(); ant++)
        {
            closestEnemyHill[*ant] = 0;
            searchQueue.push(*ant);
        }

        while(!searchQueue.empty())
        {
            cLoc = searchQueue.front();
            searchQueue.pop();

            for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = grid[cLoc].landNeighbours[d];

                if(grid[nLoc].lastSeen > -1 && closestEnemyHill[nLoc] == 9999)
                {
                    closestEnemyHill[nLoc] = closestEnemyHill[cLoc] + 1;
                    searchQueue.push(nLoc);
                }
            }
        }
    }
};


void State::updateClosestFriendAndEnemyInfo()
{
    int maxPath = 10;
    Location cLoc, nLoc;
    queue<Location> searchQueue;

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
    {
        grid[*ant].closestFriend = 0;
        searchQueue.push(*ant);
    }

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(grid[nLoc].closestFriend == 9999)
            {
                grid[nLoc].closestFriend = grid[cLoc].closestFriend + 1;

                if(grid[nLoc].closestFriend < maxPath)
                    searchQueue.push(nLoc);
            }
        }
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
    {
        grid[*ant].closestEnemy = 0;
        searchQueue.push(*ant);
    }

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];


            if(grid[nLoc].closestEnemy == 9999)
            {
                grid[nLoc].closestEnemy = grid[cLoc].closestEnemy + 1;

                if(grid[nLoc].closestEnemy < maxPath)
                    searchQueue.push(nLoc);
            }
        }
    }

};

void State::updateNotVisibleBorder()
{
    Location cLoc, nLoc;
    queue<Location> searchQueue;

    Matrix<uint8_t> searched(rows, cols, 0);
    notVisibleBorder.clear();

    for(auto ant=myHills.begin(); ant!=myHills.end(); ant++)
    {
        searchQueue.push(*ant);
        searched[*ant] = 1;
    }

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(searched[nLoc])
                continue;

            searched[nLoc] = 1;

            if(grid[nLoc].lastSeen == -1)
                continue;
            if(grid[nLoc].possibleEnemy)
                continue;
            else if(grid[nLoc].lastSeen+15 < turn)
                notVisibleBorder.insert(nLoc);
            else if(closestFriendHill[nLoc] < 9999 && closestFriendHill[nLoc] > closestFriendHill[cLoc])
                searchQueue.push(nLoc);
        }
    }
};

void State::updatePossibleEnemyBorder()
{
    Location cLoc, nLoc;
    queue<Location> searchQueue;

    Matrix<uint8_t> searched(rows, cols, 0);
    possibleEnemyBorder.clear();

    for(auto ant=myHills.begin(); ant!=myHills.end(); ant++)
    {
        searchQueue.push(*ant);
        searched[*ant] = 1;
    }

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(searched[nLoc])
                continue;

            searched[nLoc] = 1;

            if(grid[nLoc].lastSeen == -1)
                continue;
            if(grid[nLoc].possibleEnemy)
                possibleEnemyBorder.insert(nLoc);
            else if(closestFriendHill[nLoc] < 9999 && closestFriendHill[nLoc] > closestFriendHill[cLoc])
                searchQueue.push(nLoc);
        }
    }
};

void State::updateBattleBorder()
{
    Location cLoc, nLoc;
    queue<Location> searchQueue;

    Matrix<uint8_t> searched(rows, cols, 0);
    battleBorder.clear();

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
    {
        searchQueue.push(*ant);
        searched[*ant] = 1;
    }

    while(!searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(searched[nLoc])
                continue;

            searched[nLoc] = 1;

            if(grid[nLoc].soonestEnemyBattle > 1 && grid[nLoc].soonestEnemyBattle < 4 && !grid[nLoc].taken)
                battleBorder.insert(nLoc);
            else if(grid[nLoc].soonestEnemyBattle < 3)
                searchQueue.push(nLoc);
        }
    }
};

void State::updateEnemyHillBorder()
{
    enemyHillBorder.clear();

    for(auto ant=enemyHills.begin(); ant!=enemyHills.end(); ant++)
        for(int t=0; t<(int)grid.attackBorderTranslations.size(); t++)
            enemyHillBorder.insert(grid.getTranslateLocation(*ant, grid.attackBorderTranslations[t]));
};

void State::updatePossibleEnemyInfo()
{
    Location cLoc, nLoc, eLoc, hLoc;

    //adds the enemy ants locations back in
    for(auto enemy = enemyAnts.begin(); enemy!= enemyAnts.end(); enemy++)
        grid[*enemy].possibleEnemy = 1;

    //finds the new possible enemy locations
    unordered_set<Location, LocationIndex> newPossibleEnemies;
    for(int r=0; r<rows; r++)
        for(int c=0; c<cols; c++)
            if(grid[r][c].possibleEnemy)
                for(int d=0; d<(int)grid[r][c].landNeighbours.size(); d++)
                    if(!grid[grid[r][c].landNeighbours[d] ].possibleEnemy)
                        newPossibleEnemies.insert(grid[r][c].landNeighbours[d]);

    for(auto enemy = newPossibleEnemies.begin(); enemy != newPossibleEnemies.end(); enemy++)
        grid[*enemy].possibleEnemy = 1;

    //adds ants that might spawn at enemy hills
    for(auto hill = enemyHills.begin(); hill != enemyHills.end(); hill++)
        grid[*hill].possibleEnemy = 1;
};

void State::calculatePossibleBattleInfo()
{
    Location aLoc, bLoc, mLoc;
    Matrix<uint8_t> possibleFriends(rows, cols, 0), possibleEnemies(rows, cols, 0);
    unordered_set<Location, LocationIndex> friendMoveLocs, enemyMoveLocs;
    unordered_set<Location, LocationIndex> battleAnts, battleLocations;

    for(auto ant=unmovedAnts.begin(); ant != unmovedAnts.end(); ant++)
    {
        aLoc = *ant;
        possibleFriends[aLoc]++;
        friendMoveLocs.insert(aLoc);

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];
            possibleFriends[mLoc]++;
            friendMoveLocs.insert(mLoc);
        }
    }

    for(auto ant=enemyAnts.begin(); ant != enemyAnts.end(); ant++)
    {
        aLoc = *ant;
        possibleEnemies[aLoc]++;
        enemyMoveLocs.insert(aLoc);

        for(int d=0; d<(int)grid[aLoc].landNeighbours.size(); d++)
        {
            mLoc = grid[aLoc].landNeighbours[d];
            possibleEnemies[mLoc]++;
            enemyMoveLocs.insert(mLoc);
        }
    }

    for(auto it=friendMoveLocs.begin(); it!=friendMoveLocs.end(); it++)
    {
        mLoc = *it;

        for(int d=0; d<(int)grid[mLoc].possibleNeighbours.size(); d++)
        {
            bLoc = grid[mLoc].possibleNeighbours[d];

            if(grid[bLoc].ant > 0)
                battleAnts.insert(bLoc);
        }

        for(int d=0; d<(int)grid.attackTranslations.size(); d++)
        {
            bLoc = grid.getTranslateLocation(mLoc, grid.attackTranslations[d]);

            if(enemyMoveLocs.count(bLoc))
                battleLocations.insert(bLoc);
        }

        grid[mLoc].possibleBattleEnemies = min(battleLocations.size(), battleAnts.size());
        grid[mLoc].possibleBattleEnemyLocations = battleLocations.size();

        battleAnts.clear();
        battleLocations.clear();
    }

    for(auto it=enemyMoveLocs.begin(); it!=enemyMoveLocs.end(); it++)
    {
        mLoc = *it;

        for(int d=0; d<(int)grid[mLoc].possibleNeighbours.size(); d++)
        {
            bLoc = grid[mLoc].possibleNeighbours[d];

            if(grid[bLoc].ant == 0)
                battleAnts.insert(bLoc);
        }

        for(int d=0; d<(int)grid.attackTranslations.size(); d++)
        {
            bLoc = grid.getTranslateLocation(mLoc, grid.attackTranslations[d]);

            if(friendMoveLocs.count(bLoc))
                battleLocations.insert(bLoc);
        }

        grid[mLoc].possibleBattleFriends = min(battleLocations.size(), battleAnts.size());
        grid[mLoc].possibleBattleFriendLocations = battleLocations.size();

        battleAnts.clear();
        battleLocations.clear();
    }
};

void State::updateAdjToEnemyInfo()
{
    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
    {
        grid[*ant].adjToEnemy = 1;
        for(int d=0; d<(int)grid[*ant].landNeighbours.size(); d++)
            grid[grid[*ant].landNeighbours[d] ].adjToEnemy = 1;
    }
};

void State::calculateSoonestBattleInfo()
{
    int maxPath = 15;

    //works out where ants could reside next turn
    Location cLoc, nLoc, tLoc;
    queue<pair<Location, int> > searchQueue;
    pair<Location, int> searchItem;
    Matrix<uint8_t> searched(rows, cols, 0);

    //adds locations within the attack radius of each ant to the search queue
    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
    {
        cLoc = *ant;
        grid[cLoc].soonestEnemyBattle = 0;
        searchQueue.push(pair<Location, int>(cLoc, 0));

        searched[cLoc] = 1;

        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            nLoc = grid.getTranslateLocation(cLoc, grid.attackTranslations[t]);
            if(!grid[nLoc].isWater)
                grid[nLoc].soonestEnemyBattle = 0;
        }
    }

    while(!searchQueue.empty())
    {
        searchItem = searchQueue.front();
        searchQueue.pop();

        cLoc = searchItem.first;

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(!searched[nLoc])
            {
                searched[nLoc] = 1;

                if(searchItem.second + 1 < maxPath)
                    searchQueue.push(pair<Location, int>(nLoc, searchItem.second+1));

                for(int t=0; t<(int)grid.attackBorderTranslations.size(); t++)
                {
                    tLoc = grid.getTranslateLocation(nLoc, grid.attackBorderTranslations[t]);

                    if(!grid[tLoc].isWater && grid[tLoc].soonestEnemyBattle == 9999)
                        grid[tLoc].soonestEnemyBattle = searchItem.second + 1;
                }
            }
        }
    }

    searched.reset(0);

    //adds locations within the attack radius of each ant to the search queue
    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
    {
        cLoc = *ant;
        grid[cLoc].soonestFriendBattle = 0;
        searchQueue.push(pair<Location, int>(cLoc, 0));

        searched[cLoc] = 1;

        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            nLoc = grid.getTranslateLocation(cLoc, grid.attackTranslations[t]);
            if(!grid[nLoc].isWater)
                grid[nLoc].soonestFriendBattle = 0;
        }
    }

    while(!searchQueue.empty())
    {
        searchItem = searchQueue.front();
        searchQueue.pop();

        cLoc = searchItem.first;

        for(int d=0; d<(int)grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = grid[cLoc].landNeighbours[d];

            if(!searched[nLoc])
            {
                searched[nLoc] = 1;

                if(searchItem.second + 1 < maxPath)
                    searchQueue.push(pair<Location, int>(nLoc, searchItem.second+1));

                for(int t=0; t<(int)grid.attackBorderTranslations.size(); t++)
                {
                    tLoc = grid.getTranslateLocation(nLoc, grid.attackBorderTranslations[t]);

                    if(!grid[tLoc].isWater && grid[tLoc].soonestFriendBattle == 9999)
                        grid[tLoc].soonestFriendBattle = searchItem.second + 1;
                }
            }
        }
    }
};

void State::updateAdjUnmovedAntInfo()
{
    adjUnmovedFriends.reset(0);
    adjUnmovedEnemies.reset(0);

    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
    {
        adjUnmovedFriends[*ant]++;
        for(int d=0; d<(int)grid[*ant].landNeighbours.size(); d++)
            adjUnmovedFriends[grid[*ant].landNeighbours[d] ]++;
    }

    for(auto ant=enemyAnts.begin(); ant!=enemyAnts.end(); ant++)
    {
        adjUnmovedEnemies[*ant]++;
        for(int d=0; d<(int)grid[*ant].landNeighbours.size(); d++)
            adjUnmovedEnemies[grid[*ant].landNeighbours[d] ]++;
    }
};

Outcome State::evaluate(std::vector<Move> &ourMoves, std::vector<Move> &theirMoves)
{
    Outcome outcome;
    Location cLoc;
    Matrix<uint8_t> playerInfo(rows, cols, 255);
    unordered_set<Location, LocationIndex> antLocations;

    //makes our moves
    for(int m=0; m<(int)ourMoves.size(); m++)
    {
        cLoc = ourMoves[m].mLoc;
        playerInfo[cLoc] = 0;
        antLocations.insert(cLoc);
    }

    //makes enemies moves, and erases collisions
    for(int m=0; m<(int)theirMoves.size(); m++)
    {
        cLoc = theirMoves[m].mLoc;

        if(playerInfo[cLoc] == 0) //collision
        {
            playerInfo[cLoc] = 255;
            antLocations.erase(cLoc);
            outcome.friendDeaths++;
            outcome.enemyDeaths++;
        }
        else
        {
            playerInfo[cLoc] = grid[theirMoves[m].aLoc].ant;
            antLocations.insert(cLoc);
        }
    }

    //finds enemies for each ant
    vector<vector<Location> > enemies(antLocations.size(), vector<Location>());
    map<Location, int> enemyCounts;
    Location tLoc;
    int a=0;
    for(auto ant=antLocations.begin(); ant!=antLocations.end(); ant++, a++)
    {
        for(int t=0; t<(int)grid.attackTranslations.size(); t++)
        {
            tLoc = grid.getTranslateLocation(*ant, grid.attackTranslations[t]);

            if(playerInfo[tLoc] < 200 && playerInfo[*ant] != playerInfo[tLoc])
                enemies[a].push_back(tLoc);
        }

        enemyCounts[*ant] = enemies[a].size();
    }

    //checks for dead ants and razed hills
    a = 0;
    bool antIsDead;
    unordered_set<Location, LocationIndex> cFood = food;
    for(auto ant=antLocations.begin(); ant!=antLocations.end(); ant++, a++)
    {
        for(int e=0; e<(int)enemies[a].size(); e++)
        {
            if(enemyCounts[*ant] >= enemyCounts[enemies[a][e] ]) //ant had died
            {
                antIsDead = 1;
                if(playerInfo[*ant] == 0)
                    outcome.friendDeaths++;
                else
                    outcome.enemyDeaths++;
                break;
            }
        }

        if(!antIsDead) //checks for razed hills and collected food
        {
            if(playerInfo[*ant] > 0)
            {
                if(grid[*ant].hill == 0) //an enemy razed one of our hills
                {
                    outcome.enemyRazedFriendHill = 1;
                }
                else if(grid[*ant].hill > 0 && grid[*ant].hill != playerInfo[*ant]) //an enemy razed an enemy hill
                {
                    outcome.enemyRazedEnemyHill = 1;
                }
            }
            else if(grid[*ant].hill > 0) //we razed an enemy hill
            {
                outcome.friendRazedEnemyHill = 1;
            }

            for(int d=0; d<(int)grid[*ant].landNeighbours.size(); d++)
            {
                cLoc = grid[*ant].landNeighbours[d];
                if(grid[cLoc].isFood && cFood.count(cLoc))
                {
                    cFood.erase(cLoc);
                    if(playerInfo[*ant] == 0)
                        outcome.friendDeaths--;
                    else
                        outcome.enemyDeaths--;
                }
            }
        }
    }

    if(ourNoAnts > (int)enemyAnts.size() && ourMoves.size() > 2*theirMoves.size())
        outcome.defensive = 0;
    else
        outcome.defensive = 1;

    /*if(ourNoAnts < max(10, 2*int(enemyAnts.size()))) //be defensive
        return -friendDeaths;
    else if(ourMoves.size() > theirMoves.size()) //we have more battle ants than them
        return friendDeaths;*/


    return outcome;
};

//output function
ostream& operator<<(ostream &os, const State &state)
{
    for(int row=0; row<state.rows; row++)
    {
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].isWater)
                os << '%';
            else if(state.grid[row][col].hill >= 0)
                os << (char)('A' + state.grid[row][col].hill);
            else if(state.grid[row][col].ant >= 0)
                os << (char)('a' + state.grid[row][col].ant);
            else if(state.grid[row][col].available > 0)
                os << state.grid[row][col].available;
            else if(state.grid[row][col].isFood)
                os << '*';
            else if(state.grid[row][col].lastSeen == state.turn)
                os << '.';
            else if(state.grid[row][col].lastSeen >= 0)
                os << '~';
            else
                os << '?';
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    bool gameOver = 0;
    int row, col, player;
    string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            gameOver = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.turn;

            //starts the timer for the turn
            state.timer.start();
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if(state.turn == 0)
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.loadtime;
            else if(inputType == "turntime")
            {
                is >> state.turntime;
                state.turntime = min(500.0, state.turntime); //bloody tcp!
            }
            else if(inputType == "rows")
                is >> state.rows;
            else if(inputType == "cols")
                is >> state.cols;
            else if(inputType == "turns")
                is >> state.turns;
            else if(inputType == "viewradius2")
                is >> state.viewradius;
            else if(inputType == "attackradius2")
                is >> state.attackradius;
            else if(inputType == "spawnradius2")
                is >> state.harvestradius;
            else if(inputType == "ready") //end of parameter input
                break;
            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {

            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.grid.addWaterInformation(Location(row, col));
            }
            else if(inputType == "f") //food square
            {
                is >> row >> col;
                state.grid[row][col].isFood = 1;
                state.grid[row][col].taken = 1;
                state.food.insert(Location(row, col));
                state.uncollectedFood.insert(Location(row, col));
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].ant = player;
                if(player == 0)
                {
                    state.grid[row][col].available = 0;
                    state.moveableAnts.insert(Location(row, col));
                    state.unmovedAnts.insert(Location(row, col));
                }
                else
                {
                    state.enemyAnts.insert(Location(row, col));
                    state.uncollectedEnemies.insert(Location(row, col));
                }
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                //state.grid[row][col].deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;

                state.grid[row][col].hill = player;

                if(player == 0)
                    state.myHills.insert(Location(row, col));
                else
                    state.enemyHills.insert(Location(row, col));
            }
            else if(inputType == "go") //end of turn input
            {
                state.ourNoAnts = state.unmovedAnts.size();
                if(gameOver)
                    is.setstate(std::ios::failbit);
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }

    return is;
};
