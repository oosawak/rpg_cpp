
#include <iostream>
#include <vector>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <string>
#include <memory>
#include <map>

// OSごとのキー入力ライブラリのインクルードと定義
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
// Windows: 矢印キーのスキャンコード
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#else
#include <termios.h>
#include <unistd.h>
// POSIX: 矢印キーのエスケープシーケンス
#define ARROW_UP 'A'
#define ARROW_DOWN 'B'
#define ARROW_LEFT 'D'
#define ARROW_RIGHT 'C'
#endif

// 迷路のサイズ
const int MAZE_WIDTH = 21;
const int MAZE_HEIGHT = 21;
const int NUM_FLOORS = 5;
const int MONSTER_COUNT = 5;
const int MAX_HP = 100;
const int HP_RECOVERY_PER_STEP = 1;

// 武器の構造体
struct Weapon {
    std::string name;
    int attack_bonus;
};

// プレイヤーとモンスターの属性
struct Character {
    int hp;
    int base_attack;
    std::string name;
    Weapon equipped_weapon;
};

// 迷路の構造体 (各階の迷路データと階段の位置を保持)
struct MazeFloor {
    std::vector<std::vector<char>> maze_data;
    int up_stair_x, up_stair_y;
    int down_stair_x, down_stair_y;
};

// 迷路の生成とゲーム進行を管理するクラス
class MazeGame {
public:
    MazeGame() {
        player.name = "プレイヤー";
        player.hp = MAX_HP;
        player.base_attack = 10;
        player.equipped_weapon = { "素手", 0 };

        initializeWeapons();

        currentFloor = 1;
        playerX = 1;
        playerY = 1;

        srand(time(NULL));
    }

    void run() {
        // 全フロアの迷路を生成
        for (int i = 0; i < NUM_FLOORS; ++i) {
            generateMazeFloor(i + 1);
        }

        // ゲーム開始
        while (true) {
            std::vector<std::vector<char>>& current_maze = floors[currentFloor - 1].maze_data;

            // プレイヤーの位置を表示
            current_maze[playerY][playerX] = 'P';

            displayMaze();

            // プレイヤー情報表示
            std::cout << "--- " << currentFloor << "階 (HP: " << player.hp << "/" << MAX_HP
                << " | 装備: " << player.equipped_weapon.name
                << " (+" << player.equipped_weapon.attack_bonus << ")) ---" << std::endl;

            // ゲーム終了条件
            if (player.hp <= 0) {
                std::cout << "ゲームオーバー！プレイヤーは力尽きました。" << std::endl;
                break;
            }

            // 5階のゴールに到達した場合
            if (currentFloor == NUM_FLOORS && playerX == MAZE_WIDTH - 2 && playerY == MAZE_HEIGHT - 2) {
                std::cout << "おめでとうございます！ダンジョンをクリアしました！" << std::endl;
                break;
            }

            // キー入力と移動
            movePlayer(getInput());

            // プレイヤーの移動後にモンスターを動かす
            moveMonsters();
        }
    }

private:
    std::vector<MazeFloor> floors;
    int currentFloor;
    int playerX, playerY;
    Character player;
    std::vector<Weapon> availableWeapons;

    // --- 武器リストの初期化 ---
    void initializeWeapons() {
        availableWeapons.push_back({ "木刀", 5 });
        availableWeapons.push_back({ "銅の剣", 10 });
        availableWeapons.push_back({ "鉄の剣", 15 });
        availableWeapons.push_back({ "銀の剣", 25 });
        availableWeapons.push_back({ "伝説の剣", 50 });
    }

    // --- 迷路生成 (DFS) ---
    void generateMazeFloor(int floor_num) {
        MazeFloor newFloor;
        newFloor.maze_data.resize(MAZE_HEIGHT, std::vector<char>(MAZE_WIDTH, '#'));

        dfs(newFloor.maze_data, 1, 1);

        // スタート/ゴール/階段の設定
        if (floor_num == 1) {
            newFloor.maze_data[1][1] = 'S';
            placeStair(newFloor.maze_data, 'U', newFloor.up_stair_x, newFloor.up_stair_y);
        }
        else if (floor_num == NUM_FLOORS) {
            placeStair(newFloor.maze_data, 'D', newFloor.down_stair_x, newFloor.down_stair_y);
            newFloor.maze_data[MAZE_HEIGHT - 2][MAZE_WIDTH - 2] = 'E';
        }
        else {
            placeStair(newFloor.maze_data, 'U', newFloor.up_stair_x, newFloor.up_stair_y);
            placeStair(newFloor.maze_data, 'D', newFloor.down_stair_x, newFloor.down_stair_y);
        }

        // モンスターの配置
        placeMonsters(newFloor.maze_data);

        floors.push_back(newFloor);
    }

