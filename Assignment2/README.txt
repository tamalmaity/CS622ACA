Steps need to be followed to run the assignment:
1. Generate binaries on which we want to run the pin tool.
Compile the files prog1.c,prog2.c,prog3.c and prog4.c using following commands respectively.
gcc -O3 -static -pthread prog1.c -o prog1
gcc -O3 -static -pthread prog2.c -o prog2
gcc -O3 -static -pthread prog3.c -o prog3
gcc -O3 -static -pthread prog4.c -o prog4
Run the files using following commands:
	./prog1 8
	./prog2 8
	./prog3 8
	./prog4 8
	
	
	
2.Build the tool using following command:	
	make obj-intel64/1_addrtrace.so
	
	
	
3. Run pin with the tool 1_addrtrace using following command:
	../../../pin -t obj-intel64/1_addrtrace.so -- ./prog1 8
	It will ask for the program number on which you are running the tool(suffix of 	prog)
	Enter 1 for prog1.c.
	Similarly, to run pin with the tool 1_addrtrace for prog2, prog3, pro4 use the 	commands:
	../../../pin -t obj-intel64/1_addrtrace.so -- ./prog2 8
	../../../pin -t obj-intel64/1_addrtrace.so -- ./prog3 8
	../../../pin -t obj-intel64/1_addrtrace.so -- ./prog4 8
	
	It will ask for the program number on which you are running the tool(suffix of program where suffix takes the value of 1,2,3,4)
	Enter 2 for prog2.c, 3 for prog3.c and 4 for prog4.c respectively.
	The output of this tool is stored in the file “addrtrace_suffix.txt” where  suffix stands for suffix of the program
	
The output will get stored in the file “mach_acc_suffix.txt”.
	
	
	
4. To generate the output of PART II,compile and run using following command:
	g++ 2_acc_dist.cpp -o 2_acc_dist
	./2_acc_dist 
	It will ask for the same suffix to store the cumulative density function in the file named as “acc_dist_suffix.txt”
	To generate the plot in PART II, use the command:
	python3 3_plot.py 
	Enter the suffix of program to generate the graph.
	
	
	
5. To generate the output of PART III, compile and run using following command:
	g++ 4_LRU_traces.cpp -o 4_LRU_traces
	./4_LRU_traces 
	Enter the suffix of prog. The machine addresses which are missed are 	stored in file named as “addrmiss_suffix.txt”.
	
	Compile and run using following command to calculate cumulative density	function:
	g++ 5_acc_dist_miss.cpp -o 5_acc_dist_miss
	./5_acc_dist_miss 
	Enter the suffix of prog. The result will be stored in file named as 	“LRU_acc_dist_suffix.txt”.
To generate the plot in PART III, use the command:
	python3 6_plot.py 
	Enter the suffix of program to generate the graph
	
	
6. To generate the output of PART IV, use the following commands:
	g++ 7_sharing.cpp -o 7_sharing
	./7_sharing
	Enter the suffix of program as input




