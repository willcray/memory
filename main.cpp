#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

using namespace std;

vector<vector<int>> parse(string fileName)
{
	vector<vector<int>> parsedVAs;

	ifstream file;
	file.open(fileName);
	if(!file.is_open())
	{
		cout << "ERROR: file not opened properly" << endl;
		exit(1);
	}
	else
	{
		string addr;
		// file is opened properly
		while(file.good())
		{
			vector<int> j;
			getline(file, addr, '\n');
			int iAddr = stoi(addr);
			int pageNum = iAddr >> 8;
			int offset = iAddr % 256;
			j.push_back(pageNum);
			j.push_back(offset);
			parsedVAs.push_back(j);
		}
		file.close();		
	}
	return parsedVAs;
}

int main()
{
	vector<vector<int>> vAddresses = parse("addresses.txt");
	return 0;
}