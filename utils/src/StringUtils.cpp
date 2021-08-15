//
// Created by ltc on 2021/5/24.
//

#include "StringUtils.h"

vector<string> utils::split(const string& input, char spilt, int num) {
    vector<string> result;
    if (num == 0 || num == 1) {
        result.push_back(input);
        return result;
    }
    int left = -1;
    int right = 0;
    while (right < input.size()) {
        if (input[right] == spilt) {
            if (right-left != 1) {
                result.push_back(input.substr(left+1, right-left-1));
                if (result.size() == num-1) {
                    left = right - 1;
                    result.push_back(input.substr(right+1));
                    break;
                }
            }
            left = right;
        }
        right++;
    }
    if (right-left != 1){
        result.push_back(input.substr(left+1, right-left-1));
    }
    return result;
}