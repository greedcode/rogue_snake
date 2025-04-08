export module food;

import game_core;
import snake;
import obstacles;
import <vector>;
import <deque>;
import <set>;
import <queue>;

export namespace snake_game {
    struct food_item {
        position position;
        bool is_power_up = false;
        power_up_type power_up_type = power_up_type::none;
    };

    class food {
    public:
        food() {
            spawn_initial_food({}, {});
        }

        void spawn_initial_food(const std::deque<position>& snake_body, const std::vector<position>& obstacles) {
            food_items.clear();

            int food_count = 1 + get_random_in_range(0, 2);

            for (int i = 0; i < food_count; i++) {
                spawn_single_food(snake_body, obstacles);
            }
        }

        void spawn_single_food(const std::deque<position>& snake_body, const std::vector<position>& obstacles) {
            food_item new_food;
            new_food.position = find_valid_position(snake_body, obstacles);

            if (get_random_in_range(0, 9) < 3) {
                new_food.is_power_up = true;
                new_food.power_up_type = static_cast<snake_game::power_up_type>(
                    get_random_in_range(1, 5));
            }
            else {
                new_food.is_power_up = false;
                new_food.power_up_type = power_up_type::none;
            }

            food_items.push_back(new_food);
        }

        const std::vector<food_item>& get_all_food() const {
            return food_items;
        }

        bool check_eaten(const position& head, const std::deque<position>& snake_body, const std::vector<position>& obstacles) {
            for (size_t i = 0; i < food_items.size(); i++) {
                if (head == food_items[i].position) {
                    is_power_up = food_items[i].is_power_up;
                    power_up_type = food_items[i].power_up_type;
                    current_position = food_items[i].position;

                    food_items.erase(food_items.begin() + i);

                    if (food_items.size() < 3) {
                        spawn_single_food(snake_body, obstacles);
                    }

                    return true;
                }
            }
            return false;
        }

        bool is_power_up_food() const {
            return is_power_up;
        }

        power_up_type get_power_up_type() const {
            return power_up_type;
        }

    private:
        position find_valid_position(const std::deque<position>& snake_body,
            const std::vector<position>& obstacles) {
            position new_pos;
            bool valid_position = false;
            int max_attempts = 1000;
            int attempts = 0;

            while (!valid_position && attempts < max_attempts) {
                new_pos = get_random_position();
                valid_position = true;

                for (const auto& segment : snake_body) {
                    if (new_pos == segment) {
                        valid_position = false;
                        break;
                    }
                }

                if (valid_position) {
                    for (const auto& obs : obstacles) {
                        if (new_pos == obs) {
                            valid_position = false;
                            break;
                        }
                    }
                }

                if (valid_position) {
                    for (const auto& food : food_items) {
                        if (new_pos == food.position) {
                            valid_position = false;
                            break;
                        }
                    }
                }

                if (valid_position && !snake_body.empty()) {
                    if (is_dead_end(new_pos, snake_body, obstacles)) {
                        valid_position = false;
                    }

                    if (valid_position && is_in_confined_space(new_pos, snake_body, obstacles)) {
                        valid_position = false;
                    }

                    if (valid_position && !is_reachable(new_pos, snake_body, obstacles)) {
                        valid_position = false;
                    }
                }

                attempts++;
            }

            if (!valid_position) {
                for (int y = 0; y < grid_height; y++) {
                    for (int x = 0; x < grid_width; x++) {
                        position pos = { x, y };
                        bool is_open = !is_blocked(pos, snake_body, obstacles);

                        if (is_open && (snake_body.empty() || is_reachable(pos, snake_body, obstacles))) {
                            return pos;
                        }
                    }
                }

                if (snake_body.empty()) {
                    return { grid_width / 2, grid_height / 2 };
                }
                else {
                    position fallback = snake_body.front();
                    direction dir = get_snake_direction(snake_body);

                    switch (dir) {
                    case direction::up:    fallback.y -= 2; break;
                    case direction::down:  fallback.y += 2; break;
                    case direction::left:  fallback.x -= 2; break;
                    case direction::right: fallback.x += 2; break;
                    }

                    fallback.x = std::max(0, std::min(grid_width - 1, fallback.x));
                    fallback.y = std::max(0, std::min(grid_height - 1, fallback.y));
                    return fallback;
                }
            }

            return new_pos;
        }

