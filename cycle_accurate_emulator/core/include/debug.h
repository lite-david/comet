#ifndef DEBUG_H_
#define DEBUG_H_

#include <iostream>

void PrintDebugStatements(){
	std::cout << "";
}

template<typename First, typename ... Strings> 
void PrintDebugStatements(First arg, const Strings&... rest){
	std::cout << arg;
	PrintDebugStatements(rest...);
}

void PrintNewLine(){
	std::cout << std::endl;
}

#endif