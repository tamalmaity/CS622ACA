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
2. Build the tool using following command:	
	make obj-intel64/addrtrace.so
3. Run pin with the tool addrtrace using following command:
	../../../pin -t obj-intel64/addrtrace.so -- ./prog1 8
	It will ask for the program number on which you are running the tool(suffix of prog)
	Enter 1 for prog1.c.
	Similarly, to run pin with the tool addrtrace for prog2, prog3, pro4 use the commands:
	../../../pin -t obj-intel64/addrtrace.so -- ./prog2 8
	../../../pin -t obj-intel64/addrtrace.so -- ./prog3 8
	../../../pin -t obj-intel64/addrtrace.so -- ./prog4 8
	
	It will ask for the program number on which you are running the tool(suffix of program)
	Enter 2 for prog2.c, 3 for prog3.c and 4 for prog4.c respectively.
	The output of this tool is stored in the file “mach_acc_suffix.txt” where  suffix stands for suffix of the program
4. To generate the required output for the assignment, run following commands
	g++ main.cpp -o main
	./main
	It will ask for the program number on which you are running the program(suffix of prog)
