//
// Created by paul on 1.5.16.
//

#include "PathEnumerator.h"


string PathEnumerator::initPath(const string& path) {
    string rv(path);

    if (rv.empty()) {
        return string(".");
    }
    return rv;
}


PathEnumerator::PathEnumerator(const std::string& aPath)
            : mPath(initPath(aPath)), dir(0)
{

}

PathEnumerator::~PathEnumerator() {
    if (dir) {
        closedir(dir);
    }
}

bool PathEnumerator::next(string& file) const {
    if (!dir) {
        if (!(dir = opendir(mPath.c_str()))) {
            return false;
        }
    }

    struct dirent *ent;
    if (!(ent = readdir(dir))) {
        return false;
    }
    file.assign(ent->d_name);
    return true;
}


PathEnumerator* PathEnumerator::getEnumerator(const string& path) {
    return new PathEnumerator(path);
}
