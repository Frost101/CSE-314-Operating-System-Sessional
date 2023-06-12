#!/bin/bash

verbose=0           #'0' means no printing informations,'1' means print useful information
noexecute=0         #'1' means no executable file will be generated,'0' means generate executable files and outputs 
warning_msg="Usage:
./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]

-v: verbose
-noexecute: do not execute code files"
file_name="dummy"   #in each submission folder,visit() will a search for a c/java/py file and return with this parameter


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


#At first Check the command line arguments and extract necessary info
if [ "$#" -lt 4 ]
then
    #Show warning messages
    echo "$warning_msg"
    exit

elif [ "$#" -eq 5 ]
then
    #if the number of arguments is equal to 5,then the fifth argument can be either -v or -noexecute
    if [ "$5" = "-v" ]
    then
        verbose=1
    elif [ "$5" = "-noexecute" ]
    then
        noexecute=1
    else
        echo "$warning_msg"
        exit
    fi

elif [ "$#" -eq 6 ]
then
    #if the number of arguments is equal to 6,then the 5th argument is '-v' and the 6th argument is '-noexecute'
    if ( [ "$5" = "-v" ] && [ "$6" = "-noexecute" ] )
    then 
        verbose=1
        noexecute=1
    else
        echo "$warning_msg"
        exit
    fi
fi


#Extract necessary folder location information from command line arguments
submission_folder="$1"
target_folder="$2"
test_folder="$3"
ans_folder="$4"
curr_roll=0
current_direct=`pwd`

mkdir -p "temp"         #create a temporary file to store unzipped files from submission for further organizing tasks

for i in "$submission_folder"/*
do  
    curr_roll=`echo ${i%.zip}`      #At first,remove the extension from file name
    curr_roll=${curr_roll: -7}      #Then extract last 7 digits(Roll no) from the string 

    if [ $verbose -eq 1 ]
    then
        echo "Organizing files of $curr_roll"
    fi

    unzip -q "$i" -d "temp/"       #unzip it in temporary folder

    visit "temp"                   #call recursive function in that temp folder to find any c/java/py files
    extension="${file_name##*.}"   #Extract the extension
    if [ $extension = "c" ]
    then
        mkdir -p "$target_folder/C/$curr_roll"
        cp "$file_name" "$target_folder/C/$curr_roll/main.c" 
    elif [ $extension = "java" ]
    then
        mkdir -p "$target_folder/Java/$curr_roll"
        cp "$file_name" "$target_folder/Java/$curr_roll/Main.java" 
    elif [ $extension = "py" ]
    then
        mkdir -p "$target_folder/Python/$curr_roll"
        cp "$file_name" "$target_folder/Python/$curr_roll/main.py" 
    fi

    rm -r "temp"/*              #Clear the temporary folder everytime after unzipping and organizing one's submission
done

rm -r "temp"                    #Remove unnecessary temp folder



