#include "pch.h"
#include "core/src/Game.hpp"
#include <algorithm>
#include <numeric>

namespace Gomoku {
inline bool operator==(const Board& lhs, const Board& rhs) {
    // 为加快速度，只检查moveStates元素个数是否相等，就不检查元素是否一一对应了
    // 注意m_moveStates与m_moveCounts都不是std::array，而是原生数组，因此不能直接用来比较
    auto make_tied = [](const Board& b) { 
        return std::tie(b.m_curPlayer, b.m_winner, b.m_moveCounts[0], b.m_moveCounts[1], b.m_moveCounts[2]);
    };
    return make_tied(lhs) == make_tied(rhs);
}
}

using namespace Gomoku;
using std::begin;
using std::end;

// Problem found 1: getNextMove() 无限循环
// Problem found 2: isCuurentPlayer函数参数不能直接设为Position，因为要做越界检查
// Problem found 3: std::array::empty()总是返回false 不能作为条件判断使用
// Problem found 4: checkVictory::search中把正反向搜索合而为一的逻辑是有问题的
class BoardTest : public ::testing::Test {
protected:
    static const int caseSize = 10;

    // 产生一列不重复的Position
    void randomlyFill(Position* first, Position* last) {
        std::iota(first, last, rand() % (width * height / caseSize));
        std::random_shuffle(first, last);
    }

    // 对棋盘的状态进行快速检查
    ::testing::AssertionResult trivialCheck(const Board& board) {
        // 已下与未下的格子数量要互补
        if (board.moveCounts(Player::Black) + board.moveCounts(Player::White) + board.moveCounts(Player::None) != width * height) {
            return ::testing::AssertionFailure() << "Moves size sum not compatible.";
        }
        // 游戏未结束时（m_curPlayer != Player::None），一定没有赢家（m_winner = Player::None）
        if ((!(board.m_curPlayer != Player::None) || board.m_winner == Player::None) == false) { // 蕴含关系式
            return ::testing::AssertionFailure() << "Winner is not none when game does not end.";
        }
        return ::testing::AssertionSuccess();
    }

    Board board;
};

// 对称性检查：ApplyMove与RevertMove
// RevertMove()的基本情况将在这里检查
TEST_F(BoardTest, MoveSymmetry) {
    Board board_cpy(board);
    Position positions[caseSize];
    randomlyFill(begin(positions), end(positions));
    for (int i = 0; i < caseSize; ++i) {
        int offset = rand() % 3 + 1;
        // 随机选取连续几个Position进行apply与revert
        int j;
        for (j = 0; j < offset && i + j < caseSize; ++j) {
            board.applyMove(positions[i + j]);
            ASSERT_TRUE(trivialCheck(board));
        }
        for (--j; j >= 0; --j) {
            Player curPlayer = board.m_curPlayer;
            Player result = board.revertMove(positions[i + j]);
            ASSERT_TRUE(trivialCheck(board));
            EXPECT_EQ(result, (curPlayer = -curPlayer));
            // 在原位再悔一次棋，检查下一手是否仍为当前玩家（悔棋无效）
            board.revertMove(positions[i + j]);
            EXPECT_EQ(result, curPlayer);
        }
        EXPECT_EQ(board, board_cpy);
    }
}

