gcc version - 9.3.0

Source Codes:
(a) Inclusive.c (for #1A)
(b) Nine.c (for #1B)
(c) Exclusive.c (for #1C)
(d) miss_lru.cpp (for #2-part1)
(e) miss_opt.cpp (for #2-part2)

Compile the source code and run on each tracefile in terminal as :
gcc -Inclusive.c -o Inclusive
gcc -Nine.c -o Nine
gcc -Exclusive.c -o Exclusive
g++ -miss_lru.cpp -o miss_lru
g++ -miss_opt.cpp -o miss_opt
./Inclusive prefix_of_tracefile number_of_traces_in_application 
./Nine prefix_of_tracefile number_of_traces_in_application 
./Exclusive prefix_of_tracefile number_of_traces_in_application 
./miss_lru prefix_of_tracefile number_of_traces_in_application 
./miss_opt prefix_of_tracefile number_of_traces_in_application 

For eg.-
gcc -Inclusive.c -o Inclusive
./Inclusive bzip2.log_l1misstrace 2

OR 

g++ -miss_opt.cpp -o miss_opt
./miss_opt.cpp hmmer.log_l1misstrace 1

Output format for (a), (b) and (c):
Number of L1 misses: #num1
Number of L2 misses: #num2
Number of L3 misses: #num3

Output format for (d) and (e):
Number of cold misses: #num1
Number of capacity misses: #num2

The number of conflict misses are calculated as :L3 misses in inclusion policy -(cold misses + capacity misses) and are added into to the 'results.pdf'.

Maximum runtime on personal system for one tracefile in (a) : ~4s
Maximum runtime on personal system for one tracefile in (b) : ~3s
Maximum runtime on personal system for one tracefile in (c) : ~3s
Maximum runtime on personal system for one tracefile in (d) : ~10s
Maximum runtime on personal system for bzip2.log_l1misstrace tracefile in (e) : ~1 hr
Maximum runtime on personal system for gcc.log_l1misstrace tracefile in (e) : ~1 hr
Maximum runtime on personal system for h264ref.log_l1misstrace tracefile in (e) : ~15 mins
Maximum runtime on personal system for hmmer.log_l1misstrace tracefile in (e) : ~15 mins
Maximum runtime on personal system for gromacs.log_l1misstrace tracefile in (e) : ~15 mins
Maximum runtime on personal system for sphinx3.log_l1misstrace in (e) : ~4 hrs





