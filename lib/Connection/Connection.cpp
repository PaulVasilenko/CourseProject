//
// Created by user on 19.4.16.
//

#include "Connection.h"

#define DEFAULT_BUFLEN 512

void Connection::Start() {
    mThread->Start();
}

Connection::Connection(UnixSocket *s)
    : mClientSocket(s)
{
    mThread = Thread::Create(this);
    mThread->Join();
}

void Connection::Run() {
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    while (!parser.IsComplete()) {
        int r = mClientSocket->Receive(recvbuf, recvbuflen);
        if (r > 0) {
            parser.Add(recvbuf, r);
        } else if (r == 0) {
            cout << "Connection closing..." << std::endl;
            break;
        } else {
            mClientSocket->Close();
            return;
        }
    }

//    Response response(parser);
//
//    if (!response.SendHeaders(mClientSocket.get())) {
//        mClientSocket->Close();
//        return;
//    }
//
//    while (response.SendBody(mClientSocket.get())) {
//        // Transmit the body.
//    }

    // cleanup
    mClientSocket->Close();
    return;
}
