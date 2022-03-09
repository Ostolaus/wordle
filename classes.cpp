#include "classes.hpp"
extern void intToBinaryString(size_t val, char* buffer, size_t len);

std::string Pattern::JSONify() {
    char* fix = (char*)malloc(6);
    char* exi = (char*)malloc(6);
    char* non_exi = (char*)malloc(6);


    intToBinaryString(this->fixed, fix, 5);
    intToBinaryString(this->existing, exi, 5);
    intToBinaryString(this->non_existing, non_exi, 5);


    std::string return_string = "{ \"fixed\" :" + std::string(fix) + ", \"existing\" :" + std::string(exi) + ", \"non_existing\" :" + std::string(non_exi)+"}";
    return return_string;
}

std::string Combination::JSONify() {
    std::string w1(word1,5);
    std::string w2(word2,5);
    std::string return_string = "{ \"word1\" :\"" + w1 + "\", \"word2\" :\"" + w2 + "\", \"Pattern\" :" + pattern.JSONify() + "}";
    return return_string;
}
