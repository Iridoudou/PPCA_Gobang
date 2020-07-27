#include "AIController.h"
#include <utility>
#include <cstring>
#include <vector>
#include <queue>
#include <functional>
#include <random>
#include <time.h>
const int DEPTH_LIMIT = 3;
const int SEARCH_DEPTH = 4;
const int NEIGHBOUR_SEARCH_WIDTH = 1;

const int AI_WIN_5 = 100000000;
const int AI_ALIVE_4 = 10000;
const int AI_ALIVE_3 = 1000;
const int AI_ALIVE_2 = 100;
const int AI_ALIVE_1 = 10;
const int AI_DIE_4 = 1000;
const int AI_DIE_3 = 100;
const int AI_DIE_2 = 10;

const int HUMAN_WIN_5 = 1000000000;
const int HUMAN_ALIVE_4 = 10000;
const int HUMAN_ALIVE_3 = 1000;
const int HUMAN_ALIVE_2 = 100;
const int HUMAN_ALIVE_1 = 10;
const int HUMAN_DIE_4 = 1000;
const int HUMAN_DIE_3 = 100;
const int HUMAN_DIE_2 = 10;

const int NEGIFF = -2147483648;
const int IFF = 2147483647;

extern int ai_side; //0: black, 1: white
std::string ai_name = "your_ai_name_here";

int turn;
int board[15][15];
bool marked[5][15][15] = {0};
int pieceNum = 0; //棋子个数
struct direction  //检查方向
{
    int dx;
    int dy;
    void operator=(direction &other)
    {
        dx = other.dx;
        dy = other.dy;
    }
};
struct situation //棋子类型统计
{
    int win_5 = 0,
        alive_4 = 0, d_alive_4 = 0, die_4 = 0,
        alive_3 = 0, d_alive_3 = 0, die_3 = 0,
        alive_2 = 0, d_alive_2 = 0, die_2 = 0,
        alive_1 = 0, d_alive_1 = 0, die_1 = 0;
    situation operator+=(situation &other)
    {
        win_5 += other.win_5;
        alive_4 += other.alive_4;
        d_alive_4 += other.d_alive_4;
        die_4 += other.die_4;
        alive_3 += other.alive_3;
        d_alive_3 += other.d_alive_3;
        die_3 += other.die_3;
        alive_2 += other.alive_2;
        d_alive_2 += other.d_alive_2;
        die_2 += other.die_2;
        alive_1 += other.alive_1;
        d_alive_1 += other.d_alive_1;
        die_1 += other.die_1;
        return *this;
    }
};
direction dir(int i);
struct value
{
    int aiValue;
    int humanValue;
};
struct waitPoint //待选棋子
{
    value val;
    std::pair<int, int> cur;
    waitPoint(value v, std::pair<int, int> c) : val(v), cur(c) {}
};
struct cmp //比较
{
    bool operator()(waitPoint &cur1, waitPoint &cur2)
    {
        return cur1.val.aiValue < cur2.val.aiValue;
    }
};
/*
struct node
{
    int val;
    int alpha, beta;
    node *child[225];
};

int minimax(node *pos, int depth, int alpha, int beta, bool is_maximizing_player)
{
    if (depth == 0)
        return;
    
}
*/
bool isEmpty(int x, int y);                                                       //判断（x，y）是否为空
bool isFull();                                                                    //判断棋盘是否已满
bool isInBoard(int x, int y);                                                     //判断位置合法性
void place(int x, int y, int side);                                               //落子函数
void unplace(int x, int y);                                                       //取子函数
std::pair<int, int> nextPoint(std::pair<int, int> cur, int directionNo, int len); //计算邻接位置坐标
void getLinkPiece(std::pair<int, int> cur, int directionNo,
                  int side, int &len, int pace, std::pair<int, int> &linkPoint); //寻找邻接点
void getBoundary(int *left, int *right, std::pair<int, int> l, std::pair<int, int> r,
                 int directionNo, int side);                                        //获取两边四个位置以内所有信息
