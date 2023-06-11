#!/bin/bash

verbose=0       #'0' means no printing informations,'1' means print useful information
noexecute=0     #'1' means no executable file will be generated,'0' means generate executable files and outputs 
warning_msg="Usage:
./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]

-v: verbose
-noexecute: do not execute code files"

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
submission_folder="$1""/"
target_folder="$2"
test_folder="$3"
ans_folder="$4"
current_direct=`pwd`


cd "$submission_folder"

for i in *
do
    echo $i
    unzip "$i" -d "$current_direct""/temp/"
done

