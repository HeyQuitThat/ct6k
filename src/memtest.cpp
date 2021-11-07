#include <iostream>
#include <cstdint>
#include "memory.hpp"

int main()
{
	uint32_t foo;
	auto m = new Memory(10);

	m->MemWrite(5,5);
	foo = m->MemRead(0);
	std::cout << foo << "\n";
	foo = m->MemRead(5);
	std::cout << foo << "\n";
	foo = m->MemRead(55);
	std::cout << foo << "\n";
	delete m;
	return 0;
}
