#include "Bot.h"

using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    state.setup();

    //continues making moves while the game is not over
    while(cin >> state)
    {

        state.grid.updateBattleNeighbours();

        state.grid.updatePossibleNeighbours();

        state.updateVisionInformation();

        state.processOldInformation();

        state.updateClosestHillInformation();

        state.updateClosestFriendAndEnemyInfo();

        state.calculatePossibleBattleInfo();

        state.calculateSoonestBattleInfo();

        state.updatePossibleEnemyBorder();

        state.updateNotVisibleBorder();

        state.updateEnemyHillBorder();

        state.updateAdjUnmovedAntInfo();

        state.updateAdjToEnemyInfo();

        state.updateAdjFoodInformation();

        makeMoves();

        state.updatePossibleEnemyInfo();

        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    battleAnts(0.35);

    saveHills(0.38);

    collectFood(0.5);

    invadeEnemyHills(0.52);

    exploreTheMap(0.6);

    protectFriendHills(0.63);

    state.updateBattleBorder();

    collectBattleBorder(0.73);

    collectPossibleEnemiesBorder(0.75);

    collectNotRecentlySeenBorder(0.78);

    exploreRemainingAnts(0.81);
};

void Bot::performBattle(const vector<Location> &ourAnts, const vector<Location> &theirAnts)
{
    vector<vector<Move> > ourMoves, theirMoves;

    ourMoves.push_back(state.getOurSpecialOffensiveMoves(ourAnts));
    ourMoves.push_back(state.getOurOffensiveMovesMaximisingBattleEnemies(ourAnts));
    ourMoves.push_back(state.getOurOffensiveMovesMinimisingPositiveBattleEnemies(ourAnts));
    ourMoves.push_back(state.getOurNeutralMovesMaximisingBattleEnemies(ourAnts));
    ourMoves.push_back(state.getOurNeutralMovesMinimisingPositiveBattleEnemies(ourAnts));
    ourMoves.push_back(state.getOurDefensiveMovesMinimisingBattleEnemies(ourAnts));

    Outcome bestWorstOutcome, currentOutcome;
    bestWorstOutcome.setWorst();
    int bestMove = 0;

    //picks our moves using minimax
    for(int f=0; f<(int)ourMoves.size(); f++)
    {
        currentOutcome.setBest();

        theirMoves.clear();
        theirMoves.push_back(state.getTheirOffensiveMovesMinimisingPositiveBattleEnemies(theirAnts, ourMoves[f]));
        theirMoves.push_back(state.getTheirOffensiveMovesMaximisingBattleEnemies(theirAnts, ourMoves[f]));
        theirMoves.push_back(state.getTheirNeutralMovesMinimisingPositiveBattleEnemies(theirAnts, ourMoves[f]));
        theirMoves.push_back(state.getTheirNeutralMovesMaximisingBattleEnemies(theirAnts, ourMoves[f]));

        for(int e=0; e<(int)theirMoves.size() && bestWorstOutcome < currentOutcome; e++)
        {
            currentOutcome = min(currentOutcome, state.evaluate(ourMoves[f], theirMoves[e]));
        }

        if(bestWorstOutcome < currentOutcome)
        {
            bestWorstOutcome = currentOutcome;
            bestMove = f;
        }
    }

    state.makeMoves(ourMoves[bestMove]);
};

