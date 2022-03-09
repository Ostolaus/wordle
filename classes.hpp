#include <string>
#include <stdint.h>

class Pattern{
public:
    size_t fixed = 0;
    size_t existing = 0;
    size_t non_existing = 0;

    Pattern(size_t fix, size_t exi, size_t non_exi): fixed(fix), existing(exi), non_existing(non_exi){}
private:
    std::string printPattern();
};

class Combination{
public:
    char word1[5];
    char word2[5];
    Pattern pattern;

    Combination(char* w1, char* w2, Pattern pat);
    std::string JSONify();
};