#define NOMINMAX

#include <windows.h>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <chrono>
#include <string>

#pragma comment(lib, "d3d11.lib")

import game_core;
import snake;
import food;
import obstacles;
import renderer;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Rogue Snake", nullptr };
    RegisterClassExW(&wc);

    const int window_width = snake_game::grid_width * snake_game::cell_size + 40;
    const int window_height = snake_game::grid_height * snake_game::cell_size + 100;

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    int x = (screen_width - window_width) / 2;
    int y = (screen_height - window_height) / 2;

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Rogue Snake",
        WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        x, y, window_width, window_height,
        nullptr, nullptr, wc.hInstance, nullptr);

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* device_context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target_view = nullptr;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_levels[1] = { D3D_FEATURE_LEVEL_11_0 };
    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_levels, 1, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &feature_level, &device_context);

    ID3D11Texture2D* back_buffer;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, device_context);

    snake_game::snake game_snake;
    snake_game::food game_food;
    snake_game::obstacle_manager obstacles;
    snake_game::game_renderer renderer;
    snake_game::game_state state;

    state.mode = snake_game::game_mode::menu;
    state.waiting_for_input = true;
    state.previous_snake_size = 1;

    auto last_update_time = std::chrono::steady_clock::now();
    bool running = true;

    while (running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }

        if (!running)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        auto current_time = std::chrono::steady_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_update_time).count();

        if (state.active_power_up != snake_game::power_up_type::none) {
            state.power_up_timer -= delta_time;
            if (state.power_up_timer <= 0) {
                if (state.active_power_up == snake_game::power_up_type::ghost_mode) {
                    game_snake.set_ghost_mode(false);
                }
                else if (state.active_power_up == snake_game::power_up_type::invincibility) {
                    game_snake.set_invincible(false);
                    state.is_invincible = false;
                }
                state.active_power_up = snake_game::power_up_type::none;
                state.game_speed = 0.2f - (state.level - 1) * 0.02f;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            if (state.mode == snake_game::game_mode::playing) {
                state.mode = snake_game::game_mode::paused;
            }
            else if (state.mode == snake_game::game_mode::paused) {
                state.mode = snake_game::game_mode::playing;
            }
        }

        if (state.mode == snake_game::game_mode::playing) {
            bool direction_pressed = false;
            snake_game::direction new_direction = game_snake.get_direction();

            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                new_direction = snake_game::direction::up;
                direction_pressed = true;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                new_direction = snake_game::direction::down;
                direction_pressed = true;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                new_direction = snake_game::direction::left;
                direction_pressed = true;
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                new_direction = snake_game::direction::right;
                direction_pressed = true;
            }

            if (state.waiting_for_input && direction_pressed) {
                game_snake.force_direction(new_direction);

                if (state.previous_snake_size > 1) {
                    game_snake.grow(state.previous_snake_size - 1);
                }

                state.waiting_for_input = false;
            }
            else if (!state.waiting_for_input && direction_pressed) {
                game_snake.set_direction(new_direction);
            }

            if (!state.waiting_for_input && delta_time >= state.game_speed) {
                game_snake.move();

                if (game_snake.check_collision(obstacles)) {
                    state.mode = snake_game::game_mode::game_over;
                }

                if (game_food.check_eaten(game_snake.get_head(), game_snake.get_body(), obstacles.get_obstacles())) {
                    if (game_food.is_power_up_food()) {
                        snake_game::power_up_type type = game_food.get_power_up_type();
                        state.active_power_up = type;

                        if (type == snake_game::power_up_type::extra_points) {
                            state.power_up_timer = snake_game::extra_points_duration;
                        }
                        else if (type == snake_game::power_up_type::invincibility) {
                            state.power_up_timer = snake_game::invincibility_duration;
                        }
                        else {
                            state.power_up_timer = snake_game::power_up_duration;
                        }

                        switch (type) {
                        case snake_game::power_up_type::speed_boost:
                            state.game_speed = state.game_speed * snake_game::speed_boost_factor;
                            break;

                        case snake_game::power_up_type::ghost_mode:
                            game_snake.set_ghost_mode(true);
                            break;

                        case snake_game::power_up_type::extra_points:
                            state.score += snake_game::extra_points_amount;
                            break;

                        case snake_game::power_up_type::multi_grow:
                            game_snake.grow(snake_game::multi_grow_segments);
                            break;

                        case snake_game::power_up_type::invincibility:
                            game_snake.set_invincible(true);
                            state.is_invincible = true;
                            break;
                        }
                    }

                    state.score += 10;
                    game_snake.grow();
                    state.food_collected++;

                    if (state.food_collected >= state.food_to_next_level && state.level < snake_game::max_level) {
                        state.level++;
                        state.food_collected = 0;
                        state.food_to_next_level += 2;

                        state.previous_snake_size = game_snake.get_body().size();

                        obstacles.reset();
                        game_snake.reset();
                        game_food.spawn_initial_food(game_snake.get_body(), obstacles.get_obstacles());
                        state.waiting_for_input = true;
                    }
                }

                last_update_time = current_time;
            }
        }
        else if (state.mode == snake_game::game_mode::game_over) {
            if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                game_snake.reset();
                obstacles.reset();
                game_food.spawn_initial_food(game_snake.get_body(), obstacles.get_obstacles());
                state.mode = snake_game::game_mode::playing;
                state.score = 0;
                state.level = 1;
                state.food_collected = 0;
                state.food_to_next_level = 5;
                state.game_speed = 0.2f;
                state.active_power_up = snake_game::power_up_type::none;
                state.waiting_for_input = true;
                state.previous_snake_size = 1;
            }
        }

        if (state.mode == snake_game::game_mode::menu &&
            (ImGui::IsKeyPressed(ImGuiKey_Space) || ImGui::IsKeyPressed(ImGuiKey_Enter))) {
            game_snake.reset();
            obstacles.reset();
            game_food.spawn_initial_food(game_snake.get_body(), obstacles.get_obstacles());
            state.mode = snake_game::game_mode::playing;
            state.score = 0;
            state.level = 1;
            state.food_collected = 0;
            state.food_to_next_level = 5;
            state.game_speed = 0.2f;
            state.active_power_up = snake_game::power_up_type::none;
            state.waiting_for_input = true;
            state.previous_snake_size = 1;
        }

        renderer.render_game(game_snake, game_food, obstacles, state);

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (render_target_view) render_target_view->Release();
    if (swap_chain) swap_chain->Release();
    if (device_context) device_context->Release();
    if (device) device->Release();

    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
