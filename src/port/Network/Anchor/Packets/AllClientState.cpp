#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/Authority.h"
#include "port/Network/Anchor/JsonConversions.hpp"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "port/Engine.h"
#include "port/UI/Notification.h"

extern "C" {
#include "functions.h"
}

/**
 * ALL_CLIENT_STATE
 *
 * Contains a list of all clients and their CLIENT_STATE currently connected to the server
 *
 * The server itself sends this packet to all clients when a client connects or disconnects
 */

void Anchor::HandlePacket_AllClientState(nlohmann::json& payload) {
    std::vector<AnchorClient> newClients = payload["state"].get<std::vector<AnchorClient>>();
    bool isGlobalRoom = (std::string("lh-global") == CVarGetString(CVAR_REMOTE_ANCHOR("RoomId"), ""));

    std::vector<uint32_t> clientsToRemove;
    // add new clients
    for (auto& client : newClients) {
        if (!client.online && clients.contains(client.clientId)) {
            clientsToRemove.push_back(client.clientId);
        }
        if (client.self) {
            ownClientId = client.clientId;
            CVarSetInteger(CVAR_REMOTE_ANCHOR("LastClientId"), ownClientId);
            Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
            clients[client.clientId].self = true;
        } else {
            clients[client.clientId].self = false;
            if (clients.contains(client.clientId)) {
                if (clients[client.clientId].online != client.online && !isGlobalRoom) {
                    Notification::Emit({
                        .prefix = client.name,
                        .message = client.online ? "Connected" : "Disconnected",
                    });
                }
            } else if (client.online && !isGlobalRoom) {
                Notification::Emit({
                    .prefix = client.name,
                    .message = "Connected",
                });
            }
            if (clients[client.clientId].dummy == nullptr && !client.self) {
                clients[client.clientId].dummy = new DummyPlayer();
                clients[client.clientId].dummy->dummy_reset();
            }
        }

        clients[client.clientId].clientId = client.clientId;
        clients[client.clientId].name = client.name;
        clients[client.clientId].clientVersion = client.clientVersion;
        clients[client.clientId].teamId = client.teamId;
        clients[client.clientId].online = client.online;
        clients[client.clientId].seed = client.seed;
        clients[client.clientId].isSaveLoaded = client.isSaveLoaded;
        clients[client.clientId].isGameComplete = client.isGameComplete;
        clients[client.clientId].map = client.map;
        clients[client.clientId].exit = client.exit;
        Authority_OnClientStateChanged(client.clientId, client.online, client.map);
    }

    // remove clients that are no longer in the list
    for (auto& [clientId, client] : clients) {
        if (std::find_if(newClients.begin(), newClients.end(),
                         [clientId](AnchorClient& c) { return c.clientId == clientId; }) == newClients.end()) {
            clientsToRemove.push_back(clientId);
        }
    }
    // (separate loop to avoid iterator invalidation)
    for (auto& clientId : clientsToRemove) {
        Authority_OnClientStateChanged(clientId, false, -1);
        if (dummies.contains(clientId)) {
            dummies.erase(clientId);
        }
        if (clients.at(clientId).dummy != nullptr) {
            clients.at(clientId).dummy->dummy_free();
            free(clients.at(clientId).dummy);
        }
        clients.erase(clientId);
    }

    PopulateDummies((GameMap)gsworld_getMap());
    SendPacket_PlayerUpdate(true);
}