void Bot::battleAnts(double maxTimeProportion)
{
    Location aLoc, tLoc;
    queue<Location> searchQueue;
    vector<Location> ourAnts, theirAnts;
    Matrix<uint8_t> searched(state.rows, state.cols, 0);
    unordered_set<Location, LocationIndex> unmovedAnts = state.unmovedAnts;


    for(auto ant=unmovedAnts.begin(); ant!=unmovedAnts.end(); ant++)
    {
        if(state.grid[*ant].ant != 0) //already battled ant
            continue;

        ourAnts.push_back(*ant);
        searched[*ant] = 1;
        searchQueue.push(*ant);

        while(!searchQueue.empty())
        {
            aLoc = searchQueue.front();
            searchQueue.pop();

            for(int t=0; t<(int)state.grid[aLoc].battleNeighbours.size(); t++)
            {
                tLoc = state.grid[aLoc].battleNeighbours[t];

                if(searched[tLoc])
                    continue;

                if(state.grid[aLoc].ant == 0 && state.grid[tLoc].ant > 0) //found an enemy
                {
                    searched[tLoc] = 1;
                    theirAnts.push_back(tLoc);
                    searchQueue.push(tLoc);
                }
                else if(state.grid[aLoc].ant > 0 && state.grid[tLoc].ant > -1 && state.grid[aLoc].ant != state.grid[tLoc].ant) //found another battle friend
                {
                    searched[tLoc] = 1;
                    searchQueue.push(tLoc);

                    if(state.grid[tLoc].ant == 0)
                        ourAnts.push_back(tLoc);
                    else if(state.grid[tLoc].ant > 0 && state.grid[tLoc].ant != state.grid[aLoc].ant)
                        theirAnts.push_back(tLoc);
                }
            }
        }

        if(theirAnts.size() > 0)
        {
            if(state.checkTime(maxTimeProportion))
                performBattle(ourAnts, theirAnts);
            else if(state.closestFriendHill[ourAnts[0] ] < state.closestEnemyHill[ourAnts[0] ])
                state.makeMoves(state.getOurDefensiveMovesMinimisingBattleEnemies(ourAnts));
            else
                state.makeMoves(state.getOurOffensiveMovesMaximisingBattleEnemies(ourAnts));
        }

        searched.reset(0);
        ourAnts.clear();
        theirAnts.clear();
    }
};


void Bot::collectEnemyAroundHill(const Location &eLoc, double maxTimeProportion)
{
    if(!state.checkTime(maxTimeProportion) || state.unmovedAnts.size() == 0)
        return;

    Location nLoc;

    Matrix<uint8_t> searched(state.rows, state.cols, 0);
    searched[eLoc] = 1;

    EnemySearchItem searchItem;
    priority_queue<EnemySearchItem, vector<EnemySearchItem>, greater<EnemySearchItem> > searchQueue;
    searchQueue.push(EnemySearchItem(eLoc, 0, state.closestFriendHill[eLoc]));

    while(state.checkTime(maxTimeProportion) && !searchQueue.empty())
    {
        searchItem = searchQueue.top();
        searchQueue.pop();

        for(int d=0; d<(int)state.grid[searchItem.cLoc].landNeighbours.size(); d++)
        {
            nLoc = state.grid[searchItem.cLoc].landNeighbours[d];

            if(searched[nLoc])
                continue;
            searched[nLoc] = 1;

            if(state.grid[nLoc].ant == 0 && state.grid[nLoc].available + state.closestFriendHill[nLoc] <= state.closestFriendHill[eLoc]) //move the ant
            {
                if(state.grid[nLoc].available == 0 && (state.grid[searchItem.cLoc].taken || state.grid[searchItem.cLoc].ant == 0))
                    state.makeMove(Move(nLoc, nLoc, 'X'));
                else
                    state.makeMove(Move(nLoc, searchItem.cLoc, state.grid[searchItem.cLoc].oppositeLandDirections[d]));

                while(!searchQueue.empty())
                    searchQueue.pop();
                break;
            }

            if(searchItem.cPathLength > 15) //don't search for too long!
                continue;

            if(state.closestFriendHill[nLoc] <= state.closestFriendHill[eLoc] && !state.grid[nLoc].taken) //can still make it in time
            {
                searchQueue.push(EnemySearchItem(nLoc, searchItem.cPathLength+1, state.closestFriendHill[nLoc]));
            }
        }
    }
};

void Bot::protectFriendHills(double maxTimeProportion)
{
    int maxProtectionPath = state.turn;
    Location cLoc, nLoc;
    queue<Location> searchQueue;
    Matrix<uint8_t> searched(state.rows, state.cols, 0);

    for(auto hill=state.myHills.begin(); hill!=state.myHills.end(); hill++)
    {
        searched[*hill] = 1;
        searchQueue.push(*hill);
    }

    while(state.checkTime(maxTimeProportion) && state.unmovedAnts.size() > 0 && !searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = state.grid[cLoc].landNeighbours[d];

            if(searched[nLoc])
                continue;
            searched[nLoc] = 1;

            if(state.closestFriendHill[nLoc] < maxProtectionPath)
                searchQueue.push(nLoc);

            if(state.grid[nLoc].ant > 0) //need to send an ant here
                collectEnemyAroundHill(nLoc, maxTimeProportion);
        }
    }
};

