


#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <unordered_map>
#include <iostream>

double BinWidth;
double BinDepth;
double BinHeight;

int nItems;
int nbLines;
double binVolume;

using namespace std;


struct Item
{
	int ID;
	int place;
	int inputPlace;
	int x,y,z;
	//int FrontLeftX, BackRightX, BackRightY, CenterX, CenterY, TopZ;
	int width, depth, height;
	vector<int> buttomSupportItems;
	double reducedCost;
	bool selected;
	int orientation; //0 if xy orientation, 1 if xz orientation, 2 if yz orientation
	double coveredSpace;

	bool superItem;
	int superItemOnTop;
	vector<Item> superItemList;
	vector<int> superItemX;
	vector<int> superItemY;
	vector<int> superItemZ;
	vector<int> superItemWidth;
	vector<int> superItemDepth;
	vector<int> superItemHeight;
	vector<int> nbItemsInSILayer;

	struct Item(int id, int W, int D, int H, int ori)
	{
		width = W;
		depth = D;
		height = H;
		ID = id;
		superItem = false;
		selected = false;
		orientation = ori;
		coveredSpace = 0;
	}

	struct Item(int W, int D, int H)
	{
		width = W;
		depth = D;
		height = H;
		superItem = true;
		superItemOnTop = 1;
		coveredSpace = 0;
	}

	struct Item()
	{
		coveredSpace = 0;
	}

	void setX(int newX)
	{
		this->x = newX;
		//this->updateXVars();
	}

	void setY(int newY)
	{
		this->y = newY;
		//this->updateYVars();
	}

	void setZ(int newZ)
	{
		this->z = newZ;
		//this->updateZVars();
	}

	void setVars(int newX, int newY, int newZ)
	{
		this->x = newX;
		this->y = newY;
		this->z = newZ;
		//this->updateAllVars();
	}

	void superItemPosition(int newX, int newY, int newZ)
	{
		superItemX.push_back(newX);
		superItemY.push_back(newY);
		superItemZ.push_back(newZ);
	}

	void superItemDimension(int newWidth, int newDepth, int newHeight)
	{
		superItemWidth.push_back(newWidth);
		superItemDepth.push_back(newDepth);
		superItemHeight.push_back(newHeight);
		coveredSpace += newWidth*newDepth*newHeight;
	}

	void exchangeSIDimensions()
	{
		int index = 0;
		for (int i = 0; i < nbItemsInSILayer.size(); i++)
		{
			for (int j = index; j < index + nbItemsInSILayer[i]; j++)
			{
				int temp;
				temp = superItemX[j];
				superItemX[j] = superItemY[j];
				superItemY[j] = temp;
				temp = superItemWidth[j];
				superItemWidth[j] = superItemDepth[j];
				superItemDepth[j] = temp;
			}
			index += nbItemsInSILayer[i];
		}
	}

	void removeLastSIItem()
	{
		superItemList.pop_back();
		superItemX.pop_back();
		superItemY.pop_back();
		superItemZ.pop_back();
		superItemWidth.pop_back();
		superItemDepth.pop_back();
		superItemHeight.pop_back();
		nbItemsInSILayer.back()--;
	}

	void sortDimensionsWDH()
	{
		if(width < depth)
		{
			int tempWidth = width;
			width = depth;
			depth = tempWidth;
		}
		if(width < height)
		{
			int tempWidth = width;
			width = height;
			height = tempWidth;
		}
		if(depth < height)
		{
			int tempDepth = depth;
			depth = height;
			height = tempDepth;
		}
	}

	/*
	void updateXVars()
	{
		BackRightX = x + width;
		CenterX = x + width / 2;
	}

	void updateYVars()
	{
		BackRightY = y + depth;
		CenterY = y + depth / 2;
	}

	void updateZVars()
	{
		TopZ = z + height;
	}

	void updateAllVars()
	{
		BackRightX = x + width;
		CenterX = x + width / 2;
		BackRightY = y + depth;
		CenterY = y + depth / 2;
		TopZ = z + height;
	}
	*/

	bool operator< (const Item &other)
	{
		return height < other.height;
	}
	
};

