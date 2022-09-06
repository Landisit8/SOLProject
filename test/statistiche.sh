#!/bin/bash

#function that centers the text and puts in between two lines
center() 
{
    printf '=%.0s' $(seq 1 $(tput cols))
    echo "$1" | sed  -e :a -e "s/^.\{1,$(tput cols)\}$/ & /;ta" | tr -d '\n' | head -c $(tput cols)
    printf '=%.0s' $(seq 1 $(tput cols)) | sed 's/^ //'
}

# variables to save stats
nRead=0
totBr=0

nWrite=0
totBw=0

nLock=0
nOpen=0
nUnlock=0
nClose=0

maxMem=0
maxFile=0
totExp=0

nThreds=0
nRequest=0

maxCon=0

#if the log file exists
if [ -e "logFile.txt" ]; then

    #counting occurrences of key words
    nRead=$(grep "Read" "logFile.txt" | wc -l)
    nWrite=$(grep "Write" "logFile.txt" | wc -l)
    nLock=$(grep "Lock" "logFile.txt" | wc -l)
    nOpen=$(grep "Open" "logFile.txt" | wc -l)
    nUnlock=$(grep "Unlock" "logFile.txt" | wc -l)    
    nClose=$(grep "Close" "logFile.txt" | wc -l)
    maxCon=$(grep "Connessione" "logFile.txt" | wc -l)

    #taking values from stats variables
    maxMem=$(grep -e "Memoria" "logFile.txt" | cut -c 8- )
    maxFile=$(grep -e "Numero" "logFile.txt" | cut -c 10- )
    totExp=$(grep -e "LFU" "logFile.txt" | cut -c 5- )

    #counting the number of threads that were activated
    nThreds=$(grep -e "Thread" "logFile.txt" | wc -l)

    #sum of all the bytes that were written
    for i in $(grep -e "Bytes" "logFile.txt" | cut -c 7- ); do   
        totBw=$totBw+$i;
    done
    #obtaining sum
    totBw=$(bc <<< ${totBw})

    #sum of all the bytes that were written
    for i in $(grep -e "Size" "logFile.txt" | cut -c 6- ); do   
        totBr=$totBr+$i;
    done

    #obtaining sum
    totBr=$(bc <<< ${totBr})

    #printing values
    center "SERVER STATS"

    echo " "

    enable -n echo

    echo "Numero di reads: ${nRead}"

    echo -n "Average bytes read: "
    #calculating the mean value and printing it
    if [ ${nRead} != 0 ]; then
        echo "scale=0; ${totBr} / ${nRead}" | bc -l
    fi

    echo "Numero di writes: ${nWrite}"
    
    echo -n "Average bytes wrote:" 
    #calculating the mean value and printing it
    if [ ${nWrite} != 0 ]; then
        echo "scale=0; ${totBw} / ${nWrite}" | bc -l
    fi

    echo "Numero di locks: ${nLock}"
    echo "Numero di open lock: ${nOpen}"
    echo "Numero di unlock: ${nUnlock}"
    echo "Numero di close: ${nClose}"
    echo "Max memoria raggiunta: ${maxMem}"
    echo "Max file salvati: ${maxFile}"
    echo "Volte che un file è stato espulso per mancanza di capacità: ${totExp}"

    for i in $(grep "InizioThread" "logFile.txt" | cut -c 15- ); do  
        nRequest=$(grep "$i" "logFile.txt" | wc -l )
        echo "$nRequest le richieste sono state soddisfatte dal thread $i"
    done

    echo "Max Connessioni: ${maxCon}"

    echo " "
    center "END"
           
else
    echo "No log file found"
fi