void Bot::saveHills(double maxTimeProportion)
{
    Location cLoc, nLoc, eLoc, fLoc;
    queue<Location> searchQueue, friendAnts, enemyAnts;
    Matrix<uint8_t> searched(state.rows, state.cols, 0);

    for(auto fHill=state.myHills.begin(); fHill!=state.myHills.end(); fHill++)
    {
        searchQueue.push(*fHill);
        searched[*fHill] = 1;

        //finds the friends and enemies
        while(!searchQueue.empty())
        {
            cLoc = searchQueue.front();
            searchQueue.pop();

            for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                if(searched[nLoc])
                    continue;
                searched[nLoc] = 1;

                if(state.closestFriendHill[nLoc] != state.closestFriendHill[cLoc]+1) //not on the way out from the hill
                    continue;

                if(state.grid[nLoc].ant == 0)
                    friendAnts.push(nLoc);
                else if(state.grid[nLoc].ant > 0)
                {
                    if(state.closestFriendHill[nLoc] < 10) //push them away
                        collectEnemyAroundHill(nLoc, maxTimeProportion);
                    else
                        enemyAnts.push(nLoc);
                }

                searchQueue.push(nLoc);
            }
        }

        while(!enemyAnts.empty())
        {
            eLoc = enemyAnts.front();
            enemyAnts.pop();

            while(!friendAnts.empty())
            {
                fLoc = friendAnts.front();
                friendAnts.pop();

                if(state.grid[fLoc].ant != 0) //already moved ant
                    continue;

                if(state.closestFriendHill[fLoc] == state.closestFriendHill[eLoc]) //need to try and move closer
                {
                    bool moveMade = 0;
                    for(int d=0; d<(int)state.grid[fLoc].landNeighbours.size(); d++)
                    {
                        nLoc = state.grid[fLoc].landNeighbours[d];

                        if(state.grid[nLoc].ant != 0 && !state.grid[nLoc].taken && state.closestFriendHill[nLoc] < state.closestFriendHill[fLoc])
                        {
                            moveMade = 1;
                            state.makeMove(Move(fLoc, nLoc, state.grid[fLoc].landDirections[d]));
                            break;
                        }
                    }

                    if(moveMade)
                        break;
                }
                else //current ant will do
                {
                    if(state.closestFriendHill[fLoc]+1 == state.closestFriendHill[eLoc]) //need to stay still
                        state.makeMove(Move(fLoc, fLoc, 'X'));
                    break;
                }
            }
        }

        searched.reset(0);
    }
};




void Bot::invadeEnemyHills(double maxTimeProportion)
{
    if(!state.checkTime(maxTimeProportion) || state.unmovedAnts.size() == 0)
        return;

    Location cLoc, nLoc;

    queue<InvadeSearchItem> searchQueue;
    InvadeSearchItem searchItem, nSearchItem;

    Matrix<uint8_t> searched(state.rows, state.cols, 255);

    for(auto eHill=state.enemyHills.begin(); eHill!=state.enemyHills.end(); eHill++)
    {
        searched[*eHill] = 0;
        if(state.grid[*eHill].lastSeen == state.turn && state.grid[*eHill].soonestEnemyBattle > 1)
            searchQueue.push(InvadeSearchItem(*eHill, 0, state.grid[*eHill].soonestEnemyBattle));
    }

    while(state.checkTime(maxTimeProportion) && !searchQueue.empty())
    {
        searchItem = searchQueue.front();
        searchQueue.pop();

        cLoc = searchItem.cLoc;

        for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = state.grid[cLoc].landNeighbours[d];

            if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken) //current location isn't taken
            {
                if(searched[nLoc] == 255 || searched[nLoc] >= searched[cLoc]) //current location is towards a target?
                {
                    if(state.grid[nLoc].ant == 0)
                    {
                        state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                        if(state.unmovedAnts.size() == 0)
                        {
                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }
            }

            if(searched[nLoc] != 255)
                continue;
            searched[nLoc] = 1;

            nSearchItem = InvadeSearchItem(nLoc, searchItem.cPathLength+1, min(searchItem.battleSlack-1, state.grid[nLoc].soonestEnemyBattle));

            if(nSearchItem.battleSlack <= 0)
                continue;

            if(nSearchItem.cPathLength < 20)
                searchQueue.push(nSearchItem);
        }
    }
};


