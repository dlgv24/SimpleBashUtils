#!/bin/bash

python -c "import itertools
options = ['-i', '-v', '-c', '-l', '-n', '-h', '-s', '-o']
for i in range(1, 3):
    for j in itertools.combinations(options, i):
        print(' '.join(j))" > options.txt

python -c "import itertools, os
files = os.listdir('tests/')
files.remove('patterns.txt')
for i in range(len(files)):
    files[i] = 'tests/' + files[i]
for i in range(1, len(files) + 1):
    for j in itertools.combinations(files, i):
        print(' '.join(j))" > files.txt

COUNTER_SUCCESS=0
COUNTER_FAIL=0
DIFF_RES=""

while read options;
do
    while read files;
    do
        ./s21_grep -e .l -f tests/patterns.txt $options $files &> s21_grep.txt
        grep -e .l -f tests/patterns.txt $options $files &> grep.txt
        DIFF_RES=$(diff -q s21_grep.txt grep.txt)
        if [[ "$DIFF_RES" == "" ]];
        then
            (( COUNTER_SUCCESS++ ))
        else
            (( COUNTER_FAIL++ ))
        fi
    done < <(cat files.txt)
done < <(cat options.txt)

rm options.txt files.txt s21_grep.txt grep.txt

echo "$(tput setaf 2)SUCCESS:$(tput sgr0) $COUNTER_SUCCESS"
echo "$(tput setaf 1)FAIL:$(tput sgr0) $COUNTER_FAIL"
