//
// Created by user on 19.4.16.
//

#include "RequestParser.h"

static int gCount = 0;

bool RequestParser::IsComplete() {
    return complete;
}

eMethod RequestParser::GetMethod() {
    return method;
}

string RequestParser::GetTarget() {
    return target;
}

const map<string, string> &RequestParser::GetParams() const {
    return params;
}

bool RequestParser::IsRangeRequest() const {
    return hasRange;
}

bool RequestParser::IsLive() const  {
    return ContainsKey(GetParams(), "live");
}

bool RequestParser::HasSpecifiedMimeType() const {
    return ContainsKey(GetParams(), "mime");
}

string RequestParser::GetSpecifiedMimeType() const {
    assert(HasSpecifiedMimeType());
    return GetParams().find("mime")->second;
}

RequestParser::RequestParser()
    : start(0),
      complete(false),
      method(UNKNOWN),
      rangeStart(-1),
      rangeEnd(-1),
      hasRange(false),
      id(gCount++)
{
}

void RequestParser::Add(const char *buf, unsigned len) {

    assert(!complete);
    string str(buf, len);
    request += str;
    const char* linebr = "\r\n";
    assert(linebr[0] == 13);
    assert(linebr[1] == 10);

    // Continue to parse request.
    while (start < request.size()) {
        size_t end = request.find(linebr, start, 2);

        if (end == string::npos) {
            break;
        }

        if (start == end) {
            complete = true;
        }

        string l = string(request, start, end - start);

        Parse(l);

        start = end + 2;
    }

    cout << "Request " << id << std::endl << buf;
}

void RequestParser::GetRange(int64_t& start, int64_t& end) const {
    start = rangeStart;
    end = rangeEnd;
}

void RequestParser::Parse(const string& s) {
    if (start == 0) {
        // Request line.
        ParseRequestLine(s);
    } else if (s.find("Range") == 0) {
        int64_t start,end;
        if (ParseRange(s, start, end)) {
            rangeStart = start;
            rangeEnd = end;
            hasRange = true;
        }
    }
}

bool RequestParser::ParseRange(const string& s,
                               int64_t& start,
                               int64_t& end)
{
    // Range: bytes=start,end
    size_t sp = s.find(" ");
    size_t eq = s.find("=");
    if (sp == string::npos || eq == string::npos)
        return false;
    string bytes(s, sp + 1, eq - sp - 1);
    if (bytes != "bytes") {
        // The space is not followed by "bytes=", this is an invalid range param.
        return false;
    }

    string range(s, eq + 1);
    size_t dash = range.find("-");
    if (dash == string::npos)
        return false;
    string startStr(range, 0, dash);
    string endStr(range, dash + 1);

    start = atoll(startStr.c_str());
    if (endStr != "") {
        end = atoll(endStr.c_str());
    } else {
        end = -1;
    }

    return true;
}

eMethod RequestParser::ExtractMethod(const string& request) {
    size_t sp = request.find(" ");
    string m(request, 0, sp);
    if (m == "GET")
        return GET;
    else if (m == "HEAD")
        return HEAD;
    else if (m == "POST")
        return POST;
    return UNKNOWN;
}

string RequestParser::ExtractTarget(const string& request) {
    size_t start = request.find("/") + 1;
    size_t query = request.find("?", start); // Location of query params.
    size_t end = (query != string::npos) ? query : request.find(" ", start);
    // Remove trailing slashes.
    while (end > start && request.at(end - 1) == '/') {
        end--;
    }
    return string(request, start, end - start);
}

map<string, string> RequestParser::ExtractQueryParams(const string& request) {
    map<string,string> m;
    size_t query = request.find("?");
    if (query == string::npos)
        return m;
    size_t start = query + 1;
    size_t end = request.find(" ", start);
    vector<string> v;
    Tokenize(string(request, start, end - start), v, "&");
    for (unsigned i=0; i<v.size(); i++) {
        size_t eq = v[i].find("=");
        string key;
        string value;
        if (eq == string::npos) {
            key = v[i];
            value = "";
        } else {
            key = string(v[i], 0, eq);
            value = string(v[i], eq + 1, v[i].size() - eq - 1);
        }
        m[key] = value;
    }
    return m;
}

void RequestParser::ParseRequestLine(const string& request) {
    // Extract method.
    method = ExtractMethod(request);
    target = ExtractTarget(request);
    params = ExtractQueryParams(request);
    //printf("Target: '%s'\n", target.c_str());
    //printf("Params: '%s'\n", Flatten(params).c_str());
}