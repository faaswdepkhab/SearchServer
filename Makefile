compile:
	g++ -std=c++17 -c document.cpp -o document.o
	g++ -std=c++17 -c read_input_functions.cpp  -o read_input_functions.o
	g++ -std=c++17 -c search_server.cpp -o search_server.o
	g++ -std=c++17 -c request_queue.cpp -o request_queue.o
	g++ -std=c++17 -c string_processing.cpp -o string_processing.o
	g++ -std=c++17 -c test_example_functions.cpp -o test_example_functions.o
	g++ -std=c++17 -c process_queries.cpp -o process_queries.o
	g++ -std=c++17 main.cpp -lpthread -ltbb -o searchserver document.o read_input_functions.o search_server.o request_queue.o string_processing.o test_example_functions.o process_queries.o
clean:
	rm document.o
	rm read_input_functions.o
	rm request_queue.o
	rm search_server.o
	rm string_processing.o
	rm test_example_functions.o
	rm process_queries.o
	rm searchserver
