#include <iostream>
#include <bits/unique_ptr.h>
#include "lib/UnixSocket/UnixSocket.h"
#include "lib/Connection/Connection.h"

#define DEFAULT_PORT 8081

using namespace std;

bool gRunning = true;

int main(int argc, char* argv[]) {
    UnixSocket::Init();

    std::unique_ptr<UnixSocket> listener(UnixSocket::Open(DEFAULT_PORT));

    if (!listener.get()) {
        UnixSocket::Shutdown();
        return 1;
    }

    cout << "Local IP: " << listener->GetIP().c_str() << std::endl;
    cout << "Now listening on port: " << DEFAULT_PORT << std::endl;

    vector<Connection*> Connections;
    while (gRunning) {
        // Accept a single connection.
        UnixSocket* client = listener->Accept();
        if (!client) {
            continue;
        }

        Connection *c = new Connection(client);
        c->Start();
        Connections.push_back(c);
    }

    UnixSocket::Shutdown();

    return 0;
}