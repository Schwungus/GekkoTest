#include <raylib.h>

#include "S_net.h"

static void* game = NULL;

int main(int argc, char* argv[]) {
    net_init();

    InitWindow(640, 480, "Lockstep");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        net_update();

        if (game == NULL) {
            if (!net_exists()) {
                if (IsKeyPressed(KEY_H))
                    net_host(6969);
                else if (IsKeyPressed(KEY_C))
                    net_connect("localhost", 6969);
            } else if (net_connected()) {
                if (IsKeyPressed(KEY_D))
                    net_try_disconnect();
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (game == NULL) {
            if (!net_exists())
                DrawText("[H] Host\n[C] Client", 8, 8, 20, DARKGRAY);
            else if (!net_connected())
                DrawText("Connecting...", 8, 8, 20, DARKGRAY);
            else
                DrawText("[D] Disconnect", 8, 8, 20, DARKGRAY);
        }
        EndDrawing();
    }
    CloseWindow();

    net_teardown();

    return 0;
}
