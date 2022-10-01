#!/bin/bash

echo "Sto eseguendo test2..."; 
echo -e "numeroThread=4;\nmemoriaMassima=1;\nnumerofile=10;\nsocket=./canale;\nfileLogName=logFile;\npolitica=LFU;" > ./setup/config.txt

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./server &
pid=$!
sleep 2s

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -p -f ./canale -W ./file/amelia.txt -W ./file/bau.txt -W ./file/bob.txt -W ./file/eli.txt
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -p -f ./canale -W ./file/ema.txt -W ./file/fede.txt
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -p -f ./canale -W ./file/leo.txt -W ./file/lori.txt -W ./file/Matteo.txt
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./client -p -f ./canale -D ./LFU -W ./file/nico.txt -W ./file/salvini.txt

sleep 1s

kill -s SIGHUP $pid
wait $pid