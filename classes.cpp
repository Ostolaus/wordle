#include "classes.hpp"

std::string Pattern::JSONify() {
    std::string return_string = "{ \"fixed\" :" + std::to_string(fixed) + ", \"existing\" :" + std::to_string(existing) + ", \"non_existing\" :" + std::to_string(non_existing)+"}";
    return return_string;
}

std::string Combination::JSONify() {
    std::string w1(word1,5);
    std::string w2(word2,5);
    std::string return_string = "{ \"word1\" :\"" + w1 + "\", \"word2\" :\"" + w2 + "\", \"Pattern\" :" + pattern.JSONify() + "}";
    return return_string;
}
