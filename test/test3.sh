#!/bin/bash

echo "Sto eseguendo test3...";
echo -e "numeroThread=8;\nmemoriaMassima=32;\nnumerofile=100;\nsocket=./canale;\nfileLogName=logFile;" > ./setup/config.txt

timeout --signal=SIGINT 30s ./server &
pid=$!
sleep 2s

folder_index=10
file_index=10

#continue until the process is alive
while [ -e /proc/$pid/status ]
do
    #continue only if the process is still alive
    if [ -e /proc/$pid/status ]; then   
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -f ./canale -W ./stresstest/files$folder_index/file$file_index.txt -r ./stresstest/files$folder_index/file$file_index.txt -l ./stresstest/files$folder_index/file$file_index.txt -u ./stresstest/files$folder_index/file$file_index.txt -R 1
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -f ./canale -l ./stresstest/files$folder_index/file$file_index.txt -u ./stresstest/files$folder_index/file$file_index.txt -r ./stresstest/files$folder_index/file$file_index.txt -l ./stresstest/files$folder_index/file$file_index.txt -c ./stresstest/files$folder_index/file$file_index.txt
        
        file_index=$(($file_index - 1))
        
        #cycle folders and file
        if (($file_index == 0)); then
            file_index=10
            folder_index=$(($folder_index - 1))
            if (($folder_index == 0)); then
                folder_index=10
            fi
        fi
    fi
    
done