void Bot::collectFood(double maxTimeProportion)
{
    if(!state.checkTime(maxTimeProportion) || state.unmovedAnts.size() == 0 || state.uncollectedFood.size() == 0)
        return;

    Location cLoc, nLoc, fLoc;

    for(auto ant=state.unmovedAnts.begin(); ant!=state.unmovedAnts.end(); ant++)
    {
        cLoc = *ant;

        for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = state.grid[cLoc].landNeighbours[d];

            if(state.grid[nLoc].isFood && state.uncollectedFood.count(nLoc)) //don't need to worry about this existing next turn
            {
                state.makeMove(Move(cLoc, cLoc, 'X', cLoc, 1, 1));

                state.collectFoodLocation(nLoc);
                state.updateAdjFoodInformation(nLoc);
            }
        }
    }

    Matrix<uint8_t> searched(state.rows, state.cols, 0);
    unordered_set<Location, LocationIndex> uncollectedFood;
    priority_queue<FoodSearchItem, vector<FoodSearchItem>, greater<FoodSearchItem> > orderedSearchQueue;
    queue<FoodSearchItem> searchQueue;
    Move bestMove;
    int nBattleSlack, bBattleSlack;
    FoodSearchItem searchItem;
    bool movedAnt = 1;

    for(auto food=state.uncollectedFood.begin(); food!=state.uncollectedFood.end(); food++)
        if(state.closestFriendHill[*food] < 9999)
            uncollectedFood.insert(*food);

    while(movedAnt && state.checkTime(maxTimeProportion) && state.unmovedAnts.size() > 0 && uncollectedFood.size() > 0)
    {
        movedAnt = 0;
        bestMove.pathLength = 9999;
        bBattleSlack = 9999;

        for(auto foodLoc=uncollectedFood.begin(); foodLoc!=uncollectedFood.end(); foodLoc++)
        {
            searched[*foodLoc] = 1;

            for(int d=0; d<(int)state.grid[*foodLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[*foodLoc].landNeighbours[d];

                if(!searched[nLoc] && !state.grid[nLoc].isFood) //don't need to search over other uncollected food
                {
                    searched[nLoc] = 1;
                    orderedSearchQueue.push(FoodSearchItem(nLoc, state.grid[nLoc].adjFood, nLoc, 0, state.grid[nLoc].soonestEnemyBattle));
                }
            }
        }

        while(!orderedSearchQueue.empty()) //changes over to a normal queue
        {
            searchQueue.push(orderedSearchQueue.top());
            orderedSearchQueue.pop();
        }

        while(!searchQueue.empty())
        {
            searchItem = searchQueue.front();
            searchQueue.pop();
            cLoc = searchItem.cLoc;

            if(bestMove.pathLength <= searchItem.cPathLength+1) //can just make the best move
            {
                while(!searchQueue.empty())
                    searchQueue.pop();
                break;
            }

            for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                if(!searched[nLoc])
                {
                    searched[nLoc] = 1;

                    if(state.grid[nLoc].lastSeen == -1 || searchItem.cPathLength > 30)
                        continue;

                    nBattleSlack = min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1);

                    if(state.grid[nLoc].ant == 0) //found an ant to move
                    {
                        if(state.grid[cLoc].taken)
                            bestMove = Move(nLoc, nLoc, 'X', searchItem.tLoc, searchItem.cPathLength+2, 1);
                        else
                            bestMove = Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d], searchItem.tLoc, searchItem.cPathLength+1, 1);
                        bBattleSlack = nBattleSlack;


                        while(!searchQueue.empty())
                            searchQueue.pop();
                        break;
                    }
                    else if(state.grid[nLoc].available > 0)
                    {
                        //if(state.grid[nLoc].available+searchItem.cPathLength < bestMove.pathLength) //best move found so far
                        {
                            bestMove = Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d], searchItem.tLoc, searchItem.cPathLength+state.grid[nLoc].available+1, 1);
                            bBattleSlack = nBattleSlack;
                        }
                    }
                    else
                        searchQueue.push(FoodSearchItem(searchItem.tLoc, searchItem.noFoodSquares, nLoc, searchItem.cPathLength+1, nBattleSlack));
                }
            }
        }

        if(bestMove.pathLength < 9999) //found a better move to make
        {
            movedAnt = 1;
            state.makeMove(bestMove);

            for(int f=0; f<(int)state.grid[bestMove.cLoc].landNeighbours.size(); f++)
            {
                fLoc = state.grid[bestMove.cLoc].landNeighbours[f];
                if(state.grid[fLoc].isFood && uncollectedFood.count(fLoc))
                {
                    uncollectedFood.erase(fLoc);
                    state.updateAdjFoodInformation(fLoc);
                    if(bBattleSlack > 0)
                        state.collectFoodLocation(fLoc);
                }
            }
        }

        searched.reset();
    }
};



