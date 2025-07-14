#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm> // std::max用

// --------------------------------------------------
// キャラクター基底クラス (Character)
// --------------------------------------------------
class Character {
protected:
    std::string name;
    int max_hp;
    int current_hp;
    int attack_power;
    int defense; // 防御力 (追加)
    
public:
    // コンストラクタ
    Character(std::string n, int hp, int atk, int def) 
        : name(n), max_hp(hp), current_hp(hp), attack_power(atk), defense(def) {}

    // 攻撃メソッド
    // クリティカルヒット判定も行う
    void attack(Character* target) {
        int damage = attack_power;
        bool is_critical = false;
        
        // 10%の確率でクリティカルヒット
        if (std::rand() % 10 == 0) {
            damage = static_cast<int>(damage * 1.5);
            is_critical = true;
        }

        std::cout << name << " の攻撃！";
        if (is_critical) {
            std::cout << " (クリティカルヒット！)";
        }
        std::cout << " -> ";
        
        // ダメージ計算（攻撃力 - ターゲットの防御力）
        // ダメージは最低1
        int effective_damage = std::max(1, damage - target->get_defense());
        
        target->take_damage(effective_damage);
    }

    // ダメージを受けるメソッド
    void take_damage(int damage) {
        current_hp -= damage;
        std::cout << name << " に " << damage << " のダメージ！ (残りHP: " << current_hp << ")" << std::endl;
        if (current_hp < 0) {
            current_hp = 0;
        }
    }

    // 生存確認
    bool is_alive() const {
        return current_hp > 0;
    }

    // ゲッターメソッド
    std::string get_name() const { return name; }
    int get_hp() const { return current_hp; }
    int get_defense() const { return defense; }
};

// --------------------------------------------------
// プレイヤー (Player)
// --------------------------------------------------
class Player : public Character {
private:
    int current_mp; // MP (追加)

public:
    // コンストラクタ
    Player(std::string n, int hp, int atk, int def, int mp) 
        : Character(n, hp, atk, def), current_mp(mp) {}

    // 魔法攻撃メソッド (追加)
    bool cast_spell(Character* target) {
        int mp_cost = 10;
        int spell_damage = 40; // 魔法ダメージは固定

        if (current_mp < mp_cost) {
            std::cout << "MPが足りない！" << std::endl;
            return false;
        }

        current_mp -= mp_cost;
        std::cout << name << " は魔法を唱えた！ -> ";
        
        // 魔法は防御力を無視する（今回は）
        target->take_damage(spell_damage);
        
        return true;
    }

    int get_mp() const { return current_mp; }
};

// --------------------------------------------------
// モンスター (Monster)
// --------------------------------------------------
class Monster : public Character {
public:
    Monster(std::string n, int hp, int atk, int def) : Character(n, hp, atk, def) {}
};

// --------------------------------------------------
// メインの戦闘ロジック
// --------------------------------------------------
int main() {
    // 乱数シードの設定
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // プレイヤーとモンスターの作成 (HP, 攻撃力, 防御力, MP)
    Player player("勇者", 100, 20, 10, 30);
    // (HP, 攻撃力, 防御力)
    Monster monster("ゴブリン", 70, 18, 5);

    std::cout << "--- 戦闘開始！ ---" << std::endl;

    // 戦闘ループ
    while (player.is_alive() && monster.is_alive()) {
        std::cout << "\n--- " << player.get_name() << "のターン ---" << std::endl;
        std::cout << "1. 攻撃  2. 魔法  (現在のMP: " << player.get_mp() << ")" << std::endl;
        
        int choice;
        std::cin >> choice;

        if (choice == 1) {
            // 物理攻撃
            player.attack(&monster);
        } else if (choice == 2) {
            // 魔法攻撃
            player.cast_spell(&monster);
        } else {
            std::cout << "無効な入力です。攻撃します。" << std::endl;
            player.attack(&monster);
        }

        // モンスターが倒れたかチェック
        if (!monster.is_alive()) {
            std::cout << "\n--- " << monster.get_name() << "を倒した！ ---" << std::endl;
            break;
        }

        std::cout << "\n--- " << monster.get_name() << "のターン ---" << std::endl;
        // モンスターの攻撃
        monster.attack(&player);

        // プレイヤーが倒れたかチェック
        if (!player.is_alive()) {
            std::cout << "\n--- " << player.get_name() << "は倒れた... ゲームオーバー ---" << std::endl;
            break;
        }
    }

    std::cout << "\n--- 戦闘終了 ---" << std::endl;
    return 0;
}