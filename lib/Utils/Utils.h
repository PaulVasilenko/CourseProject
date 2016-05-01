//
// Created by user on 19.4.16.
//

#ifndef COURSEPROJECTSERVER_UTILS_H
#define COURSEPROJECTSERVER_UTILS_H

#include <stdint.h>

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <iostream>

using std::map;
using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;

string ToString(int64_t i);
string ToString(int i);

string Flatten(const map<string, string>& m);

bool ContainsKey(const map<string,string> m, const string& key);

inline string& StrToLower(string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void Tokenize(const string& str,
              vector<string>& tokens,
              const string& delimiters);

#define ARRAY_LENGTH(array_) \
  (sizeof(array_)/sizeof(array_[0]))

#endif //COURSEPROJECTSERVER_UTILS_H