void Bot::exploreTheMap(double maxTimeProportion)
{
    if(!state.checkTime(maxTimeProportion) || state.unseenBorder.size() == 0 || state.unmovedAnts.size() == 0)
        return;

    int maxPath = 20;
    Location cLoc, nLoc;
    queue<ExploreSearchItem> searchQueue;
    ExploreSearchItem searchItem;
    unordered_set<Location, LocationIndex> unseenBorder = state.unseenBorder;
    Matrix<uint8_t> searched(state.rows, state.cols, 255);
    bool moveMade = 1;

    while(moveMade && state.checkTime(maxTimeProportion) && unseenBorder.size() > 0 && state.unmovedAnts.size() > 0)
    {
        moveMade = 0;

        for(auto uLoc=unseenBorder.begin(); uLoc!=unseenBorder.end(); uLoc++)
        {
            searched[*uLoc] = 0;
            searchQueue.push(ExploreSearchItem(*uLoc, 0, *uLoc, state.grid[*uLoc].soonestEnemyBattle));
        }

        while(!searchQueue.empty())
        {
            searchItem = searchQueue.front();
            cLoc = searchItem.cLoc;
            searchQueue.pop();

            int noDirections = state.grid[cLoc].landNeighbours.size();
            for(int d=rand()%noDirections, r=0; r<noDirections; r++, d = (d+1)%noDirections)
            //for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken)
                {
                    if(searched[nLoc] == 255 || searched[nLoc] >= searched[cLoc])
                    {
                        if(state.grid[nLoc].ant == 0) //found an ant to move
                        {
                            moveMade = 1;

                            state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }

                if(searched[nLoc] != 255)
                    continue;
                searched[nLoc] = searched[cLoc]+1;

                if(state.grid[nLoc].soonestEnemyBattle < 2)
                    continue;

                if(state.grid[nLoc].lastSeen == -1 || searchItem.cPathLength > maxPath)
                    continue;

                //if(min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1) > 0)
                    searchQueue.push(ExploreSearchItem(nLoc, searchItem.cPathLength+1, searchItem.uLoc, min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1)));
            }
        }

        unseenBorder.erase(searchItem.uLoc);
        for(int t=0; t<(int)state.grid.attackTranslations.size(); t++)
        {
            nLoc = state.grid.getTranslateLocation(searchItem.uLoc, state.grid.attackTranslations[t]);

            if(state.grid[nLoc].lastSeen == -1)
                unseenBorder.erase(nLoc);
        }

        searched.reset(255);
    }
};


void Bot::collectPossibleEnemiesBorder(double maxTimeProportion)
{
    int maxPath = 20;
    Location cLoc, nLoc;
    Matrix<uint8_t> searched(state.rows, state.cols, 255);

    unordered_set<Location, LocationIndex> targetBorder = state.possibleEnemyBorder;

    queue<ExploreSearchItem> searchQueue;
    ExploreSearchItem searchItem;

    bool moveMade = 1;
    while(moveMade && state.checkTime(maxTimeProportion) && targetBorder.size() > 0 && state.unmovedAnts.size() > 0)
    {
        moveMade = 0;

        for(auto uLoc=targetBorder.begin(); uLoc!=targetBorder.end(); uLoc++)
        {
            searched[*uLoc] = 0;
            searchQueue.push(ExploreSearchItem(*uLoc, 0, *uLoc, state.grid[*uLoc].soonestEnemyBattle));
        }

        while(!searchQueue.empty())
        {
            searchItem = searchQueue.front();
            cLoc = searchItem.cLoc;
            searchQueue.pop();

            for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken)
                {
                    if(searched[nLoc] == 255 || searched[nLoc] >= searched[cLoc])
                    {
                        if(state.grid[nLoc].ant == 0) //found an ant to move
                        {
                            moveMade = 1;

                            state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }

                if(searched[nLoc] != 255)
                    continue;
                searched[nLoc] = searched[cLoc] + 11;

                if(searched[nLoc] > maxPath)
                    continue;

                if(min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1) > 0)
                    searchQueue.push(ExploreSearchItem(nLoc, searchItem.cPathLength+1, searchItem.uLoc, min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1)));
            }
        }

        for(int t=0; t<(int)state.grid.viewTranslations.size(); t++)
        {
            nLoc = state.grid.getTranslateLocation(searchItem.uLoc, state.grid.viewTranslations[t]);

            targetBorder.erase(nLoc);
        }

        searched.reset(255);
    }
};

