export module snake;

import game_core;
import obstacles;
import <vector>;
import <deque>;

export namespace snake_game {
    class snake {
    public:
        snake() {
            reset();
        }

        void reset() {
            body.clear();
            body.push_back({ grid_width / 2, grid_height / 2 });
            current_direction = direction::right;
            growth_pending = 0;
            ghost_mode = false;
            invincible = false;
        }

        void move() {
            position new_head = body.front();

            switch (current_direction) {
            case direction::up:
                new_head.y--;
                break;
            case direction::down:
                new_head.y++;
                break;
            case direction::left:
                new_head.x--;
                break;
            case direction::right:
                new_head.x++;
                break;
            }

            body.push_front(new_head);

            if (growth_pending > 0) {
                growth_pending--;
            }
            else {
                body.pop_back();
            }
        }

        bool check_collision(const obstacle_manager& obstacles) {
            const position& head = body.front();

            if (head.x < 0 || head.x >= grid_width ||
                head.y < 0 || head.y >= grid_height) {
                if (ghost_mode) {
                    position& wrapped_head = body.front();
                    if (wrapped_head.x < 0) wrapped_head.x = grid_width - 1;
                    if (wrapped_head.x >= grid_width) wrapped_head.x = 0;
                    if (wrapped_head.y < 0) wrapped_head.y = grid_height - 1;
                    if (wrapped_head.y >= grid_height) wrapped_head.y = 0;
                    return false;
                }
                return !invincible;
            }

            if (!invincible) {
                for (auto it = body.begin() + 1; it != body.end(); ++it) {
                    if (head == *it) {
                        return true;
                    }
                }
            }

            if (!ghost_mode && !invincible && obstacles.check_collision(head)) {
                return true;
            }

            return false;
        }

        void set_direction(direction dir) {
            if ((current_direction == direction::up && dir == direction::down) ||
                (current_direction == direction::down && dir == direction::up) ||
                (current_direction == direction::left && dir == direction::right) ||
                (current_direction == direction::right && dir == direction::left)) {
                return;
            }

            if (body.size() > 1) {
                const position& head = body.front();
                const position& neck = body[1];

                if ((dir == direction::up && head.y - 1 == neck.y) ||
                    (dir == direction::down && head.y + 1 == neck.y) ||
                    (dir == direction::left && head.x - 1 == neck.x) ||
                    (dir == direction::right && head.x + 1 == neck.x)) {
                    return;
                }
            }

            current_direction = dir;
        }

        direction get_direction() const {
            return current_direction;
        }

        void grow(int segments = 1) {
            growth_pending += segments;
        }

        const std::deque<position>& get_body() const {
            return body;
        }

        const position& get_head() const {
            return body.front();
        }

        void set_ghost_mode(bool enabled) {
            ghost_mode = enabled;
        }

        bool is_ghost_mode() const {
            return ghost_mode;
        }

        void set_invincible(bool enabled) {
            invincible = enabled;
        }

        bool is_invincible() const {
            return invincible;
        }

        void force_direction(direction dir) {
            current_direction = dir;
        }

    private:
        std::deque<position> body;
        direction current_direction;
        int growth_pending = 0;
        bool ghost_mode = false;
        bool invincible = false;
    };
}