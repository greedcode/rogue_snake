export module game_core;

import <vector>;
import <random>;
import <chrono>;
import <array>;

export namespace snake_game {
    struct position {
        int x;
        int y;

        bool operator==(const position& other) const {
            return x == other.x && y == other.y;
        }

        bool operator<(const position& other) const {
            return (x < other.x) || (x == other.x && y < other.y);
        }
    };

    enum class direction {
        up,
        down,
        left,
        right
    };

    enum class game_mode {
        menu,
        playing,
        paused,
        game_over
    };

    enum class power_up_type {
        none,
        speed_boost,
        ghost_mode,
        extra_points,
        multi_grow,
        invincibility
    };

    constexpr int grid_width = 20;
    constexpr int grid_height = 20;
    constexpr int cell_size = 20;

    constexpr int max_level = 5;

    constexpr float power_up_duration = 20.0f;     // Default duration for most power-ups
    constexpr float extra_points_duration = 30.0f; // Extended duration for extra points power-up
    constexpr float invincibility_duration = 25.0f; // Extended duration for invincibility power-up
    constexpr float speed_boost_factor = 0.5f;
    constexpr int extra_points_amount = 100;
    constexpr int multi_grow_segments = 3;

    struct game_state {
        game_mode mode = game_mode::menu;
        int score = 0;
        float game_speed = 0.2f;
        int level = 1;
        int food_collected = 0;
        int food_to_next_level = 5;
        power_up_type active_power_up = power_up_type::none;
        float power_up_timer = 0.0f;
        bool is_invincible = false;
        bool waiting_for_input = true;
        int previous_snake_size = 1;
    };

    inline std::mt19937 create_random_engine() {
        std::random_device rd;
        return std::mt19937(rd());
    }

    template<typename T>
    T get_random_in_range(T min, T max) {
        static auto engine = create_random_engine();
        std::uniform_int_distribution<T> dist(min, max);
        return dist(engine);
    }

    inline position get_random_position() {
        return {
            get_random_in_range(0, grid_width - 1),
            get_random_in_range(0, grid_height - 1)
        };
    }

    inline std::vector<position> generate_zigzag_pattern(position start, int length, bool horizontal) {
        std::vector<position> pattern;
        for (int i = 0; i < length; i++) {
            position pos = start;
            if (horizontal) {
                pos.x += i;
                pos.y += (i % 2 == 0) ? 0 : 1;
            }
            else {
                pos.y += i;
                pos.x += (i % 2 == 0) ? 0 : 1;
            }

            if (pos.x >= 0 && pos.x < grid_width && pos.y >= 0 && pos.y < grid_height) {
                pattern.push_back(pos);
            }
        }
        return pattern;
    }

    inline std::vector<position> generate_spiral_pattern(position center, int radius) {
        std::vector<position> pattern;

        int x = 0, y = 0;
        int dx = 0, dy = -1;
        int step = 1;

        for (int i = 0; i < radius * 8; i++) {
            if ((-radius / 2 <= x && x <= radius / 2) && (-radius / 2 <= y && y <= radius / 2)) {
                position pos = center;
                pos.x += x;
                pos.y += y;

                if (pos.x >= 0 && pos.x < grid_width && pos.y >= 0 && pos.y < grid_height) {
                    pattern.push_back(pos);
                }
            }

            if (x == y || (x < 0 && x == -y) || (x > 0 && x == 1 - y)) {
                int temp = dx;
                dx = -dy;
                dy = temp;
            }

            x += dx;
            y += dy;
        }

        return pattern;
    }
}