void Bot::collectBattleBorder(double maxTimeProportion)
{
    int maxPath = 20;
    Location cLoc, nLoc;
    Matrix<uint8_t> searched(state.rows, state.cols, 255);

    priority_queue<BorderSearchItem, vector<BorderSearchItem>, greater<BorderSearchItem> > searchQueue;
    queue<Location> eSearchQueue;
    BorderSearchItem searchItem;

    for(auto bLoc = state.battleBorder.begin(); bLoc!=state.battleBorder.end();)
    {
        if(state.grid[*bLoc].ant == 0)
        {
            state.makeMove(Move(*bLoc, *bLoc, 'X'));
            bLoc = state.battleBorder.erase(bLoc);
        }
        else
            bLoc++;
    }

    bool moveMade = 1;
    while(moveMade && state.checkTime(maxTimeProportion) && state.battleBorder.size() > 0 && state.unmovedAnts.size() > 0)
    {
        moveMade = 0;

        for(auto uLoc=state.battleBorder.begin(); uLoc!=state.battleBorder.end(); uLoc++)
        {
            searched[*uLoc] = 0;
            searchQueue.push(BorderSearchItem(*uLoc, 0, 0, *uLoc));
        }

        while(!searchQueue.empty())
        {
            searchItem = searchQueue.top();
            cLoc = searchItem.cLoc;
            searchQueue.pop();

            //int noDirections = state.grid[cLoc].landNeighbours.size();
            //for(int d=rand()%noDirections, r=0; r<noDirections; r++, d = (d+1)%noDirections)
            for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                //if(state.grid[nLoc].soonestEnemyBattle < state.grid[cLoc].soonestEnemyBattle)
                  //  continue;

                if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken) //current location isn't taken
                {
                    if(searched[nLoc] == 255 || searched[nLoc] >= searched[cLoc]) //current location is towards a target?
                    {
                        if(state.grid[nLoc].ant == 0)
                        {
                            moveMade = 1;

                            state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }

                if(searched[nLoc] != 255)
                    continue;
                searched[nLoc] = searched[cLoc]+1;

                if(searchItem.cPathLength < maxPath)
                {
                    searchQueue.push(BorderSearchItem(nLoc, searchItem.cPathLength+1, 0, searchItem.tLoc));
                }
            }
        }

        state.battleBorder.erase(searchItem.tLoc);

        searched.reset(255);
    }
};


