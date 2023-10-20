#pragma once
#include <string>


std::string delimiter(std::string str, std::string delimiter = "\\") {

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        str.erase(0, pos + delimiter.length());
    }
    return str;
}
