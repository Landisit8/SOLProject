#!/bin/bash

echo "Test2:"; 
echo -e "numeroThread=4;\nmemoriaMassima=1;\nnumerofile=10;\nsocket=./canale;\nfileLogName=logFile;\npolitica=FIFO;" > ./setup/config.txt

./server &
pid=$!
sleep 2s

./client -p -f ./canale -W ./file/amelia.txt -W ./file/bau.txt -W ./file/bob.txt -W ./file/eli.txt
./client -p -f ./canale -W ./file/ema.txt -W ./file/fede.txt
./client -p -f ./canale -W ./file/leo.txt -W ./file/lori.txt -W ./file/Matteo.txt
./client -p -f ./canale -D ./LFU -W ./file/nico.txt -W ./file/salvini.txt

sleep 1s

kill -s SIGHUP $pid
wait $pid