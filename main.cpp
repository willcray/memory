#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <array>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <queue>

using namespace std;

struct page
{
	int present;
	int ptFrameNum;
};

struct tlbPage
{
	int present;
	int pageNum;
	int frameNum;
};

vector<int> timestamps(8);
vector<bool> freeFrames(8);
vector<page> pageTable(16);
vector<tlbPage> tlb(4);
queue<tlbPage> tlbQueue;

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
	for (int i = 0; i < timestamps.size(); ++i)
	{
		++timestamps.at(i);
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

int searchTLB(int p)
{
	for (int i = 0; i < 4; ++i)
	{
		if (tlb.at(i).pageNum == p)
		{
			return i;
		}
	}
	return 5;
}

bool checkTLBFull()
{
	for (int i = 0; i < 4; ++i)
	{
		if (tlb.at(i).present == 0)
		{
			return false;
		}
	}
	return true;
}

void updateTLB(int frameNum, int pageNum)
{
	int tlbEntry = 0;
	if(checkTLBFull())
	{
		// use FIFO to update
		tlbPage newPage = tlbQueue.front();
		tlbQueue.pop();

		// update in TLB
		for (int i = 0; i < 4; ++i)
		{
			if (newPage.pageNum == tlb.at(i).pageNum)
			{
				tlb.at(i).pageNum = pageNum;
				tlb.at(i).frameNum = frameNum;
				tlb.at(i).present = 1;
				tlbEntry = i;
			}
		}
		// update in queue
		newPage.present = 1;
		newPage.pageNum = pageNum;
		newPage.frameNum = frameNum;
		tlbQueue.push(newPage);
	}
	else
	{
		// find first open spot when TLB isn't full
		for (int i = 0; i < 4; ++i)
		{
			if (tlb.at(i).present == 0)
			{
				// update in TLB 
				tlb.at(i).present = 1;
				tlb.at(i).pageNum = pageNum;
				tlb.at(i).frameNum = frameNum;
				tlbEntry = i;

				// update in queue
				tlbPage newPage;
				newPage.present = 1;
				newPage.pageNum = pageNum;
				newPage.frameNum = frameNum;
				tlbQueue.push(newPage);
				break;
			}
		}
	}
	cout << "TLB LOAD: frame " << frameNum<< " containing page " << pageNum << " is stored in TLB entry " << tlbEntry << endl;
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
		if (freeFrames.at(i))
		{
			return i;
		}
	}
	return 8;
}

int main()
{
	vector<int> vAddresses = parse("addresses.txt");

	FILE * backFile;
	backFile = fopen("BACKING_STORE.bin", "rb");
	// load from .bin file into this frame
	if (!backFile)
	{
		cout << "ERROR: could not read BACKING_STORE.bin file" << endl;
		return 0;
	}

	// READ IN ADDRESSES
	for (int i = 0; i < vAddresses.size(); ++i)
	{
		int pageNum = vAddresses.at(i) >> 8;

		// update timestamps for each reference
		incTimestamps();

		// TLB HIT
		int tlbIndex = searchTLB(pageNum);		
		if (tlbIndex != 5)
		{
			cout << "TLB HIT: page " << pageNum << " is contained in frame " << tlb.at(tlbIndex).frameNum << ", found in TLB entry " << tlbIndex << endl;
			resetTimestamp(tlb.at(tlbIndex).frameNum);
		}
		// TLB MISS
		else
		{
			cout << "TLB MISS: page " << pageNum << " is not in the TLB" << endl;
		
			// PAGE FAULT
			if (!pageTable.at(pageNum).present)
			{
				cout << "PAGE FAULT: virtual address " << vAddresses.at(i) << " contained in page " << pageNum << " caused a page fault!" << endl;

				int firstFrame = firstFit();

				// MEMORY FULL, USE LRU
				if (firstFrame == 8)
				{
					int frameToReplace = findLRUFrame();
					
					fseek(backFile, frameToReplace * 256, SEEK_SET);
					char * buffer = new char [256];
					fread(buffer, 1, 256, backFile);
					memcpy(&memory[frameToReplace], buffer, 256);
					free (buffer);					

					cout << "LOADED: page " << pageNum << " is loaded into frame " << frameToReplace << endl;

					// reset that frame's timestamp to 0
					resetTimestamp(frameToReplace);

					// update free frame list
					freeFrames.at(frameToReplace) = false;

					// update page table with new frame info
					pageTable.at(pageNum).present = 1;
					pageTable.at(pageNum).ptFrameNum = frameToReplace;

					// update the TLB
					updateTLB(frameToReplace, pageNum);
				}
				// FRAMES AVAILABLE
				else
				{
					// get line from binary file with associated page number
					fseek(backFile, firstFrame * 256, SEEK_SET);
					char * buffer = new char [256];
					fread(buffer, 1, 256, backFile);
					memcpy(&memory[firstFrame], buffer, 256);
					free (buffer);
					rewind(backFile);
					cout << "LOADED: page " << pageNum << " is loaded into frame " << firstFrame << endl;

					// reset that frame's timestamp to 0
					resetTimestamp(firstFrame);

					// update free frame list
					freeFrames.at(firstFrame) = false;

					// update page table
					pageTable.at(pageNum).present = 1;
					pageTable.at(pageNum).ptFrameNum = firstFrame;

					// UPDATE TLB

					updateTLB(firstFrame, pageNum);
				}

			}
			// NO PAGE FAULT
			else
			{
				resetTimestamp(pageTable.at(pageNum).ptFrameNum);
				cout << "FOUND: page " << pageNum << " is contained in frame " << pageTable.at(pageNum).ptFrameNum << endl;
				// update TLB
				updateTLB(pageTable.at(pageNum).ptFrameNum, pageNum);
			}
		}
		cout << endl;
	}
	fclose(backFile);

	cout << "Total address references: " << vAddresses.size() << endl;
	cout << "TLB Hits: ";
	// print contents of page table
	// print contents of page frames
	// print page-fault rate
	// print TLB hit rate

	return 0;
}