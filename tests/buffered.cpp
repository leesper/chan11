#include <iostream>
#include "../src/chan.h"
using namespace std;
using namespace chan11;

int main()
{
	Chan ch = make_chan(2);
	ch.send(1);
	ch.send(2);

	cout << "first element: " << ch.recv() << endl;
	cout << "second element: " << ch.recv() << endl;

	return 0;
}
