//
// Created by paul on 1.5.16.
//

#ifndef COURSEPROJECTSERVER_PATHENUMERATOR_H
#define COURSEPROJECTSERVER_PATHENUMERATOR_H

#include <string>
#include <dirent.h>
#include <sys/types.h>

using std::string;


class PathEnumerator {
public:
    static PathEnumerator* getEnumerator(const std::string& path);
    virtual ~PathEnumerator();
    virtual bool next(std::string& file) const;
protected:
    PathEnumerator(const std::string& aPath);
private:
    const string mPath;
    mutable DIR* dir;
    static string initPath(const string& path);
};

#endif //COURSEPROJECTSERVER_PATHENUMERATOR_H
