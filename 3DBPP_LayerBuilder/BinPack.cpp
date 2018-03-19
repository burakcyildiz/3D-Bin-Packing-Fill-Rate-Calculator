#include "Common.h"
#include "LayerGenerator.h"
#include <cstdio>
#include <iostream>

int main(int argc, char **argv)
{
	vector<string> filesToRead;
	BinWidth = 100;
	BinDepth = 100;
	BinHeight = 100;
	binVolume = BinWidth * BinDepth * BinHeight;

	ifstream myfile1;
	myfile1.open("filesToRead.txt");

	string line;

	size_t pos = 0;
	string token;
	int tempTime;

	const string delimiter = "\t";
	vector<vector<string>> v;
	vector<string> fields;

	//Read data
	for (myfile1; getline(myfile1, line);)
	{
		v.push_back(split(fields, line, delimiter));
	}
	myfile1.close();

	for (int i = 0; i < v.size(); i++)
	{
		filesToRead.push_back(v.at(i).at(0).c_str());
	}

	string filename;

	ofstream fillrateop;
	fillrateop.open("Fill Rate.txt");

	fillrateop << "Case Name" << "\t"
		<< "Fill Rate" << "\t"
		<< "Fill Rate (50%)" << endl;

	for (int q = 0; q < filesToRead.size(); q++)
	{
		string filename = filesToRead[q];

		//srand(unsigned(time(0)));
		//string filename = "50_Cases_4";
		parseInput(filename);
		//generateSupportSuperItems();

		double fillrate = 0;
		double fillrate50 = 0;
		vector<double> fillrates;
		vector<double> fillrates50;
		vector<double> volumes;
		vector<double> volumes50;

		fillrates.resize(bins.size());
		fillrates50.resize(bins.size());
		volumes.resize(bins.size());
		volumes50.resize(bins.size());

		for (int b = 0; b < bins.size(); b++)
		{
			for (int i = 0; i < bins[b].widths.size(); i++)
			{
				volumes[b] += bins[b].widths[i] * bins[b].depths[i] * bins[b].heights[i];

				if(bins[b].yCoords[i] + bins[b].depths[i] <= 0.5 * BinDepth)
					volumes50[b] += bins[b].widths[i] * bins[b].depths[i] * bins[b].heights[i];
				else if (bins[b].yCoords[i] < 0.5 * BinDepth)
					volumes50[b] += bins[b].widths[i] * (0.5 * BinDepth - bins[b].yCoords[i]) * bins[b].heights[i];
			}

			fillrates[b] = volumes[b] / binVolume;
			fillrates50[b] = volumes50[b] / (binVolume * 0.5);
		}

		double fillratesum = 0;
		double fillrate50sum = 0;

		for (int b = 0; b < bins.size(); b++)
		{
			fillratesum += fillrates[b];
			fillrate50sum = fillrates50[b];
		}

		fillrate = fillratesum / bins.size();
		fillrate50 = fillrate50sum / bins.size();

		

		fillrateop << filename << "\t"
				<< fillrate << "\t"
				<< fillrate50 << endl;

		bins.clear();
	}
}
