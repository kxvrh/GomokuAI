<h1 align="center">
<span style="font-size:32px;">Gomoku AI Project</span>
</h1>
<h3 align="center">
<span style="font-size:24px;">Collaborators</span> 
</h3>
<div align="center">
<span style="font-size:18px;">09016319 叶志浩</span><br />
<span style="font-size:18px;">09016317 曹放</span> 
</div>
<h3 align="center">
<span style="font-size:24px;">Current Bot Version</span> 
</h3>
<div align="center">
<strong><span style="font-size:18px;">Git commits</span></strong><span style="font-size:18px;">: 38</span><br />
<strong><span style="font-size:18px;">Latest Commit</span></strong><span style="font-size:18px;">: 635847a</span><br />
<strong><span style="font-size:18px;">Git Repo Status</span></strong><span style="font-size:18px;">: Private </span><br />
<strong><span style="font-size:18px;">Documents</span></strong><span style="font-size:18px;">: https://vigilans-yea.github.io/GomokuAI/</span> 
</div>



# Current Performance

### Week 3

* 目前，我们将注意力集中于结构与性能的优化，因此暂时不急于参与至天梯中。
* 因此Botzone分数为：1000(〜￣△￣)〜。
* 不过，即使是目前的仍然有潜在Bug的只会横下五子的AI，也能打赢绝大部分的随机AI了。
* 此外，不知为何，目前的Bot代码能在Windows下正常运行，但一旦在Botzone的Linux环境下跑，就会出现Runtime Error_ (:з」∠ )_


### Week 5

* 本期仍为开发的过渡期，主要工作是建立起来了项目的完整结构与模块间的功能依赖关系，且明确了要做一个神经网络版AI的目标。
* 目前，AlphaZero版AI的开发接近完成，等待着实地的训练与测试。当它能够稳定的变强以后，我们便有了一个稳靠的火器，届时也将正式投入与他人的竞技之中。




# General Report

## Idea of Design

我们的目标是~~：没有蛀牙！~~：

* High Performance
* High Readablity
* High Scalbility

在可读性与扩展性上，我们采用工程化手段进行AI的开发，使用Python作为胶水语言；

在性能上，我们采用C++编写AI中性能关键的模块，并包装成为Python模块的扩展。



## Expectations

简单来说，它应能够达成如下的效果：

1. 在性能上达到一个极致的平衡，没有浪费的操作。
2. 可以方便地在不同的算法之间切换。
3. 可以提供一个方便的与Botzone交互的接口。

（待更新）



## Further Plans & Directions

### Week 3

* 在开始写报告后，我们为MCTS设计了`Policy`这个重要的结构。有了它，我们可以方便地对MCTS的关键算法进行替换，由此可以对各种不同算法的性能进行改进与评估。因此，未来的报告中，或许可以方便地列出各种baseline算法相互比较的图表。

* 起初，我们组的一个大方向是「使用C++内核 + Python搭建神经网络来实现AI」。现在，这个方向反而变得不那么明晰了。因为我们不太清楚不使用神经网络，且只用纯C++代码到底能将AI性能推进到什么样的程度。

  我们将在实现一些有名的MCTS优化算法（如：RAVE算法）后，根据AI的表现来决定是否执行最初的路线。


### Week 5

* 在得知4班的某位同学成功跑起来了AlphaZero版AI后，我们组终于定下心来决定跟进。本来是打算在最后一期做出来作为收尾之笔的，但经过仔细考虑后，提早做出来有不少的好处：

  * 考虑到神经网络需要较长的训练时间，早早做出来，显然为之后的优化与等待留有较大的余地。
  * 神经网络是一个在开发过程中，你能持续明显感觉到「它在变强」的东西。因此，它可以成为我们的基线模型，在训练过程中产生的各种checkpoint也可以用以与后续的其他模型比较。

* 在AlphaZero版AI能够顺畅地运行以后，我们将继续研究传统方法，以作为未来的主要基线方法，及备选参赛选手。不过，我们不打算再分别实现很多种不同算法了。我们将维护一个单独的「TraditionalAgent」，使用各种算法对其集中优化，以用来和AlphaZeroAgent进行有意义的比较。

