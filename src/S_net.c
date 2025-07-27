#include <stdbool.h>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include "S_log.h"
#include "S_net.h"

#define MAX_PEERS 4

#define NPT_INVALID (uint8_t)0
#define NPT_HOST_ACCEPT_CLIENT (uint8_t)1

struct PacketHostAcceptClientPlayer {
    uint32_t slot;
};

struct PacketHostAcceptClient {
    uint8_t type;
    uint32_t slot;
    uint32_t num_players;
    struct PacketHostAcceptClientPlayer players[MAX_PEERS];
};

struct NetPlayer {
    bool active;
};

struct Network {
    bool active;
    ENetHost* server;
    ENetPeer* host;

    ENetPeer* peers[MAX_PEERS];
    size_t num_peers;

    struct NetPlayer players[MAX_PEERS];
    struct NetPlayer* local_player;
    size_t num_players;
};

static struct Network network = {0};

void net_init() {
    enet_initialize();
}

static struct NetPlayer* net_assign_player(size_t i) {
    if (network.players[i].active)
        return NULL;
    network.players[i].active = true;
    network.num_players++;
    return &(network.players[i]);
}

static bool net_unassign_player(size_t i) {
    if (!network.players[i].active)
        return false;
    network.players[i].active = false;
    network.num_players--;
    return true;
}

static void net_handle_packet(ENetEvent* event) {
    if (event->packet->dataLength < sizeof(uint8_t))
        return;

    uint8_t type = ((uint8_t*)(event->packet->data))[0];
    switch (type) {
        default: {
            WARN("Invalid packet type %u", type);
            break;
        }

        case NPT_HOST_ACCEPT_CLIENT: {
            if (network.host != NULL && !network.active) {
                network.active = true;
                INFO("Connected");
            }
            break;
        }
    }
}

void net_update() {
    if (network.server == NULL)
        return;
    ENetEvent event;
    while (enet_host_service(network.server, &event, 1) > 0)
        switch (event.type) {
            default:
                break;

            case ENET_EVENT_TYPE_CONNECT: {
                if (network.host != NULL)
                    break;

                struct PacketHostAcceptClient data = {0};
                data.type = NPT_HOST_ACCEPT_CLIENT;

                size_t i = 0;
                for (i = 0; i < MAX_PEERS; i++)
                    if (!network.players[i].active)
                        break;
                if (i >= MAX_PEERS) {
                    enet_peer_disconnect_now(event.peer, 0);
                    break;
                }
                data.slot = i;
                network.peers[i] = event.peer;
                net_assign_player(i);

                data.num_players = network.num_players - 1;
                size_t k = 0;
                for (size_t j = 0; j < MAX_PEERS; j++) {
                    if (j == i)
                        continue;
                    data.players[k++].slot = j;
                }

                ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
                if (packet != NULL && enet_peer_send(event.peer, 0, packet) < 0)
                    enet_packet_destroy(packet);
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                net_handle_packet(&event);
                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                if (event.peer == network.host) {
                    net_disconnect();
                    return;
                }
                for (size_t i = 0; i < MAX_PEERS; i++)
                    if (network.peers[i] == event.peer) {
                        net_unassign_player(i);
                        network.peers[i] = NULL;
                    }
                break;
            }
        }
}

void net_teardown() {
    net_disconnect();
    enet_deinitialize();
}

void net_connect(const char* ip, uint16_t port) {
    net_disconnect();

    network.server = enet_host_create(NULL, MAX_PEERS, 1, 0, 0);
    if (network.server == NULL) {
        WTF("Failed to listen");
        return;
    }

    ENetAddress address;
    enet_address_set_host_ip_new(&address, ip);
    address.port = port;
    network.host = enet_host_connect(network.server, &address, 1, 0);
    if (network.host == NULL) {
        enet_host_destroy(network.server);
        network.server = NULL;
        WTF("Failed to connect");
        return;
    }

    INFO("Connecting");
}

void net_host(uint16_t port) {
    net_disconnect();

    network.server = enet_host_create(&(const ENetAddress){ENET_HOST_ANY, port}, MAX_PEERS, 1, 0, 0);
    if (network.server == NULL) {
        WTF("Failed to host");
        return;
    }

    network.local_player = net_assign_player(0);
    network.active = true;
    INFO("Hosting");
}

void net_disconnect() {
    if (network.active)
        INFO("Disconnected");

    for (size_t i = 0; i < MAX_PEERS; i++)
        net_unassign_player(i);
    for (size_t i = 0; i < MAX_PEERS && network.num_peers > 0; i++) {
        enet_peer_disconnect_now(network.peers[i], 0);
        network.peers[i] = NULL;
        network.num_peers--;
    }
    if (network.host != NULL) {
        enet_peer_disconnect_now(network.host, 0);
        network.host = NULL;
    }
    if (network.server != NULL) {
        enet_host_destroy(network.server);
        network.server = NULL;
    }
    network.active = false;
}

bool net_exists() {
    return network.server != NULL;
}

bool net_connected() {
    return network.active;
}
