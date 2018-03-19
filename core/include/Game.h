#ifndef GOMOKU_GAME_H_
#define GOMOKU_GAME_H_
#include <utility> // std::pair, std::size_t
#include <array>   // std::array, 

namespace Gomoku {

// 游戏的基本配置
enum GameConfig {
    WIDTH = 15, HEIGHT = 15, MAX_RENJU = 5, BOARD_SIZE = WIDTH*HEIGHT
};


// 玩家概念的抽象封装
enum class Player : char { 
    White = -1, None = 0, Black = 1 
};

// 返回对手的Player值。Player::None的对手仍是Player::None。
constexpr Player operator-(Player player) { 
    return Player(-static_cast<char>(player)); 
}

// 返回游戏结束后的得分。同号（winner与player相同）为正，异号为负，平局为零。
constexpr double getFinalScore(Player player, Player winner) { 
    return static_cast<double>(player) * static_cast<double>(winner); 
}


// 可用来表示第一/第四象限的坐标。也就是说，x/y坐标必须同正或同负。
struct Position {
    int id;

    Position(int id = -1) : id(id) {}
    Position(int x, int y) : id(y * WIDTH + x) {}
    Position(std::pair<int, int> pose) : id(pose.second * WIDTH + pose.first) {}

    operator int() const { return id; }
    constexpr int x() const { return id % WIDTH; }
    constexpr int y() const { return id / WIDTH; }

    // 保证Position可以作为哈希表的key使用
    struct Hasher {
        std::size_t operator()(const Position& position) const {
            return position.id;
        }
    };
};

/*
    时间开销分析：
    由于需要在单线程的基础上将性能优化到极致，因此各种操作的开销需要尽量低。
    * 经查阅资料知，为了方便神经网络的训练，最好每一种棋子状态（黑/白/无）分别用一个容器存储。
    因此棋盘可以如此设计： Array[3] = { 状态容器(白子), 状态容器(无子), 状态容器(黑子) }。
    在此之上，对存储各状态所使用的容器进行分析：
        1. unordered_set<Position>: 特点是可以不用存储整个棋盘，只需存符合对应状态的位置即可。
        但是，经过profiler检测，实际上多数常用操作的开销相对来说显得过高了点。
        2. std::array<bool>: 特点是存储了整个棋盘，index代表位置，value表示该位置是否满足对应状态。
        优点是在当前设计下，每一种操作都可以做到非常快。但需要额外存储一个size_t数组来记录每种状态的棋子数目。
        3. bitset: 使用bitset,可以起到与std::array一样的效果（因为3是2的紧凑版本）。
        具体效率如何有待profiler检测。

    空间开销分析：
    由于这个对象可能会被频繁拷贝，若要考虑这点的话，拷贝成本需设计得尽量低。
    棋盘的存储有几种选择：
        1. bitset: 使用bitset存储整个棋盘。由于每个格子需要标记三种落子状态（黑/白/无），至少需要2bit存储。
        因此一个19*19的棋盘需要至少19*19*2=722bit=90.25Byte。
        2. pos -> player的map。一个unordered_map<Position, Player>在MSVC下为>=32Byte。一个{pos, player}对至少为3字节。
        由于采用Hash Table机制， 空间需求很可能更大……
        3. 同时间开销分析的2, 3。由于2是栈上数组，3是栈上bit列，因此它们的拷贝开销相对来说都较低一些。
*/
class Board {
// 公开接口部分
public:
    Board();

    /*
        下棋。返回值类型为Player，代表下一轮应下的玩家。具体解释：
        - 若Player为对手，则代表下一步应对手下，正常继续。
        - 若Player为己方，则代表这一手无效，应该重下。
        - 若Player为None，则代表这一步后游戏已结束。此时，可以通过m_winner获取游戏结果。
    */
    Player applyMove(Position move, bool checkVictory = true);

    /*
        悔棋。返回值类型为Player，代表下一轮应下的玩家。具体解释：
        - 若Player为对手，则代表悔棋成功，改为对手玩家下棋。
        - 若Player为己方，则代表悔棋失败，棋盘状态不变。
    */
    Player revertMove(Position move);

    // 每下一步都进行终局检查，这样便只需对当前落子周围进行遍历即可。
    Position getRandomMove() const;

    // 越界与无子检查。暂无禁手规则。
    bool checkMove(Position move);

    // 每下一步都进行终局检查，这样便只需对当前落子周围进行遍历即可。
    bool checkGameEnd(Position move);
    
// 数据成员封装部分
public: 
    // 返回当前游戏状态
    const auto status() const {
        struct { bool end; Player curPlayer; Player winner; } status {
            m_curPlayer == Player::None, m_curPlayer, m_winner 
        };
        return status;
    }

    // 通过Player枚举获取对应棋盘状态
    std::array<bool, BOARD_SIZE>&       moveStates(Player player) { return m_moveStates[static_cast<int>(player) + 1]; }
    const std::array<bool, BOARD_SIZE>& moveStates(Player player) const { return m_moveStates[static_cast<int>(player) + 1]; }

    // 通过Player枚举获取已落子/未落子总数
    std::size_t& moveCounts(Player player) { return m_moveCounts[static_cast<int>(player) + 1]; }
    std::size_t  moveCounts(Player player) const { return m_moveCounts[static_cast<int>(player) + 1]; }

// 数据成员部分
public:
    /*
        表示在当前棋盘状态下应下的玩家。
        当值为Player::None时，代表游戏结束。
        默认为黑棋先下。
    */
    Player m_curPlayer = Player::Black;

    /*
        当棋局结束后，其值代表最终游戏结果:
        - Player::Black: 黑赢
        - Player::White: 白赢
        - Player::None:  和局
        当棋局还未结束时（m_curPlayer != Player::None），值始终保持为None，代表还没有赢家。
    */
    Player m_winner = Player::None;

    /*
        两个数组表示了棋盘上的状态与已落子个数。各下标对应关系为：
        0 - Player::White - 白子放置情况
        1 - Player::None  - 可落子位置情况
        2 - Player::Black - 黑子放置情况
        附：为什么不使用std::vector呢？因为vector<bool> :)。
    */
    std::array<bool, BOARD_SIZE> m_moveStates[3] = {};
    std::size_t m_moveCounts[3] = {};
};

}

#endif // !GAME_H_
