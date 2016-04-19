//
// Created by Pavel Vasilenka on 9.2.16.
//

#ifndef COURSEPROJECTSERVER_UNIXSOCKET_H
#define COURSEPROJECTSERVER_UNIXSOCKET_H

#include <string>
using std::string;

class UnixSocket
{
    protected:
        int socket_;

        UnixSocket() {}

        UnixSocket(int aSocket) : socket_(aSocket) {}

        bool WaitForReading(unsigned timeout);
    public:
        virtual ~UnixSocket();

        static int Init();

        static int Shutdown();

        static UnixSocket* Open(int aPort);

        virtual UnixSocket* Accept();

        virtual void Close();

        virtual int Send(const char* aBuf, int aSize);

        virtual int Receive(char* aBuf, int aSize);

        string GetIP() const;

};


#endif //COURSEPROJECTSERVER_UNIXSOCKET_H
