// signed conversion test
#include <cstdint>
#include <iostream>
#include <climits>


int main()
{
	using namespace std;
	uint32_t tmp1 = 0xFFFFFFFF;
	int32_t tmp2 = INT_MAX;


	cout << "tmp1 = " << tmp1 << "\n";
	tmp1++;
	cout << "after increment, tmp1 = " << tmp1 << "\n";
	cout << "tmp2 = " << tmp2 << "\n";
	tmp2++;
	cout << "after increment, tmp2 = ";
	cout << hex << tmp2;
	cout << "\n";
	cout << "INT_MIN =" << INT_MIN << "\n";
	tmp1 = tmp2;
	cout << "after conversion to unsigned, tmp1 = ";
	cout << hex << tmp1;
	cout << "\n";
	return 0;
}