struct Bin {
	vector<int> xCoords;
	vector<int> yCoords;
	vector<int> zCoords;
	vector<int> widths;
	vector<int> depths;
	vector<int> heights;

	Bin()
	{

	}
	
	~Bin()
	{

	}

};


vector<Item> uniqueItemList;
vector<Bin> bins;

template <typename Container>
Container& split(Container& result,
	const typename Container::value_type& s,
	const typename Container::value_type& delimiters)
{
	result.clear();
	size_t current;
	size_t next = -1;
	do {
		current = next + 1;
		next = s.find_first_of(delimiters, current);
		result.push_back(s.substr(current, next - current));
	} while (next != Container::value_type::npos);
	return result;
}


void parseInput(string fileName)
{
	ifstream myfile1;
	myfile1.open("Parameters/Class9_100/" + fileName + ".txt");

	string line;

	size_t pos = 0;
	string token;
	int tempTime;

	const string delimiter = " ";
	vector<vector<string>> v;
	vector<string> fields;

	//Read data
	for (myfile1; getline(myfile1, line);)
	{
		v.push_back(split(fields, line, delimiter));
	}
	myfile1.close();


	int index = 0;
	int supIndex = 0;

	for (int i = 0; i < v.size(); i++)
	{
		for (int j = 0; j < v[i].size(); j++)
		{
			if (v[i][j].empty())
			{
				v[i].erase(v[i].begin() + j);
				j--;
			}
		}

		string w = v.at(i).at(1).c_str();
		w.erase(remove(w.begin(), w.end(), ','), w.end());
		w.erase(remove(w.begin(), w.end(), ' '), w.end());

		string d = v.at(i).at(2).c_str();
		d.erase(remove(d.begin(), d.end(), ','), d.end());
		d.erase(remove(d.begin(), d.end(), ' '), d.end());

		string h = v.at(i).at(3).c_str();
		h.erase(remove(h.begin(), h.end(), ','), h.end());
		h.erase(remove(h.begin(), h.end(), ' '), h.end());

		string b = v.at(i).at(4).c_str();
		b.erase(remove(b.begin(), b.end(), ','), b.end());
		b.erase(remove(b.begin(), b.end(), ' '), b.end());

		string x = v.at(i).at(5).c_str();
		x.erase(remove(x.begin(), x.end(), ','), x.end());
		x.erase(remove(x.begin(), x.end(), ' '), x.end());

		string y = v.at(i).at(6).c_str();
		y.erase(remove(y.begin(), y.end(), ','), y.end());
		y.erase(remove(y.begin(), y.end(), ' '), y.end());

		string z = v.at(i).at(7).c_str();
		z.erase(remove(z.begin(), z.end(), ','), z.end());
		z.erase(remove(z.begin(), z.end(), ' '), z.end());

		if (stoi(b) > bins.size())
			bins.resize(stoi(b));

		bins[stoi(b) - 1].widths.push_back(stoi(w));
		bins[stoi(b) - 1].depths.push_back(stoi(d));
		bins[stoi(b) - 1].heights.push_back(stoi(h));
		bins[stoi(b) - 1].xCoords.push_back(stoi(x));
		bins[stoi(b) - 1].yCoords.push_back(stoi(y));
		bins[stoi(b) - 1].zCoords.push_back(stoi(z));
	}

	v.clear();
	fields.clear();

}

