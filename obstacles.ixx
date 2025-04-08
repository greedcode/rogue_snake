export module obstacles;

import game_core;
import <vector>;
import <random>;
import <queue>;
import <set>;

export namespace snake_game {
    class obstacle_manager {
    public:
        obstacle_manager() {
            reset();
        }

        void reset() {
            obstacles.clear();
            do {
                generate_obstacles();
            } while (!validate_obstacle_layout());
        }

        void generate_obstacles() {
            obstacles.clear();

            int base_obstacle_count = 10 + get_random_in_range(3, 6);

            for (int i = 0; i < base_obstacle_count; ++i) {
                int pattern_type = get_random_in_range(0, 4);

                position start = get_random_position();

                while (std::abs(start.x - grid_width / 2) < 5 &&
                    std::abs(start.y - grid_height / 2) < 5) {
                    start = get_random_position();
                }

                std::vector<position> pattern_positions;

                switch (pattern_type) {
                case 0: {
                    int length = get_random_in_range(5, 10);
                    bool horizontal = get_random_in_range(0, 1) == 0;

                    for (int j = 0; j < length; ++j) {
                        position obs_pos = start;
                        if (horizontal) {
                            obs_pos.x += j;
                        }
                        else {
                            obs_pos.y += j;
                        }

                        if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                            obs_pos.y >= 0 && obs_pos.y < grid_height) {
                            pattern_positions.push_back(obs_pos);
                        }
                    }
                    break;
                }
                case 1: {
                    int horz_length = get_random_in_range(3, 6);
                    int vert_length = get_random_in_range(3, 6);

                    for (int j = 0; j < horz_length; ++j) {
                        position obs_pos = start;
                        obs_pos.x += j;

                        if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                            obs_pos.y >= 0 && obs_pos.y < grid_height) {
                            pattern_positions.push_back(obs_pos);
                        }
                    }

                    for (int j = 1; j < vert_length; ++j) {
                        position obs_pos = start;
                        obs_pos.y += j;

                        if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                            obs_pos.y >= 0 && obs_pos.y < grid_height) {
                            pattern_positions.push_back(obs_pos);
                        }
                    }
                    break;
                }
                case 2: {
                    pattern_positions = generate_zigzag_pattern(start, get_random_in_range(6, 10),
                        get_random_in_range(0, 1) == 0);
                    break;
                }
                case 3: {
                    if (get_random_in_range(0, 1) == 0) {
                        pattern_positions = generate_spiral_pattern(start, get_random_in_range(3, 5));
                    }
                    else {
                        int size = get_random_in_range(3, 5);
                        for (int x = 0; x < size; x++) {
                            for (int y = 0; y < size; y++) {
                                position obs_pos = start;
                                obs_pos.x += x;
                                obs_pos.y += y;

                                if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                                    obs_pos.y >= 0 && obs_pos.y < grid_height) {
                                    pattern_positions.push_back(obs_pos);
                                }
                            }
                        }
                    }
                    break;
                }
                case 4: {
                    int arm_length = get_random_in_range(3, 5);

                    for (int j = -arm_length; j <= arm_length; ++j) {
                        position obs_pos = start;
                        obs_pos.x += j;

                        if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                            obs_pos.y >= 0 && obs_pos.y < grid_height) {
                            pattern_positions.push_back(obs_pos);
                        }
                    }

                    for (int j = -arm_length; j <= arm_length; ++j) {
                        position obs_pos = start;
                        obs_pos.y += j;

                        if (obs_pos.x >= 0 && obs_pos.x < grid_width &&
                            obs_pos.y >= 0 && obs_pos.y < grid_height) {
                            pattern_positions.push_back(obs_pos);
                        }
                    }
                    break;
                }
                }

                obstacles.insert(obstacles.end(), pattern_positions.begin(), pattern_positions.end());
            }
        }

        const std::vector<position>& get_obstacles() const {
            return obstacles;
        }

        bool check_collision(const position& pos) const {
            for (const auto& obs : obstacles) {
                if (obs == pos) {
                    return true;
                }
            }
            return false;
        }

    private:
        std::vector<position> obstacles;

        bool validate_obstacle_layout() const {
            std::set<position> visited;
            std::queue<position> to_visit;

            position start = { grid_width / 2, grid_height / 2 };
            to_visit.push(start);

            while (!to_visit.empty()) {
                position current = to_visit.front();
                to_visit.pop();

                if (visited.count(current)) {
                    continue;
                }

                visited.insert(current);

                for (const auto& neighbor : get_neighbors(current)) {
                    if (neighbor.x >= 0 && neighbor.x < grid_width &&
                        neighbor.y >= 0 && neighbor.y < grid_height &&
                        !check_collision(neighbor) && !visited.count(neighbor)) {
                        to_visit.push(neighbor);
                    }
                }
            }

            int open_cells = grid_width * grid_height - obstacles.size();
            return visited.size() >= open_cells * 0.8;
        }

        std::vector<position> get_neighbors(const position& pos) const {
            return {
                { pos.x + 1, pos.y },
                { pos.x - 1, pos.y },
                { pos.x, pos.y + 1 },
                { pos.x, pos.y - 1 }
            };
        }
    };
}