#include <raylib.h>
#define FIX_IMPLEMENTATION
#include <S_fixed.h>

#include "S_log.h"
#include "S_net.h"

static void* game = NULL;

int main(int argc, char* argv[]) {
    net_init();

    InitWindow(640, 480, "Lockstep");
    SetTargetFPS(60);

    INFO();
    INFO("FIXED POINT TEST");
    INFO();
    INFO(
        "01. Zero: %.1f, Half: %.1f, One: %.1f", fix_to_double(FIX_ZERO), fix_to_double(FIX_HALF),
        fix_to_double(FIX_ONE)
    );
    INFO(
        "02. 90 deg = %.5f rad, 180 deg = %.5f rad, 360 deg = %.5f rad", fix_to_double(FIX_HALF_PI),
        fix_to_double(FIX_PI), fix_to_double(FIX_DOUBLE_PI)
    );
    INFO();
    INFO("03. 4 + 9 = %i", fix_to_int(fix_add(fix_from_int(4), fix_from_int(9))));
    INFO("04. 5 - 8 = %i", fix_to_int(fix_sub(fix_from_int(5), fix_from_int(8))));
    INFO("05. 6 * 7 = %i", fix_to_int(fix_mul(fix_from_int(6), fix_from_int(7))));
    INFO("06. 7 / 6 = %.5f", fix_to_double(fix_div(fix_from_int(7), fix_from_int(6))));
    INFO();
    INFO("07. 9 / 2 = %.1f", fix_to_double(fix_half(fix_from_int(9))));
    INFO("08. 9 * 2 = %i", fix_to_int(fix_double(fix_from_int(9))));
    INFO("09. frac(127.0509) = %.4f", fix_to_double(fix_frac(fix_from_double(127.0509))));
    INFO("10. floor(127.0509) = %.4f", fix_to_double(fix_floor(fix_from_double(127.0509))));
    INFO("11. ceil(127.0509) = %.4f", fix_to_double(fix_ceil(fix_from_double(127.0509))));
    INFO("12. |-123.123| = %.3f", fix_to_double(fix_abs(fix_from_double(-123.123))));
    INFO();
    INFO("13. 9 ^ 2 = %i", fix_to_int(fix_sqr(fix_from_int(9))));
    INFO("14. sqrt(81) = %i", fix_to_int(fix_sqrt(fix_from_int(81))));
    INFO();

    const fix16_t angle = FIX_QUARTER_PI;
    const fix16_t sine = fix_sin(angle);
    const fix16_t cosine = fix_cos(angle);
    const fix16_t tangent = fix_tan(angle);

    INFO("15. sin(%.5f rad) = %.5f", fix_to_double(angle), fix_to_double(sine));
    INFO("16. cos(%.5f rad) = %.5f", fix_to_double(angle), fix_to_double(cosine));
    INFO("17. tan(%.5f rad) = %.5f", fix_to_double(angle), fix_to_double(tangent));
    INFO("18. asin(%.5f) = %.5f rad", fix_to_double(sine), fix_to_double(fix_asin(sine)));
    INFO("19. acos(%.5f) = %.5f rad", fix_to_double(cosine), fix_to_double(fix_acos(cosine)));
    INFO("20. atan(%.5f) = %.5f rad", fix_to_double(tangent), fix_to_double(fix_atan(tangent)));
    INFO(
        "21. atan2(%.5f, %.5f) = %.5f rad", fix_to_double(sine), fix_to_double(cosine),
        fix_to_double(fix_atan2(sine, cosine))
    );
    INFO();

    while (!WindowShouldClose()) {
        net_update();

        if (game == NULL) {
            if (!net_exists()) {
                if (IsKeyPressed(KEY_H))
                    net_host(8378);
                else if (IsKeyPressed(KEY_C))
                    net_connect("127.0.0.1", 8378);
            } else if (net_connected()) {
                if (IsKeyPressed(KEY_D))
                    net_disconnect();
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
