#include <stdbool.h>

#define ENET_IMPLEMENTATION
#include <enet.h>

#include "S_log.h"
#include "S_net.h"

enum PacketTypes {
    NPT_HOST_ACCEPT_PEER = 1,
};

struct Network {
    bool active, disconnecting;

    ENetAddress address;
    ENetHost* host;
    ENetPeer* peer;
};

static struct Network network = {0};

static enum PacketTypes read_packet_type(enet_uint8** data) {
    enum PacketTypes result = *((enum PacketTypes*)data);
    *data += sizeof(enum PacketTypes);
    return result;
}

void net_init() {
    enet_initialize();
}

void net_update() {
    if (network.host == NULL)
        return;
    ENetEvent event;
    while (enet_host_service(network.host, &event, 1) > 0)
        switch (event.type) {
            default:
                break;

            case ENET_EVENT_TYPE_CONNECT: {
                if (network.peer == NULL) {
                    INFO("Server got peer %p", event.peer);
                    enum PacketTypes data[] = {NPT_HOST_ACCEPT_PEER};
                    ENetPacket* packet = enet_packet_create(data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
                    if (enet_peer_send(event.peer, 0, packet) < 0)
                        enet_packet_destroy(packet);
                }
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                if (network.peer != NULL) {
                    INFO("Bye");
                    net_disconnect();
                    return;
                }

                INFO("Server lost peer %p", event.peer);
                break;
            }

            case ENET_EVENT_TYPE_RECEIVE: {
                INFO("Client got packet %p", event.packet);

                if (network.disconnecting || event.packet->dataLength < 1) {
                    enet_packet_destroy(event.packet);
                    break;
                }

                if (((enum PacketTypes*)(event.packet->data))[0] == NPT_HOST_ACCEPT_PEER) {
                    INFO("Hi");
                    if (network.peer != NULL) {
                        INFO("Hello");
                        if (!network.active) {
                            INFO("Welcome");
                            network.active = true;
                        }
                    }
                }

                enet_packet_destroy(event.packet);
                break;
            }
        }
}

void net_teardown() {
    net_disconnect();
    enet_deinitialize();
}

void net_host(uint16_t port) {
    network.address.host = ENET_HOST_ANY;
    network.address.port = (enet_uint16)port;

    network.host = enet_host_create(&(network.address), 4, 1, 0, 0);
    if (network.host == NULL)
        return;
    network.peer = NULL;

    network.active = true;
    network.disconnecting = false;
}

void net_connect(const char* ip, uint16_t port) {
    enet_address_set_host(&(network.address), ip);
    network.address.port = port;

    network.host = enet_host_create(NULL, 4, 1, 0, 0);
    if (network.host == NULL)
        return;
    network.peer = enet_host_connect(network.host, &(network.address), 1, 0);
    if (network.peer == NULL) {
        enet_host_destroy(network.host);
        network.host = NULL;
        return;
    }

    network.active = false;
    network.disconnecting = false;
}

void net_disconnect() {
    if (network.peer != NULL) {
        enet_peer_disconnect(network.peer, 0);
        network.peer = NULL;
    }
    if (network.host != NULL) {
        enet_host_destroy(network.host);
        network.host = NULL;
    }
    network.active = false;
}

void net_try_disconnect() {
    if (network.peer == NULL) {
        net_disconnect();
    } else if (!network.disconnecting) {
        enet_peer_disconnect(network.peer, 0);
        network.disconnecting = true;
    }
}

bool net_exists() {
    return network.host != NULL;
}

bool net_connected() {
    return net_exists() && network.active;
}
