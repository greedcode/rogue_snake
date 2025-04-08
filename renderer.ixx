export module renderer;

import game_core;
import snake;
import food;
import obstacles;
import <string>;
import <imgui.h>;

export namespace snake_game {
    class game_renderer {
    public:
        void render_game(const snake& game_snake, const food& game_food,
            const obstacle_manager& obstacles, game_state& state) {
            const float window_width = grid_width * cell_size;
            const float window_height = grid_height * cell_size;

            ImGuiIO& io = ImGui::GetIO();
            ImVec2 center_pos(
                (io.DisplaySize.x - (window_width + 20)) * 0.5f,
                (io.DisplaySize.y - (window_height + 60)) * 0.5f
            );

            ImGui::SetNextWindowPos(center_pos, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(window_width + 20, window_height + 60));
            ImGui::Begin("Rogue Snake", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

            if (state.mode == game_mode::menu) {
                if (render_menu(window_width, window_height)) {
                    state.mode = game_mode::playing;
                }
            }
            else {
                ImGui::Text("Score: %d   Level: %d   Food: %d/%d", state.score, state.level,
                    state.food_collected, state.food_to_next_level);

                if (state.active_power_up != power_up_type::none) {
                    const char* power_text = "";
                    ImVec4 power_color(1.0f, 1.0f, 1.0f, 1.0f);

                    switch (state.active_power_up) {
                    case power_up_type::speed_boost:
                        power_text = "SPEED BOOST";
                        power_color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
                        break;
                    case power_up_type::ghost_mode:
                        power_text = "GHOST MODE";
                        power_color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
                        break;
                    case power_up_type::extra_points:
                        power_text = "EXTRA POINTS";
                        power_color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                        break;
                    case power_up_type::multi_grow:
                        power_text = "MULTI GROW";
                        power_color = ImVec4(0.8f, 0.2f, 0.8f, 1.0f);
                        break;
                    case power_up_type::invincibility:
                        power_text = "INVINCIBLE";
                        power_color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                        break;
                    }

                    ImGui::SameLine(window_width - 120);
                    ImGui::TextColored(power_color, "%s: %.1f", power_text, state.power_up_timer);
                }

                ImGui::BeginChild("GameBoard", ImVec2(window_width, window_height));

                auto draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

                draw_list->AddRectFilled(
                    canvas_pos,
                    ImVec2(canvas_pos.x + window_width, canvas_pos.y + window_height),
                    IM_COL32(40, 40, 40, 255)
                );

                const auto& obs_list = obstacles.get_obstacles();
                for (const auto& obs : obs_list) {
                    draw_list->AddRectFilled(
                        ImVec2(canvas_pos.x + obs.x * cell_size, canvas_pos.y + obs.y * cell_size),
                        ImVec2(canvas_pos.x + (obs.x + 1) * cell_size - 1, canvas_pos.y + (obs.y + 1) * cell_size - 1),
                        IM_COL32(120, 120, 120, 255)
                    );
                }

                const auto& snake_body = game_snake.get_body();

                ImU32 snake_color;
                if (game_snake.is_invincible()) {
                    float time = ImGui::GetTime();
                    snake_color = IM_COL32(
                        (int)(127.5f + 127.5f * sin(time * 2.0f)),
                        (int)(127.5f + 127.5f * sin(time * 2.0f + 2.0f)),
                        (int)(127.5f + 127.5f * sin(time * 2.0f + 4.0f)),
                        255
                    );
                }
                else if (game_snake.is_ghost_mode()) {
                    snake_color = IM_COL32(80, 80, 255, 180);
                }
                else {
                    snake_color = IM_COL32(0, 255, 0, 255);
                }

                for (const auto& segment : snake_body) {
                    draw_list->AddRectFilled(
                        ImVec2(canvas_pos.x + segment.x * cell_size, canvas_pos.y + segment.y * cell_size),
                        ImVec2(canvas_pos.x + (segment.x + 1) * cell_size - 1, canvas_pos.y + (segment.y + 1) * cell_size - 1),
                        snake_color
                    );
                }

                const auto& food_items = game_food.get_all_food();
                for (const auto& food_item : food_items) {
                    ImU32 food_color = IM_COL32(255, 0, 0, 255);

                    if (food_item.is_power_up) {
                        switch (food_item.power_up_type) {
                        case power_up_type::speed_boost:
                            food_color = IM_COL32(0, 255, 255, 255);
                            break;
                        case power_up_type::ghost_mode:
                            food_color = IM_COL32(128, 128, 255, 255);
                            break;
                        case power_up_type::extra_points:
                            food_color = IM_COL32(255, 215, 0, 255);
                            break;
                        case power_up_type::multi_grow:
                            food_color = IM_COL32(204, 51, 204, 255);
                            break;
                        case power_up_type::invincibility:
                            food_color = IM_COL32(255, 105, 105, 255);
                            break;
                        }

                        float pulse = (sinf(ImGui::GetTime() * 4.0f) * 0.5f + 0.5f) * 50.0f;

                        draw_list->AddCircleFilled(
                            ImVec2(canvas_pos.x + food_item.position.x * cell_size + cell_size / 2,
                                canvas_pos.y + food_item.position.y * cell_size + cell_size / 2),
                            cell_size / 2 + pulse / 10.0f,
                            IM_COL32((food_color >> 0) & 0xFF,
                                (food_color >> 8) & 0xFF,
                                (food_color >> 16) & 0xFF, 100)
                        );
                    }

                    draw_list->AddRectFilled(
                        ImVec2(canvas_pos.x + food_item.position.x * cell_size,
                            canvas_pos.y + food_item.position.y * cell_size),
                        ImVec2(canvas_pos.x + (food_item.position.x + 1) * cell_size - 1,
                            canvas_pos.y + (food_item.position.y + 1) * cell_size - 1),
                        food_color
                    );
                }

                if (state.waiting_for_input && state.mode == game_mode::playing) {
                    const char* status_text = "Press arrow keys to move";
                    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                    auto text_size = ImGui::CalcTextSize(status_text);
                    float text_x = canvas_pos.x + (window_width - text_size.x) * 0.5f;
                    float text_y = canvas_pos.y + window_height / 2 - 20;

                    draw_list->AddRectFilled(
                        ImVec2(text_x - 10, text_y - 5),
                        ImVec2(text_x + text_size.x + 10, text_y + text_size.y + 5),
                        IM_COL32(0, 0, 0, 200)
                    );

                    draw_list->AddText(
                        ImVec2(text_x, text_y),
                        ImGui::ColorConvertFloat4ToU32(text_color),
                        status_text
                    );
                }

                if (state.mode == game_mode::paused || state.mode == game_mode::game_over) {
                    const char* status_text = (state.mode == game_mode::paused)
                        ? "Game Paused - Press ESC to resume"
                        : "Game Over! Press R to restart";

                    ImVec4 text_color = (state.mode == game_mode::paused)
                        ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f)
                        : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

                    auto text_size = ImGui::CalcTextSize(status_text);
                    float text_x = canvas_pos.x + (window_width - text_size.x) * 0.5f;
                    float text_y = canvas_pos.y + window_height / 2 - 20;

                    draw_list->AddRectFilled(
                        ImVec2(text_x - 10, text_y - 5),
                        ImVec2(text_x + text_size.x + 10, text_y + text_size.y + 5),
                        IM_COL32(0, 0, 0, 200)
                    );

                    draw_list->AddText(
                        ImVec2(text_x, text_y),
                        ImGui::ColorConvertFloat4ToU32(text_color),
                        status_text
                    );
                }

                ImGui::EndChild();
            }

            ImGui::End();
        }

    private:
        bool render_menu(float window_width, float window_height) {
            ImGui::SetCursorPos(ImVec2(10, 20));

            const char* title_text = "ROGUE SNAKE";
            auto title_size = ImGui::CalcTextSize(title_text);
            ImGui::SetCursorPos(ImVec2((window_width - title_size.x) * 0.5f + 10, 50));
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            ImGui::Text("%s", title_text);
            ImGui::PopFont();

            ImGui::SetCursorPos(ImVec2(window_width / 4 + 10, window_height / 2));

            ImVec2 button_size(window_width / 2, 40);
            bool start_pressed = false;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));

            if (ImGui::Button("Start Game", button_size)) {
                start_pressed = true;
            }

            ImGui::SetCursorPos(ImVec2(window_width / 4 + 10, window_height / 2 + 60));

            if (ImGui::Button("Exit Game", button_size)) {
                exit(0);
            }

            ImGui::PopStyleColor(2);

            ImGui::SetCursorPos(ImVec2(10, window_height - 40));
            ImGui::TextDisabled("Use arrow keys to move, ESC to pause");
            ImGui::SetCursorPos(ImVec2(10, window_height - 25));
            ImGui::TextDisabled("Collect food to advance to the next level");

            return start_pressed;
        }
    };
}