* 未来的主要优化方向：

  * 算法上：

    由于AlphaZero需要大量的计算力来达到较好的训练速度，（例如论文中使用了5000个TPU v1来生成自对弈棋谱），而我们显然无法负担起如此庞大的算力，因此需要使用一些手段来对其进行优化。

    未来一期将采取的优化手段为：

    * [Decisive and Anti-Decisive Moves](https://hal.inria.fr/inria-00495078/document)。它以一个简单的思想利用领域知识来强化Select与Simulate过程。这种优化的特点是它不会对AI的自我学习造成负面影响，仅仅是加速了AI的收敛速度：因为其优化点在某种程度上是AI必须抵达的终点。
    * 设计卷积核的初始权重，以期加速收敛。

  * 工程上：

    * Memory Pool。经过Profiler测试显示，在目前的默认策略与MCTS配置下，有40%~50%的时间开销花在了expand阶段，并且几乎所有时间开销都集中在`operator new`，也就是树结点的内存分配上。MCTS的记录显示，一轮Playout后，结点的个数高达$4×10^7​$个。

      写一个高性能的内存池，应该能够对此性能瓶颈有较好的改善。

    * Cache。目前每次backPropogate操作都会将棋盘回退到开始局面，再进行下一轮Playout。但是，当Exploitation的值足够大时，很有可能下一轮Playout大部分仍走原来的路径。如果要加上盘面检查机制的话，这种重复操作的开销可能是很大的。

      我们可以添加一个简单的Cache机制，即每轮Playout后Board暂时保持叶结点的局面，在下一轮Playout中，一直到Select阶段选到的结点不在上一轮路径为止，才对Board进行回退操作。




# Code Structure

## Core

Core C++模块实现了所有Performance-Critical的组件。

### Game.h

#### Player: enum

```cpp
enum class Player : char { White = -1, None = 0, Black = 1 };
```

一个玩家完全可以使用一个整数来存储与表示。

而`enum`类型便是「**纯整数存储+信息封装+概念抽象**」的最好选择。

使用了`enum`之后，我们可以在一个很低的时空开销下抽象出`Player`这么一个事物。`enum class`的表示特征（必须使用`Player::XXX`表示，无法隐式与整数互转）极大地提高了代码的可读性。

同时，利用其内部整数的存储特征，我们可以很巧妙地实现如下的功能：

* 己方→对方的转换。我们实现如下一个函数：

  ```cpp
  constexpr Player operator-(Player player) { 
      return Player(-static_cast<char>(player)); 
  }
  ```

  此后，对于Player对象，只需添加`-`号，便可将其转化为对手玩家。（注意`Player::None`的对立方仍为`Player::None`）

* 游戏结束后，玩家得分的计算：

  ```cpp
  constexpr double getFinalScore(Player player, Player winner) { 
      return static_cast<double>(player) * static_cast<double>(winner); 
  }
  ```

  可以发现，同号（winner与player相同）为正1，异号为负1，平局为零。利用这一巧妙的数学性质，我们避免了条件判断，轻松实现了各方玩家的绝对价值计算。

* 通过`Player`检索棋盘状态：

  ```cpp
  std::array<bool, GameConfig::BOARD_SIZE>& Board::moveStates(Player player) { 
      return m_moveStates[static_cast<int>(player) + 1]; 
  }
  ```

  将[-1, 0, 1]往右偏移1单位，便得到了他们在棋盘上的状态数组。



#### Position: struct

```cpp
struct Position {
    short id;

    Position(int id = -1)         : id(id) {}
    Position(int x, int y)        : id(y*WIDTH + x) {}
    Position(pair<int, int> pose) : id(pose.second*WIDTH + pose.first) {}
    
    int x() const { return id % WIDTH; }
    int y() const { return id / WIDTH; }
    operator int () const { return id; }
};
```

与`Player`一样，`Position`只是一个整数的一层浅浅的封装。在很多情况下，将棋盘表示为一个225格的一维数组要比表示成15*15格的二维数组要更加方便与高效。

为了使它同时看起来像「一个整数」与「一个坐标」，我们为其拓展出了多个(x, y) 与单个id间的相互转换方法。

同时，通过提供`std::pair`作为构造参数，我们常常可以写出 `{x, y}` 这样方便的初始化格式。

内部存储使用short，其余情况均转换为int计算。

#### Board: class

```cpp
class Board {
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
    
	// 获取随机的一手。即使有禁手规则，也保证返回可下的一手。
    Position getRandomMove() const;

    // 越界与无子检查。暂无禁手规则。
    bool checkMove(Position move);
    
    // 每下一步都进行终局检查，这样便只需对当前落子周围进行遍历即可。
    bool checkGameEnd(Position move);
    
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
        整个数组表示了棋盘上每个点的状态。各下标对应关系为：
        0 - Player::White - 白子放置情况
        1 - Player::None  - 可落子位置情况
        2 - Player::Black - 黑子放置情况
        附：为什么不使用std::vector呢？因为vector<bool> :)。
    */
    std::size_t                    m_moveCounts[3] = {};
    std::array<bool, WIDTH*HEIGHT> m_moveStates[3] = {};
};
```

Board的每一步方法都被仔细地设计过，力求无任何冗余操作。

举例：大多数AI的胜利检查都会遍历整个棋盘，并且每个回合都会检查一次。由于有「每回合检查一次」这个必要的特点，checkGameEnd()函数利用「五子棋每下一子，只需检查当前子周围是否连成五子即可」的推理，使得在O(1)的时间内，就可以完成游戏是否结束的检查。

> 关于Board类是如何被迭代设计到现今的样子的，可参阅*Review*中的[*Improvements*](#Improvements Accumulated)章节。



### MCTS.h

#### Node: struct

```cpp
struct Node {
    /* 
        树结构部分 - 父结点。
        结点为观察指针，不对其拥有所有权。
    */
    Node* parent = nullptr;
    /* 
        棋盘状态部分：
          * position: 当前局面下最近一手的位置。
          * player: 当前局面下应走一下手的玩家。
    */
    Position position = Position(-1);
    Player player = Player::None;
    /* 
        结点价值部分：
          * state_value: 此结点对应的当前局面的评分。一般为胜率。
          * action_prob: 在父结点对应的局面下，选择该动作的概率。
    */
    float state_value = 0.0;
    float action_prob = 0.0;
    size_t node_visits = 0;
    /*
        树结构部分 - 子结点。
        结点为独有指针集合，对每个子结点拥有所有权。
    */
    std::vector<std::unique_ptr<Node>> children = {};
    
    /* 
        构造与赋值函数。
        显式声明两个函数的移动版本，以阻止复制版本的自动生成。 
    */
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;
    
    /* 辅助判断函数 */
    bool isLeaf() const;
    bool isFull(const Board& board) const;
};
```

蒙特卡洛树结点。由于整个树的结点数量十分庞大，因此其内存布局务必谨慎设计。

32位下，Node的大小为32字节；64位下则为56字节。

#### Policy: class

```cpp
class Policy {
public:
    // Tree-Policy中的选择阶段策略函数。
    using SelectFunc = std::function<Node*(const Node*, double)>;

    /*
        Tree-Policy中的扩展策略函数：
          ① 直接扩展一整层结点，并根据EvalResult中的概率向量为每个结点赋初值。
          ② 返回值为新增的结点数。
    */
    using ExpandFunc = std::function<size_t(Node*, std::vector<double>)>;

    /*
    	当Tree-Policy抵达中止点时，用于将棋下完（可选）并评估场面价值的Default-Policy：
          ① 在函数调用前后，一般应保证棋盘的状态不变。
          ② 返回值为 <场面价值, 各处落子的概率> 。
    */
    using EvalResult = std::tuple<double, std::vector<double>>;
    using EvalFunc = std::function<EvalResult(Board&)>;

    /*
    	Tree-Policy中的反向传播策略函数：
          ① 此阶段棋盘将被重置回初始状态（与Select过程的路线对称）
          ② 结点的各种属性均在此函数中被更新。
    */
    using UpdateFunc = std::function<void(Node*, Board&, double)>;

    // 当其中某一项传入nullptr时，该项将使用一个默认策略初始化。
    Policy(SelectFunc = nullptr, ExpandFunc = nullptr, EvalFunc = nullptr, UpdateFunc = nullptr);

    SelectFunc select;
    ExpandFunc expand;
    EvalFunc   simulate;
    UpdateFunc backPropogate;

public:
    std::any container; // 用以存放Actions的容器，可以在不同的Policy中表现为任何形式。
};
```

蒙特卡洛树策略。该类将MCTS一轮Playout中四种主要方法抽象出来，使得我们可以用各种不同的算法进行填充，形成不同的策略。

#### MCTS: class

```cpp
class MCTS {
public:
    MCTS(
        Position last_move    = -1,
        Player   last_player  = Player::White,
        Policy*  policy       = nullptr,
        size_t   c_iterations = 20000,
        double   c_puct       = sqrt(2)
    );

    Position getNextMove(Board& board);

    // Tree-policy的评估函数
    Policy::EvalResult evalState(Board& board);
    
    // 将MCTS往深推进一层
    Node* stepForward();                      // 选出子结点中的最好手
    Node* stepForward(Position next_move);    // 根据提供的动作往下走

    // 蒙特卡洛树的一轮迭代
    size_t playout(Board& board);

public:
    std::unique_ptr<Node> m_root;
    std::unique_ptr<Policy> m_policy;
    size_t m_size;
    size_t c_iterations;
    double c_puct;
};
```

在Node与Policy独立实现之后，MCTS类仅仅是这两个类上的一层Wrapper。它提供参数的配置，将Node与Policy的各单元有机地组合起来。



## Network

Network模块实现了AlphaZero中策略价值网络部分，以及配套训练方法的模块。

### model.py

```python
# 定义一个卷积块。
def ConvBlock(
        filter_size=256,
        kernel_size=(3, 3),
        activation=None
    ) -> list:
    pass

# 定义一个残差卷积块。
def ResBlock(identity_input) -> list:
    pass

# 策略价值网络类。
class PolicyValueNetwork:
    # 构造函数，在此完成网络的构建。
    def __init__(self, config):
        input_layer = Input(...) # 输入结构定义
        shared_net = Sequential(input_layer, ...) # 公共网络定义
        policy_head = Sequential(shared_net, ...) # 策略端定义
        value_head = Sequential(shared_net, ...)  # 价值端定义
        
        # 模型最终定义
        self.model = Model(
        	input_layer,
            [policy_head, value_head]
        )
    
    # 评估传入的场面价值与接下来每处的落子概率（即为MCTS的Simulate Policy）
    def eval_state(self, state):
        pass
```



## Agents

Agent模块包装了各种不同的算法策略，形成一个抽象的玩家。

基类中定义了一套与Botzone进行交互的方法，使得每一个子类文件都可作为入口点文件提交到Botzone上。

（待更新）

### agent.py

Agent基类。

### random_agent.py

最基础的随机下棋玩家。



# Review

## Improvements Accumulated

### Week 3

我们目前的Core模块版本相对于之前的几个比较大的版本，累积了如下的改进：

#### Board类：时空开销决策

* 最初，我们考虑到了「蒙特卡洛树是很适合做多线程的」这一特点，便自然想在代码设计时对这一特性加以利用。并行处理中最重要的一点就是线程安全，而为了保证这点，在每次迭代中单独拷贝一份Board对象出来是一种不错的做法。

  因此，在最初的Board类的设计中，我们的做法是使Board的拷贝成本尽量低。但这对结构的设计与性能的优化带来了不小的考验。

  查阅相关资料得知，最终的比赛平台Botzone提供的机器CPU为单核，故对AI下棋这种「计算密集型」任务，多线程处理并不会带来什么收益。因此，就算我们利用多线程版本来加速AI的训练，最终比赛时还是需要提交一份高性能的单线程版本代码。

  经过权衡以后，我们最终的选择是：无视棋盘的空间开销，写一个单线程、优化到极致性能的Board类，并保证一盘棋局中Board经过初始化后，永远不会被拷贝。至于并行化处理功能，由于AI的训练是计算密集型任务，能开的线程数量大致等于CPU的核心数，因而在不同的CPU核上分别开几个Board进行训练即可。

#### Board类：存储结构改进

* 最初，在我们定下来「写高性能、不在意空间开销的Board类」的设计方针后，我们使用了哈希表来作为棋盘的核心存储结构：

    ```cpp
    std::unordered_map<Position, Player> m_appliedMoves;
    std::unordered_set<Position> m_availableMoves;
    ```

    哈希表有着无需存下整个棋盘位置，并且能在O(1)时间内根据位置索引到状态的优势，因此我们一开始认为使用它的效率会很高。但是，经过Profiler的测试，我们发现STL哈希表相关的接口占用了相当巨大的开销，而且这些接口的调用是完全无法避免的。

    因此，我们决定更换一个存储容器。经过仔细的分析，我们最终选择了`std::array`，并作了如下的设计：

    1. 将棋盘盘面切割为三个不同的状态数组，每个数组记录了棋盘的所有位置。但是，我们不存棋盘上某个位置现在是什么棋子（状态），而是存「某个棋子（状态）是否放置在某个位置上」。

       也就是说，曾经的检索顺序是Position->Player，现在的检索顺序是Player->Position->true or false。

       这样做有不少的好处。首先是**棋盘状态的检索及修改可以变得非常快**：不管采用何种检索方案，都是真正的随机访问（数组下标索引），而状态的修改仅仅涉及到了布尔值的修改。以及，按照一些资料所述，**这种结构（One-hot编码）更适合用来表述神经网络**[[1]](#Literatures Reference)。

    2. 额外添加一个数组，用以记录每个棋子（状态）在棋盘中已经有了多少（即每个状态数组中值为true的元素数量）。这避免了每次求棋盘空位数量时，对数组进行的遍历。

    这种设计无疑会让Board的体积进一步膨胀。但是，考虑到现在Board变成了一个纯栈上对象（数组为静态栈上数组），在拷贝时反而会得到一定的优化。此外，若实在对体积有所要求，存储容器也可由`std::array`改为`std::bitset`，因为在目前的棋盘设计下，后者就是前者的紧凑版本。

> 关于Profiler的具体测试结果，请参阅*Performance Profile*中的[*Profiler Report*](#Profiler Report)章节。
>
> 关于详细的时空开销分析，请参阅*Performance Profile*中的[*Cost Analysis*](#Board 时空开销分析)章节。

### Week 5

相对于前一个版本，此阶段我们主要累积了一些工程上的改进，如Node，Policy，MCTS类的开放性结构设计。

详情请见Week7报告。

## Literatures Reference

### Already In Use

- [x] \[1\][*PENG Bo*: 从围棋盘看卷积神经网络CNN的具体工作过程](https://zhuanlan.zhihu.com/p/25345778)

  虽然目前没有用上CNN，研究对象也不是围棋，但文中提出的「One-hot编码」对我们组的棋盘设计有着很大的启发。采用这种形式构造的棋盘，带来了许多意料之外的好处（尤其是速度）。

### Work-In-Progress

- [ ] [Mastering the game of Go without human knowledge](https://www.gwern.net/docs/rl/2017-silver.pdf)

- [ ] [Mastering Chess and Shogi by Self-Play with a General Reinforcement Learning Algorithm](https://arxiv.org/pdf/1712.01815.pdf?utm_source=utkukhraman)

  AlphaZero的两篇论文。其中，第一篇论文（AlphaGo Zero）提供了详细的复现方法，目前正被实现中。


- [ ] [*Browne et al*: A Survey of Monte Carlo Tree Search Methods](http://mcts.ai/pubs/mcts-survey-master.pdf)

  关于MCTS方法的大宗型调研论文。涵盖了从select到backPropogate的MCTS所有子过程的理解、优化与应用。是我们组未来对AI进行优化时最为重要的参考大纲。

### Candidates For Future

- [ ] [*Gelly et al*: Monte-Carlo tree search and rapid action value estimation in computer Go](http://www.ics.uci.edu/~dechter/courses/ics-295/winter-2018/papers/mcts-gelly-silver.pdf)

  RAVE算法。这是作者在2011年发布的版本。整个算法似乎利用到了「重用子问题」的思想，是非常值得研究的一种优化策略。

- [ ] [*Frydenberg et al*: Investigating MCTS Modiﬁcations in General Video Game Playing]()

- [ ] [*Lanzi et al*: Evolving UCT Alternatives for General Video Game Playing](https://www.politesi.polimi.it/bitstream/10589/133902/3/tesi.pdf)

  以上两篇论文都提出了一些UCB1的修改算法。若能从中找到一个较好的无需父结点数据的公式，则可以对MCTS的select过程实现一个很重要的性能优化：通过维护一个大根堆来快速选出最优子结点。


# Performance Profile

## Cost Analysis

### Board 时空开销分析

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

### MCTS::expand 扩展策略分析

相关变量解释：

* \#1：一次expand只扩展一个结点，扩展完一层时各函数的调用次数；
* \#2：一次expand直接扩展一层结点，扩展完一层时各函数的调用次数。
* $E_1,E_2$：对应\#1 \#2情况下的expand函数时间复杂度。
* $h$：从根节点算起，当前蒙特卡洛树的树高。
* $n_i$：当前结点下，可下的位置数量。

|      select       |    函数    |          expand          |
| :---------------: | :--------: | :----------------------: |
| $S(n_i)=O(N-n_i)$ | 单次复杂度 | $E_1=O(1)$, $E_2=O(N-n)$ |
|     $(N-n)h$      |    \#1     |          $N-n$           |
|        $h$        |    \#2     |            1             |

默认根节点为棋盘初始状态时，总时间开销：

1. $(N-n)\sum_{n_i=h}^NS(n_i)+(N-n)E_1$
2. $\sum_{n_i=h}^NS(n_i)+E_2=\sum_{n_i=h}^NS(n_i)+(N-n)E_1$

目前，select()开销占比约50%。设：

1. MCTS从棋局刚开始时开始进行足够多次playout。
2. 最远扩展到了$n$层，第$i$层的结点来源于第$i-1$层的某$i-1$个结点。

则：

1. 第$i$层有$(i-1)(N-i)$个结点
2. select()在每一层都执行了大约同样多的次数（树高相较于playout次数很小）。

设优化前总时间为$T$, 优化后的时间占比应约为：

$$\frac{\frac{T}{2n}\sum_{i=0}^n\frac{1}{N-i}}{\frac{T}{2}+\frac{T}{2n}\sum_{i=0}^n\frac{1}{N-i}}≈\frac{\ln(\frac{N+1}{N-n})}{n+\ln(\frac{N+1}{N-n})}=1-\frac{1}{1+\ln(\frac{N+1}{N-n})/n}$$

当$N=255,n=127$时，上式约为$1-\frac{1}{1+\ln2/127}≈0.3\%$。



## Profiler Report


### 2018/3/11 DEBUG下性能瓶颈：
* playout() -> 92.01%
  * simulate() -> 71.14%
    * getRandomMove() -> 40.65%
	* applyMove() -> 28.26%
  * reset() -> 18.69%
  * expand() -> 0.18%

* 总计：applyMove() -> 34.48%
  * checkMove()（DEBUG版） -> 2.18%
  * hashset[xxx] = xxx -> 6.90%
  * **hashset.erase() -> 11.43%**
  * **checkVictory() -> 13.79%**
    * 几乎全部集中在以下代码中：
	  ```cpp
		(x >= 0 && x < width) && (y >= 0 && y < height) 
        && (m_appliedMoves.count(pose) && m_appliedMoves[pose] == m_curPlayer);
	  ```

* 总计：getRandomMove() -> 41.38%
  * **hashset.bucket_size() -> 36.30%**
  * bucket = (bucket + 1) % hashset.bucket_count() -> 2.54%
  * hashset.begin(bucket), rand () % bucketSize -> 1.63


**总结**：DEBUG模式下，STL的开销成为了绝对的大头。

### 2018/3/12 将棋盘容器改为std::array，并修改一些BUG后，RELEASE下的性能分析：

相对于`2018/3/11`版本，性能提升了约50倍。
在$10^6$次playout下，一次测试耗时约4秒。

为了获取更详细的函数工作开销，我们关闭了编译时内联扩展，再进行了一次测试：
尽量不考虑一些明显因禁止内联扩展而使得开销增大的函数，目前的CPU开销分布为：

* playout() -> 98.45%
* * applyMove() -> 50.50%
  * reset() -> 23.18%
  * select() -> 10.47% （禁止内联展开引起的开销）
* 总计：applyMove() -> 50.61%
  * checkVictory() -> 39.58%
    * 与`2018/3/11`版本一样，开销仍旧集中在isCurrentPlayer()函数中。
  * setState() -> 2.78%
  * unsetState() -> 1.82%
  * checkMove() -> 1.19%
* 总计：reset() -> 23.18%
  * deque::back() -> 13.02%
  * revertMove() -> 7.21%
    * setState() -> 2.53%
    * unsetState() -> 2.14%


**总结**：

1. 更换std::array后，尽管性能得到了质的提升，但applyMove()与revertMove()仍旧是两个大头。

2. 值得注意的是，在目前的实现中，checkMove()与isCurrentPlayer()所做的事几乎一模一样，但其开销总占比却差了33倍之多。

   可能的原因是：在每次applyMove()中，checkMove()与checkVictory()各被调用一次，但checkVictory会检查4个方向，每个方向至多会检查2*4个子，也就是isCurrentPlayer()最高会被调用32次，造成了巨大的开销。

3. 另外，用于取栈顶元素的deque::back()也值得注意。尽管禁止了内联展开，但其开销还是显得过大了。


**可能的性能建议**：

1. 由于目前版本下isCurrentPlayer()只在一处代码被调用，因此可以将其从函数封装中拆出来。
2. 进一步优化胜利检查的算法，减少isCurrentPlayer()等操作的频繁嗲用
3. 将std::stack的Container改为std::vector，或干脆就将std::vector作为栈使用。


### 对MCTS的playout机制进一步调优后，RELEASE下性能分析：

在$10^6$次playout下，耗时约950ms。相对于上一个版本，性能提升了4倍。
也就是说，相对于昨天的版本，性能整体提升了200倍。

未禁止内联扩展时，CPU开销分布情况：

* playout() -> 99.75%
  * backPropogate() -> 43.18%
    * 里面包含了revertMove()
  * applyMove() ->  39.43%
  * checkGameEnd() -> 7.86%
  * select() -> 7.44%
  * Node::isLeaf() -> 1.44%

禁止内联扩展后，经过Profiler测试，发现applyMove()与revertMove()的大头都明显是由无内联优化而造成的。

~~这就是说，就目前而言纯随机下法优化能带来的提升暂时不大了。~~不存在的

### 2018/3/14 MCTS中的select()方法优化

在修复了一些MCTS逻辑上的BUG后，性能开销重新增大（或者应该说变正常了……），在随机simulation的条件下，1s能完成的playout次数落回到10^4左右。

其中，select()方法占据了主要的开销（40%~50%），急需进行优化。

0. 最直观的一个优化点：

   每秒能运行的playout次数提升到了5*10^4。
   但是，select()的开销占比仍然很大，而且这样做带来了其他的一些问题。

下面是一些其他的解决方案思考：

1. 调整UCB1公式，将子结点改用大根堆存储。

   ~~具体分析，以后报告中待续……~~

2. 调整expand()公式，使每一轮playout效用更大，从而减少select()的调用次数。

   尽管每秒playout的次数会变少，但平摊到每个结点后，性能实际上是提升了的。
   该用什么方法测量呢？可以再写一个测试，添加一个结点个数的记录，到达结点个数后退出测试。
   看在相同的节点生成数目下，耗时分别是多少。

   > 关于具体的分析，请参考*Cost Analysis* 中的 *MCTS::expand 扩展策略分析* 章节。 

3. 细节性优化。如visits过少则不进行simulation啥的……（这个不是select的吧？）

---

追求极致的旅途，还远远没有结束呢！