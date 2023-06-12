#!/bin/bash

file_name="sfsfs"

g++ "target/C/1805121/main.c" -o "target/C/1805121/main.out"
chmod u+x "target/C/1805121/main.out"
executable="target/C/1805121/main.out"
input_file="tests/test3.txt"
"$executable" < "$input_file"
#`target/C/1805121/main.out` "tests/test1.txt"


ans_file="$ans_file/ans$file_no.txt"
                diff "$output_file" "$ans_file" > /dev/null                                #/dev/null will stop the terminal from showing differences of the two files
                matched=$?                                                                 #diff will return exit status 0 if everything is same,and a nonzero integer if not 
                if [ $matched -eq 0 ]; then
                    correct=`expr $correct+1`
                else
                    incorrect=`expr $incorrect+1`
                fi