void Bot::collectNotRecentlySeenBorder(double maxTimeProportion)
{
    int maxPath = 20;
    Location cLoc, nLoc;
    Matrix<uint8_t> searched(state.rows, state.cols, 255);

    unordered_set<Location, LocationIndex> targetBorder = state.notVisibleBorder;

    queue<ExploreSearchItem> searchQueue;
    queue<Location> eSearchQueue;
    ExploreSearchItem searchItem;

    bool moveMade = 1;
    while(moveMade && state.checkTime(maxTimeProportion) && targetBorder.size() > 0 && state.unmovedAnts.size() > 0)
    {
        moveMade = 0;

        for(auto uLoc=targetBorder.begin(); uLoc!=targetBorder.end(); uLoc++)
        {
            searched[*uLoc] = 1;
            searchQueue.push(ExploreSearchItem(*uLoc, 0, *uLoc, state.grid[*uLoc].soonestEnemyBattle));
        }

        while(!searchQueue.empty())
        {
            searchItem = searchQueue.front();
            cLoc = searchItem.cLoc;
            searchQueue.pop();

            for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
            {
                nLoc = state.grid[cLoc].landNeighbours[d];

                if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken) //current location isn't taken
                {
                    if(searched[nLoc] == 255 || searched[nLoc] >= searched[cLoc]+1) //current location is towards a target?
                    {
                        if(state.grid[nLoc].ant == 0)
                        {
                            moveMade = 1;

                            state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }

                if(searched[nLoc] != 255)
                    continue;
                searched[nLoc] = searched[cLoc] + 1;

                if(state.grid[nLoc].lastSeen == -1 || searchItem.cPathLength > maxPath)
                    continue;

                if(min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1) > 0)
                    searchQueue.push(ExploreSearchItem(nLoc, searchItem.cPathLength+1, searchItem.uLoc, min(state.grid[nLoc].soonestEnemyBattle, searchItem.battleSlack-1)));
            }
        }

        for(int t=0; t<(int)state.grid.viewTranslations.size(); t++)
        {
            nLoc = state.grid.getTranslateLocation(searchItem.uLoc, state.grid.viewTranslations[t]);

            targetBorder.erase(nLoc);
        }

        searched.reset(255);
    }
};





void Bot::exploreRemainingAnts(double maxTimeProportion)
{
    if(!state.checkTime(maxTimeProportion) || state.unmovedAnts.size() == 0)
        return;

    Location cLoc, nLoc;

    queue<Location> searchQueue;

    Matrix<int> searched(state.rows, state.cols, -1);

    for(auto eHill=state.enemyHills.begin(); eHill!=state.enemyHills.end(); eHill++)
    {
        searched[*eHill] = 0;
        searchQueue.push(*eHill);
    }

    for(auto ant=state.battleBorder.begin(); ant!=state.battleBorder.end(); ant++)
    {
        searched[*ant] = 0;
            searchQueue.push(*ant);
    }

    /*for(auto ant=state.enemyAnts.begin(); ant!=state.enemyAnts.end(); ant++)
    {
        searched[*ant] = 1;
        searchQueue.push(*ant);
    }*/

    for(auto uLoc=state.unseenBorder.begin(); uLoc!=state.unseenBorder.end(); uLoc++)
    {
        searched[*uLoc] = 0;
        searchQueue.push(*uLoc);
    }

    if(state.unmovedAnts.size() > 10) //try and get to possible enemies
    {
        for(auto pLoc=state.possibleEnemyBorder.begin(); pLoc!=state.possibleEnemyBorder.end(); pLoc++)
        {
            searched[*pLoc] = 0;
            searchQueue.push(*pLoc);
        }
    }

    while(state.checkTime(maxTimeProportion) && !searchQueue.empty())
    {
        cLoc = searchQueue.front();
        searchQueue.pop();

        int noDirections = state.grid[cLoc].landNeighbours.size();
        for(int d=rand()%noDirections, r=0; r<noDirections; r++, d = (d+1)%noDirections)
        //for(int d=0; d<(int)state.grid[cLoc].landNeighbours.size(); d++)
        {
            nLoc = state.grid[cLoc].landNeighbours[d];

            if(state.grid[cLoc].ant != 0 && !state.grid[cLoc].taken) //current location isn't taken
            {
                if(searched[nLoc] == -1 || searched[nLoc] >= searched[cLoc]) //current location is towards a target?
                {
                    if(state.grid[nLoc].ant == 0)
                    {
                        state.makeMove(Move(nLoc, cLoc, state.grid[cLoc].oppositeLandDirections[d]));

                        if(state.unmovedAnts.size() == 0)
                        {
                            while(!searchQueue.empty())
                                searchQueue.pop();
                            break;
                        }
                    }
                }
            }

            if(searched[nLoc] != -1)
                continue;
            searched[nLoc] = searched[cLoc]+1;

            if(state.grid[nLoc].soonestEnemyBattle < 1)
                continue;

            if(state.grid[nLoc].soonestEnemyBattle <4 && state.grid[nLoc].taken)
                continue;

            searchQueue.push(nLoc);
        }
    }
};



//finishes the turn
void Bot::endTurn()
{
    state.reset();
    cout << "go" << endl;
};
