#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>
#define FIX_IMPLEMENTATION
#include <S_fixed.h>
#include <gekkonet.h>

#define INFO(...) SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ##__VA_ARGS__)

#define FATAL(...)                                                                                                     \
    do {                                                                                                               \
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, ##__VA_ARGS__);                                                  \
        exit(1);                                                                                                       \
    } while (0)

#define MAX_PLAYERS 4

struct GameState {
    bool active[MAX_PLAYERS];
    fix16_t pos[MAX_PLAYERS][2], vel[MAX_PLAYERS][2];
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

void tick_state(struct GameState* state, struct GameInput inputs[MAX_PLAYERS]) {
    for (size_t i = 0; i < MAX_PLAYERS; i++) {
        if (!state->active[i])
            continue;
        state->vel[i][0] =
            Fadd(Fmul(state->vel[i][0], 0x0000E666), FfInt(inputs[i].input.move.left - inputs[i].input.move.right));
        state->vel[i][1] =
            Fadd(Fmul(state->vel[i][1], 0x0000E666), FfInt(inputs[i].input.move.up - inputs[i].input.move.down));

        state->pos[i][0] = Fclamp(Fadd(state->pos[i][0], state->vel[i][0]), FxZero, FfInt(640L));
        state->pos[i][1] = Fclamp(Fadd(state->pos[i][1], state->vel[i][1]), FxZero, FfInt(480L));
    }
}

void draw_state(SDL_Renderer* renderer, struct GameState* state) {
    for (size_t i = 0; i < MAX_PLAYERS; i++) {
        if (!state->active[i])
            continue;
        switch (i) {
            default:
                break;
            case 0:
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                break;
        }
        SDL_RenderFillRect(
            renderer, &(SDL_FRect){FtFloat(state->pos[i][0]) - 16, FtFloat(state->pos[i][1]) - 16, 32, 32}
        );
    }
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
#endif

    int local_port = 6969;
    int local_player = 0;
    int num_players = 2;
    const char* ip[MAX_PLAYERS] = {NULL, "127.0.0.1:6700"};

    if (argc > 1) {
        size_t i = 0;
        num_players = SDL_strtol(argv[++i], NULL, 0);
        if (num_players < 1 || num_players > MAX_PLAYERS)
            FATAL("Player amount must be between 1 and %i", MAX_PLAYERS);
        if (argc < 2 + num_players)
            FATAL("Gimme all the IPs");
        for (size_t j = 0; j < num_players; j++) {
            char* player = argv[++i];
            if (SDL_strlen(player) <= 5) {
                local_port = SDL_strtol(player, NULL, 0);
                local_player = (int)j;
                ip[j] = NULL;
            } else {
                ip[j] = player;
            }
        }
    }

    INFO("Playing as player %i (port %i)", local_player, local_port);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_EVENTS))
        FATAL("SDL_Init fail: %s", SDL_GetError());

    SDL_Window* window = SDL_CreateWindow("lockstep", 640, 480, 0);
    if (window == NULL)
        FATAL("SDL_CreateWindow fail: %s", SDL_GetError());
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
        FATAL("SDL_CreateRenderer fail: %s", SDL_GetError());

    GekkoSession* session = NULL;
    if (!gekko_create(&session))
        FATAL("gekko_create fail");
    GekkoConfig config = {0};
    config.num_players = num_players;
    config.max_spectators = 0;
    config.input_prediction_window = 3;
    config.state_size = sizeof(struct GameState);
    config.input_size = sizeof(struct GameInput);
    config.desync_detection = true;
    gekko_start(session, &config);
    gekko_net_adapter_set(session, gekko_default_adapter(local_port));

    for (size_t i = 0; i < num_players; i++) {
        if (ip[i] == NULL)
            gekko_add_actor(session, LocalPlayer, NULL);
        else
            gekko_add_actor(session, RemotePlayer, &((GekkoNetAddress){(void*)ip[i], SDL_strlen(ip[i])}));
    }
    if (num_players > 1)
        gekko_set_local_delay(session, local_player, 3);

    struct GameState state = {0};
    for (size_t i = 0; i < num_players; i++) {
        state.active[i] = true;
        state.pos[i][0] = FfInt(320);
        state.pos[i][1] = FfInt(240);
    }
    struct GameInput inputs[MAX_PLAYERS] = {0};

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
            (float)(current_time - last_time) * ((float)((gekko_frames_ahead(session) >= 0.75f) ? 51 : 50) / 1000.0f);
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
                        INFO("Player %i connected", connect.handle);
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
                        for (size_t j = 0; j < num_players; j++)
                            inputs[j].input.value = event->data.adv.inputs[j];
                        tick = event->data.adv.frame;
                        tick_state(&state, inputs);
                        break;
                    }
                }
            }

            ticks -= 1;
        }

        // draw the state every iteration
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_RenderClear(renderer);
        draw_state(renderer, &state);
        SDL_RenderPresent(renderer);
    }

    gekko_destroy(session);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

#ifdef _MSC_VER
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