    int dx[4] = { 0, 0, 2, -2 };
    int dy[4] = { 2, -2, 0, 0 };

    void dfs(std::vector<std::vector<char>>& current_maze, int x, int y) {
        current_maze[y][x] = ' ';

        std::vector<int> directions = { 0, 1, 2, 3 };
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(directions.begin(), directions.end(), g);

        for (int dir : directions) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (nx > 0 && nx < MAZE_WIDTH - 1 && ny > 0 && ny < MAZE_HEIGHT - 1) {
                if (current_maze[ny][nx] == '#') {
                    current_maze[y + dy[dir] / 2][x + dx[dir] / 2] = ' ';
                    dfs(current_maze, nx, ny);
                }
            }
        }
    }

    // --- 階段の配置 ---
    void placeStair(std::vector<std::vector<char>>& current_maze, char type, int& stair_x, int& stair_y) {
        int placedCount = 0;
        while (placedCount == 0) {
            int sx = (rand() % (MAZE_WIDTH - 2)) + 1;
            int sy = (rand() % (MAZE_HEIGHT - 2)) + 1;

            if (current_maze[sy][sx] == ' ') {
                current_maze[sy][sx] = type;
                stair_x = sx;
                stair_y = sy;
                placedCount++;
            }
        }
    }

    // --- モンスターの配置 ---
    void placeMonsters(std::vector<std::vector<char>>& current_maze) {
        int placedCount = 0;
        while (placedCount < MONSTER_COUNT) {
            int mx = (rand() % (MAZE_WIDTH - 2)) + 1;
            int my = (rand() % (MAZE_HEIGHT - 2)) + 1;

            if (current_maze[my][mx] == ' ') {
                current_maze[my][mx] = 'M';
                placedCount++;
            }
        }
    }

    // --- モンスターの移動 ---
    void moveMonsters() {
        std::vector<std::vector<char>>& current_maze = floors[currentFloor - 1].maze_data;
        std::vector<std::pair<int, int>> monster_positions;

        // 現在のモンスターの位置を収集
        for (int y = 1; y < MAZE_HEIGHT - 1; ++y) {
            for (int x = 1; x < MAZE_WIDTH - 1; ++x) {
                if (current_maze[y][x] == 'M') {
                    monster_positions.push_back({ x, y });
                }
            }
        }

        // 各モンスターをランダムに移動させる
        for (const auto& pos : monster_positions) {
            int mx = pos.first;
            int my = pos.second;

            // 4方向 (上, 下, 左, 右)
            int move_dx[] = { 0, 0, -1, 1 };
            int move_dy[] = { -1, 1, 0, 0 };

            std::vector<int> directions = { 0, 1, 2, 3 };
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(directions.begin(), directions.end(), g);

            bool moved = false;
            for (int dir : directions) {
                int next_mx = mx + move_dx[dir];
                int next_my = my + move_dy[dir];

                // 移動先のチェック
                if (next_mx > 0 && next_mx < MAZE_WIDTH - 1 && next_my > 0 && next_my < MAZE_HEIGHT - 1) {
                    char target = current_maze[next_my][next_mx];

                    // 通路 (' ') または階段 ('U', 'D'), スタート ('S'), ゴール ('E') に移動可能
                    if (target == ' ' || target == 'U' || target == 'D' || target == 'S' || target == 'E') {
                        // プレイヤーの位置には移動しない
                        if (next_mx != playerX || next_my != playerY) {
                            // 現在の位置を空にする
                            current_maze[my][mx] = ' ';

                            // 新しい位置にモンスターを配置
                            current_maze[next_my][next_mx] = 'M';
                            moved = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // --- 戦闘システム ---
    bool startBattle() {
        std::cout << "\nモンスターが出現しました！戦闘開始！" << std::endl;

        Character monster;
        monster.name = "モンスター";

        // 階層に基づいたモンスターの強化
        int floor_bonus_hp = (currentFloor - 1) * 15;
        int floor_bonus_attack = (currentFloor - 1) * 5;

        monster.hp = 40 + floor_bonus_hp + (rand() % 30);
        monster.base_attack = 10 + floor_bonus_attack + (rand() % 10);

        std::cout << "モンスターHP: " << monster.hp << ", 攻撃力: " << monster.base_attack << std::endl;

        int player_total_attack = player.base_attack + player.equipped_weapon.attack_bonus;

        while (player.hp > 0 && monster.hp > 0) {
            int playerDamage = rand() % player_total_attack + 1;
            monster.hp -= playerDamage;
            std::cout << "プレイヤーの攻撃！モンスターに " << playerDamage << " ダメージを与えた。(残りHP: " << monster.hp << ")" << std::endl;

            if (monster.hp <= 0) {
                std::cout << "モンスターを倒した！" << std::endl;
                handleWeaponDrop();
                return true;
            }

            int monsterDamage = rand() % monster.base_attack + 1;
            player.hp -= monsterDamage;
            std::cout << "モンスターの攻撃！プレイヤーは " << monsterDamage << " ダメージを受けた。(残りHP: " << player.hp << ")" << std::endl;

            if (player.hp <= 0) {
                return false;
            }

#ifdef _WIN32
            Sleep(500);
#else
            usleep(500000);
#endif
        }
        return false;
    }

    // 武器ドロップと装備処理
    void handleWeaponDrop() {
        if (rand() % 100 < 50) {
            int weapon_index = rand() % availableWeapons.size();
            Weapon dropped_weapon = availableWeapons[weapon_index];

            std::cout << "\n\033[32m新しい武器を獲得しました: " << dropped_weapon.name
                << " (攻撃力+" << dropped_weapon.attack_bonus << ")\033[0m" << std::endl;

            if (dropped_weapon.attack_bonus > player.equipped_weapon.attack_bonus) {
                std::cout << "\033[36m" << dropped_weapon.name << " を装備しました。\033[0m" << std::endl;
                player.equipped_weapon = dropped_weapon;
            }
            else {
                std::cout << "現在装備中の " << player.equipped_weapon.name << " の方が強力です。" << std::endl;
            }
        }
    }

    // --- プレイヤーの移動とキー入力 ---
    void movePlayer(char key) {
        int nextX = playerX;
        int nextY = playerY;

        // WASDまたは矢印キーによる移動
        switch (key) {
            // WASD
        case 'w': case 'W': nextY--; break;
        case 's': case 'S': nextY++; break;
        case 'a': case 'A': nextX--; break;
        case 'd': case 'D': nextX++; break;

            // 矢印キー (OS依存)
#ifdef _WIN32
        case KEY_UP: nextY--; break;
        case KEY_DOWN: nextY++; break;
        case KEY_LEFT: nextX--; break;
        case KEY_RIGHT: nextX++; break;
#else
        case ARROW_UP: nextY--; break;
        case ARROW_DOWN: nextY++; break;
        case ARROW_LEFT: nextX--; break;
        case ARROW_RIGHT: nextX++; break;
#endif

        case 'q': case 'Q':
            std::cout << "ゲームを終了します。" << std::endl;
            exit(0);
        }

        // 移動先のチェック
        if (nextX >= 0 && nextX < MAZE_WIDTH && nextY >= 0 && nextY < MAZE_HEIGHT) {
            std::vector<std::vector<char>>& current_maze = floors[currentFloor - 1].maze_data;
            char target = current_maze[nextY][nextX];

            // プレイヤーが移動元のマスをリセットする
            resetPlayerPosition(current_maze);

            if (target == ' ' || target == 'S' || target == 'E') {
                updatePlayerPosition(nextX, nextY);
                recoverHP();
            }
            else if (target == 'M') {
                // モンスターとの戦闘
                bool result = startBattle();
                if (result) {
                    // 勝利した場合
                    // モンスターがいた場所を通路に戻す
                    current_maze[nextY][nextX] = ' ';
                    updatePlayerPosition(nextX, nextY);
                    recoverHP();
                }
                else {
                    // 戦闘敗北時は移動しないため、リセットした場所にプレイヤーを再描画する
                    current_maze[playerY][playerX] = 'P';
                }
            }
            else if (target == 'U' && currentFloor < NUM_FLOORS) {
                gotoNextFloor();
            }
            else if (target == 'D' && currentFloor > 1) {
                gotoPreviousFloor();
            }
            else {
                // 壁 ('#') やその他の無効な移動
                // リセットした場所にプレイヤーを再描画する
                current_maze[playerY][playerX] = 'P';
            }
        }
    }

    // プレイヤーの位置を更新
    void updatePlayerPosition(int newX, int newY) {
        playerX = newX;
        playerY = newY;
    }

    // 移動前のP表示をリセットする
    void resetPlayerPosition(std::vector<std::vector<char>>& current_maze) {
        char cell_at_current_pos = current_maze[playerY][playerX];

        // プレイヤーがいた場所がスタートマス ('S') でなければ、通路 (' ') に戻す
        if (cell_at_current_pos != 'S') {
            current_maze[playerY][playerX] = ' ';
        }

        // 階段の上にいる場合は元の階段表示に戻す
        MazeFloor& current_floor_data = floors[currentFloor - 1];
        if (playerX == current_floor_data.up_stair_x && playerY == current_floor_data.up_stair_y) {
            current_maze[playerY][playerX] = 'U';
        }
        else if (playerX == current_floor_data.down_stair_x && playerY == current_floor_data.down_stair_y) {
            current_maze[playerY][playerX] = 'D';
        }
    }

    // HP回復処理
    void recoverHP() {
        if (player.hp < MAX_HP) {
            player.hp += HP_RECOVERY_PER_STEP;
            if (player.hp > MAX_HP) {
                player.hp = MAX_HP;
            }
        }
    }

    // 階段移動 (次のフロアへ)
    void gotoNextFloor() {
        // 現在のフロアの上り階段を 'U' に戻す
        MazeFloor& current_floor_data = floors[currentFloor - 1];
        current_floor_data.maze_data[current_floor_data.up_stair_y][current_floor_data.up_stair_x] = 'U';

        currentFloor++;
        // 次のフロアの下り階段の位置に移動
        MazeFloor& nextFloor = floors[currentFloor - 1];
        playerX = nextFloor.down_stair_x;
        playerY = nextFloor.down_stair_y;

        std::cout << "\n\033[33m階段を上り、" << currentFloor << "階に到達しました。\033[0m" << std::endl;
    }

    // 階段移動 (前のフロアへ)
    void gotoPreviousFloor() {
        // 現在のフロアの下り階段を 'D' に戻す
        MazeFloor& current_floor_data = floors[currentFloor - 1];
        current_floor_data.maze_data[current_floor_data.down_stair_y][current_floor_data.down_stair_x] = 'D';

        currentFloor--;
        // 前のフロアの上り階段の位置に移動
        MazeFloor& prevFloor = floors[currentFloor - 1];
        playerX = prevFloor.up_stair_x;
        playerY = prevFloor.up_stair_y;

        std::cout << "\n\033[33m階段を下り、" << currentFloor << "階に戻りました。\033[0m" << std::endl;
    }

    // キーボード入力の取得 (OS依存)
    char getInput() {
#ifdef _WIN32
        int key = _getch();
        if (key == 0 || key == 224) {
            return _getch();
        }
        return (char)key;
#else
        struct termios oldt, newt;
        char ch;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        ch = getchar();

        if (ch == 27) {
            if (getchar() == 91) {
                ch = getchar();
            }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ch;
#endif
    }

    // 迷路の表示と画面クリア
    void displayMaze() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        const std::vector<std::vector<char>>& current_maze = floors[currentFloor - 1].maze_data;

        for (const auto& row : current_maze) {
            for (char cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "\nWASD または 矢印キーで移動 (Qで終了), P:プレイヤー, M:モンスター, S:スタート, E:ゴール, U:上り階段, D:下り階段, #:壁" << std::endl;
    }
};

int main() {
    MazeGame game;
    game.run();

    return 0;
}