#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cerr << "need a filename!" << endl;
    return -1;
  }

  ifstream infile(argv[1]);
  if (!infile.is_open())
  {
    cerr << "cannot open file " << argv[1] << endl;
  }

  cout << "Dude!" << endl;
  return 0;
}