// TODO: 查revertMove()在游戏结束后是否能调用成功
TEST_F(BoardTest, CheckVictory) {
    // 黑棋能赢的一种手顺
    Player curPlayer = Player::Black;
    Position blacks[9] = { {3,3}, {3,4}, {4,4}, {3,5}, {5,5}, {3,6}, {6,6}, {3,7}, {7,7} };
    for (int i = 0; i < sizeof(blacks) / sizeof(Position); ++i) {
        EXPECT_NE(curPlayer, board.applyMove(blacks[i])); // 下的一定是有效的一手
        curPlayer = -curPlayer;
    }
    ASSERT_TRUE(board.status().end);
    ASSERT_EQ(board.status().winner, Player::Black);
    for (int i = sizeof(blacks) / sizeof(Position) - 1; i >= 0; --i) {
        EXPECT_NE(curPlayer, board.revertMove(blacks[i])); // 一定悔棋成功了
        curPlayer = -curPlayer;
    }
    // 白棋能赢的一种手顺
    Position whites[10] = { {3,3}, {3,4}, {4,4}, {3,5}, {5,5}, {3,6}, {6,6}, {3,7}, {8,8}, {3,8} };
    for (int i = 0; i < sizeof(whites) / sizeof(Position); ++i) {
        EXPECT_NE(curPlayer, board.applyMove(whites[i])); // 下的一定是有效的一手
        curPlayer = -curPlayer;
    }
    ASSERT_TRUE(board.status().end);
    ASSERT_EQ(board.status().winner, Player::White);
    for (int i = sizeof(whites) / sizeof(Position) - 1; i >= 0; --i) {
        EXPECT_NE(curPlayer, board.revertMove(whites[i])); // 一定悔棋成功了
        curPlayer = -curPlayer;
    }
}

// 利用一种可以和棋的下法进行检查
TEST_F(BoardTest, CheckTie) {
    for (int j = 0; j < height; ++j) {
        // y的映射方式为：
        // 低height/2位：由0位开始，每位映射为0,2,4,6,8...
        // 高height/2位：由(height+1)/2位开始，每位映射为1,3,5,7,9...
        int y = j <= height/2 ? 2*j : 2*(j - height/2) - 1;
        for (int i = 0; i < width; ++i) {
            // x的映射方式为：i -> x
            int x = i;
            // 下棋到{x, y}并进行相关检查
            Player result = board.applyMove({x, y});
            ASSERT_TRUE(trivialCheck(board));
            if (j * width + i == width * height - 1) {
                EXPECT_EQ(result, Player::None);
                EXPECT_EQ(board.m_curPlayer, Player::None);
                EXPECT_EQ(board.m_winner, Player::None);
            } else {
                // 游戏一定没有结束
                ASSERT_NE(result, Player::None);
                ASSERT_NE(board.m_curPlayer, Player::None);
            }
        }
    }
    // 检查棋盘下满后，再随机取子是否会抛出异常
    ASSERT_THROW(board.getRandomMove(), std::exception) << "Random move does not throw exception when board is full.";
}

// 随机下完一盘完整的棋
// getRandomMove()与applyMove()的所有情况会在这里检查
TEST_F(BoardTest, RandomRollout) {
    Board board_cpy(board);
    while (true) {
        Position move = board.getRandomMove();
        Player curPlayer = board.m_curPlayer;
        Player result = board.applyMove(move);
        auto status = board.status();
        ASSERT_TRUE(trivialCheck(board));
        EXPECT_NE(result, curPlayer); // getRandomMove()需要保证一定不会下到无效位置（就算有禁手也是一样）
        if (board.m_curPlayer != Player::None) {
            // 若游戏未结束
            EXPECT_EQ(result, -curPlayer); // 下一步变为对手下
            EXPECT_EQ(status.end, false);
            EXPECT_EQ(status.winner, Player::None);
        } else {
            // 若游戏已结束
            EXPECT_EQ(result, Player::None);
            EXPECT_EQ(status.end, true);
            EXPECT_NE(status.winner, -curPlayer); // 赢家一定不会是对手
            break;
        }
        // 开始对无效手进行检查
        curPlayer = result; // 将当前玩家转换到原board下了一手后应下的玩家
        board_cpy.applyMove(move); // 拷贝版board跟上一手
        result = board.applyMove(move); // 在原board原位置再下一子
        EXPECT_EQ(board, board_cpy) << "board does not remain the same after applied an invalid move"; // 因为该手无效，所以原board应该无变化
        ASSERT_EQ(result, curPlayer); // result一定得为当前玩家（当前玩家需要重下）
    }
}
