#include "Connection.h"
#include "../Response/Response.h"

#define DEFAULT_BUFLEN 512

void Connection::Start() {
    mThread->Start();
}

Connection::Connection(UnixSocket *s)
    : mClientSocket(s)
{
    mThread = Thread::Create(this);
    closed = false;
}

void Connection::Run() {
    char receiveBuffer[DEFAULT_BUFLEN];
    int receiveBufferLength = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    while (!parser.IsComplete()) {
        int r = mClientSocket->Receive(receiveBuffer, receiveBufferLength);
        if (r > 0) {
            parser.Add(receiveBuffer, (unsigned int) r);
        } else if (r == 0) {
            cout << "Connection closing..." << std::endl;
            break;
        } else {
            mClientSocket->Close();
            return;
        }
    }

    Response response(parser);

    if (!response.SendHeaders(mClientSocket.get())) {
        mClientSocket->Close();
        return;
    }

    while (response.SendBody(mClientSocket.get())) {
        // Transmit the body.
    }

    // cleanup
    mClientSocket->Close();

    delete mThread;

    closed = true;

    return;
}
