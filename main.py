import csv
import threading
from os import cpu_count
import time
import json
from tqdm import tqdm, trange

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
                    patterns.append(Pattern(format(fixed, "05b"), format(existing, "05b"), format(non_existing, "05b")))


def combinationPossible(word1, word2, pattern):
    for i in range(0, 5):
        if pattern.fixed[i] == '1' and word1[i] != word2[i]:
            return 0

    for i in range(0, 5):
        # Maybe sonderregelung bei 2x gleichem Buchstaben
        if pattern.existing[i] == '1' and (not word1[i] in word2 or word1[i] == word2[i]):
            return 0

    for i in range(0, 5):
        if pattern.non_existing[i] == '1' and word1[i] in word2:
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
    max_iter = len(words)

    while progress < max_iter:
        if old_progress != progress:
            print(f"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b{progress}/{max_iter}  {last_word}", end="")
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
        start = i*words_per_thread
        end = (i+1)*words_per_thread

        if i == cpu_count()-1 and carry:
            end += carry

        thr = threading.Thread(target=generateCombinations, args=(start, end, ))
        thr.start()
        threads.append(thr)

    for thr in threads:
        thr.join()


    progress_thread.join()
    with open('combs.json', 'w') as f:
        json.dump(combinations, f)

    print("Took --- %s seconds" % (time.time()-start_time))