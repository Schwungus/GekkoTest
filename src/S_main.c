#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>
#define FIX_IMPLEMENTATION
#include <S_fixed.h>
#include <gekkonet.h>

#define FATAL(...)                                                                                                     \
    do {                                                                                                               \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ##__VA_ARGS__);                                                  \
        exit(1);                                                                                                       \
    } while (0)

struct GameState {
    fix16_t pos[2][2], vel[2][2];
};

struct GameInput {
    union UGameInput {
        struct UGameInputMove {
            char up : 1;
            char down : 1;
            char left : 1;
            char right : 1;
        } move;
        unsigned char value;
    } input;
};

uint32_t fletcher32(const uint16_t* data, size_t len) {
    uint32_t c0, c1;
    len = (len + 1) & ~1;
    for (c0 = c1 = 0; len > 0;) {
        size_t blocklen = len;
        if (blocklen > 720)
            blocklen = 720;
        len -= blocklen;
        do {
            c0 = c0 + *data++;
            c1 = c1 + c0;
        } while ((blocklen -= 2));
        c0 = c0 % 65535;
        c1 = c1 % 65535;
    }
    return (c1 << 16 | c0);
}

void save_state(struct GameState* state, GekkoGameEvent* event) {
    *event->data.save.state_len = sizeof(struct GameState);
    *event->data.save.checksum = fletcher32((uint16_t*)state, sizeof(struct GameState));
    SDL_memcpy(event->data.save.state, state, sizeof(struct GameState));
}

void load_state(struct GameState* state, GekkoGameEvent* event) {
    SDL_memcpy(state, event->data.load.state, sizeof(struct GameState));
}

void tick_state(struct GameState* state, struct GameInput inputs[2]) {
    for (size_t i = 0; i < 2; i++) {
        state->vel[i][0] = fix_mul(state->vel[i][0], 58578);
        state->vel[i][1] = fix_mul(state->vel[i][1], 58578);
        state->vel[i][0] =
            fix_add(state->vel[i][0], fix_from_int(inputs[i].input.move.left - inputs[i].input.move.right));
        state->vel[i][1] = fix_add(state->vel[i][1], fix_from_int(inputs[i].input.move.up - inputs[i].input.move.down));

        state->pos[i][0] = fix_clamp(fix_add(state->pos[i][0], state->vel[i][0]), FIX_ZERO, fix_from_int(640));
        state->pos[i][1] = fix_clamp(fix_add(state->pos[i][1], state->vel[i][1]), FIX_ZERO, fix_from_int(480));
    }
}

void draw_state(SDL_Renderer* renderer, struct GameState* state) {
    for (size_t i = 0; i < 2; i++) {
        const float x = fix_to_float(state->pos[i][0]);
        const float y = fix_to_float(state->pos[i][1]);
        SDL_SetRenderDrawColor(renderer, 255, 255, i * 255, 255);
        SDL_RenderFillRect(renderer, &(SDL_FRect){x - 16, y - 16, 32, 32});
    }
}

