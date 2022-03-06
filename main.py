import csv
import threading
from os import cpu_count
import time
import json
from json import JSONEncoder


words = []
patterns = []
combinations = []

progress = 0
last_word = ""
progress_lock = threading.Lock()

class Pattern:
    def __init__(self, fix, existing, non_existing):
        self.fixed = fix
        self.existing = existing
        self.non_existing = non_existing

    def printPattern(self):
        return f"Fix: {self.fixed}\t Existing: {self.existing}\t Not existing: {self.non_existing}"


class Combination:
    def __init__(self, word1, word2, pattern):
        self.word1 = list(word1)
        self.word2 = list(word2)
        self.pattern = pattern

    def printCombi(self):
        return f"Word 1: {self.word1}\t Word 2: {self.word2}\t Pattern: {self.pattern.printPattern()}"

    def jsonify(self):
        return f'{{"Word1" : {self.word1}, "Word2" : {self.word2}, "Pattern": {{"Fixed" : {self.pattern.fixed: 05b}, "Existing" : {self.pattern.existing:05b}, "Non_Existing" : {self.pattern.non_existing:05b}}}}}'

def readCSV():
    csvfile = open('wordle.csv')
    wordleReader = csv.reader(csvfile)

    for row in wordleReader:
        words.append(row[0])

def generatePatterns():
    for non_existing in range(0b100000):
        for existing in range(0b100000):
            for fixed in range(0b11111):
                if non_existing + existing + fixed == 0b11111:
                    patterns.append(Pattern(fixed, existing, non_existing))


def combinationPossible(word1, word2, pattern):
    for i in range(5):
        letter_is_equal = word1[i] == word2[i]
        if (pattern.fixed >> (4-i)) & 1 and not letter_is_equal:
            return 0

        letter_in_word = word1[i] in word2
        if (pattern.existing >> (4-i)) & 1 and letter_in_word:
            return 0

        # Maybe sonderregelung bei 2x gleichem Buchstaben
        if (pattern.existing >> (4-i)) & 1 and (not letter_in_word or letter_is_equal):
            return 0

    return 1


def generateCombinations(start, stop):
    global progress, last_word
    for word1 in words[start:stop]:
        for pattern in patterns:
            for word2 in words:
                if word1 != word2 and combinationPossible(word1, word2, pattern):
                    tmp = Combination(word1, word2, pattern)
                    combinations.append(tmp)

        progress_lock.acquire(True, -1)
        progress += 1
        last_word = word1
        progress_lock.release()

    return 1


def updateProgressBar():
    global progress
    old_progress = -1
    max_iter = 8#len(words)

    while progress < max_iter:
        if old_progress != progress:
            print('\b'*100 + f"{progress}/{max_iter}  {last_word}", end="")
            old_progress = progress



if __name__ == '__main__':
    start_time = time.time()
    threads = []

    readCSV()
    words_per_thread = len(words)//cpu_count()
    carry = len(words) % cpu_count()
    print("CSV read")

    generatePatterns()
    print("Patterns generated")

    progress_thread = threading.Thread(target=updateProgressBar, args=())
    progress_thread.start()

    for i in range(cpu_count()):
        start = 0#i*words_per_thread
        end = 1#(i+1)*words_per_thread

        if i == cpu_count()-1 and carry:
            end += carry

        thr = threading.Thread(target=generateCombinations, args=(start, end, ))
        thr.start()
        threads.append(thr)

    for thr in threads:
        thr.join()


    progress_thread.join()
    with open('combs.json', 'w') as f:
        f.write("[")
        for comb in combinations:
            f.write(comb.jsonify())
            f.write(",\n")
        f.write("]")

    print("\nTook --- %s seconds" % (time.time()-start_time))