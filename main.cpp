#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <array>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

struct page
{
	int present;
	int ptFrameNum;
};

vector<int> timestamps(8);

vector<page> pageTable(16);
/*
struct tlbPage
{
	int free;
	int pageNum;
	int tlbFrameNum;
}
*/	

char memory[8][256];

vector<int> parse(string fileName)
{
	vector<int> virtualAddrs;

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
			getline(file, addr, '\n');
			virtualAddrs.push_back(stoi(addr));
		}
		file.close();		
	}
	return virtualAddrs;
}

void incTimestamps()
{
	for (auto &timestamp : timestamps)
	{
		++timestamp;
	}
}

void resetTimestamp(int frame)
{
	timestamps.at(frame) = 0;
}

void printMemory()
{
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			cout << " " << memory[i][j];
		}
		cout << endl;
	}
}

int findLRUFrame()
{
	int maxTime = 0;
	int LRUFrame = 0;
	for (int i = 0; i < timestamps.size(); ++i)
	{
		if (timestamps.at(i) > maxTime)
		{
			maxTime = timestamps.at(i);
			LRUFrame = i;
		}
	}
	return LRUFrame;
}

int firstFit()
{
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// if a one is found in this frame
			if (memory[i][j] == '1')
			{
				break;
			}
			if (j >= 255)
			{
				return i;
			}
		}
	}
	return 8;
}

int main()
{
	vector<int> vAddresses = parse("addresses.txt");
	FILE * backFile;
	//ifstream backFile ("addresses.txt", ifstream::binary);
	// backFile = fopen("BACKING_STORE.bin", "rb");
	// load from .bin file into this frame
	if (!backFile)
	{
		cout << "ERROR: could not read BACKING_STORE.bin file" << endl;
		return 0;
	}

	// READ IN ADDRESSES
	for(auto &addr : vAddresses)
	{		
		int pageNum = addr >> 8;

		// update timestamps for each reference
		incTimestamps();

		// PAGE FAULT
		if (!pageTable.at(pageNum).present)
		{
			cout << "PAGE FAULT: virtual address " << addr << " contained in page " << pageNum << " caused a page fault!" << endl;

			int firstFrame = firstFit();
			if (firstFrame == 8)
			{
				// if no frames available, use LRU
				int frameToReplace = findLRUFrame();
				/*
				char * buffer = (char*) malloc (sizeof(char) * 256);
				backFile.seekg(256 * pageNum, backFile.beg);
				backFile.read(buffer, 256);
				*/

				// get line from binary file with associated page number
				
				fseek(backFile, frameToReplace * 256, SEEK_SET);
				char * buffer = new char [256];
				fread(buffer, 1, 256, backFile);
				/*
				if(addr == 1828)
				{
					cout << "before mem transfer" << endl;
					for (int i = 0; i < 256; ++i)
					{
						cout << memory[frameToReplace][i];
					}
					cout << endl;
				}
				*/
				memcpy(&memory[frameToReplace], buffer, 256);
				free (buffer);
				// rewind(backFile);
				/*
				if(addr == 1828)
				{
					cout << "after mem transfer" << endl;
					for (int i = 0; i < 256; ++i)
					{
						cout << memory[frameToReplace][i];
					}
					cout << endl;
				}
				*/
				

				cout << "page " << pageNum << " is loaded into frame " << frameToReplace << endl;

				// reset that frame's timestamp to 0
				resetTimestamp(frameToReplace);

				// update page table with new frame info
				pageTable.at(pageNum).present = 1;
				pageTable.at(pageNum).ptFrameNum = frameToReplace;
			}
			else
			{
				// if first fit finds a frame
				/*
				if(addr == 1828)
				{
					cout << "before mem transfer" << endl;
					for (int i = 0; i < 256; ++i)
					{
						cout << memory[firstFrame][i];
					}
					cout << endl;
				}
				*/
				// get line from binary file with associated page number
				fseek(backFile, firstFrame * 256, SEEK_SET);
				char * buffer = new char [256];
				fread(buffer, 1, 256, backFile);
				memcpy(&memory[firstFrame], buffer, 256);
				/*
				char * buffer = (char*) malloc (sizeof(char) * 256);
				backFile.seekg(256 * pageNum, backFile.beg);
				backFile.read(buffer, 256);
				memcpy(&memory[firstFrame], buffer, 256);
				*/
				/*
				if(addr == 1828)
				{
					cout << "buffer contains these elements: " << endl;
					for (int i = 0; i < 256; ++i)
					{
						cout << buffer[i];
					}
				cout << endl;	
				}
				*/
				// memcpy(&memory[firstFrame], buffer, 256);
				free (buffer);
				// rewind(backFile);
				/*
				if(addr == 1828)
				{
					cout << "after mem transer" << endl;
					for (int i = 0; i < 256; ++i)
					{
						cout << memory[firstFrame][i];
					}
				cout << endl;	
				}
				*/
				
				
				

				cout << "page " << pageNum << " is loaded into frame " << firstFrame << endl;

				// update page table
				pageTable.at(pageNum).present = 1;
				pageTable.at(pageNum).ptFrameNum = firstFrame;
			}


			
			
		}
		// PAGE IS PRESENT IN MEM
		else
		{
			cout << "page " << pageNum << " is contained in frame " << pageTable.at(pageNum).ptFrameNum << endl;

			// get frame number and update TLB using FIFO if necessary
		}
	}
	fclose(backFile);
	printMemory();
	//backFile.close();

	// print contents of page table
	// print contents of page frames
	// print page-fault rate
	// print TLB hit rate

	return 0;
}