#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "Response.h"
#include "../PathEnumerator/PathEnumerator.h"
#include <unistd.h>
#include <cstring>

#define fseek64 fseeko64
#define ftell64 ftello64
#define DIR_LIST_CHARSET "text/html; charset=utf-8"
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void Sleep(int ms) {
    usleep(ms * 1000);
}

static int fopen_s(FILE** file, const char* path, const char* mode) {
    FILE *f = fopen(path, mode);
    if (!f) {
        return 1;
    }
    *file = f;
    return 0;
}


static const char* gContentTypes[][2] = {
        {"ogv", "video/ogg"},
        {"ogg", "video/ogg"},
        {"oga", "audio/ogg"},
        {"webm", "video/webm"},
        {"wav", "audio/x-wav"},
        {"html", "text/html; charset=utf-8"},
        {"txt", "text/plain; charset=utf-8"},
        {"js", "text/javascript; charset=utf-8"},
        {"js", "text/css; charset=utf-8"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"}
};

Response::Response(RequestParser p)
        : parser(p),
          mode(INTERNAL_ERROR),
          fileLength(-1),
          file(0),
          rangeStart(0),
          rangeEnd(0),
          offset(0),
          bytesRemaining(0)
{
    string target = parser.GetTarget();

    if (target == "") {
        mode = DIR_LIST;
        path = ".";
    } else if (target.find("..") != string::npos) {
        mode = ERROR_FILE_NOT_EXIST;
    } else {
        // Determine if the file exists, and if it is a directory.
        struct stat buf;
        char dir[255];
        getcwd(dir, 255);
        int result = stat(target.c_str(), &buf);
        if (result == -1) {
            cerr << strerror(errno) << std::endl;
            mode = ERROR_FILE_NOT_EXIST;
        } else if (result != 0) {
            mode = INTERNAL_ERROR;
        } else if (S_ISDIR(buf.st_mode)) {
            mode = DIR_LIST;
            path = parser.GetTarget();
        } else if (parser.IsRangeRequest() && !parser.IsLive()) {
            mode = GET_FILE_RANGE;
            path = parser.GetTarget();
            parser.GetRange(rangeStart, rangeEnd);
            if (rangeEnd == -1) {
                rangeEnd = buf.st_size;
            }
            fileLength = buf.st_size;
        } else {
            mode = GET_ENTIRE_FILE;
            path = parser.GetTarget();
            fileLength = buf.st_size;
        }
    }
}

bool Response::SendHeaders(UnixSocket* aSocket) {
    string headers;
    headers.append("HTTP/1.1 ");
    headers.append(StatusCode(mode));
    headers.append("\r\n");
    headers.append("Connection: close\r\n");
    headers.append(GetDate());
    headers.append("\r\n");
    headers.append("Server: HttpMediaServer/0.1\r\n");
    headers.append("Accept-Ranges: bytes\r\n");
    headers.append("Content-Length: ");
    headers.append(ToString(rangeEnd - rangeStart));
    headers.append("\r\n");
    headers.append("Content-Range: bytes ");
    headers.append(ToString(rangeStart));
    headers.append("-");
    headers.append(ToString(rangeStart));
    headers.append("/");
    headers.append(ToString(fileLength));
    headers.append("\r\n");
    headers.append("Content-Type: ");
    if (parser.HasSpecifiedMimeType()) {
        headers.append(parser.GetSpecifiedMimeType());
    } else {
        headers.append(ExtractContentType(path, mode));
    }
    headers.append("\r\n\r\n");

    cout << "Sending Headers " << parser.id << std::endl << headers;

    return aSocket->Send(headers.c_str(), (int)headers.size()) != -1;
}

// Returns true if we need to call again.
bool Response::SendBody(UnixSocket *aSocket) {
    if (mode == ERROR_FILE_NOT_EXIST) {
        cout << "Sent (empty) body (" << parser.id << ")" << std::endl;
        return false;
    }

    int len = 1024;
    unsigned wait = 0;
    string rateStr;

    if (mode == GET_ENTIRE_FILE) {
        if (!file) {
            if (fopen_s(&file, path.c_str(), "rb")) {
                file = 0;
                return false;
            }
        }

        if (feof(file)) {
            // Transmitted entire file!
            fclose(file);
            file = 0;
            return false;
        }

        // Transmit the next segment.
        char* buf = new char[len];

        int x = (int)fread(buf, 1, len, file);
        int r = aSocket->Send(buf, x);
        delete buf;
        if (r < 0) {
            // Some kind of error.
            return false;
        }

        if (wait > 0) {
            Sleep(wait);
        }
        // Else we transmitted that segment, we're ok.
        return true;

    } else if (mode == GET_FILE_RANGE) {
        if (!file) {
            if (fopen_s(&file, path.c_str(), "rb")) {
                file = 0;
                return false;
            }
            fseek64(file, rangeStart, SEEK_SET);
            offset = rangeStart;
            bytesRemaining = rangeEnd - rangeStart;
        }
        if (feof(file) || bytesRemaining == 0) {
            // Transmitted entire range.
            fclose(file);
            file = 0;
            return false;
        }

        // Transmit the next segment.
        char* buf = new char[len];

        len = (unsigned)MIN(bytesRemaining, len);
        size_t bytesSent = fread(buf, 1, len, file);
        bytesRemaining -= bytesSent;
        int r = aSocket->Send(buf, (int)bytesSent);
        delete buf;
        if (r < 0) {
            // Some kind of error.
            return false;
        }
        offset += bytesSent;
        assert(ftell64(file) == offset);

        if (wait > 0) {
            Sleep(wait);
        }

        // Else we tranmitted that segment, we're ok.
        return true;
    } else if (mode == DIR_LIST) {
        std::stringstream response;
        PathEnumerator *enumerator = PathEnumerator::getEnumerator(path);
        if (enumerator) {
            response << "<!DOCTYPE html>\n<ul>";
            string href;
            while (enumerator->next(href)) {
                if (href == "." || path == "." && href == "..") {
                    continue;
                }
                response << "<li><a href=\"" << path + "/" + href;
                if (!rateStr.empty()) {
                    response << "?rate=" + rateStr;
                }
                response << "\">" << href << "</a></li>";
            }
            response << "</ul>";
            delete enumerator;
        }
        string _r = response.str();
        aSocket->Send(_r.c_str(), (int)_r.size());
        return false;
    }

    return false;
}


string Response::StatusCode(eMode mode) {
    switch (mode) {
        case GET_ENTIRE_FILE: return string("200 OK");
        case DIR_LIST: return string("200 OK");
        case GET_FILE_RANGE: return string("206 OK");
        case ERROR_FILE_NOT_EXIST: return string("404 File Not Found");
        case INTERNAL_ERROR:
        default:
            return string("500 Internal Server Error");
    };
}

string Response::ExtractContentType(const string& file, eMode mode) {
    if (mode == ERROR_FILE_NOT_EXIST || mode == INTERNAL_ERROR)
        return "text/html; charset=utf-8";
    if (mode == DIR_LIST)
        return DIR_LIST_CHARSET;

    size_t dot = file.rfind(".");
    if (dot == string::npos) {
        // File has no extension. Just send it as binary.
        return "application/octet-stream";
    }

    string extension(file, dot+1, file.size() - dot - 1);
    StrToLower(extension);

    for (unsigned i=0; i<ARRAY_LENGTH(gContentTypes); i++) {
        if (extension == gContentTypes[i][0]) {
            return string(gContentTypes[i][1]);
        }
    }
    return "application/octet-stream";
}

static char const month[12][4] = {
        "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static char const day[7][4] = {
        "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

string Response::GetDate() {
    time_t rawtime;
    time(&rawtime);
    struct tm t;
    if (!gmtime_r(&rawtime, &t)) {
        return "Date: Thu Jan 01 1970 00:00:00 GMT";
    }

    char buf[128];
    unsigned len = snprintf(buf, ARRAY_LENGTH(buf),
                            "Date: %s, %d %s %d %.2d:%.2d:%.2d GMT",
                            day[t.tm_wday], t.tm_mday, month[t.tm_mon],
                            1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);

    return string(buf, len);
}