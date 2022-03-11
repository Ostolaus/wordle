#include "classes.hpp"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <map>


#include <fstream>

#define WORKING_THREAD_COUNT 8

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::ios_base;
using std::map;


char** wordlist;
size_t wordCount = 0;

Pattern* patterns;
size_t patternCount = 0;

size_t words_done = 0;
char last_word[5];
pthread_mutex_t progress_lock;

ofstream ofs;
size_t result_count = 0;
pthread_mutex_t result_lock;

enum Modes{COUNT, COMBS};
Modes mode = COMBS;



typedef struct workingParameters{
    size_t tid;
    size_t start;
    size_t end;
}workingParameters;

void readCSV() {
    wordlist = (char**) malloc((wordCount+1)*sizeof(char *));
    ifstream indata;
    indata.open("../words.csv");

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

void intToBinaryString(size_t val, char* buffer, size_t len){
    char bin[len+1];

    for (int i = 0; i < len; ++i) {
        bin[i] = ((val >> (len- 1 - i))&1) ? '1' : '0';
    }

    bin[len] = 0;
    memcpy(buffer, bin, len);
}

void printPatterns(){
    for (int i = 0; i < patternCount; ++i) {
        char* fix = (char*)malloc(6);
        char* ex = (char*)malloc(6);
        char* non_ex = (char*)malloc(6);
        assert(fix && ex&& non_ex);
        intToBinaryString(patterns[i].fixed, fix, 5);
        intToBinaryString(patterns[i].existing, ex, 5);
        intToBinaryString(patterns[i].non_existing, non_ex, 5);
        printf("Fixed:\t\t %s\nExisting:\t %s\nNon_Existing:\t %s\n \n", fix, ex, non_ex);
    }
}

bool isSpecialPattern(Pattern pattern){
    size_t fixedSum = 0, existingSum = 0;
    for (int i = 0; i < 5; ++i) {
        if((pattern.fixed >> i) & 1)
            fixedSum++;
        if((pattern.existing >> i) & 1)
            existingSum++;
    }
    if(fixedSum == 4 && existingSum == 1){
        return 0;
    }
    return 1;
}

void generatePatterns()
{
    patterns = (Pattern*) malloc(sizeof(Pattern) * (patternCount + 1));
    for (size_t non_existing = 0; non_existing < 0b100000; ++non_existing) {
        for (size_t existing = 0; existing < 0b100000; ++existing) {
            for (size_t fixed = 0; fixed < 0b11111; ++fixed) {
                Pattern currentPattern(fixed, existing, non_existing);
                if (non_existing + existing + fixed == 0b11111
                    && !(non_existing & existing & fixed)
                    && isSpecialPattern(currentPattern)){
                    patterns[patternCount] = currentPattern;
                    patternCount++;
                    patterns = (Pattern*) realloc(patterns, sizeof(Pattern) * (patternCount + 1));
                }
            }
        }
    }

    //printPatterns();

    printf("Patterncount: %zu\n", patternCount);
}

void* printProgress(void* param){
    printf("ProgressThread created\n");
    size_t max_iter = wordCount;

    do{
        pthread_mutex_lock(&progress_lock);
        cout  << words_done << "/" << max_iter << " " <<last_word << endl << "\x1b[A";
        pthread_mutex_unlock(&progress_lock);
        sleep(2);
    }while(words_done < max_iter);

    return (void*)0;
}

bool wordsEqual(char* word1, char* word2){
    for (int i = 0; i < 5; ++i) {
        if(word1[i] != word2[i]){
            return false;
        }
    }
    return true;
}

bool charInWord(char c, char* word){
    for (int i = 0; i < 5; ++i) {
        if(word[i] == c) {
            return true;
        }
    }
    return false;
}

bool combinationPossible(char *word1, char *word2, Pattern pattern) {
    for (int i = 0; i < 5; ++i) {
        char current_char = word1[i];
        bool fix = ((pattern.fixed >> (4 - i)) & 1);
        bool existing = ((pattern.existing >> (4 - i)) & 1);
        bool non_existing = ((pattern.non_existing >> (4 - i)) & 1);
        bool char_equal = (current_char == word2[i]);

        //Check for fixed position
        if (fix && !char_equal)
            return false;

        if (!fix && char_equal)
            return false;

        //Check for non existing positions
        size_t char_in_word = charInWord(current_char, word2);
        if (non_existing && char_in_word)
            return false;


        //Basic existing check
        if (existing && (!char_in_word || char_equal)) {
            return false;
        }
    }

    //Removing double existing chars
    std::map<char, int> w1_fixed_existing_dict;
    std::map<char, int> w2_total_dict;
    for (int word_idx = 0; word_idx < 5; ++word_idx) {
        char c1 = word1[word_idx];
        char c2 = word2[word_idx];
        bool fix = ((pattern.fixed >> (4 - word_idx)) & 1);
        bool existing = ((pattern.existing >> (4 - word_idx)) & 1);

        bool char_contained = w1_fixed_existing_dict.contains(c1);
        if(char_contained && (existing || fix)) {
            w1_fixed_existing_dict[c1] ++;
        }else if(!char_contained && (existing || fix)){
            w1_fixed_existing_dict.insert(std::pair<char, int>(c1, 1));
        }


        if (w2_total_dict.contains(c2)) {
            w2_total_dict[c2]++;
        } else {
            w2_total_dict.insert(std::pair<char, int>(c2, 1));
        }
    }



    for (int i = 0; i < 5; ++i){
        auto w1_count = w1_fixed_existing_dict.find(word1[i]);
        auto w2_count = w2_total_dict.find(word1[i]);
        if(w1_count->second != w2_count->second){
            return false;
        }
    }

    return true;
}

void* generateCombinations(void* params){
    workingParameters args = *(workingParameters*)params;
    for (size_t i = args.start; i < args.end; ++i) {
        char* word1 = wordlist[i];
        Word current_word(word1);
        for (int j = 0; j < patternCount; ++j) {
            Pattern current_pattern = patterns[j];
            for (int k = 0; k < wordCount; ++k) {
                char* word2 = wordlist[k];
                if(!wordsEqual(word1, word2) && combinationPossible(word1, word2, current_pattern)){
                    if(mode == COUNT){
                        current_word.count++;
                    }

                    if(mode == COMBS){
                        //Append to finished list
                        Combination current_combination(word1, word2, current_pattern);
                        pthread_mutex_lock(&result_lock);
                        result_count++;
                        ofs<< current_combination.JSONify().c_str() << ","<<endl;
                        pthread_mutex_unlock(&result_lock);
                    }
                }
            }

        }

        pthread_mutex_lock(&progress_lock);

        if(mode == COUNT){
            pthread_mutex_lock(&result_lock);
            result_count++;
            ofs<< current_word.JSONify().c_str() << ","<<endl;
            pthread_mutex_unlock(&result_lock);
        }

        words_done++;
        memcpy(last_word, word1, 5);
        pthread_mutex_unlock(&progress_lock);

    }

    return (void*)0;
}

int main()
{
    if(mode == COUNT){
        ofs.open("count.json", ios_base::out|ios_base::app);
    }else if(mode == COMBS){
        ofs.open("combs.json", ios_base::out|ios_base::app);
    }

    assert(ofs.is_open());
    ofs << "[";

    cout << "Starting" << endl;
    readCSV();
    printf("CSV Read\n");
    generatePatterns();
    printf("Patterns generated\n");

    // Progress Thread creation
    pthread_t progress_thread;
    pthread_create(&progress_thread, nullptr, printProgress, (void*)0);


    // Working Thread creation
    pthread_t working_threads[WORKING_THREAD_COUNT];
    workingParameters param[WORKING_THREAD_COUNT];
    size_t words_per_thread = wordCount / WORKING_THREAD_COUNT;
    size_t carry = wordCount % WORKING_THREAD_COUNT;

    for (int i = 0; i < WORKING_THREAD_COUNT; ++i) {
        param[i].tid = i;
        param[i].start = i * words_per_thread;
        param[i].end = (i + 1) * words_per_thread;
        if (i == (WORKING_THREAD_COUNT -1) && carry){
            param[i].end += carry;
        }
        pthread_create(&working_threads[i], nullptr, generateCombinations, (void*)&param[i]);
    }


    for(int i = 0; i < WORKING_THREAD_COUNT; ++i) {
        void* result = 0;
        pthread_join(working_threads[i], &result);
    }

    void* result = 0;
    pthread_join(progress_thread, &result);

    ofs << "\b]";
    ofs.close();
  return 0;
}
