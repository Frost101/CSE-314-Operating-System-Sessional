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


show_execution_msg()
{
    #Show execution message
    if [ $noexecute -eq 0 ] && [ $verbose -eq 1 ]
    then
        echo "Executing files of $1"
    fi
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
#But at first,check if aubmission folder,test folder and answer folder path are correctly given or not
if [ -d "$1" ] && [ -d "$3" ] && [ -d "$4" ]
then
    submission_folder="$1"
    target_folder="$2"
    test_folder="$3"
    ans_folder="$4"
else
    echo "$warning_msg"
    exit
fi

curr_roll=0                                #To extract Roll number from submission zip file and store
current_direct=`pwd`


#If any target folder exists from previous code,delete it
if [ -d "$target_folder" ]
then
    rm -r "$target_folder"
fi


mkdir -p "temp"                 #create a temporary file to store unzipped files from submission for further organizing tasks
mkdir -p "$target_folder"       #Create target directory for file organization
#Create a .csv file if no execute = 0
if [ $noexecute -eq 0 ]
then
    csv_file="$target_folder/result.csv"
    echo "student_id,type,matched,not_matched" >> "$csv_file"
fi


#To show the message "Found n test files"
test_file_count=0
for i in "$test_folder"/*.txt
do
    test_file_count=`expr $test_file_count + 1`
done
if [ $verbose -eq 1 ]
then
    echo "Found $test_file_count test files"
fi



#Unzip,Organize,execute,match,generate CSV
for i in "$submission_folder"/*
do  
    curr_roll=`echo ${i%.zip}`     #At first,remove the extension from file name
    curr_roll=${curr_roll: -7}     #Then extract last 7 digits(Roll no) from the string 
    unzip -q "$i" -d "temp/"       #unzip it in temporary folder
    visit "temp"                   #call recursive function in that temp folder to find any c/java/py files
    extension="${file_name##*.}"   #Extract the extension

    #Show msg of organizing files if verbose is 1
    if [ $verbose -eq 1 ]
    then
        echo "Organizing files of $curr_roll"
    fi

    #check for the file's extension and start organizing and executing it

    #Create a folder for "C"
    if [ $extension = "c" ]
    then
        mkdir -p "$target_folder/C/$curr_roll"                                                  #Open a folder name C and a subdirectory with roll no                                              
        cp "$file_name" "$target_folder/C/$curr_roll/main.c"                                    #copy that source file into specific destination

        show_execution_msg "$curr_roll"                                                         #Show execution message
        #execute only if noexecute is off
        if [ $noexecute -eq 0 ]
        then
            g++ "$target_folder/C/$curr_roll/main.c" -o "$target_folder/C/$curr_roll/main.out"      #build that .c file
            executable="$target_folder/C/$curr_roll/main.out"
            chmod u+x "$executable"                                                                 #Give permission to execute-Important***

            correct=0                                                                      #no of matched output files
            incorrect=0                                                                    #no of unmatched output files
            
            #now execute the files
            for input_file in "$test_folder"/*.txt
            do  
                #execute and  generate output like out1.txt,out2.txt,out3.txt
                file_no=`echo ${input_file%.txt}`                                          #extract the name from input file..ex:test4.txt...extract test4 from it
                file_no=${file_no: -1}                                                     #extract the number from input file..ex:"test4"...extract 4 from it
                output_file="$target_folder/C/$curr_roll/out$file_no.txt"                  #Output File path "target/C/1905101/out4.txt"
                "$executable" < "$input_file" > "$output_file"                             #run

                #now match and generate CSV
                ans_file="$ans_folder/ans$file_no.txt"
                diff "$output_file" "$ans_file" > /dev/null                                #/dev/null will stop the terminal from showing differences of the two files
                matched=$?                                                                 #diff will return exit status 0 if everything is same,and a nonzero integer if not 
                if [ $matched -eq 0 ]; then
                    correct=`expr $correct + 1`
                else
                    incorrect=`expr $incorrect + 1`
                fi
            done
            echo "$curr_roll,C,$correct,$incorrect" >> "$csv_file"                         #update The csv file
        fi


    #Create a folder for Java and execute
    elif [ $extension = "java" ]
    then
        mkdir -p "$target_folder/Java/$curr_roll"                                               #Open a directory named Java and subdirectory 1905101
        cp "$file_name" "$target_folder/Java/$curr_roll/Main.java"                              #Copy that .java file 

        show_execution_msg "$curr_roll"                                                         #Show execution message
        #execute only if noexecute is off
        if [ $noexecute -eq 0 ]
        then
            javac "$target_folder/Java/$curr_roll/Main.java"
            executable="$target_folder/Java/$curr_roll/Main.java"
            chmod u+x "$executable"  

            correct=0
            incorrect=0

            #now execute the files
            for input_file in "$test_folder"/*.txt
            do
                #execute and  generate output like out1.txt,out2.txt,out3.txt
                file_no=`echo ${input_file%.txt}`                                          #extract the name from input file..ex:test4.txt...extract test4 from it
                file_no=${file_no: -1}                                                     #extract the number from input file..ex:"test4"...extract 4 from it
                output_file="$target_folder/Java/$curr_roll/out$file_no.txt"               #Output File path "target/Java/1905101/out4.txt"
                java "$executable" < "$input_file" > "$output_file"                        #run


                #now match and generate CSV
                ans_file="$ans_folder/ans$file_no.txt"
                diff "$output_file" "$ans_file" > /dev/null                                #/dev/null will stop the terminal from showing differences of the two files
                matched=$?                                                                 #diff will return exit status 0 if everything is same,and a nonzero integer if not 
                if [ $matched -eq 0 ]; then
                    correct=`expr $correct + 1`
                else
                    incorrect=`expr $incorrect + 1`
                fi
            done
            echo "$curr_roll,Java,$correct,$incorrect" >> "$csv_file"                       #update The csv file
        fi
        
        

    #Create a folder for Python and execute
    elif [ $extension = "py" ]
    then
        mkdir -p "$target_folder/Python/$curr_roll"
        cp "$file_name" "$target_folder/Python/$curr_roll/main.py"

        show_execution_msg "$curr_roll"                                                     #Show execution message
        #execute only if noexecute is off
        if [ $noexecute -eq 0 ]
        then
            correct=0                                                                      #no of matched output files
            incorrect=0  

            #now execute the files
            for input_file in "$test_folder"/*.txt
            do
                #execute and  generate output like out1.txt,out2.txt,out3.txt
                file_no=`echo ${input_file%.txt}`                                          #extract the name from input file..ex:test4.txt...extract test4 from it
                file_no=${file_no: -1}                                                     #extract the number from input file..ex:"test4"...extract 4 from it
                output_file="$target_folder/Python/$curr_roll/out$file_no.txt"                  #Output File path "target/Python/1905101/out4.txt"

                python3 "$target_folder/Python/$curr_roll/main.py" < "$input_file" > "$output_file"      #run

                #now match and generate CSV
                ans_file="$ans_folder/ans$file_no.txt"
                diff "$output_file" "$ans_file" > /dev/null                                #/dev/null will stop the terminal from showing differences of the two files
                matched=$?                                                                 #diff will return exit status 0 if everything is same,and a nonzero integer if not 
                if [ $matched -eq 0 ]; then
                    correct=`expr $correct + 1`
                else
                    incorrect=`expr $incorrect + 1`
                fi
            done 
            echo "$curr_roll,Python,$correct,$incorrect" >> "$csv_file"                         #update The csv file
        fi
    fi

    rm -r "temp"/*              #Clear the temporary folder everytime after unzipping and organizing one's submission
done

rm -r "temp"                    #Remove unnecessary temp folder



