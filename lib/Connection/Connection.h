//
// Created by user on 19.4.16.
//

#ifndef COURSEPROJECTSERVER_CONNECTION_H
#define COURSEPROJECTSERVER_CONNECTION_H

#include <bits/unique_ptr.h>
#include "../Thread/Runnable.h"
#include "../Thread/Thread.h"
#include "../UnixSocket/UnixSocket.h"
#include "../RequestParser/RequestParser.h"

class Connection : public Runnable {
public:
    Connection(UnixSocket* s);

    void Start();

    virtual void Run();

private:
    std::unique_ptr<UnixSocket> mClientSocket;
    RequestParser parser;
    Thread* mThread;
};


#endif //COURSEPROJECTSERVER_CONNECTION_H