        direction get_snake_direction(const std::deque<position>& snake_body) const {
            if (snake_body.size() < 2) return direction::right;

            const position& head = snake_body[0];
            const position& neck = snake_body[1];

            if (head.x > neck.x) return direction::right;
            if (head.x < neck.x) return direction::left;
            if (head.y > neck.y) return direction::down;
            return direction::up;
        }

        bool is_dead_end(const position& pos, const std::deque<position>& snake_body,
            const std::vector<position>& obstacles) const {
            int blocked_sides = 0;

            std::vector<position> neighbors = {
                { pos.x + 1, pos.y }, // Right
                { pos.x - 1, pos.y }, // Left
                { pos.x, pos.y + 1 }, // Down
                { pos.x, pos.y - 1 }  // Up
            };

            for (const auto& neighbor : neighbors) {
                if (neighbor.x < 0 || neighbor.x >= grid_width ||
                    neighbor.y < 0 || neighbor.y >= grid_height ||
                    is_blocked(neighbor, snake_body, obstacles)) {
                    blocked_sides++;
                }
            }

            return blocked_sides >= 3;
        }

        bool is_in_confined_space(const position& pos, const std::deque<position>& snake_body,
            const std::vector<position>& obstacles) const {
            std::set<position> accessible_area;
            std::queue<position> to_visit;
            to_visit.push(pos);

            while (!to_visit.empty() && accessible_area.size() < 10) {
                position current = to_visit.front();
                to_visit.pop();

                if (accessible_area.count(current)) {
                    continue;
                }

                accessible_area.insert(current);

                for (const auto& neighbor : get_neighbors(current)) {
                    if (neighbor.x >= 0 && neighbor.x < grid_width &&
                        neighbor.y >= 0 && neighbor.y < grid_height &&
                        !accessible_area.count(neighbor) &&
                        !is_blocked(neighbor, snake_body, obstacles)) {
                        to_visit.push(neighbor);
                    }
                }
            }

            return accessible_area.size() < 10;
        }

        bool is_reachable(const position& food_pos, const std::deque<position>& snake_body,
            const std::vector<position>& obstacles) const {
            if (snake_body.empty()) {
                return true;
            }

            std::set<position> visited;
            std::queue<position> to_visit;
            to_visit.push(snake_body.front());

            while (!to_visit.empty()) {
                position current = to_visit.front();
                to_visit.pop();

                if (visited.count(current)) {
                    continue;
                }

                visited.insert(current);

                if (current == food_pos) {
                    return true;
                }

                for (const auto& neighbor : get_neighbors(current)) {
                    if (neighbor.x >= 0 && neighbor.x < grid_width &&
                        neighbor.y >= 0 && neighbor.y < grid_height &&
                        !visited.count(neighbor) &&
                        !is_blocked(neighbor, snake_body, obstacles)) {
                        to_visit.push(neighbor);
                    }
                }
            }

            return false;
        }

        bool is_blocked(const position& pos, const std::deque<position>& snake_body,
            const std::vector<position>& obstacles) const {
            for (const auto& segment : snake_body) {
                if (pos == segment) {
                    return true;
                }
            }

            for (const auto& obs : obstacles) {
                if (pos == obs) {
                    return true;
                }
            }

            return false;
        }

        std::vector<position> get_neighbors(const position& pos) const {
            return {
                { pos.x + 1, pos.y },
                { pos.x - 1, pos.y },
                { pos.x, pos.y + 1 },
                { pos.x, pos.y - 1 }
            };
        }

        position current_position;
        bool is_power_up = false;
        power_up_type power_up_type = power_up_type::none;

        std::vector<food_item> food_items;
    };
}