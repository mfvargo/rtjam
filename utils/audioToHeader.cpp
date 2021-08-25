#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
  vector<float> samples;
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
  float number = 0;
  while (infile >> number)
  {
    samples.push_back(number);
  }
  cout << "samples read: " << samples.size() << endl;

  cout << "Dude!" << endl;
  return 0;
}