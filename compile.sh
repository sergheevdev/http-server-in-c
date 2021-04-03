#!/bin/bash
echo
echo "Compiling the program..."
echo
gcc -O0 -g -o main main.c -no-pie -lpthread
echo
echo "Checking for memory leaks..."
echo
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=report.txt \
         ./main
echo
echo "Printing profiler report..."
echo
tail -n7 report.txt

