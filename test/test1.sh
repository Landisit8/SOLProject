#!/bin/bash

echo "Sto eseguendo test1..."; 
echo -e "numeroThread=1;\nmemoriaMassima=128;\nnumerofile=10000;\nsocket=./canale;\nfileLogName=logFile;" > ./setup/config.txt

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./server &
pid=$!
sleep 2s
#-t 200
./client -p -f ./canale -W ./file/Matteo.txt
./client -p -f ./canale -r ./file/Matteo.txt
./client -f ./canale -h

sleep 1s

kill -s SIGHUP $pid
wait $pid