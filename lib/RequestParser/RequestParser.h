//
// Created by user on 19.4.16.
//

#ifndef COURSEPROJECTSERVER_REQUESTPARSER_H
#define COURSEPROJECTSERVER_REQUESTPARSER_H

#include <string>
#include <assert.h>
#include "../Utils/Utils.h"

using std::string;

enum eMethod { UNKNOWN, HEAD, GET, POST };

class RequestParser {
public:
    RequestParser();

    void Add(const char* buf, unsigned len);

    bool IsComplete();

    eMethod GetMethod();

    string GetTarget();

    const map<string, string>& GetParams() const;

    void GetRange(int64_t& start, int64_t& end) const;

    bool IsRangeRequest() const;

    bool IsLive() const;

    bool HasSpecifiedMimeType() const;

    string GetSpecifiedMimeType() const;

    const int id;
private:
    void Parse(const string& s);

    static bool ParseRange(const string& s, int64_t& start, int64_t& end);

    static eMethod ExtractMethod(const string& request);

    static string ExtractTarget(const string& request);

    static map<string, string> ExtractQueryParams(const string& request);

    void ParseRequestLine(const string& request);

    string request;
    size_t start;
    vector<string> lines;
    bool complete;
    eMethod method;
    string target;
    map<string, string> params;
    int64_t rangeStart, rangeEnd;
    bool hasRange;
};


#endif //COURSEPROJECTSERVER_REQUESTPARSER_H