bool judge(std::pair<int, int> cur, int side);                                      //单点检测是否连成五子
situation situationAnalysis(int len, int side, int *left, int *right);              //分析单点情形
int singleEvaluation(std::pair<int, int> cur, int side);                            //单点估值
value wholeEvaluation();                                                            //全局估值
bool hasNeighbour(std::pair<int, int> cur, int side);                               //两步以内存在邻居
std::vector<std::pair<int, int>> inspiredSearch(int side);                          //启发式搜索
value findAiMove(std::pair<int, int> &bestMove, int depth, int alpha, int beta);    //对策分析
value findHumanMove(std::pair<int, int> &bestMove, int depth, int alpha, int beta); //对策分析
void intelligentRandom(std::pair<int, int> &bestMove, value &val,
                       std::priority_queue<waitPoint, std::vector<waitPoint>, cmp> &q); //智能化随机函数
value minimax(std::pair<int, int> &bestMove, int depth, int alpha, int beta, int side); //minimax算法
void flip();                                                                            //换手

//init function is called once at the beginning
void init()
{
    /* TODO: Replace this by your code */
    turn = 0;
    memset(board, -1, sizeof(board));
}

// loc is the action of your opponent
// Initially, loc being (-1,-1) means it's your first move
// If this is the third step(with 2 black ), where you can use the swap rule, your output could be either (-1, -1) to indicate that you choose a swap, or a coordinate (x,y) as normal.

std::pair<int, int> action(std::pair<int, int> loc)
{
    if (loc.first == -1 && loc.second == -1 && pieceNum == 0)
    {
        place(7, 7, ai_side);
        ++turn;
        return std::make_pair(7, 7);
    }
    else if (loc.first == -1 && loc.second == -1) //换手
    {
        ++turn;
        for (register int i = 0; i < 15; ++i)
            for (register int j = 0; j < 15; ++j)
                if (board[i][j] != -1)
                    board[i][j] = 1 - board[i][j];
        goto find;
    }
    place(loc.first, loc.second, 1 - ai_side);
    ++turn;
find:
    if (pieceNum == 1)
    {
        place(0, 0, ai_side);
        ++turn;
        return std::make_pair(0, 0);
    }
    if (turn != 3)
    {
        std::pair<int, int> bestMove;
        minimax(bestMove, SEARCH_DEPTH, NEGIFF, IFF, ai_side);
        place(bestMove.first, bestMove.second, ai_side);
        ++turn;
        return bestMove;
    }
    else
    {
        std::pair<int, int> bestMove;
        flip();
        value val1 = wholeEvaluation();
        flip();
        value val2 = wholeEvaluation();
        if (val1.aiValue > val2.aiValue) //换手
        {
            ++turn;
            flip();
            return std::make_pair(-1, -1);
        }
        else
        {
            place(bestMove.first, bestMove.second, ai_side);
            ++turn;
            return bestMove;
        }
    }
}

