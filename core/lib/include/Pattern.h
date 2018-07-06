#ifndef GOMOKU_PATTERN_MATCHING_H_
#define GOMOKU_PATTERN_MATCHING_H_
#include "Game.h"
#include <utility>
#include <string_view>

namespace Gomoku {

inline namespace Config {
// 模式匹配模块的相关配置
enum PatternConfig {
    MAX_PATTERN_LEN = 7,
    BLOCK_SIZE = 2*3 + 1,
    TARGET_LEN = 2 * MAX_PATTERN_LEN - 1
};
}

enum class Direction : char {
    Horizontal, Vertical, LeftDiag, RightDiag
};

// 将方向枚举解包成具体的(Δx, Δy)值。
constexpr std::pair<int, int> operator*(Direction direction) {
    switch (direction) {
        case Direction::Horizontal: return { 1, 0 };
        case Direction::Vertical:   return { 0, 1 };
        case Direction::LeftDiag:   return { 1, 1 };
        case Direction::RightDiag:  return { -1, 1 };
        default: return { 0, 0 };
    }
}

constexpr Position Shift(Position pose, int offset, Direction dir) {
    auto [x, y] = pose; auto [dx, dy] = *dir;
    return { x + offset * dx, y + offset * dy };
}


struct Pattern {
    enum Type {
        DeadOne, LiveOne, 
        DeadTwo, LiveTwo, 
        DeadThree, LiveThree, 
        DeadFour, LiveFour, 
        Five, Size
    };

    std::string str;
    Player favour;
    Type type;
    int score;

    Pattern(std::string_view proto, Type type, int score);
    Pattern(const Pattern&) = default;
    Pattern(Pattern&&) = default;
};


class PatternSearch {
public:
    // 利用friend指明类实现里使用了AC自动机。
    friend class AhoCorasickBuilder;

    using Entry = std::tuple<const Pattern&, int>;

    struct generator {
        std::string_view target = "";
        PatternSearch* ref = nullptr;
        size_t offset = 0, state = 0;

        const generator& begin() { return state == 0 ? ++(*this) : *this; }
        const generator& end()   { return generator{}; }

        Entry operator*() const;
        const generator& operator++();
        bool operator!=(const generator& other) const { return target != other.target; }
    };

    PatternSearch(std::initializer_list<Pattern> protos);
    
    generator execute(std::string_view target);
    std::vector<Entry> matches(std::string_view target);

private:
    std::vector<int> m_base;
    std::vector<int> m_check;
    std::vector<int> m_fail;
    std::vector<Pattern> m_patterns;
};


class BoardMap {
public:
    static std::tuple<int, int> ParseIndex(Position pose, Direction direction);

    explicit BoardMap(Board* board = nullptr);

    std::string_view lineView(Position pose, Direction direction);

    Player applyMove(Position move);

    Player revertMove(size_t count = 1);

    void reset();

public:
    std::unique_ptr<Board> m_board;
    std::array<std::string, 3*(WIDTH + HEIGHT) - 2> m_lineMap;
    unsigned long long m_hash;
};


class Evaluator {
public:
    struct Record {
        unsigned short field; // 4 White-Black组合 * 4 方向位 || 2 White/Black分割 * 8计数位
        void set(int delta, Player player); // 按玩家类型设置8位计数位。
        void set(int delta, Player favour, Player perspective, Direction dir);
        bool get(Player favour, Player perspective, Direction dir) const; // 获取某一组的某一方向位是否设置。
        unsigned get(Player favour, Player perspective) const; // 打包返回一组下的4个方向位。
        unsigned get(Player player) const; // 按玩家类型返回8位计数位。
    };

    struct Compound {
        enum Type { DoubleThree, FourThree, DoubleFour, Size };
        static bool Test(Evaluator* ev, Position pose, Player player);
        static void Update(Evaluator* ev, int delta, Position pose, Player player);
        static const Pattern::Type Components[3]; // 被用于组成复合模式的单模式类型。
        static const int BaseScore; // 双三/四三/双四共用一个分数。
    };

    // 按 { Player::Black, Player::White } 构成的2*2列联表分组。
    static constexpr int Group(Player favour, Player perspective) {
        return (favour == Player::Black) << 1 | (perspective == Player::Black);
    }

    // 基于AC自动机实现的多模式匹配器。
    static PatternSearch Patterns;

    // 基于Eigen向量化操作与Map引用实现的区域棋子密度计数器，tuple组成: { 权重， 分数 }。
    static std::tuple<Eigen::Array<int, BLOCK_SIZE, BLOCK_SIZE>, int> BlockWeights;

public:
    explicit Evaluator(Board* board = nullptr);

    auto& board() { return *m_boardMap.m_board; }

    auto& scores(Player player, Player perspect) { return m_scores[Group(player, perspect)]; }

    auto& density(Player player) { return m_density[player == Player::Black]; }

    Player applyMove(Position move);

    Player revertMove(size_t count = 1);

    bool checkGameEnd();  // 利用Evaluator，我们可以做到快速检查游戏是否结束。

    void syncWithBoard(const Board& board); // 同步Evaluator至传入的Board状态。

    void reset();

private:
    void updateMove(Position move, Player src_player);

    void updateLine(std::string_view target, int delta, Position move, Direction dir);

    void updateBlock(int delta, Position move, Player src_player);

public:
    BoardMap m_boardMap; // 内部维护了一个Board, 避免受到外部的干扰
    std::vector<Record> m_patternDist[Pattern::Size - 1]; // 不统计Pattern::Five分布
    std::vector<Record> m_compoundDist[Compound::Size];
    Eigen::VectorXi m_density[2];
    Eigen::VectorXi m_scores[4];
};

}

#endif // !GOMOKU_PATTERN_MATCHING_H_