vector<int> calculateLBs()
{
	vector<int> returnVector;

	int L1WH = 0;
	int L1WD = 0;
	int L1DH = 0;
	int L2WH = 0;
	int L2WD = 0;
	int L2DH = 0;

	vector<int> IWH;
	vector<int> IWD;
	vector<int> IDH;

	cout << "Calculating L1..." << endl;

	for (int i = 0; i < uniqueItemList.size(); i++)
	{
		if (uniqueItemList[i].width > BinWidth / 2 && uniqueItemList[i].height > BinHeight / 2)
			IWH.push_back(i);
		if (uniqueItemList[i].width > BinWidth / 2 && uniqueItemList[i].depth > BinDepth / 2)
			IWD.push_back(i);
		if (uniqueItemList[i].depth > BinDepth / 2 && uniqueItemList[i].height > BinHeight / 2)
			IDH.push_back(i);
	}

	vector<int> IlpWH;
	vector<int> IlpWD;
	vector<int> IlpDH;
	vector<int> IspWH;
	vector<int> IspWD;
	vector<int> IspDH;

	int maxPWH = 0;
	int maxPWD = 0;
	int maxPDH = 0;

	for (int p = 1; p < BinDepth / 2; p++)
	{
		for (int i = 0; i < IWH.size(); i++)
		{
			if (uniqueItemList[IWH[i]].depth > BinDepth / 2 && uniqueItemList[IWH[i]].depth <= BinDepth - p)
				IlpWH.push_back(IWH[i]);
			if (uniqueItemList[IWH[i]].depth >= p && uniqueItemList[IWH[i]].depth <= BinDepth / 2)
				IspWH.push_back(IWH[i]);
		}
		for (int i = 0; i < IWD.size(); i++)
		{
			if (uniqueItemList[IWD[i]].height > BinHeight / 2 && uniqueItemList[IWD[i]].height <= BinHeight - p)
				IlpWD.push_back(IWD[i]);
			if (uniqueItemList[IWD[i]].height >= p && uniqueItemList[IWD[i]].height <= BinHeight / 2)
				IspWD.push_back(IWD[i]);
		}
		for (int i = 0; i < IDH.size(); i++)
		{
			if (uniqueItemList[IDH[i]].width > BinWidth / 2 && uniqueItemList[IDH[i]].width <= BinWidth - p)
				IlpDH.push_back(IDH[i]);
			if (uniqueItemList[IDH[i]].width >= p && uniqueItemList[IDH[i]].width <= BinWidth / 2)
				IspDH.push_back(IDH[i]);
		}

		double dSums = 0;
		double wSums = 0;
		double hSums = 0;

		int maxIndex = 0;
		maxIndex = max(IspWH.size(), max(IspWD.size(), IspDH.size()));

		for (int i = 0; i < maxIndex; i++)
		{
			if(i < IspWH.size())
				dSums += uniqueItemList[IspWH[i]].depth;
			if(i < IspDH.size())
				wSums += uniqueItemList[IspDH[i]].width;
			if (i < IspWD.size())
				hSums += uniqueItemList[IspWD[i]].height;
		}

		maxIndex = max(IlpWH.size(), max(IlpWD.size(), IlpDH.size()));

		double dSuml = 0;
		double wSuml = 0;
		double hSuml = 0;
		int sumWH = 0;
		int sumWD = 0;
		int sumDH = 0;

		for (int i = 0; i < maxIndex; i++)
		{
			if (i < IlpWH.size())
			{
				dSuml += uniqueItemList[IlpWH[i]].depth;
				sumWH += floor((BinDepth - uniqueItemList[IlpWH[i]].depth) / p);
			}
			if (i < IlpDH.size())
			{
				wSuml += uniqueItemList[IlpDH[i]].width;
				sumDH += floor((BinWidth - uniqueItemList[IlpDH[i]].width) / p);
			}
			if (i < IlpWD.size())
			{
				hSuml += uniqueItemList[IlpWD[i]].height;
				sumWD += floor((BinHeight - uniqueItemList[IlpWD[i]].height) / p);
			}
		}

		int maxPWHTemp = 0; 
		int leftSide = 0;
		leftSide = (int)(((dSums - (IlpWH.size()*BinDepth - dSuml)) / BinDepth) + 1);
		int below = (int)(BinDepth / p);
		int rightSide = 0;
		int wtf = IspWH.size() - sumWH;
		rightSide = ((wtf / below) + 1);

		maxPWHTemp = max(leftSide, rightSide);
		//cout << maxPWHTemp << endl;
		if (maxPWHTemp > maxPWH)
			maxPWH = maxPWHTemp;

		int maxPWDTemp = 0;
		leftSide = (int)(((hSums - (IlpWD.size()*BinHeight - hSuml)) / BinHeight) + 1);
		below = (int)(BinHeight / p);
		wtf = IspWD.size() - sumWD;
		rightSide = ((wtf / below) + 1);

		maxPWDTemp = max(leftSide, rightSide);

		if (maxPWDTemp > maxPWD)
			maxPWD = maxPWDTemp;

		int maxPDHTemp = 0;
		leftSide = (int)(((wSums - (IlpDH.size()*BinWidth - wSuml)) / BinWidth) + 1);
		below = (int)(BinWidth / p);
		wtf = IspDH.size() - sumDH;
		rightSide = ((wtf / below) + 1);

		maxPDHTemp = max(leftSide, rightSide);

		if (maxPDHTemp > maxPDH)
			maxPDH = maxPDHTemp;

		IlpWH.clear();
		IlpWD.clear();
		IlpDH.clear();
		IspWH.clear();
		IspWD.clear();
		IspDH.clear();
	}//for (int p = 1; p < BinDepth / 2; p++)

	int counterWH = 0;
	int counterWD = 0;
	int counterDH = 0;

	for (int i = 0; i < IWH.size(); i++)
	{
		if (uniqueItemList[IWH[i]].depth > BinDepth / 2)
			counterWH++;
	}
	for (int i = 0; i < IWD.size(); i++)
	{
		if (uniqueItemList[IWD[i]].height > BinHeight / 2)
			counterWD++;
	}
	for (int i = 0; i < IDH.size(); i++)
	{
		if (uniqueItemList[IDH[i]].width > BinWidth / 2)
			counterDH++;
	}

	L1WH = counterWH + maxPWH;
	L1WD = counterWD + maxPWD;
	L1DH = counterDH + maxPDH;

	int L1 = max(L1WH, max(L1WD, L1DH));
	returnVector.push_back(L1);

	IWH.clear();
	IWD.clear();
	IDH.clear();

	cout << "Calculating L2..." << endl;

	vector<int> Kv;
	vector<int> Kl;
	vector<int> Ks;

	vector<int> KvRemaining;
	vector<int> KlRemaining;

	for (int p = 1; p <= BinWidth / 2; p++)
	{
		for (int q = 1; q <= BinHeight / 2; q++)
		{
			for (int i = 0; i < uniqueItemList.size(); i++)
			{
				if (uniqueItemList[i].width > BinWidth - p && uniqueItemList[i].height > BinHeight - q)
					Kv.push_back(i);
				else
					KvRemaining.push_back(i);
			}
			for (int i = 0; i < KvRemaining.size(); i++)
			{
				if (uniqueItemList[KvRemaining[i]].width > BinWidth / 2 && uniqueItemList[KvRemaining[i]].height > BinHeight / 2)
					Kl.push_back(KvRemaining[i]);
				else
					KlRemaining.push_back(KvRemaining[i]);
			}
			for (int i = 0; i < KlRemaining.size(); i++)
			{
				if (uniqueItemList[KlRemaining[i]].width >= p && uniqueItemList[KlRemaining[i]].height >= q)
					Ks.push_back(KlRemaining[i]);
			}

			double vSum = 0;

			for (int i = 0; i < Kl.size(); i++)
			{
				vSum += uniqueItemList[Kl[i]].width * uniqueItemList[Kl[i]].depth * uniqueItemList[Kl[i]].height;
			}
			for (int i = 0; i < Ks.size(); i++)
			{
				vSum += uniqueItemList[Ks[i]].width * uniqueItemList[Ks[i]].depth * uniqueItemList[Ks[i]].height;
			}

			double dSum = 0;

			for (int i = 0; i < Kv.size(); i++)
			{
				dSum += uniqueItemList[Kv[i]].depth;
			}

			int L2WHpq = L1WH + max(0, (int)ceil((vSum - (BinDepth * L1WH - dSum)*BinWidth*BinHeight) / binVolume));

			if (L2WHpq > L2WH)
				L2WH = L2WHpq;

			Kv.clear();
			Kl.clear();
			Ks.clear();
			KvRemaining.clear();
			KlRemaining.clear();
		}//for (int q = 0; q <= BinHeight; q++)
	}//for (int p = 0; p <= BinWidth; p++)

	for (int p = 1; p <= BinWidth / 2; p++)
	{
		for (int q = 1; q <= BinDepth / 2; q++)
		{
			for (int i = 0; i < uniqueItemList.size(); i++)
			{
				if(uniqueItemList[i].width > BinWidth - p && uniqueItemList[i].depth > BinDepth - q)
					Kv.push_back(i);
				else
					KvRemaining.push_back(i);
			}
			for (int i = 0; i < KvRemaining.size(); i++)
			{
				if(uniqueItemList[KvRemaining[i]].width > BinWidth / 2 && uniqueItemList[KvRemaining[i]].depth > BinDepth / 2)
					Kl.push_back(KvRemaining[i]);
				else
					KlRemaining.push_back(KvRemaining[i]);
			}
			for (int i = 0; i < KlRemaining.size(); i++)
			{
				if(uniqueItemList[KlRemaining[i]].width >= p && uniqueItemList[KlRemaining[i]].depth >= q)
					Ks.push_back(KlRemaining[i]);
			}

			double vSum = 0;

			for (int i = 0; i < Kl.size(); i++)
			{
				vSum += uniqueItemList[Kl[i]].width * uniqueItemList[Kl[i]].depth * uniqueItemList[Kl[i]].height;
			}
			for (int i = 0; i < Ks.size(); i++)
			{
				vSum += uniqueItemList[Ks[i]].width * uniqueItemList[Ks[i]].depth * uniqueItemList[Ks[i]].height;
			}

			double hSum = 0;

			for (int i = 0; i < Kv.size(); i++)
			{
				hSum += uniqueItemList[Kv[i]].height;
			}

			int L2WDpq = L1WD + max(0, (int)ceil((vSum - (BinHeight * L1WD - hSum)*BinWidth*BinDepth) / binVolume));

			if(L2WDpq > L2WD)
				L2WD = L2WDpq;

			Kv.clear();
			Kl.clear();
			Ks.clear();
			KvRemaining.clear();
			KlRemaining.clear();

		}//for (int q = 0; q <= BinDepth / 2; q++)
	}//for (int p = 0; p <= BinWidth / 2; p++)

	for (int p = 1; p <= BinDepth / 2; p++)
	{
		for (int q = 1; q <= BinHeight / 2; q++)
		{
			for (int i = 0; i < uniqueItemList.size(); i++)
			{
				if (uniqueItemList[i].depth > BinDepth - p && uniqueItemList[i].height > BinHeight - q)
					Kv.push_back(i);
				else
					KvRemaining.push_back(i);
			}
			for (int i = 0; i < KvRemaining.size(); i++)
			{
				if (uniqueItemList[KvRemaining[i]].depth > BinDepth / 2 && uniqueItemList[KvRemaining[i]].height > BinHeight / 2)
					Kl.push_back(KvRemaining[i]);
				else
					KlRemaining.push_back(KvRemaining[i]);
			}
			for (int i = 0; i < KlRemaining.size(); i++)
			{
				if (uniqueItemList[KlRemaining[i]].depth >= p && uniqueItemList[KlRemaining[i]].height >= q)
					Ks.push_back(KlRemaining[i]);
			}

			double vSum = 0;

			for (int i = 0; i < Kl.size(); i++)
			{
				vSum += uniqueItemList[Kl[i]].width * uniqueItemList[Kl[i]].depth * uniqueItemList[Kl[i]].height;
			}
			for (int i = 0; i < Ks.size(); i++)
			{
				vSum += uniqueItemList[Ks[i]].width * uniqueItemList[Ks[i]].depth * uniqueItemList[Ks[i]].height;
			}

			double wSum = 0;

			for (int i = 0; i < Kv.size(); i++)
			{
				wSum += uniqueItemList[Kv[i]].width;
			}

			int L2DHpq = L1DH + max(0, (int)ceil((vSum - (BinWidth * L1DH - wSum)*BinDepth*BinHeight) / (BinWidth*BinDepth*BinHeight)));

			if (L2DHpq > L2DH)
				L2DH = L2DHpq;

			Kv.clear();
			Kl.clear();
			Ks.clear();
			KvRemaining.clear();
			KlRemaining.clear();

		}//for (int q = 0; q <= BinDepth / 2; q++)
	}//for (int p = 0; p <= BinWidth / 2; p++)

	int L2 = max(L2WH, max(L2WD, L2DH));

	returnVector.push_back(L2);

	return returnVector;
}