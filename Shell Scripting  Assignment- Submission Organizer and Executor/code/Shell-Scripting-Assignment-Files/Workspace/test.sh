#!/bin/bash

file_name="sfsfs"

visit()
{
    for file in "$1"/*
    do
        if [ -d "$file" ]
        then
            visit "$file"
        else
            extension="${file##*.}"
            if [ $extension = "c" ] || [ $extension = "java" ] || [ $extension = "py" ] 
            then
                file_name="$file"
                return
            fi
        fi
    done
}

visit $1
echo $file_name