bool isEmpty(int x, int y)
{
    return board[x][y] == -1;
}
void place(int x, int y, int side)
{
    board[x][y] = side;
    ++pieceNum;
}
void unplace(int x, int y)
{
    board[x][y] = -1;
    --pieceNum;
}
bool isFull()
{
    return pieceNum == 225;
}
bool isInBoard(int x, int y)
{
    if (x < 0 || y < 0 || x >= 15 || y >= 15)
        return false;
    return true;
}
direction dir(int i)
{
    direction dir1 = {1, 0};
    direction dir2 = {0, 1};
    direction dir3 = {1, 1};
    direction dir4 = {1, -1};
    switch (i)
    {
    case 1:
        return dir1;
        break;
    case 2:
        return dir2;
        break;
    case 3:
        return dir3;
        break;
    case 4:
        return dir4;
        break;
    }
}
std::pair<int, int> nextPoint(std::pair<int, int> cur, int directionNo, int len)
{
    direction d = dir(directionNo);
    int x = cur.first + d.dx * len;
    int y = cur.second + d.dy * len;
    return std::make_pair(x, y);
}
void getLinkPiece(std::pair<int, int> cur, int directionNo, int side, int &len, int pace, std::pair<int, int> &linkPoint)
{
    direction d = dir(directionNo);
    std::pair<int, int> tmp = nextPoint(cur, directionNo, pace);
    linkPoint = cur;
    while (isInBoard(tmp.first, tmp.second) && board[tmp.first][tmp.second] == side)
    {
        linkPoint = tmp;
        tmp = nextPoint(tmp, directionNo, pace);
        ++len;
    }
}
void getBoundary(int *left, int *right, std::pair<int, int> l, std::pair<int, int> r, int directionNo, int side)
{
    direction d = dir(directionNo);
    int opposite = 1 - ai_side;
    for (register int i = 1; i <= 4; ++i)
    {
        std::pair<int, int> tmp = nextPoint(l, directionNo, -i);
        if (isInBoard(tmp.first, tmp.second))
            left[i] = board[tmp.first][tmp.second];
        else
            left[i] = opposite;
        tmp = nextPoint(r, directionNo, i);
        if (isInBoard(tmp.first, tmp.second))
            right[i] = board[tmp.first][tmp.second];
        else
            right[i] = opposite;
    }
}
bool judge(std::pair<int, int> cur, int side)
{
    int i = cur.first;
    int j = cur.second;
    if (!isEmpty(i, j) && board[i][j] == side)
    {
        for (register int k = 1; k <= 4; ++k)
        {
            direction d = dir(k);
            int len = 1;
            std::pair<int, int> tmpLeft, tmpRight;
            getLinkPiece(cur, k, side, len, 1, tmpRight);
            getLinkPiece(cur, k, side, len, -1, tmpLeft);
            if (len >= 5)
                return true;
        }
    }
    return false;
}
situation situationAnalysis(int len, int side, int *left, int *right)
{
    situation ans;
    if (len == 5)
        ++ans.win_5;
    else if (len == 4)
    {
        if (left[1] == -1 && right[1] == -1)
            ++ans.alive_4;
        else if (left[1] == -1 || right[1] == -1)
            ++ans.d_alive_4;
        else
            ++ans.die_4;
    }
    else if (len == 3)
    {
        if ((left[1] == -1 && left[2] == side) || (right[1] == -1 && right[2] == side))
            ++ans.d_alive_4;
        else if (left[1] == -1 && right[1] == -1 && (left[2] == -1 || right[2] == -1))
            ++ans.alive_3;
        else if ((left[1] == -1 && left[2] == -1) || (right[1] == -1 && right[2] == -1))
            ++ans.d_alive_3;
        else
            ++ans.die_3;
    }
    else if (len == 2)
    {
        if (left[1] == -1 && left[2] == side && left[3] == side && right[1] == -1 && right[2] == side && right[3] == side)
            ++ans.d_alive_4;
        else if (left[1] == -1 && right[1] == -1 && ((left[2] == side && left[3] == -1) || (right[2] == side && right[3] == -1)))
            ++ans.d_alive_3;
        else if ((left[1] == -1 && left[2] == side && left[3] == -1) || (right[1] == -1 && right[2] == side && right[3] == -1))
            ++ans.die_3;
        else if (left[1] == -1 && right[1] == -1 && (left[2] == side || right[2] == side))
            ++ans.die_3;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == side) || (right[2] = -1 && right[3] == side && right[1] == -1))
            ++ans.die_3;
        else if ((left[1] == -1 && right[1] == -1 && right[2] == -1 && right[3] == -1) || (left[1] == -1 && left[2] == -1 && right[1] == -1 && right[2] == -1) || right[1] == -1 && left[1] == -1 && left[2] == -1 && left[3] == -1)
            ++ans.alive_2;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == -1) || (right[1] == -1 && right[2] == -1 && right[3] == -1))
            ++ans.die_2;
    }
    else if (len == 1)
    {
        if ((left[1] == -1 && left[2] == side && left[3] == side && left[4] == side) || (right[1] == -1 && right[2] == -1 && right[3] == -1 && right[4] == -1))
            ++ans.d_alive_4;
        else if ((left[1] == -1 && right[1] == -1) && ((left[2] == side && left[3] == side && left[4] == -1) || (right[2] == side && right[3] == side && right[4] == -1)))
            ++ans.d_alive_3;
        else if ((left[1] == -1 && right[1] == -1) && ((left[2] == side && left[3] == side) || (right[2] == side && right[3] == side)))
            ++ans.die_3;
        else if ((left[1] == -1 && left[2] == side && left[3] == side && left[4] == -1) || (right[1] == -1 && right[2] == side && right[3] == side && right[4] == -1))
            ++ans.die_3;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == side && left[4] == side) || (right[1] == -1 && right[2] == -1 && right[3] == side && right[4] == side))
            ++ans.die_3;
        else if ((left[1] == -1 && left[2] == side && left[3] == -1 && left[4] == side) || (right[1] == -1 && right[2] == side && right[3] == -1 && right[4] == side))
            ++ans.die_3;
        else if ((left[1] == -1 && right[1] == -1 && right[3] == -1 && right[2] == side) && (left[2] == -1 || right[4] == -1))
            ++ans.d_alive_2;
        else if ((right[1] == -1 && left[1] == -1 && left[3] == -1 && left[2] == side) && (right[2] == -1 || left[4] == -1))
            ++ans.d_alive_2;
        else if ((left[1] == -1 && right[1] == -1 && right[2] == -1 && right[4] == -1 && right[3] == side) || (right[1] == -1 && left[1] == -1 && left[2] == -1 && left[4] == -1 && left[3] == side))
            ++ans.d_alive_2;
        else if ((left[1] == -1 && left[2] == side && left[3] == -1 && left[4] == -1) || (right[1] == -1 && right[2] == side && right[3] == -1 && right[4] == -1))
            ++ans.die_2;
        else if ((left[1] == -1 && right[1] == -1 && right[2] == -1 && left[2] == -1) || (left[1] == -1 && right[1] == -1 && left[2] == -1 && right[2] == side))
            ++ans.die_2;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == side && left[4] == -1) || (right[1] == -1 && right[2] == -1 && right[3] == side && right[4] == -1))
            ++ans.die_2;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == side && right[1] == -1) || (right[1] == -1 && right[2] == -1 && right[3] == side && left[1] == -1))
            ++ans.die_2;
        else if ((left[1] == -1 && left[2] == -1 && left[3] == -1 && left[4] == side) || (right[1] == -1 && right[2] == -1 && right[3] == -1 && right[4] == side))
            ++ans.die_2;
        else if (left[1] == -1 && right[1] == -1)
            ++ans.alive_1;
        else if (left[1] == -1 || right[1] == -1)
            ++ans.d_alive_1;
        else
            ++ans.die_1;
    }
    return ans;
}
int singleEvaluation(std::pair<int, int> cur, int side)
{
    int ans = 0;
    situation res, tmp;
    for (register int i = 1; i <= 4; ++i)
    {
        direction d = dir(i);
        int len = 1;
        std::pair<int, int> l, r, tmpPoint;
        int left[5], right[5];
        getLinkPiece(cur, i, side, len, -1, l);
        getLinkPiece(cur, i, side, len, 1, r);
        getBoundary(left, right, l, r, i, side);
        tmp = situationAnalysis(len, side, left, right);
        res += tmp;
    }
    if (res.win_5 >= 1)
        ans += 20000000;
    if (res.alive_4 >= 1 || res.d_alive_4 >= 2 || res.alive_3 >= 2 || (res.d_alive_4 >= 1 && res.alive_3 >= 1)) //绝杀
        ans += 10000000;
    ans += (res.d_alive_4 * 10000 + res.die_4 * 5000 + res.alive_3 * 10000 + res.d_alive_4 * 1000 + res.die_3 * 500 + res.alive_2 * 1000 + res.d_alive_2 * 100 + res.die_2 * 50 + res.alive_1 * 100 + res.d_alive_1 * 10 + res.die_1 * 5);
    return ans;
}
value wholeEvaluation()
{
    value val;

    for (register int i = 0; i < 15; ++i)
        for (register int j = 0; j < 15; ++j)
        {
            if (!isEmpty(i, j))
            {
                if (board[i][j] == ai_side)
                    val.aiValue += singleEvaluation({i, j}, ai_side);
                else
                    val.humanValue += singleEvaluation({i, j}, 1 - ai_side);
            }
        }
    val.aiValue -= val.humanValue;
    return val;

    std::pair<int, int> left, right;
    int len;
    situation curSituation_ai, curSituation_human;
    //横着数
    for (register int i = 0; i < 15; ++i)
    {
        left = std::make_pair(i, 0);
    continueCount1:
        while (board[left.first][left.second] == -1)
        {
            left.second++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop1;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first, right.second + 1) && board[left.first][left.second] == board[right.first][right.second + 1])
        {
            ++right.second;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first, right.second + 1) && board[right.first][right.second + 1] == -1 && isInBoard(left.first, left.second - 1) && board[left.first][left.second - 1])
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first, right.second + 1) && board[right.first][right.second + 1] == -1) || (isInBoard(left.first, left.second - 1) && board[left.first][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first, right.second + 1) && board[right.first][right.second + 1] == -1 && isInBoard(left.first, left.second - 1) && board[left.first][left.second - 1] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first, right.second + 1) && board[right.first][right.second + 1] == -1) || (isInBoard(left.first, left.second - 1) && board[left.first][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first, right.second + 1))
        {
            left = std::make_pair(right.first, right.second + 1);
            goto continueCount1;
        }
    nextLoop1:;
    }
    //竖着数
    for (register int i = 0; i < 15; ++i)
    {
        left = std::make_pair(0, i);
    continueCount2:
        while (board[left.first][left.second] == -1)
        {
            left.first++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop2;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first + 1, right.second) && board[left.first][left.second] == board[right.first + 1][right.second])
        {
            ++right.first;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second) && board[right.first + 1][right.second] == -1 && isInBoard(left.first - 1, left.second) && board[left.first - 1][left.second] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second) && board[right.first + 1][right.second] == -1) || (isInBoard(left.first - 1, left.second) && board[left.first - 1][left.second] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second) && board[right.first + 1][right.second] == -1 && isInBoard(left.first - 1, left.second) && board[left.first - 1][left.second] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second) && board[right.first + 1][right.second] == -1) || (isInBoard(left.first - 1, left.second) && board[left.first - 1][left.second] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first + 1, right.second))
        {
            left = std::make_pair(right.first + 1, right.second);
            goto continueCount2;
        }
    nextLoop2:;
    }
    //斜下数
    for (register int i = 0; i < 15; ++i)
    {
        left = std::make_pair(0, i);
    continueCount3:
        while (board[left.first][left.second] == -1)
        {
            left.first++;
            left.second++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop3;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first + 1, right.second + 1) && board[left.first][left.second] == board[right.first + 1][right.second + 1])
        {
            ++right.first;
            ++right.second;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1 && isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1) || (isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1 && isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1])
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1) || (isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1]))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first + 1, right.second + 1))
        {
            left = std::make_pair(right.first + 1, right.second + 1);
            goto continueCount3;
        }
    nextLoop3:;
    }
    for (register int i = 0; i < 14; ++i)
    {
        left = std::make_pair(i, 0);
    continueCount4:
        while (board[left.first][left.second] == -1)
        {
            left.first++;
            left.second++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop4;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first + 1, right.second + 1) && board[left.first][left.second] == board[right.first + 1][right.second + 1])
        {
            ++right.first;
            ++right.second;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1 && isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1) || (isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1 && isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1])
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first + 1, right.second + 1) && board[right.first + 1][right.second + 1] == -1) || (isInBoard(left.first - 1, left.second - 1) && board[left.first - 1][left.second - 1]))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first + 1, right.second + 1))
        {
            left = std::make_pair(right.first + 1, right.second + 1);
            goto continueCount4;
        }
    nextLoop4:;
    }
    //斜上数
    for (register int i = 0; i < 15; ++i)
    {
        left = std::make_pair(i, 0);
    continueCount5:
        while (board[left.first][left.second] == -1)
        {
            left.first--;
            left.second++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop5;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first - 1, right.second + 1) && board[left.first][left.second] == board[right.first - 1][right.second + 1])
        {
            --right.first;
            ++right.second;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1 && isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1) || (isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1 && isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1])
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1) || (isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1]))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first - 1, right.second + 1))
        {
            left = std::make_pair(right.first - 1, right.second + 1);
            goto continueCount5;
        }
    nextLoop5:;
    }
    for (register int i = 1; i < 15; ++i)
    {
        left = std::make_pair(14, i);
    continueCount6:
        while (board[left.first][left.second] == -1)
        {
            left.first--;
            left.second++;
            if (!isInBoard(left.first, left.second))
                goto nextLoop6;
        }
        right = left;
        len = 1;
        while (isInBoard(right.first - 1, right.second + 1) && board[left.first][left.second] == board[right.first - 1][right.second + 1])
        {
            --right.first;
            ++right.second;
            ++len;
        }
        if (board[left.first][left.second] == ai_side)
        {
            if (len == 5)
                ++curSituation_ai.win_5;
            else
            {
                if (isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1 && isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1] == -1)
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.alive_4;
                    case 3:
                        ++curSituation_ai.alive_3;
                    case 2:
                        ++curSituation_ai.alive_2;
                    case 1:
                        ++curSituation_ai.alive_1;
                    }
                else if ((isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1) || (isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1] == -1))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_ai.die_4;
                    case 3:
                        ++curSituation_ai.die_3;
                    case 2:
                        ++curSituation_ai.die_2;
                    }
                }
            }
        }
        else
        {
            if (len == 5)
                ++curSituation_human.win_5;
            else
            {
                if (isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1 && isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1])
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.alive_4;
                    case 3:
                        ++curSituation_human.alive_3;
                    case 2:
                        ++curSituation_human.alive_2;
                    case 1:
                        ++curSituation_human.alive_1;
                    }
                else if ((isInBoard(right.first - 1, right.second + 1) && board[right.first - 1][right.second + 1] == -1) || (isInBoard(left.first + 1, left.second - 1) && board[left.first + 1][left.second - 1]))
                {
                    switch (len)
                    {
                    case 4:
                        ++curSituation_human.die_4;
                    case 3:
                        ++curSituation_human.die_3;
                    case 2:
                        ++curSituation_human.die_2;
                    }
                }
            }
        }
        if (isInBoard(right.first - 1, right.second + 1))
        {
            left = std::make_pair(right.first - 1, right.second + 1);
            goto continueCount6;
        }
    nextLoop6:;
    }
    val.aiValue += (curSituation_ai.win_5 * AI_WIN_5 + curSituation_ai.alive_4 * AI_ALIVE_4 + curSituation_ai.alive_3 * AI_ALIVE_3 + curSituation_ai.alive_2 * AI_ALIVE_2 + curSituation_ai.alive_1 * AI_ALIVE_1 + curSituation_ai.die_4 * AI_DIE_4 + curSituation_ai.die_3 * AI_DIE_3 + curSituation_ai.die_2 * AI_DIE_2);
    val.humanValue += (curSituation_human.win_5 * HUMAN_WIN_5 + curSituation_human.alive_4 * HUMAN_ALIVE_4 + curSituation_human.alive_3 * HUMAN_ALIVE_3 + curSituation_human.alive_2 * HUMAN_ALIVE_2 + curSituation_human.alive_1 * HUMAN_ALIVE_1 + curSituation_human.die_4 * HUMAN_DIE_4 + curSituation_human.die_3 * HUMAN_DIE_3 + curSituation_human.die_2 * HUMAN_DIE_2);
    val.aiValue -= val.humanValue;
    return val;
}
bool hasNeighbour(std::pair<int, int> cur, int side)
{
    for (register int i = 1; i <= 4; ++i)
    {
        direction d = dir(i);
        for (register int j = 1; j <= NEIGHBOUR_SEARCH_WIDTH; ++j)
        {
            std::pair<int, int> tmp = nextPoint(cur, i, j);
            if (isInBoard(tmp.first, tmp.second) && board[tmp.first][tmp.second] != -1)
                return true;
            tmp = nextPoint(cur, i, -j);
            if (isInBoard(tmp.first, tmp.second) && board[tmp.first][tmp.second] != -1)
                return true;
        }
    }
    return false;
}
std::vector<std::pair<int, int>> inspiredSearch(int side)
{
    std::vector<std::pair<int, int>> ans;
    for (register int i = 0; i < 15; ++i)
        for (register int j = 0; j < 15; ++j)
        {
            if (isEmpty(i, j) && hasNeighbour(std::make_pair(i, j), side))
                ans.push_back({i, j});
        }
    return ans;
}
value findAiMove(std::pair<int, int> &bestMove, int depth, int alpha, int beta)
{
    std::pair<int, int> dc;
    value val, responseVal;
    std::priority_queue<waitPoint, std::vector<waitPoint>, cmp> q;
    if (isFull())
        return wholeEvaluation();
    else if (depth == DEPTH_LIMIT)
        return wholeEvaluation();
    else
    {
        std::vector<std::pair<int, int>> tmp = inspiredSearch(ai_side);
        val.aiValue = alpha;
        bestMove = tmp[0];
        for (auto p : tmp)
        {
            if (val.aiValue > beta)
                break;
            if (isEmpty(p.first, p.second))
            {
                place(p.first, p.second, ai_side);
                responseVal = findHumanMove(dc, depth + 1, val.aiValue, beta);
                unplace(p.first, p.second);
                if (depth == 1)
                    q.push(waitPoint(responseVal, p));
                else if (responseVal.aiValue > val.aiValue)
                {
                    val = responseVal;
                    bestMove = {p.first, p.second};
                }
            }
        }
    }
    if (depth == 1)
        intelligentRandom(bestMove, val, q);
    return val;
}
value findHumanMove(std::pair<int, int> &bestMove, int depth, int alpha, int beta)
{
    std::pair<int, int> dc;
    value val, responseVal;
    if (isFull())
        return wholeEvaluation();
    else if (depth == DEPTH_LIMIT)
        return wholeEvaluation();
    else
    {
        std::vector<std::pair<int, int>> tmp = inspiredSearch(ai_side);
        val.aiValue = beta;
        bestMove = tmp[0];
        for (auto p : tmp)
        {
            if (alpha > val.aiValue)
                break;
            if (isEmpty(p.first, p.second))
            {
                place(p.first, p.second, 1 - ai_side);
                responseVal = findHumanMove(dc, depth + 1, alpha, val.aiValue);
                unplace(p.first, p.second);
                if (responseVal.aiValue < val.aiValue)
                {
                    val = responseVal;
                    bestMove = {p.first, p.second};
                }
            }
        }
    }
    return val;
}
void intelligentRandom(std::pair<int, int> &bestMove, value &val, std::priority_queue<waitPoint, std::vector<waitPoint>, cmp> &q)
{
    waitPoint first = q.top();
    q.pop();
    waitPoint second = q.top();
    if (first.val.aiValue <= second.val.aiValue * 50)
    {
        std::default_random_engine e(time(0));
        std::bernoulli_distribution u;
        if (u(e) == 1)
        {
            bestMove = first.cur;
            val = first.val;
        }
        else
        {
            bestMove = second.cur;
            val = second.val;
        }
    }
    else
    {
        bestMove = first.cur;
        val = first.val;
    }
}
value minimax(std::pair<int, int> &bestMove, int depth, int alpha, int beta, int side)
{
    value val, maxVal, minVal;
    std::pair<int, int> dc;
    if (depth == 0)
        return wholeEvaluation();
    if (side == ai_side)
    {
        maxVal.aiValue = NEGIFF;
        std::vector<std::pair<int, int>> tmp = inspiredSearch(ai_side);
        for (auto p : tmp)
        {
            place(p.first, p.second, ai_side);
            val = minimax(dc, depth - 1, alpha, beta, 1 - ai_side);
            unplace(p.first, p.second);
            if (val.aiValue > maxVal.aiValue)
            {
                maxVal = val;
                bestMove = {p.first, p.second};
            }
            alpha = std::max(alpha, val.aiValue);
            if (beta <= alpha)
                break;
        }
        return maxVal;
    }
    else
    {
        minVal.aiValue = IFF;
        std::vector<std::pair<int, int>> tmp = inspiredSearch(1 - ai_side);
        for (auto p : tmp)
        {
            place(p.first, p.second, 1 - ai_side);
            val = minimax(dc, depth - 1, alpha, beta, ai_side);
            unplace(p.first, p.second);
            if (val.aiValue < minVal.aiValue)
            {
                minVal = val;
                bestMove = {p.first, p.second};
            }
            beta = std::min(beta, val.aiValue);
            if (beta <= alpha)
                break;
        }
    }
    return minVal;
}
void flip()
{
    for (register int i = 0; i < 15; ++i)
        for (register int j = 0; j < 15; ++j)
            if (board[i][j] != -1)
                board[i][j] = 1 - board[i][j];
}