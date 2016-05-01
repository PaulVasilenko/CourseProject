//
// Created by paul on 1.5.16.
//

#ifndef COURSEPROJECTSERVER_RESPONSE_H
#define COURSEPROJECTSERVER_RESPONSE_H

#include "../RequestParser/RequestParser.h"
#include "../UnixSocket/UnixSocket.h"

class Response {

    enum eMode {
        GET_ENTIRE_FILE,
        GET_FILE_RANGE,
        DIR_LIST,
        ERROR_FILE_NOT_EXIST,
        INTERNAL_ERROR
    };

public:
    Response(RequestParser p);

    bool SendHeaders(UnixSocket* aSocket);

    // Returns true if we need to call again.
    bool SendBody(UnixSocket *aSocket);

private:

    static string StatusCode(eMode mode);

    static string ExtractContentType(const string& file, eMode mode);

    static string GetDate();

    int64_t fileLength;
    RequestParser parser;
    eMode mode;
    string path;
    FILE* file;
    int64_t rangeStart;
    int64_t rangeEnd;
    int64_t offset;
    int64_t bytesRemaining;
};


#endif //COURSEPROJECTSERVER_RESPONSE_H
