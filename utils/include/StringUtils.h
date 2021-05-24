//
// Created by ltc on 2021/5/24.
//

#ifndef UTILS_STRINGUTILS_H
#define UTILS_STRINGUTILS_H

#include <vector>
#include <string>

using namespace std;

namespace utils{
    vector<string> split(const string& input, char spilt, int num=-1);
}

#endif //UTILS_STRINGUTILS_H
