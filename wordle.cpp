#include "classes.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <string.h>
#include <assert.h>

#include <fstream>

using std::cout;
using std::endl;
using std::ifstream;



char** wordlist;
size_t wordCount = 0;

Pattern* patterns;
size_t patternCount = 0;


void readCSV() {
    wordlist = (char**) malloc((wordCount+1)*sizeof(char *));
    ifstream indata;
    indata.open("../wordle.csv");

    if(!indata.is_open()){
        cout << "Opening File failed\n";
    }

    char c;
    char word[5];
    uint8_t count = 0;
    while(indata.get(c)){
        if (c != ';'){
            word[count] = c;
            count++;
        }else{
            wordlist = (char**)realloc(wordlist, sizeof(char*)*(wordCount+1));
            assert(wordlist && "Realloc failed");
            wordlist[wordCount] = (char*)malloc(5);
            assert(wordlist[wordCount] && "malloc failed");
            memcpy(wordlist[wordCount],  word, 5);
            wordCount++;
            count = 0;
        }

    }
    printf("%zu Words read\n",wordCount);
    indata.close();
    wordlist = (char**)realloc(wordlist, sizeof(char*)*(wordCount));
}

void generatePatterns()
{
    patterns = (Pattern*) malloc(sizeof(Pattern) * (patternCount + 1));
    for (size_t non_existing = 0; non_existing < 0b100000; ++non_existing) {
        for (size_t existing = 0; existing < 0b100000; ++existing) {
            for (size_t fixed = 0; fixed < 0b11111; ++fixed) {
                if (non_existing + existing + fixed == 0b11111){
                    patterns[patternCount] = Pattern(fixed, existing, non_existing);
                    patternCount++;
                    patterns = (Pattern*) realloc(patterns, sizeof(Pattern) * (patternCount + 1));
                }
            }
        }
    }

    for (int i = 0; i < patternCount; ++i) {
        printf("Fixed: %05X, Existing: %05X, Non_Existing: %05X \n",patterns[i].fixed,patterns[i].existing,patterns[i].non_existing );
    }

    printf("Patterncount: %zu\n", patternCount);
}

int main()
{
    cout << "Starting" << endl;
    readCSV();
    printf("CSV Read\n");
    generatePatterns();
    printf("Patterns generated\n");
  // Progress Thread creation

  // Working Thread creation

  // File appending

  return 0;
}