int main(int argc, char** argv) {
    int local_player = 0;
    int local_port = 6969;
    char* ip = "127.0.0.1:6700";

    if (argc > 3) {
        local_player = SDL_atoi(argv[1]);
        local_port = SDL_atoi(argv[2]);
        ip = argv[3];
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Player: %i, port: %i, IP: %s", local_player, local_port, ip);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS))
        FATAL("SDL_Init fail: %s", SDL_GetError());

    SDL_Window* window = SDL_CreateWindow("lockstep", 640, 480, 0);
    if (window == NULL)
        FATAL("SDL_CreateWindow fail: %s", SDL_GetError());
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        FATAL("SDL_CreateRenderer fail: %s", SDL_GetError());
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderScale(renderer, 1, 1);
    SDL_SetRenderViewport(renderer, &(SDL_Rect){0, 0, 640, 480});
    SDL_SetRenderClipRect(renderer, &(SDL_Rect){0, 0, 640, 480});

    GekkoSession* session = NULL;
    if (!gekko_create(&session))
        FATAL("gekko_create fail");
    GekkoConfig config = {0};
    config.num_players = 2;
    config.max_spectators = 0;
    config.input_prediction_window = 3;
    config.state_size = sizeof(struct GameState);
    config.input_size = sizeof(struct GameInput);
    config.desync_detection = true;
    gekko_start(session, &config);
    gekko_net_adapter_set(session, gekko_default_adapter(local_port));

    if (local_player <= 0) {
        local_player = gekko_add_actor(session, LocalPlayer, NULL);
        GekkoNetAddress address = {(void*)ip, SDL_strlen(ip)};
        gekko_add_actor(session, RemotePlayer, &address);
    } else {
        GekkoNetAddress address = {(void*)ip, SDL_strlen(ip)};
        gekko_add_actor(session, RemotePlayer, &address);
        local_player = gekko_add_actor(session, LocalPlayer, NULL);
    }
    gekko_set_local_delay(session, local_player, 3);

    struct GameState state = {
        {{fix_from_int(320), fix_from_int(240)}, {fix_from_int(320), fix_from_int(240)}},
        {{FIX_ZERO, FIX_ZERO}, {FIX_ZERO, FIX_ZERO}}
    };
    struct GameInput inputs[2] = {0};

    uint64_t last_time = 0;
    float ticks = 0;
    uint64_t tick = 0;
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                default:
                    break;
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
            }
        }

        const uint64_t current_time = SDL_GetTicks();
        ticks +=
            (float)(current_time - last_time) * ((float)((gekko_frames_ahead(session) >= 0.75f) ? 61 : 60) / 1000.0f);
        last_time = current_time;

        gekko_network_poll(session);

        while (ticks >= 1) {
            struct GameInput input = {0};
            const bool* keyboard = SDL_GetKeyboardState(NULL);
            input.input.move.up = (char)keyboard[SDL_SCANCODE_W];
            input.input.move.left = (char)keyboard[SDL_SCANCODE_A];
            input.input.move.down = (char)keyboard[SDL_SCANCODE_S];
            input.input.move.right = (char)keyboard[SDL_SCANCODE_D];
            gekko_add_local_input(session, local_player, &input);

            int count = 0;
            GekkoSessionEvent** events = gekko_session_events(session, &count);
            for (int i = 0; i < count; i++) {
                GekkoSessionEvent* event = events[i];
                switch (event->type) {
                    default:
                        break;

                    case DesyncDetected: {
                        struct Desynced desync = event->data.desynced;
                        FATAL(
                            "DESYNC!!! f:%d, rh:%d, lc:%u, rc:%u", desync.frame, desync.remote_handle,
                            desync.local_checksum, desync.remote_checksum
                        );
                        break;
                    }

                    case PlayerConnected: {
                        struct Connected connect = event->data.connected;
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Player %i connected", connect.handle);
                        break;
                    }

                    case PlayerDisconnected: {
                        struct Disconnected disconnect = event->data.disconnected;
                        FATAL("Player %i disconnected", disconnect.handle);
                        break;
                    }
                }
            }

            count = 0;
            GekkoGameEvent** updates = gekko_update_session(session, &count);
            for (int i = 0; i < count; i++) {
                GekkoGameEvent* event = updates[i];
                switch (event->type) {
                    default:
                        break;
                    case SaveEvent:
                        save_state(&state, event);
                        break;
                    case LoadEvent:
                        load_state(&state, event);
                        break;
                    case AdvanceEvent: {
                        inputs[0].input.value = event->data.adv.inputs[0];
                        inputs[1].input.value = event->data.adv.inputs[1];
                        tick = event->data.adv.frame;
                        tick_state(&state, inputs);
                        break;
                    }
                }
            }

            ticks -= 1;
        }

        // draw the state every iteration
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_state(renderer, &state);
        SDL_RenderPresent(renderer);
    }

    gekko_destroy(session);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
