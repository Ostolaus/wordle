#ifndef CLASSES_HPP_
#define CLASSES_HPP_
#include <string.h>
#include <string>
#include <stdint.h>


class Pattern{
public:
    size_t fixed = 0;
    size_t existing = 0;
    size_t non_existing = 0;

    Pattern(size_t fix, size_t exi, size_t non_exi): fixed(fix), existing(exi), non_existing(non_exi){}
    std::string JSONify();
};

class Combination{
public:
    char word1[5];
    char word2[5];
    Pattern pattern;

    Combination(char* w1, char* w2, Pattern pat): pattern(pat){
        memcpy(word1, w1, 5);
        memcpy(word2, w2, 5);
    };
    std::string JSONify();
};

#endif
