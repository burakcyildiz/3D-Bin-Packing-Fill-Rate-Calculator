


#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include "Rect.h"
#include "MaxRectsBinPack.h"
#include "ilcplex\ilocplex.h"
#include <boost/functional/hash.hpp>
#include <boost/unordered_set.hpp>
#include <unordered_map>

#define BinWidth 1200
#define BinDepth 800
#define BinHeight 2055				
#define LayerHeightTolerance 5			// Default 5
#define superItemWidthTolerance 0.6		// Default 0.6
#define superItemDepthTolerance 0.6		// Default 0.6
#define replacementWidthTolerance 0.6
#define replacementDepthTolerance 0.6
#define supportSuperItemWidthCover 0.95	// Support super-items' support legs should cover at least 70% of the width of the top item
#define supportSuperItemDepthCover 0.95	// Support super-items' support legs should cover at least 70% of the depth of the top item
#define supportSuperItemDepthTol   20	// Maximum depth difference between the legs of support super-items
#define improvementLimit 10				// Default 20
#define sortByAlpha 1				
#define MAXRECTSHeuristic 3				// 0: CP	1: LS	2: SS	3: A	4: BL
#define maximumSuperItem 4				// Default 4
#define nbTimesItemCovered 3			// Default 3
#define nbPreviouslySelectedItems 2		// Default 2
#define normalLayerHeight 1400			// Maximum height of the bin that will be filled with normal layers
#define all5Heuristics 0				// Generate a layer with each heuristic if this is 1, use MAXRECTSHeuristic if it is 0
#define twoHalvedBins 0					// Use 2 layers throughout the width of the bin. Apply if most of the layers have width occupancy <= 0.5 * BinWidth
#define balanced 0						// 1 if the normal bins should be distributed evenly by total height, 0 otherwise

int nItems;
int nbLines;

using namespace std;

void generateSupportSuperItems();

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


struct Layer
{
	int layerID;
	double layerOccupancy;
	int layerHeight;
	double alpha;

	vector<int> itemList;
	vector<int> spaceList;
	vector<int> orientationList;
	vector<int> itemWidths;
	vector<int> itemDepths;
	vector<int> itemHeights;
	vector<int> itemXCoords;
	vector<int> itemYCoords;

	vector<int> emptyXCoords;
	vector<int> emptyYCoords;
	vector<int> emptyWidths;
	vector<int> emptyDepths;
	vector<int> emptyHeights;

	struct Layer()
	{
		itemList.clear();
		itemWidths.clear();
		itemDepths.clear();
		itemXCoords.clear();
		itemYCoords.clear();
		layerHeight = 0;
		alpha = 0;
	}

	void insertRect(rbp::Rect r)
	{
		itemWidths.push_back(r.width);
		itemDepths.push_back(r.height);
		itemXCoords.push_back(r.x);
		itemYCoords.push_back(r.y);
	}

	void insertItem(int ID, int width, int depth, int height, int xCoord, int yCoord, int orientation, int space)
	{
		itemList.push_back(ID);
		orientationList.push_back(orientation);
		itemWidths.push_back(width);
		itemHeights.push_back(height);
		itemDepths.push_back(depth);
		itemXCoords.push_back(xCoord);
		itemYCoords.push_back(yCoord);
		spaceList.push_back(space);
	}

	void deleteItem(int index)
	{
		itemList.erase(itemList.begin() + index);
		//orientationList.erase(orientationList.begin() + index);

		emptyWidths.push_back(itemWidths[index]);
		emptyDepths.push_back(itemDepths[index]);
		emptyHeights.push_back(itemHeights[index]);
		emptyXCoords.push_back(itemXCoords[index]);
		emptyYCoords.push_back(itemYCoords[index]);

		itemWidths.erase(itemWidths.begin() + index);
		itemDepths.erase(itemDepths.begin() + index);
		itemHeights.erase(itemHeights.begin() + index);
		itemXCoords.erase(itemXCoords.begin() + index);
		itemYCoords.erase(itemYCoords.begin() + index);
	}

	void replaceItem(int index, Item i)
	{
		itemList[index] = i.ID;
		int x = (itemWidths[index] - i.width) / 2;
		int y = (itemDepths[index] - i.depth) / 2;
		itemWidths[index] = i.width;
		itemDepths[index] = i.depth;
		itemHeights[index] = i.height;
		itemXCoords[index] = x;
		itemYCoords[index] = y;
		spaceList[index] = i.coveredSpace;
	}

	void calculateLayerOccupancy()
	{
		double coveredSpace = 0;

		for (int i = 0; i < spaceList.size(); i++)
		{
			coveredSpace += spaceList[i];
		}

		layerOccupancy = (coveredSpace / (BinWidth * BinDepth * layerHeight)) * 100;
	}

	bool operator< (const Layer &otherL)
	{
		return layerOccupancy > otherL.layerOccupancy;
	}

	bool operator==(const Layer &other) const
	{ 
		return itemList == other.itemList;
	}

	friend std::size_t hash_value(Layer const& l)
    {
        size_t hash = boost::hash_range(l.itemList.begin(), l.itemList.end());

        return hash;
    }
};


struct Bin
{
	vector<Layer> binLayers;
	int height;

	struct Bin()
	{
		height = 0;
	}
};


struct KeyHasher
{
  std::size_t operator()(const Layer& l) const
  {
      //using boost::hash_value;
      //using boost::hash_combine;

	  size_t hash = boost::hash_range(l.itemList.begin(), l.itemList.end());
      /*
	  // Start with a hash value of 0    .
      std::size_t seed = 0;

      // Modify 'seed' by XORing and bit-shifting in
      // one member of 'Key' after the other:
      hash_combine(seed,hash_value(k.first));
      hash_combine(seed,hash_value(k.second));
      hash_combine(seed,hash_value(k.third));
	  */
      // Return the result.
      return hash;
  }
};

vector<Item> itemList;
vector<Item> uniqueItemList;
vector<Item> superItems;
vector<int> unselectedItemList;
vector<Layer> layerList;
vector<Layer> topLayerList;
vector<Layer> selectedLayerList;
vector<Layer> selectedTopLayerList;
vector<Bin> binList;

vector<vector<int>> itemGroups;
vector<int> topLayerIG;

//unordered_map<Layer, int> layerHash;
boost::unordered_set<Layer> layerSet;

bool sortDescReducedCost(const int &i1, const int &i2)
{
	Item item1;
	Item item2;
	if(i1 < nItems)
		item1 = itemList[i1];
	else
		item1 = superItems[i1 - nItems];

	if(i2 < nItems)
		item2 = itemList[i2];
	else
		item2 = superItems[i2 - nItems];

	return item1.reducedCost > item2.reducedCost;
}

bool sortDescHeight(const Item &i1, const Item &i2)
{
	return i1.height > i2.height;
}

bool sortAscHeight(const Item &i1, const Item &i2)
{
	return i1.height < i2.height;
}

bool sortItems(const Item &i1, const Item &i2)
{
	return i1.width * i1.depth > i2.width * i2.depth;
}

bool sortLayers(const Layer &l1, const Layer &l2)
{
	return l1.alpha > l2.alpha;
}

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
	myfile1.open("Parameters/" + fileName + ".txt");

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

	int itemCount = 0;

	for (int i = 0; i < v.size(); i++)
	{
		itemCount += atoi(v.at(i).at(15).c_str());
	}

	//nItems = itemCount;

	//To count 3 main orientations of the items
	nItems = itemCount * 6;

	int index = 0;
	int supIndex = 0;

	vector<Item> otherItemList;

	for (int i = 0; i < v.size(); i++)
	{
		int repetition = atoi(v.at(i).at(15).c_str());
		nbLines++;
		for(int j = 0; j < repetition; j++)
		{
			int id = index;
			int width = atoi(v.at(i).at(0).c_str());
			int depth = atoi(v.at(i).at(1).c_str());
			int height = atoi(v.at(i).at(2).c_str());

			Item newItem(id, width, depth, height, 1);
			newItem.inputPlace = i;
			newItem.coveredSpace = width * depth * height;
			itemList.push_back(newItem);
			uniqueItemList.push_back(newItem);
			//itemIDs.push_back(id);

			Item newItem2(id, width, height, depth, 2);
			newItem2.coveredSpace = width * depth * height;
			newItem2.inputPlace = i;
			itemList.push_back(newItem2);

			Item newItem3(id, height, depth, width, 3);
			newItem3.coveredSpace = width * depth * height;
			newItem3.inputPlace = i;
			itemList.push_back(newItem3);

			Item newItem4(id, depth, width, height, 1);
			newItem4.coveredSpace = width * depth * height;
			newItem4.inputPlace = i;
			otherItemList.push_back(newItem4);

			Item newItem5(id, depth, height, width, 2);
			newItem5.coveredSpace = width * depth * height;
			newItem5.inputPlace = i;
			otherItemList.push_back(newItem5);

			Item newItem6(id, height, width, depth, 3);
			newItem6.coveredSpace = width * depth * height;
			newItem6.inputPlace = i;
			otherItemList.push_back(newItem6);

			index++;
		}

		//Create initial super-item by stacking identical items horizontally
		if(repetition >= 2)
		{
			int startIndex = itemList.size() - repetition * 6;
			for (int j = startIndex; j < itemList.size() - 6; j += 12)
			{
				for (int k = j + 6; k < itemList.size(); k += 12)
				{
					//Super-item side-by-side - orientation 1
					Item newItem(itemList[j].width * 2, itemList[j].depth, itemList[j].height);
					newItem.superItemList.push_back(itemList[j]);
					newItem.superItemPosition(0, 0, 0);
					newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
					newItem.superItemList.push_back(itemList[k]);
					newItem.superItemPosition(itemList[j].width, 0, 0);
					newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
					newItem.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem);
					
					//Super-item side-by-side - orientation 2
					Item newItem2(itemList[j + 1].width * 2, itemList[j + 1].depth, itemList[j + 1].height);
					newItem2.superItemList.push_back(itemList[j + 1]);
					newItem2.superItemPosition(0, 0, 0);
					newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
					newItem2.superItemList.push_back(itemList[k + 1]);
					newItem2.superItemPosition(itemList[j + 1].width, 0, 0);
					newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
					newItem2.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem2);

					//Super-item side-by-side - orientation 3
					Item newItem3(itemList[j + 2].width * 2, itemList[j + 2].depth, itemList[j + 2].height);
					newItem3.superItemList.push_back(itemList[j + 2]);
					newItem3.superItemPosition(0, 0, 0);
					newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
					newItem3.superItemList.push_back(itemList[k + 2]);
					newItem3.superItemPosition(itemList[j + 2].width, 0, 0);
					newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
					newItem3.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem3);

					//Super-item back-to-back - orientation 1
					Item newItem4(itemList[j].width, itemList[j].depth * 2, itemList[j].height);
					newItem4.superItemList.push_back(itemList[j]);
					newItem4.superItemPosition(0, 0, 0);
					newItem4.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
					newItem4.superItemList.push_back(itemList[k]);
					newItem4.superItemPosition(0, itemList[j].depth, 0);
					newItem4.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
					newItem4.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem4);

					//Super-item back-to-back - orientation 2
					Item newItem5(itemList[j+1].width, itemList[j+1].depth * 2, itemList[j+1].height);
					newItem5.superItemList.push_back(itemList[j+1]);
					newItem5.superItemPosition(0, 0, 0);
					newItem5.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
					newItem5.superItemList.push_back(itemList[k+1]);
					newItem5.superItemPosition(0, itemList[j+1].depth, 0);
					newItem5.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
					newItem5.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem5);

					//Super-item back-to-back - orientation 3
					Item newItem6(itemList[j+2].width, itemList[j+2].depth * 2, itemList[j+2].height);
					newItem6.superItemList.push_back(itemList[j+2]);
					newItem6.superItemPosition(0, 0, 0);
					newItem6.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
					newItem6.superItemList.push_back(itemList[k+2]);
					newItem6.superItemPosition(0, itemList[j+2].depth, 0);
					newItem6.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
					newItem6.nbItemsInSILayer.push_back(2);
					superItems.push_back(newItem6);
				}//for (int k = j + 3; k < itemList.size(); k += 3)
			}//for (int j = startIndex; j < itemList.size() - 1; j += 3)
		}//if(repetition >= 2)

		if(repetition >= 4)
		{
			int startIndex = itemList.size() - repetition * 6;
			for (int j = startIndex; j < itemList.size() - 18; j += 24)
			{
				for (int k = j + 6; k < itemList.size() - 12; k += 24)
				{
					for (int l = k + 6; l < itemList.size() - 6; l += 24)
					{
						for (int m = l + 6; m < itemList.size(); m += 24)
						{
							//Put items horizontally 2-by-2 - orientation 1
							Item newItem(itemList[j].width * 2, itemList[j].depth * 2, itemList[j].height);
							newItem.superItemList.push_back(itemList[j]);
							newItem.superItemPosition(0, 0, 0);
							newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
							newItem.superItemList.push_back(itemList[k]);
							newItem.superItemPosition(itemList[j].width, 0, 0);
							newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
							newItem.superItemList.push_back(itemList[l]);
							newItem.superItemPosition(0, itemList[j].depth, 0);
							newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
							newItem.superItemList.push_back(itemList[m]);
							newItem.superItemPosition(itemList[j].width, itemList[j].depth, 0);
							newItem.superItemDimension(itemList[j].width, itemList[j].depth, itemList[j].height);
							newItem.nbItemsInSILayer.push_back(4);
							superItems.push_back(newItem);

							//Put items horizontally 2-by-2 - orientation 2
							Item newItem2(itemList[j+1].width * 2, itemList[j+1].depth * 2, itemList[j+1].height);
							newItem2.superItemList.push_back(itemList[j+1]);
							newItem2.superItemPosition(0, 0, 0);
							newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
							newItem2.superItemList.push_back(itemList[k+1]);
							newItem2.superItemPosition(itemList[j+1].width, 0, 0);
							newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
							newItem2.superItemList.push_back(itemList[l+1]);
							newItem2.superItemPosition(0, itemList[j+1].depth, 0);
							newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
							newItem2.superItemList.push_back(itemList[m+1]);
							newItem2.superItemPosition(itemList[j+1].width, itemList[j+1].depth, 0);
							newItem2.superItemDimension(itemList[j + 1].width, itemList[j + 1].depth, itemList[j + 1].height);
							newItem2.nbItemsInSILayer.push_back(4);
							superItems.push_back(newItem2);

							//Put items horizontally 2-by-2 - orientation 3
							Item newItem3(itemList[j+2].width * 2, itemList[j+2].depth * 2, itemList[j+2].height);
							newItem3.superItemList.push_back(itemList[j+2]);
							newItem3.superItemPosition(0, 0, 0);
							newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
							newItem3.superItemList.push_back(itemList[k+2]);
							newItem3.superItemPosition(itemList[j+2].width, 0, 0);
							newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
							newItem3.superItemList.push_back(itemList[l+2]);
							newItem3.superItemPosition(0, itemList[j+2].depth, 0);
							newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
							newItem3.superItemList.push_back(itemList[m+2]);
							newItem3.superItemPosition(itemList[j+2].width, itemList[j+2].depth, 0);
							newItem3.superItemDimension(itemList[j + 2].width, itemList[j + 2].depth, itemList[j + 2].height);
							newItem3.nbItemsInSILayer.push_back(4);
							superItems.push_back(newItem3);

						}//for (int m = l + 3; m < itemList.size(); m += 3)
					}//for (int l = k + 3; l < itemList.size() - 3; l += 3)
				}//for (int k = j + 3; k < itemList.size(); k += 3)
			}//for (int j = startIndex; j < itemList.size() - 1; j += 3)
		}//if(repetition >= 2)
	}

	v.clear();
	fields.clear();

	//Sort items by decreasing area
	std::sort(itemList.begin(), itemList.end(), &sortItems);

	//Create initial super-items by stacking items vertically
	for (int i = 0; i < itemList.size(); i++)
	{
		Item bottomItem = itemList[i];

		for (int j = i + 1; j < itemList.size(); j++)
		{
			Item topItem = itemList[j];

			if(topItem.width <= bottomItem.width && topItem.width >= bottomItem.width * superItemWidthTolerance && bottomItem.ID != topItem.ID)
			{
				if(topItem.depth <= bottomItem.depth && topItem.depth >= bottomItem.depth * superItemDepthTolerance)
				{
					Item newItem(bottomItem.width, bottomItem.depth, bottomItem.height + topItem.height);
					newItem.superItemOnTop++;
					newItem.superItemList.push_back(bottomItem);
					newItem.superItemPosition(0,0,0);
					newItem.superItemDimension(bottomItem.width, bottomItem.depth, bottomItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					newItem.superItemList.push_back(topItem);
					newItem.superItemPosition((bottomItem.width - topItem.width) / 2, (bottomItem.depth - topItem.depth) / 2, bottomItem.height);
					newItem.superItemDimension(topItem.width, topItem.depth, topItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					superItems.push_back(newItem);
					break;
				}
				else if(topItem.depth > bottomItem.depth && topItem.depth <= bottomItem.depth * 1.2)
				{
					Item newItem(bottomItem.width, topItem.depth, bottomItem.height + topItem.height);
					newItem.superItemOnTop++;
					newItem.superItemList.push_back(bottomItem);
					newItem.superItemPosition(0, (topItem.depth - bottomItem.depth) / 2, 0);
					newItem.superItemDimension(bottomItem.width, bottomItem.depth, bottomItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					newItem.superItemList.push_back(topItem);
					newItem.superItemPosition((bottomItem.width - topItem.width) / 2, 0, bottomItem.height);
					newItem.superItemDimension(topItem.width, topItem.depth, topItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					superItems.push_back(newItem);
					break;
				}
			}//if

			else if(topItem.width > bottomItem.width && topItem.width <= bottomItem.width * 1.2 && bottomItem.ID != topItem.ID)
			{
				if(topItem.depth <= bottomItem.depth && topItem.depth >= bottomItem.depth * superItemDepthTolerance)
				{
					Item newItem(topItem.width, bottomItem.depth, bottomItem.height + topItem.height);
					newItem.superItemOnTop++;
					newItem.superItemList.push_back(bottomItem);
					newItem.superItemPosition((topItem.width - bottomItem.width) / 2, 0, 0);
					newItem.superItemDimension(bottomItem.width, bottomItem.depth, bottomItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					newItem.superItemList.push_back(topItem);
					newItem.superItemPosition(0, (bottomItem.depth - topItem.depth) / 2, bottomItem.height);
					newItem.superItemDimension(topItem.width, topItem.depth, topItem.height);
					newItem.nbItemsInSILayer.push_back(1);

					superItems.push_back(newItem);
					break;
				}
				else if(topItem.depth > bottomItem.depth && topItem.depth <= bottomItem.depth * 1.2)
				{
					Item newItem(topItem.width, topItem.depth, bottomItem.height + topItem.height);
					newItem.superItemOnTop++;
					newItem.superItemList.push_back(bottomItem);
					newItem.superItemPosition((topItem.width - bottomItem.width) / 2, (topItem.depth - bottomItem.depth) / 2, 0);
					newItem.superItemDimension(bottomItem.width, bottomItem.depth, bottomItem.height);
					newItem.nbItemsInSILayer.push_back(1);
					
					newItem.superItemList.push_back(topItem);
					newItem.superItemPosition(0, 0, bottomItem.height);
					newItem.superItemDimension(topItem.width, topItem.depth, topItem.height);
					newItem.nbItemsInSILayer.push_back(1);
					
					superItems.push_back(newItem);
					break;
				}
			}//else if

		}//for (int j = i + 1; j < itemList.size(); j++)
	}//for (int i = 0; i < itemList.size(); i++)
	//cout << superItems.size() << endl;
	//system("pause");

	for (int i = 0; i < otherItemList.size(); i++)
	{
		itemList.push_back(otherItemList[i]);
	}

	sort(itemList.begin(), itemList.end(), &sortDescHeight);

	generateSupportSuperItems();

	//Create super-items, based on other super-items
	for (int i = 0; i < superItems.size(); i++)
	{
		Item superItem = superItems[i];
		//Item bottomItem = superItems[i].superItemList[0];
		//cout << superItems.size() << endl;
		if(superItem.superItemOnTop == maximumSuperItem)
			continue;

		for (int j = 0; j < itemList.size(); j++)
		{
			int check = 0;
			Item topItem = itemList[j];

			//if(topItem.orientation != 1)
			//	continue;

			
			for (int k = 0; k < superItem.superItemList.size(); k++)
			{
				if(topItem.ID == superItem.superItemList[k].ID)
				{
					check = 1;
					break;
				}
			}
			

			if(check == 0)
			{
				if(topItem.width <= superItem.width && topItem.depth <= superItem.depth)
				{
					if(topItem.width >= superItemWidthTolerance * superItem.width && topItem.depth >= superItemDepthTolerance * superItem.depth)
					{
						Item newItem(superItem.width, superItem.depth, superItem.height + topItem.height);
						newItem.superItemList = superItem.superItemList;
						newItem.superItemX = superItem.superItemX;
						newItem.superItemY = superItem.superItemY;
						newItem.superItemZ = superItem.superItemZ;
						newItem.superItemOnTop = superItem.superItemOnTop + 1;
						newItem.coveredSpace = superItem.coveredSpace;
						newItem.superItemWidth = superItem.superItemWidth;
						newItem.superItemDepth = superItem.superItemDepth;
						newItem.superItemHeight = superItem.superItemHeight;
						newItem.nbItemsInSILayer = superItem.nbItemsInSILayer;

						newItem.superItemList.push_back(topItem);
						newItem.superItemPosition((superItem.width - topItem.width) / 2, (superItem.depth - topItem.depth) / 2, superItem.height);
						newItem.superItemDimension(topItem.width, topItem.depth, topItem.height);
						newItem.nbItemsInSILayer.push_back(1);
						superItems.push_back(newItem);
						break;
					}
				}
			}
		}
	}


	sort(itemList.begin(), itemList.end(), &sortDescHeight);

	for (int i = 0; i < itemList.size(); i++)
	{
		itemList[i].place = i;
	}

	if(superItems.size() > 0)
	{
		sort(superItems.begin(), superItems.end());

		for (int i = 0; i < superItems.size(); i++)
		{
			superItems[i].ID = nItems + i;
		}
	}

}

///Groups items and superitems into lists based on their heights
///so that they can be used in BuildLayer
void groupItems()
{
	vector<int> groupElements;
	vector<int> groupStartIndexList;

	groupStartIndexList.push_back(0);

	for (int i = 0; i < itemList.size(); i++)
	{
		int itemHeight = itemList[i].height;
		
		if(i > 0)
			if(itemHeight != itemList[i - 1].height)
			{
				groupStartIndexList.push_back(i);
			}
	}

	//Use superitems as group starters

	if(superItems.size() > 0)
		groupStartIndexList.push_back(superItems[0].ID);

	for (int i = 0; i < superItems.size(); i++)
	{
		int itemHeight = superItems[i].height;

		if(i > 0)
			if(itemHeight != superItems[i - 1].height)
				groupStartIndexList.push_back(superItems[i-1].ID);
	}

	for (int i = 0; i < groupStartIndexList.size(); i++)
	{
		int groupStartHeight;

		if(groupStartIndexList[i] < nItems)
		{
			groupStartHeight = itemList[groupStartIndexList[i]].height;
			for (int j = groupStartIndexList[i]; j < itemList.size(); j++)
			{
				if(groupStartHeight - itemList[j].height <= LayerHeightTolerance)
					groupElements.push_back(j);
				else
					break;
			}

			for (int j = 0; j < superItems.size(); j++)
			{
				if(groupStartHeight >= superItems[j].height && groupStartHeight - superItems[j].height <= LayerHeightTolerance)
					groupElements.push_back(superItems[j].ID);

			}
		}//if(groupStartIndexList[i] < nItems)
		else
		{
			groupStartHeight = superItems[groupStartIndexList[i] - nItems].height;

			for (int j = 0; j < itemList.size(); j++)
			{
				if(groupStartHeight >= itemList[j].height && groupStartHeight - itemList[j].height <= LayerHeightTolerance)
					groupElements.push_back(j);

			}

			for (int j = 0; j < superItems.size(); j++)
			{
				if(groupStartHeight >= superItems[j].height && groupStartHeight - superItems[j].height <= LayerHeightTolerance)
					groupElements.push_back(superItems[j].ID);

			}
		}//else
		if(groupElements.size() > 1)
			itemGroups.push_back(groupElements);
		
		groupElements.clear();
	}//for (int i = 0; i < groupStartIndexList.size(); i++)

	for (int i = 0; i < itemList.size(); i++)
	{
		topLayerIG.push_back(i);
	}

	for (int i = 0; i < superItems.size(); i++)
	{
		if(superItems[i].height > 500)
			continue;

		topLayerIG.push_back(superItems[i].ID);
	}
}

bool BuildLayer(vector<int> itemsToPack, int iteration, string layerType, int heuristicNb)
{
	using namespace rbp;
	bool result = false;

	//Initialize layer sizes
	MaxRectsBinPack bin;
	bin.Init(BinWidth, BinDepth);
	Layer newLayer;
	double reducedCost = 0;
	//Pack each item into the layer
	for (int i = 0; i < itemsToPack.size(); i++)
	{
		//Get the item
		Item newItem;

		/*
		if(newLayer.itemList.size() == 2 && itemsToPack[i] == 451)
		{
			system("pause");
		}
		*/

		if(itemsToPack[i] < nItems)
			newItem = itemList[itemsToPack[i]];
		else
			newItem = superItems[itemsToPack[i] - nItems];

		bool match = false;

		//Check if the item already exists in the layer
		for (int j = 0; j < newLayer.itemList.size(); j++)
		{
			//If the item compared in the layer is a normal item
			if(newLayer.itemList[j] < nItems)
			{
				//If the new item is a normal item
				if(newItem.ID < nItems)
				{
					if(newItem.ID == newLayer.itemList[j])
					{
						match = true;
						break;
					}
				}
				//If the new item is a super-item
				else
				{
					for (int k = 0; k < newItem.superItemList.size(); k++)
					{
						if(newItem.superItemList[k].ID == newLayer.itemList[j])
						{
							match = true; 
							break;
						}
					}//for (int k = 0; k < superItems[newItem.ID - nItems].superItemList.size(); k++)

					if(match == true)
						break;
				}
			}
			//If the item compared in the layer is a super-item
			else
			{
				//If the new item is a normal item
				if(newItem.ID < nItems)
				{
					for (int k = 0; k < superItems[newLayer.itemList[j] - nItems].superItemList.size(); k++)
					{
						if(newItem.ID == superItems[newLayer.itemList[j] - nItems].superItemList[k].ID)
						{
							match = true;
							break;
						}
					}//for (int k = 0; k < superItems[newLayer.itemList[j] - nItems].superItemList.size(); k++)

					if(match == true)
						break;
				}
				//If the new item is a super-item
				else
				{
					for (int k = 0; k < newItem.superItemList.size(); k++)
					{
						for (int l = 0; l < superItems[newLayer.itemList[j] - nItems].superItemList.size(); l++)
						{
							if(newItem.superItemList[k].ID == superItems[newLayer.itemList[j] - nItems].superItemList[l].ID)
							{
								match = true;
								break;
							}
						}//for (int l = 0; l < superItems[newLayer.itemList[j] - nItems].superItemList.size(); l++)

						if(match == true)
							break;

					}//for (int k = 0; k < superItems[newItem.ID - nItems].superItemList.size(); k++)

					if(match == true)
						break;

				}
			}

		}//for (int j = 0; j < newLayer.itemList.size(); j++)

		if(match == true)
			continue;

		//Get item sizes
		int itemWidth = newItem.width;
		int itemDepth = newItem.depth;
		int itemHeight = newItem.height;

		MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;
		//Perform the packing
		/*
		if(MAXRECTSHeuristic == 0)
			heuristic = MaxRectsBinPack::RectContactPointRule;
		else if(MAXRECTSHeuristic == 1)
			heuristic = MaxRectsBinPack::RectBestLongSideFit;
		else if(MAXRECTSHeuristic == 2)
			heuristic = MaxRectsBinPack::RectBestShortSideFit;
		else if(MAXRECTSHeuristic == 3)
			heuristic = MaxRectsBinPack::RectBestAreaFit;
		else
			heuristic = MaxRectsBinPack::RectBottomLeftRule;
		*/
		if(heuristicNb == 0)
			heuristic = MaxRectsBinPack::RectContactPointRule;
		else if(heuristicNb == 1)
			heuristic = MaxRectsBinPack::RectBestLongSideFit;
		else if(heuristicNb == 2)
			heuristic = MaxRectsBinPack::RectBestShortSideFit;
		else if(heuristicNb == 3)
			heuristic = MaxRectsBinPack::RectBestAreaFit;
		else
			heuristic = MaxRectsBinPack::RectBottomLeftRule;

		Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);
		

		// Test success or failure.
		if (packedRect.height > 0)
		{
			//newLayer.itemList.push_back(itemsToPack[i]);
			if(itemsToPack[i] < nItems)
			{
				newLayer.itemList.push_back(newItem.ID);
				newLayer.orientationList.push_back(0);
				newLayer.itemHeights.push_back(itemHeight);
				reducedCost += newItem.reducedCost;
			}
			else
			{
				newLayer.itemList.push_back(itemsToPack[i]);
				newLayer.itemHeights.push_back(itemHeight);
				reducedCost += newItem.reducedCost;
				
				if(packedRect.width == itemDepth)
					newLayer.orientationList.push_back(2);
				else if(packedRect.width == itemWidth)
					newLayer.orientationList.push_back(1);
			}

			newLayer.insertRect(packedRect);
			newLayer.spaceList.push_back(newItem.coveredSpace);
			

			if(newItem.height > newLayer.layerHeight)
				newLayer.layerHeight = newItem.height;

			newLayer.calculateLayerOccupancy();
			
			if(newLayer.itemXCoords.back() + newLayer.itemWidths.back() > BinWidth || newLayer.itemYCoords.back() + newLayer.itemDepths.back() > BinDepth)
			{
				cout << "Infeasible layer!" << endl;
				cout << packedRect.x << "\t" << packedRect.y << "\t" << packedRect.width << "\t" << packedRect.height << endl;
				cout << newItem.width << "\t" << newItem.depth << endl;
				system("pause");
			}
			
			//printf("Packed to (x,y)=(%d,%d), (w,h)=(%d,%d). Free space left: %.2f%%\n", packedRect.x, packedRect.y, packedRect.width, packedRect.height, 100.f - bin.Occupancy()*100.f);
		}

		/*
		else
			printf("Failed! Could not find a proper position to pack this rectangle into. Skipping this one.\n");
		*/
	}

	reducedCost = newLayer.layerHeight - reducedCost;

	//sort(newLayer.itemList.begin(), newLayer.itemList.end());

	if(iteration > 1 && layerType == "normal")
	{
		if(/*reducedCost < 0 && */ layerSet.find(newLayer) == layerSet.end())
		{
			result = true;

			if(layerType == "normal")
				layerList.push_back(newLayer);

			layerSet.insert(newLayer);
		}
	}
	else if (iteration == 0 && layerType == "normal")
	{
		if(layerSet.find(newLayer) == layerSet.end())
		{
			result = true;

			layerList.push_back(newLayer);

			layerSet.insert(newLayer);
		}
	}
	else if(layerType == "top")
	{
		//if(layerSet.find(newLayer) == layerSet.end())
		//{
			topLayerList.push_back(newLayer);
		//	layerSet.insert(newLayer);
			result = true;
			//}
	}

	if(layerType == "normal" && iteration > 1)
	{
		for (int i = 0; i < newLayer.itemList.size(); i++)
		{
			if(newLayer.itemXCoords[i] == 0)
				continue;

			Layer modifiedLayer = newLayer;
			random_shuffle(itemsToPack.begin(), itemsToPack.end());

			bool itemFound = false;

			for (int j = 0; j < itemsToPack.size(); j++)
			{
				//Get the item
				Item newItem;

				if(itemsToPack[j] < nItems)
					newItem = itemList[itemsToPack[j]];
				else
					newItem = superItems[itemsToPack[j] - nItems];

				bool match = false;

				//Check if the item already exists in the layer
				for (int m = 0; m < modifiedLayer.itemList.size(); m++)
				{
					//If the item compared in the layer is a normal item
					if(modifiedLayer.itemList[m] < nItems)
					{
						//If the new item is a normal item
						if(newItem.ID < nItems)
						{
							if(newItem.ID == modifiedLayer.itemList[m])
							{
								match = true;
								break;
							}
						}
						//If the new item is a super-item
						else
						{
							for (int k = 0; k < newItem.superItemList.size(); k++)
							{
								if(newItem.superItemList[k].ID == modifiedLayer.itemList[m])
								{
									match = true; 
									break;
								}
							}//for (int k = 0; k < superItems[newItem.ID - nItems].superItemList.size(); k++)

							if(match == true)
								break;
						}
					}
					//If the item compared in the layer is a super-item
					else
					{
						//If the new item is a normal item
						if(newItem.ID < nItems)
						{
							for (int k = 0; k < superItems[modifiedLayer.itemList[m] - nItems].superItemList.size(); k++)
							{
								if(newItem.ID == superItems[modifiedLayer.itemList[m] - nItems].superItemList[k].ID)
								{
									match = true;
									break;
								}
							}//for (int k = 0; k < superItems[newLayer.itemList[j] - nItems].superItemList.size(); k++)

							if(match == true)
								break;
						}
						//If the new item is a super-item
						else
						{
							for (int k = 0; k < newItem.superItemList.size(); k++)
							{
								for (int l = 0; l < superItems[modifiedLayer.itemList[m] - nItems].superItemList.size(); l++)
								{
									if(newItem.superItemList[k].ID == superItems[modifiedLayer.itemList[m] - nItems].superItemList[l].ID)
									{
										match = true;
										break;
									}
								}//for (int l = 0; l < superItems[newLayer.itemList[j] - nItems].superItemList.size(); l++)

								if(match == true)
									break;

							}//for (int k = 0; k < superItems[newItem.ID - nItems].superItemList.size(); k++)

							if(match == true)
								break;

						}
					}

				}//for (int m = 0; m < newLayer.itemList.size(); m++)

				if(match == true)
					continue;

				//Get item sizes
				int itemWidth = newItem.width;
				int itemDepth = newItem.depth;
				int itemHeight = newItem.height;

				if(itemWidth > modifiedLayer.itemWidths[i] || itemWidth < replacementWidthTolerance * modifiedLayer.itemWidths[i])
					continue;
				else if(itemDepth > modifiedLayer.itemDepths[i] || itemDepth < replacementDepthTolerance * modifiedLayer.itemDepths[i])
					continue;
				else
					itemFound = true;


				//newLayer.itemList.push_back(itemsToPack[i]);
				modifiedLayer.replaceItem(i, newItem);

				if(newItem.height > modifiedLayer.layerHeight)
					modifiedLayer.layerHeight = newItem.height;

				if(itemFound == true)
					break;

			}//for (int j = 0; j < itemsToPack.size(); j++)

			if(itemFound == true)
			{
				result = true;
				layerList.push_back(modifiedLayer);
			}

		}//for (int i = 0; i < newLayer.itemList.size(); i++)
	}//if(layerType == "normal")


	return result;
}


void generateInitialLayers()
{
	for (int i = 0; i < itemGroups.size(); i++)
	{
		//Generate a layer for the group, sorted from tallest to shortest
		if(all5Heuristics == 1)
		{
			for (int j = 0; j < 5; j++)
			{
				BuildLayer(itemGroups[i], 1, "normal", j);
			}
		}
		else
			BuildLayer(itemGroups[i], 1, "normal", MAXRECTSHeuristic);

		//Also reverse the group and generate a layer
		vector<int> groupCopy = itemGroups[i];
		reverse(groupCopy.begin(), groupCopy.end());

		if(all5Heuristics == 1)
		{
			for (int j = 0; j < 5; j++)
			{
				BuildLayer(groupCopy, 1, "normal", j);
			}
		}
		else
			BuildLayer(groupCopy, 1, "normal", MAXRECTSHeuristic);
		
		for (int j = 0; j < 5; j++)
		{
			random_shuffle(groupCopy.begin(), groupCopy.end());

			if(all5Heuristics == 1)
			{
				for (int k = 0; k < 5; k++)
				{
					BuildLayer(groupCopy, 1, "normal", k);
				}
			}
			else
				BuildLayer(groupCopy, 1, "normal", MAXRECTSHeuristic);
			
		}
		groupCopy.clear();
	}

	//BuildLayer(topLayerIG, 1, "top");
}

bool generateLayers(string layerType)
{
	bool result = false;
	bool overallResult = false;

	if(layerType == "normal")
	{
		for (int i = 0; i < itemGroups.size(); i++)
		{
			vector<int> groupCopy = itemGroups[i];
			sort(groupCopy.begin(), groupCopy.end(), &sortDescReducedCost);

			if(all5Heuristics == 1)
			{
				for (int j = 0; j < 5; j++)
				{
					result = BuildLayer(groupCopy, 2, "normal", j);
				}
			}
			else
				result = BuildLayer(groupCopy, 2, "normal", MAXRECTSHeuristic);

			groupCopy.clear();

			if(overallResult == false && result == true)
				overallResult = true;
		}
	}
	else if(layerType == "top")
	{
		sort(topLayerIG.begin(), topLayerIG.end(), &sortDescReducedCost);

		if(all5Heuristics == 1)
		{
			for (int i = 0; i < 5; i++)
			{
				result = BuildLayer(topLayerIG, 2, "top", i);
			}
		}
		else
			result = BuildLayer(topLayerIG, 2, "top", MAXRECTSHeuristic);
		

		if(overallResult == false && result == true)
			overallResult = true;


		//for (int i = 0; i < 50; i++)
		//{
		//	random_shuffle(topLayerIG.begin(), topLayerIG.end());
		//	result = BuildLayer(topLayerIG, 2, "top");
		//
		//	if(overallResult == false && result == true)
		//		overallResult = true;
		//}
	}
	else
	{
		cout << "Wrong argument. Use either 'normal' or 'top'" << endl;
		system("pause");
	}
	

	return overallResult;
}


void binGenerator(vector<Layer> finalList, vector<Layer> finalTopList, double LB)
{
	int finalTopListSize = finalTopList.size();
	int nbBins = max((int)ceil(LB / normalLayerHeight), finalTopListSize);

	
	binList.resize(nbBins);
	int nbClosedBins = 0;
	vector<int> closedBins(nbBins, 0);
	int fullCheck = 0;
	int counter = 0;

	for (int i = 0; i < finalTopList.size(); i++)
	{
		Layer l = finalTopList[i];

		binList[i].binLayers.push_back(l);
		binList[i].height += l.layerHeight;
	}

	int halfCounter = 0;
	for (int i = 0; i < finalList.size(); i++)
	{
		double spaceOccupied = 0;

		for (int j = 0; j < finalList[i].itemList.size(); j++)
		{
			spaceOccupied += finalList[i].itemWidths[j] * finalList[i].itemDepths[j] * finalList[i].itemHeights[j];
		}

		finalList[i].layerOccupancy = spaceOccupied * 100 / (BinWidth * BinDepth * finalList[i].layerHeight);

		int maxX = 0;

		for (int j = 0; j < finalList[i].itemList.size(); j++)
		{
			if(finalList[i].itemXCoords[j] + finalList[i].itemWidths[j] > maxX)
				maxX = finalList[i].itemXCoords[j] + finalList[i].itemWidths[j];
		}

		if(maxX <= 0.5 * BinWidth)
		{
			halfCounter++;
		}
	}

	sort(finalList.begin(), finalList.end());

	for (int i = 0; i < finalList.size(); i++)
	{
		if(nbClosedBins == nbBins)
		{
			nbBins++;
			binList.resize(nbBins);
			closedBins.resize(nbBins);
			closedBins.back() = 0;
			fullCheck = 1;
		}

		Layer l = finalList[i];
		if(l.itemList.size() == 1)
			continue;

		/*
		int binID = counter % nbBins;

		if(fullCheck == 0 && closedBins[binID] == 0)
		{
			if(binList[binID].height + l.layerHeight <= BinHeight)
			{
				binList[binID].binLayers.push_back(l);
				binList[binID].height += l.layerHeight;
			}
			else
			{
				closedBins[binID] = 1;
				nbClosedBins++;
			}
		}
		else
		{
			if(fullCheck == 1 && closedBins.back() == 0)
			{
				if(binList.back().height + l.layerHeight <= BinHeight)
				{
					binList.back().binLayers.push_back(l);
					binList.back().height += l.layerHeight;
				}
				else
				{
					closedBins.back() = 1;
					nbClosedBins++;
				}
			}
		}

		counter++;

		*/
		int binsFull = 0;
		for (int j = 0; j < nbBins; j++)
		{
			if(binList[j].height + l.layerHeight <= BinHeight)
			{
				binList[j].binLayers.push_back(l);
				binList[j].height += l.layerHeight;
				break;
			}
			else
				binsFull++;
		}

		if(binsFull == nbBins)
		{
			nbClosedBins = nbBins;
			i--;
		}
	}


	for (int i = 0; i < finalTopList.size(); i++)
	{
		if(binList[i].binLayers.size() == 0)
			continue;

		Layer tempLayer = binList[i].binLayers.front();
		for (int j = 1; j < binList[i].binLayers.size(); j++)
		{
			binList[i].binLayers[j - 1] = binList[i].binLayers[j];
		}

		binList[i].binLayers.back() = tempLayer;
	}

}

void binGeneratorBalanced(vector<Layer> finalList, vector<Layer> finalTopList, double LB)
{
	int finalTopListSize = finalTopList.size();
	int nbBins = max((int)ceil(LB / normalLayerHeight), finalTopListSize);

	
	binList.resize(nbBins);
	int nbClosedBins = 0;
	vector<int> closedBins(nbBins, 0);
	int fullCheck = 0;
	int counter = 0;

	for (int i = 0; i < finalTopList.size(); i++)
	{
		Layer l = finalTopList[i];

		binList[i].binLayers.push_back(l);
		binList[i].height += l.layerHeight;
	}
	double totalHeight = 0;
	int halfCounter = 0;
	for (int i = 0; i < finalList.size(); i++)
	{
		double spaceOccupied = 0;

		for (int j = 0; j < finalList[i].itemList.size(); j++)
		{
			spaceOccupied += finalList[i].itemWidths[j] * finalList[i].itemDepths[j] * finalList[i].itemHeights[j];
		}

		finalList[i].layerOccupancy = spaceOccupied * 100 / (BinWidth * BinDepth * finalList[i].layerHeight);

		int maxX = 0;

		for (int j = 0; j < finalList[i].itemList.size(); j++)
		{
			if(finalList[i].itemXCoords[j] + finalList[i].itemWidths[j] > maxX)
				maxX = finalList[i].itemXCoords[j] + finalList[i].itemWidths[j];
		}

		if(maxX <= 0.5 * BinWidth)
		{
			halfCounter++;
		}
		totalHeight += finalList[i].layerHeight;
	}

	int nbNormalBins = (int)ceil(totalHeight / normalLayerHeight);

	sort(finalList.begin(), finalList.end());

	int ct = 0;
	int checkCt = 0;
	for (int i = 0; i < finalList.size(); i++)
	{
		if(nbClosedBins == nbBins)
		{
			nbBins++;
			binList.resize(nbBins);
			closedBins.resize(nbBins);
			closedBins.back() = 0;
			fullCheck = 1;
		}

		Layer l = finalList[i];
		if(l.itemList.size() == 1)
			continue;

		/*
		int binID = counter % nbBins;

		if(fullCheck == 0 && closedBins[binID] == 0)
		{
			if(binList[binID].height + l.layerHeight <= BinHeight)
			{
				binList[binID].binLayers.push_back(l);
				binList[binID].height += l.layerHeight;
			}
			else
			{
				closedBins[binID] = 1;
				nbClosedBins++;
			}
		}
		else
		{
			if(fullCheck == 1 && closedBins.back() == 0)
			{
				if(binList.back().height + l.layerHeight <= BinHeight)
				{
					binList.back().binLayers.push_back(l);
					binList.back().height += l.layerHeight;
				}
				else
				{
					closedBins.back() = 1;
					nbClosedBins++;
				}
			}
		}

		counter++;

		*/
		int binsFull = 0;

		if(binList[ct % (nbNormalBins + 1)].height + l.layerHeight <= BinHeight)
		{
			binList[ct % (nbNormalBins + 1)].binLayers.push_back(l);
			binList[ct % (nbNormalBins + 1)].height += l.layerHeight;

			if(checkCt > 0)
				checkCt = 0;
		}
		else
		{
			checkCt++;
		}

		if(checkCt == nbNormalBins + 1)
		{
			binList[nbNormalBins + 2].binLayers.push_back(l);
			binList[nbNormalBins + 2].height += l.layerHeight;
			checkCt = 0;
		}
		else if (checkCt > 0)
			i--;

		if(binsFull == nbBins)
		{
			nbClosedBins = nbBins;
			i--;
		}
		ct++;
	}


	for (int i = 0; i < finalTopList.size(); i++)
	{
		if(binList[i].binLayers.size() == 0)
			continue;

		Layer tempLayer = binList[i].binLayers.front();
		for (int j = 1; j < binList[i].binLayers.size(); j++)
		{
			binList[i].binLayers[j - 1] = binList[i].binLayers[j];
		}

		binList[i].binLayers.back() = tempLayer;
	}

}

void generateSupportSuperItems()
{
	/*
	//Generate 2-item support super-items
	for (int i = 0; i < itemList.size() - 1; i++)
	{
		for (int j = i + 1; j < itemList.size(); j++)
		{
			//Check if two items can be use as support legs
			if(abs(itemList[i].height - itemList[j].height) <= LayerHeightTolerance && abs(itemList[i].depth - itemList[j].depth) <= supportSuperItemDepthTol && itemList[i].ID != itemList[j].ID)
			{
				//Check for larger items that can be put on top of the support legs
				for (int l = 0; l < itemList.size(); l++)
				{
					//Go to the next item if current item is one of the support legs
					if(itemList[l].ID == itemList[i].ID || itemList[l].ID == itemList[j].ID)
						continue;

					int boxWidth, boxDepth;

					//Check if the top item is wide or deep enough to be used as a top item
					if(itemList[l].width >= supportSuperItemWidthCover * (itemList[i].width + itemList[j].width) && itemList[l].depth <= max(itemList[i].depth, itemList[j].depth) && itemList[l].depth >= 1.0 * min(itemList[i].depth, itemList[j].depth))
					{
						int w, x1, x2, d, y1, y2, h, z1, z2;
						w = itemList[l].width;
						x1 = itemList[i].width;
						x2 = itemList[j].width;
						d = itemList[l].depth;
						y1 = itemList[i].depth;
						y2 = itemList[j].depth;
						h = itemList[l].height;
						z1 = itemList[i].height;
						z2 = itemList[j].height;
						Item newSuperItem(w + (1 - supportSuperItemWidthCover) * (x1 + x2), max(y1, y2), max(z1, z2) + h);

						//Set the support super-item up
						if(y1 >= y2)
						{
							newSuperItem.superItemOnTop = 2;

							newSuperItem.superItemList.push_back(itemList[i]);
							newSuperItem.superItemPosition(0, 0, 0);
							newSuperItem.superItemDimension(x1, y1, z1);

							newSuperItem.superItemList.push_back(itemList[j]);
							newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x1 - supportSuperItemWidthCover*x2, (y1 - y2) / 2, 0);
							newSuperItem.superItemDimension(x2, y2, z2);
							newSuperItem.nbItemsInSILayer.push_back(2);

							newSuperItem.superItemList.push_back(itemList[l]);
							newSuperItem.superItemPosition((1 - supportSuperItemWidthCover)*x1, (y1 - d) / 2, max(z1, z2));
							newSuperItem.superItemDimension(w, d, h);
							newSuperItem.nbItemsInSILayer.push_back(1);
						}
						else
						{
							newSuperItem.superItemOnTop = 2;

							newSuperItem.superItemList.push_back(itemList[j]);
							newSuperItem.superItemPosition(0, 0, 0);
							newSuperItem.superItemDimension(x2, y2, z2);

							newSuperItem.superItemList.push_back(itemList[i]);
							newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x2 - supportSuperItemWidthCover*x1, (y2 - y1) / 2, 0);
							newSuperItem.superItemDimension(x1, y1, z1);
							newSuperItem.nbItemsInSILayer.push_back(2);

							newSuperItem.superItemList.push_back(itemList[l]);
							newSuperItem.superItemPosition((1 - supportSuperItemWidthCover)*x2, (y2 - d) / 2, max(z1, z2));
							newSuperItem.superItemDimension(w, d, h);
							newSuperItem.nbItemsInSILayer.push_back(1);
						}

						//Calculate the empty area between support legs
						boxWidth = w - supportSuperItemWidthCover*(x1 + x2);
						boxDepth = max(y1, y2);
						Layer tempLayer;
						tempLayer.itemList.push_back(itemList[i].ID);
						tempLayer.itemList.push_back(itemList[j].ID);
						tempLayer.itemList.push_back(itemList[l].ID);

						using namespace rbp;

						//Initialize layer sizes
						MaxRectsBinPack bin;
						bin.Init(boxWidth, boxDepth);
						
						//Try to fit as many items in the empty area as you can
						for (int m = 0; m < itemList.size(); m++)
						{
							//Get the item
							Item newItem = itemList[m];

							if(newItem.height > max(z1, z2))
								continue;

							bool match = false;

							//Check if the item already exists in the layer
							for (int k = 0; k < tempLayer.itemList.size(); k++)
							{
								if(newItem.ID == tempLayer.itemList[k])
								{
									match = true;
									break;
								}
							}//for (int j = 0; j < newLayer.itemList.size(); j++)

							if(match == true)
								continue;

							//Get item sizes
							int itemWidth = newItem.width;
							int itemDepth = newItem.depth;
							int itemHeight = newItem.height;

							MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;

							//Perform the packing
							if(MAXRECTSHeuristic == 0)
							heuristic = MaxRectsBinPack::RectContactPointRule;
							else if(MAXRECTSHeuristic == 1)
							heuristic = MaxRectsBinPack::RectBestLongSideFit;
							else if(MAXRECTSHeuristic == 2)
							heuristic = MaxRectsBinPack::RectBestShortSideFit;
							else if(MAXRECTSHeuristic == 3)
							heuristic = MaxRectsBinPack::RectBestAreaFit;
							else
							heuristic = MaxRectsBinPack::RectBottomLeftRule;
							
							Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

							// Test success or failure.
							if (packedRect.height > 0)
							{
								//newLayer.itemList.push_back(itemsToPack[i]);

								tempLayer.itemList.push_back(newItem.ID);

								newSuperItem.superItemList.push_back(newItem);
								newSuperItem.superItemPosition(newSuperItem.superItemWidth[0] + packedRect.x, packedRect.y, 0);
								newSuperItem.superItemDimension(packedRect.width, packedRect.height, newItem.height);
								newSuperItem.nbItemsInSILayer[0]++;
							}

						}//for (int m = 0; m < itemList.size(); m++)

						superItems.push_back(newSuperItem);

					}//if(itemList[l].width >= 0.7 * (itemList[i].width + itemList[j].width) && itemList[l].depth <= itemList[i].depth && itemList[l].depth <= itemList[j].depth)
				}//for (int l = 0; l < itemList.size(); l++)
			}//if(itemList[i].height - itemList[j].height <= LayerHeightTolerance && abs(itemList[i].depth - itemList[j].depth) <= supportSuperItemDepthTol)
		}//for (int j = i + 1; j < itemList.size(); j++)
	}//for (int i = 0; i < itemList.size() - 1; i++)
	*/
		/*

		//Generate 2-item support super-items
		for (int i = 0; i < superItems.size(); i++)
		{
			if(superItems[i].superItemOnTop == 1 && superItems[i].nbItemsInSILayer[0] == 2)
			{
				//Check for larger items that can be put on top of the support legs
				for (int l = 0; l < itemList.size(); l++)
				{
					//Go to the next item if current item is one of the support legs
					if(itemList[l].ID == superItems[i].superItemList[0].ID || itemList[l].ID == superItems[i].superItemList[1].ID)
						continue;

					int boxWidth, boxDepth;

					//Check if the top item is wide or deep enough to be used as a top item
					if(itemList[l].width >= supportSuperItemWidthCover * superItems[i].width && itemList[l].depth == superItems[i].depth &&
						superItems[i].height / itemList[l].width <= 0.75)
					{
						int w, x1, x2, d, y1, y2, h, z1, z2;
						w = itemList[l].width;
						x1 = superItems[i].superItemWidth[0];
						x2 = superItems[i].superItemWidth[1];
						d = itemList[l].depth;
						y1 = superItems[i].superItemDepth[0];
						y2 = superItems[i].superItemDepth[1];
						h = itemList[l].height;
						z1 = superItems[i].superItemHeight[0];
						z2 = superItems[i].superItemHeight[1];
						Item newSuperItem(w + (1 - supportSuperItemWidthCover) * (x1 + x2), max(y1, y2), max(z1, z2) + h);

						//Set the support super-item up
						if(y1 >= y2)
						{
							newSuperItem.superItemOnTop = 2;

							newSuperItem.superItemList.push_back(superItems[i].superItemList[0]);
							newSuperItem.superItemPosition(0, 0, 0);
							newSuperItem.superItemDimension(x1, y1, z1);

							newSuperItem.superItemList.push_back(superItems[i].superItemList[1]);
							newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x1 - supportSuperItemWidthCover*x2, (y1 - y2) / 2, 0);
							newSuperItem.superItemDimension(x2, y2, z2);
							newSuperItem.nbItemsInSILayer.push_back(2);

							newSuperItem.superItemList.push_back(itemList[l]);
							newSuperItem.superItemPosition((1 - supportSuperItemWidthCover)*x1, (y1 - d) / 2, max(z1, z2));
							newSuperItem.superItemDimension(w, d, h);
							newSuperItem.nbItemsInSILayer.push_back(1);
						}
						else
						{
							newSuperItem.superItemOnTop = 2;

							newSuperItem.superItemList.push_back(superItems[i].superItemList[0]);
							newSuperItem.superItemPosition(0, 0, 0);
							newSuperItem.superItemDimension(x2, y2, z2);

							newSuperItem.superItemList.push_back(superItems[i].superItemList[1]);
							newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x2 - supportSuperItemWidthCover*x1, (y2 - y1) / 2, 0);
							newSuperItem.superItemDimension(x1, y1, z1);
							newSuperItem.nbItemsInSILayer.push_back(2);

							newSuperItem.superItemList.push_back(itemList[l]);
							newSuperItem.superItemPosition((1 - supportSuperItemWidthCover)*x2, (y2 - d) / 2, max(z1, z2));
							newSuperItem.superItemDimension(w, d, h);
							newSuperItem.nbItemsInSILayer.push_back(1);
						}

						//Calculate the empty area between support legs
						boxWidth = w - supportSuperItemWidthCover*(x1 + x2);
						boxDepth = max(y1, y2);
						Layer tempLayer;
						tempLayer.itemList.push_back(superItems[i].superItemList[0].ID);
						tempLayer.itemList.push_back(superItems[i].superItemList[1].ID);
						tempLayer.itemList.push_back(itemList[l].ID);

						using namespace rbp;

						//Initialize layer sizes
						MaxRectsBinPack bin;
						bin.Init(boxWidth, boxDepth);

						//Try to fit as many items in the empty area as you can
						for (int m = 0; m < itemList.size(); m++)
						{
							//Get the item
							Item newItem = itemList[m];

							if(newItem.height > max(z1, z2))
								continue;

							bool match = false;

							//Check if the item already exists in the layer
							for (int k = 0; k < tempLayer.itemList.size(); k++)
							{
								if(newItem.ID == tempLayer.itemList[k])
								{
									match = true;
									break;
								}
							}//for (int j = 0; j < newLayer.itemList.size(); j++)

							if(match == true)
								continue;

							//Get item sizes
							int itemWidth = newItem.width;
							int itemDepth = newItem.depth;
							int itemHeight = newItem.height;

							MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;

							//Perform the packing
							if(MAXRECTSHeuristic == 0)
								heuristic = MaxRectsBinPack::RectContactPointRule;
							else if(MAXRECTSHeuristic == 1)
								heuristic = MaxRectsBinPack::RectBestLongSideFit;
							else if(MAXRECTSHeuristic == 2)
								heuristic = MaxRectsBinPack::RectBestShortSideFit;
							else if(MAXRECTSHeuristic == 3)
								heuristic = MaxRectsBinPack::RectBestAreaFit;
							else
								heuristic = MaxRectsBinPack::RectBottomLeftRule;

							Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

							// Test success or failure.
							if (packedRect.height > 0)
							{
								//newLayer.itemList.push_back(itemsToPack[i]);

								tempLayer.itemList.push_back(newItem.ID);

								newSuperItem.superItemList.push_back(newItem);
								newSuperItem.superItemPosition(newSuperItem.superItemWidth[0] + packedRect.x, packedRect.y, 0);
								newSuperItem.superItemDimension(packedRect.width, packedRect.height, newItem.height);
								newSuperItem.nbItemsInSILayer[0]++;
							}

						}//for (int m = 0; m < itemList.size(); m++)

						superItems.push_back(newSuperItem);

					}//if(itemList[l].width >= supportSuperItemWidthCover * superItems[i].width && itemList[l].depth == superItems[i].depth && superItems[i].height / itemList[l].width <= 0.75)
				}//for (int l = 0; l < itemList.size(); l++)
			}//if(superItems[i].superItemOnTop == 1 && superItems[i].nbItemsInSILayer[0] == 2)
		}//for (int i = 0; i < superItems.size(); i++)

		*/
	/*
	
	for (int i = 0; i < itemList.size() - 3; i++)
	{
		for (int j = i + 1; j < itemList.size() - 2; j++)
		{
			if(abs(itemList[i].height - itemList[j].height) <= LayerHeightTolerance && abs(itemList[i].depth - itemList[j].depth) <= supportSuperItemDepthTol && itemList[i].ID != itemList[j].ID)
			{
				for (int k = j + 1; k < itemList.size() - 1; k++)
				{
					if(abs(itemList[i].height - itemList[k].height) <= LayerHeightTolerance && abs(itemList[i].width - itemList[k].width) <= supportSuperItemDepthTol && itemList[j].ID != itemList[k].ID)
					{
						for (int l = k + 1; l < itemList.size(); l++)
						{
							if(abs(itemList[i].height - itemList[l].height) <= LayerHeightTolerance && abs(itemList[k].depth - itemList[l].depth) <= supportSuperItemDepthTol && itemList[k].ID != itemList[l].ID)
							{
								for (int m = 0; m < itemList.size(); m++)
								{
									if(itemList[m].ID == itemList[i].ID || itemList[m].ID == itemList[j].ID || itemList[m].ID == itemList[k].ID || itemList[m].ID == itemList[l].ID)
										continue;

									int boxWidth, boxDepth, boxWidth2, boxDepth2, boxWidth3, boxDepth3;

									//Check if the top item is wide or deep enough to be used as a top item
									if(itemList[m].width >= supportSuperItemWidthCover * (itemList[i].width + itemList[j].width) && itemList[m].width >= supportSuperItemWidthCover * (itemList[k].width + itemList[l].width)
										&& itemList[m].depth >= supportSuperItemDepthCover * (itemList[i].depth + itemList[k].depth) && itemList[m].depth >= supportSuperItemDepthCover * (itemList[j].depth + itemList[l].depth))
									{
										int w, x1, x2, x3, x4, d, y1, y2, y3, y4, h, z1, z2, z3, z4;
										w = itemList[m].width;
										x1 = itemList[i].width;
										x2 = itemList[j].width;
										x3 = itemList[k].width;
										x4 = itemList[l].width;
										d = itemList[m].depth;
										y1 = itemList[i].depth;
										y2 = itemList[j].depth;
										y3 = itemList[k].depth;
										y4 = itemList[l].depth;
										h = itemList[m].height;
										z1 = itemList[i].height;
										z2 = itemList[j].height;
										z3 = itemList[k].height;
										z4 = itemList[l].height;

										Item newSuperItem(max(w + (1 - supportSuperItemWidthCover)*(x1 + x2), w + (1 - supportSuperItemWidthCover)*(x3 + x4)), 
														  max(d + (1 - supportSuperItemDepthCover)*(y1 + y3), d + (1 - supportSuperItemDepthCover)*(y2 + y4)), 
														  max(max(z1, z2), max(z3, z4)) + h);

										//Set the support super-item up

										newSuperItem.superItemOnTop = 2;

										newSuperItem.superItemList.push_back(itemList[i]);
										newSuperItem.superItemPosition(0, 0, 0);
										newSuperItem.superItemDimension(x1, y1, z1);

										newSuperItem.superItemList.push_back(itemList[j]);
										newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x1 - supportSuperItemWidthCover*x2, 0, 0);
										newSuperItem.superItemDimension(x2, y2, z2);

										newSuperItem.superItemList.push_back(itemList[k]);
										newSuperItem.superItemPosition(0, d + (1 - supportSuperItemDepthCover)*y1 - supportSuperItemDepthCover*y3, 0);
										newSuperItem.superItemDimension(x3, y3, z3);

										newSuperItem.superItemList.push_back(itemList[l]);
										newSuperItem.superItemPosition(w + (1 - supportSuperItemWidthCover)*x3 - supportSuperItemWidthCover*x4, d + (1 - supportSuperItemDepthCover)*y2 - supportSuperItemDepthCover*y4, 0);
										newSuperItem.superItemDimension(x4, y4, z4);
										newSuperItem.nbItemsInSILayer.push_back(4);

										newSuperItem.superItemList.push_back(itemList[m]);
										newSuperItem.superItemPosition((1 - supportSuperItemWidthCover)*x1, (1 - supportSuperItemDepthCover)*y1, max(max(z1, z2), max(z3,z4)));
										newSuperItem.superItemDimension(w, d, h);
										newSuperItem.nbItemsInSILayer.push_back(1);

										boxWidth = newSuperItem.width;
										boxDepth = min(d - supportSuperItemDepthCover*(y1 + y3), d - supportSuperItemDepthCover*(y2 + y4));
										boxWidth2 = w - supportSuperItemWidthCover*(x1 + x2);
										boxDepth2 = max(y1, y2);
										boxWidth3 = w - supportSuperItemWidthCover*(x3 + x4);
										boxDepth3 = max(y3, y4);

										Layer tempLayer;
										tempLayer.itemList.push_back(itemList[i].ID);
										tempLayer.itemList.push_back(itemList[j].ID);
										tempLayer.itemList.push_back(itemList[k].ID);
										tempLayer.itemList.push_back(itemList[l].ID);
										tempLayer.itemList.push_back(itemList[m].ID);

										using namespace rbp;

										//Initialize layer sizes
										MaxRectsBinPack bin;
										bin.Init(boxWidth, boxDepth);

										//Try to fit as many items in the empty area as you can
										for (int n = 0; n < itemList.size(); n++)
										{
											//Get the item
											Item newItem = itemList[n];

											if(newItem.height > max(max(z1, z2), max(z3, z4)))
												continue;

											bool match = false;

											//Check if the item already exists in the layer
											for (int o = 0; o < tempLayer.itemList.size(); o++)
											{
												if(newItem.ID == tempLayer.itemList[o])
												{
													match = true;
													break;
												}
											}//for (int j = 0; j < newLayer.itemList.size(); j++)

											if(match == true)
												continue;

											//Get item sizes
											int itemWidth = newItem.width;
											int itemDepth = newItem.depth;
											int itemHeight = newItem.height;

											MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;

											//Perform the packing
											if(MAXRECTSHeuristic == 0)
												heuristic = MaxRectsBinPack::RectContactPointRule;
											else if(MAXRECTSHeuristic == 1)
												heuristic = MaxRectsBinPack::RectBestLongSideFit;
											else if(MAXRECTSHeuristic == 2)
												heuristic = MaxRectsBinPack::RectBestShortSideFit;
											else if(MAXRECTSHeuristic == 3)
												heuristic = MaxRectsBinPack::RectBestAreaFit;
											else
												heuristic = MaxRectsBinPack::RectBottomLeftRule;

											Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

											// Test success or failure.
											if (packedRect.height > 0)
											{
												//newLayer.itemList.push_back(itemsToPack[i]);

												tempLayer.itemList.push_back(newItem.ID);

												newSuperItem.superItemList.push_back(newItem);
												newSuperItem.superItemPosition(packedRect.x, max(y1, y2) + packedRect.y, 0);
												newSuperItem.superItemDimension(packedRect.width, packedRect.height, newItem.height);
												newSuperItem.nbItemsInSILayer[0]++;
											}

										}//for (int m = 0; m < itemList.size(); m++)

										//Initialize layer sizes
										MaxRectsBinPack bin2;
										bin2.Init(boxWidth2, boxDepth2);

										//Try to fit as many items in the empty area as you can
										for (int n = 0; n < itemList.size(); n++)
										{
											//Get the item
											Item newItem = itemList[n];

											if(newItem.height > max(max(z1, z2), max(z3, z4)))
												continue;

											bool match = false;

											//Check if the item already exists in the layer
											for (int o = 0; o < tempLayer.itemList.size(); o++)
											{
												if(newItem.ID == tempLayer.itemList[o])
												{
													match = true;
													break;
												}
											}//for (int j = 0; j < newLayer.itemList.size(); j++)

											if(match == true)
												continue;

											//Get item sizes
											int itemWidth = newItem.width;
											int itemDepth = newItem.depth;
											int itemHeight = newItem.height;

											MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;

											//Perform the packing
											if(MAXRECTSHeuristic == 0)
												heuristic = MaxRectsBinPack::RectContactPointRule;
											else if(MAXRECTSHeuristic == 1)
												heuristic = MaxRectsBinPack::RectBestLongSideFit;
											else if(MAXRECTSHeuristic == 2)
												heuristic = MaxRectsBinPack::RectBestShortSideFit;
											else if(MAXRECTSHeuristic == 3)
												heuristic = MaxRectsBinPack::RectBestAreaFit;
											else
												heuristic = MaxRectsBinPack::RectBottomLeftRule;

											Rect packedRect = bin2.Insert(itemWidth, itemDepth, heuristic);

											// Test success or failure.
											if (packedRect.height > 0)
											{
												//newLayer.itemList.push_back(itemsToPack[i]);

												tempLayer.itemList.push_back(newItem.ID);

												newSuperItem.superItemList.push_back(newItem);
												newSuperItem.superItemPosition(newSuperItem.superItemWidth[0] + packedRect.x, packedRect.y, 0);
												newSuperItem.superItemDimension(packedRect.width, packedRect.height, newItem.height);
												newSuperItem.nbItemsInSILayer[0]++;
											}

										}//for (int m = 0; m < itemList.size(); m++)

										//Initialize layer sizes
										MaxRectsBinPack bin3;
										bin3.Init(boxWidth3, boxDepth3);

										//Try to fit as many items in the empty area as you can
										for (int n = 0; n < itemList.size(); n++)
										{
											//Get the item
											Item newItem = itemList[n];

											if(newItem.height > max(max(z1, z2), max(z3, z4)))
												continue;

											bool match = false;

											//Check if the item already exists in the layer
											for (int o = 0; o < tempLayer.itemList.size(); o++)
											{
												if(newItem.ID == tempLayer.itemList[o])
												{
													match = true;
													break;
												}
											}//for (int j = 0; j < newLayer.itemList.size(); j++)

											if(match == true)
												continue;

											//Get item sizes
											int itemWidth = newItem.width;
											int itemDepth = newItem.depth;
											int itemHeight = newItem.height;

											MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;

											//Perform the packing
											if(MAXRECTSHeuristic == 0)
												heuristic = MaxRectsBinPack::RectContactPointRule;
											else if(MAXRECTSHeuristic == 1)
												heuristic = MaxRectsBinPack::RectBestLongSideFit;
											else if(MAXRECTSHeuristic == 2)
												heuristic = MaxRectsBinPack::RectBestShortSideFit;
											else if(MAXRECTSHeuristic == 3)
												heuristic = MaxRectsBinPack::RectBestAreaFit;
											else
												heuristic = MaxRectsBinPack::RectBottomLeftRule;

											Rect packedRect = bin3.Insert(itemWidth, itemDepth, heuristic);

											// Test success or failure.
											if (packedRect.height > 0)
											{
												//newLayer.itemList.push_back(itemsToPack[i]);

												tempLayer.itemList.push_back(newItem.ID);

												newSuperItem.superItemList.push_back(newItem);
												newSuperItem.superItemPosition(newSuperItem.superItemWidth[2] + packedRect.x, newSuperItem.depth - max(y3, y4) + packedRect.y, 0);
												newSuperItem.superItemDimension(packedRect.width, packedRect.height, newItem.height);
												newSuperItem.nbItemsInSILayer[0]++;
											}

										}//for (int m = 0; m < itemList.size(); m++)

										superItems.push_back(newSuperItem);
									}
								}//for (int m = 0; m < itemList.size(); m++)
							}//if(abs(itemList[i].height - itemList[l].height) <= LayerHeightTolerance && abs(itemList[k].depth - itemList[l].depth) <= supportSuperItemDepthTol)
						}//for (int l = k + 1; l < itemList.size(); l++)
					}//if(abs(itemList[i].height - itemList[k].height) <= LayerHeightTolerance && abs(itemList[i].depth - itemList[j].depth) <= supportSuperItemDepthTol)
				}//for (int k = j + 1; k < itemList.size() - 1; k++)
			}//if(itemList[i].height - itemList[j].height <= LayerHeightTolerance && abs(itemList[i].depth - itemList[j].depth) <= supportSuperItemDepthTol)
		}//for (int j = i + 1; j < itemList.size() - 3; j++)
	}//for (int i = 0; i < itemList.size() - 4; i++)
	*/
}//void generateSupportSuperItems()

//If any edge of the top item is inside the edges of the bottom item, it will be supported by the bottom item
bool bottomSupport(int xCoord, int yCoord, int width, int depth, int leftX, int rightX, int frontY, int backY)			
{
	if((xCoord >= leftX && xCoord <= rightX) || (xCoord + width >= leftX && xCoord + width <= rightX))
	{
		if((yCoord >= frontY && yCoord <= backY) || (yCoord + depth >= frontY && yCoord + depth <= backY))
			return true;

		if((frontY >= yCoord && frontY <= yCoord + depth) && (backY >= yCoord && backY <= yCoord + depth))
			return true;
	}

	//Previous check should take care of this
	/*
	if((top.x >= bottom.x && top.x <= bottom.BackRightX) || (top.BackRightX >= bottom.x && top.BackRightX <= bottom.BackRightX))
	{
		if((bottom.y >= top.y && bottom.y <= top.BackRightY) && (bottom.BackRightY >= top.y && bottom.BackRightY <= top.BackRightY))
			return true;
	}
	*/

	if((yCoord >= frontY && yCoord <= backY) || (yCoord + depth >= frontY && yCoord + depth <= backY))
	{
		if((leftX >= xCoord && leftX <= xCoord + width) && (rightX >= xCoord && rightX <= xCoord + width))
			return true;
	}

	if(yCoord <= frontY && yCoord + depth >= backY && xCoord <= leftX && xCoord + width >= rightX)
		return true;

	return false;
}

vector<int> putRight(Item &item, Item &superItem, int xCoord, int yCoord, int zCoord)
{
	vector<int> returnVector;
	returnVector.resize(3);

	if(yCoord == 0 && zCoord == 0)	//If it's the first row of the first layer of the bin
	{
		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord, yCoord, zCoord);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer.push_back(1);

		if(zCoord + item.height > superItem.height)
			superItem.height = item.height;
	}
	else if(zCoord == 0)			//If it's not the first row of the first layer of the top layer
	{
		int tempY = 0;

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			int placedLeftX = superItem.superItemX[i];
			int placedRightX = superItem.superItemX[i] + superItem.superItemWidth[i];
			int placedBackY = superItem.superItemY[i] + superItem.superItemDepth[i];
			
			if((placedLeftX >= xCoord && placedLeftX <= xCoord + item.width) || (placedRightX >= xCoord && placedRightX <= xCoord + item.width)
				|| (xCoord >= placedLeftX && xCoord + item.width <= placedRightX) || (xCoord >= placedLeftX && xCoord + item.width <= placedRightX))
			{
				if(placedBackY >= tempY && placedBackY <= yCoord)
					tempY = placedBackY;
			}
		}
		
		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord, tempY, zCoord);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer[0]++;

		if(zCoord + item.height > superItem.height)
			superItem.height = item.height;
	}
	else if(yCoord == 0)			//If it's the first row of the layers greater than 1 of the top layer
	{
		int tempZ = 0;

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			if(bottomSupport(xCoord, yCoord, item.width, item.depth, superItem.superItemX[i], superItem.superItemX[i] + superItem.superItemWidth[i], 
				superItem.superItemY[i], superItem.superItemY[i] + superItem.superItemDepth[i]))
			{
				int zLevel =  superItem.superItemZ[i] + superItem.superItemHeight[i];

				if(zLevel >= tempZ && zLevel <= zCoord)
					tempZ = zLevel;
			}
		}

		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord, yCoord, tempZ);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer.back()++;

		if(tempZ + item.height > superItem.height)
			superItem.height = item.height;
		
	}
	else
	{												
		int tempY = 0;
		int tempZ = 0;

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			int placedLeftX = superItem.superItemX[i];
			int placedRightX = superItem.superItemX[i] + superItem.superItemWidth[i];
			int placedBackY = superItem.superItemY[i] + superItem.superItemDepth[i];
			int placedTop = superItem.superItemZ[i] + superItem.superItemHeight[i];;

			if(placedTop > zCoord)
			{
				if((placedLeftX >= xCoord && placedLeftX <= xCoord + item.width) || (placedRightX >= xCoord && placedRightX <= xCoord + item.width)
					|| (xCoord >= placedLeftX && xCoord + item.width <= placedRightX) || (xCoord >= placedLeftX && xCoord + item.width <= placedRightX))
				{
					if(placedBackY >= tempY && placedBackY <= yCoord)
						tempY = placedBackY;
				}
			}

		}

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			if(bottomSupport(xCoord, tempY, item.width, item.depth, superItem.superItemX[i], superItem.superItemX[i] + superItem.superItemWidth[i], 
				superItem.superItemY[i], superItem.superItemY[i] + superItem.superItemDepth[i]))
			{
				int zLevel =  superItem.superItemZ[i] + superItem.superItemHeight[i];

				if(zLevel >= tempZ && zLevel <= zCoord)
					tempZ = zLevel;
			}
		}

		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord, tempY, tempZ);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer.back()++;

		if(tempZ + item.height > superItem.height)
			superItem.height = item.height;
	}

	returnVector[0] = superItem.superItemX.back() + superItem.superItemWidth.back();

	if(superItem.superItemY.back() + superItem.superItemDepth.back() <= BinDepth)
		returnVector[1] = superItem.superItemY.back() + superItem.superItemDepth.back();
	else
		returnVector[1] = -1;

	if(superItem.superItemZ.back() + superItem.superItemHeight.back() <= BinHeight - normalLayerHeight)
		returnVector[2] = superItem.superItemZ.back() + superItem.superItemHeight.back();
	else
		returnVector[2] = -1;

	return returnVector;
}

vector<int> putLeft(Item &item, Item &superItem, int xCoord, int yCoord, int zCoord)
{
	vector<int> returnVector;
	returnVector.resize(3);

	if(zCoord == 0)			//If it's not the first row of the first layer of the bin
	{
		int tempY = 0;

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			int placedLeftX = superItem.superItemX[i];
			int placedRightX = superItem.superItemX[i] + superItem.superItemWidth[i];
			int placedBackY = superItem.superItemY[i] + superItem.superItemDepth[i];
			
			if((placedLeftX <= xCoord && placedLeftX >= xCoord - item.width) || (placedRightX >= xCoord - item.width && placedRightX <= xCoord)
				|| (xCoord - item.width >= placedLeftX && xCoord <= placedRightX) || (xCoord - item.width >= placedLeftX && xCoord <= placedRightX))
			{
				if(placedBackY >= tempY && placedBackY <= yCoord)
					tempY = placedBackY;
			}
		}

		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord - item.width, tempY, zCoord);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer[0]++;

		if(zCoord + item.height > superItem.height)
			superItem.height = item.height;
	}
	else
	{
		item.setX(xCoord - item.width);
		int tempY = 0;
		int tempZ = 0;

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{
			int placedLeftX = superItem.superItemX[i];
			int placedRightX = superItem.superItemX[i] + superItem.superItemWidth[i];
			int placedBackY = superItem.superItemY[i] + superItem.superItemDepth[i];
			int placedTop = superItem.superItemZ[i] + superItem.superItemHeight[i];

			if(placedTop > zCoord)
			{
				if((placedLeftX >= xCoord - item.width && placedLeftX <= xCoord) || (placedRightX >= xCoord - item.width && placedRightX <= xCoord)
					|| (xCoord - item.width >= placedLeftX && xCoord <= placedRightX) || (xCoord - item.width >= placedLeftX && xCoord <= placedRightX))
				{
					if(placedBackY >= tempY && placedBackY <= yCoord)
						tempY = placedBackY;
				}
			}

		}

		for(int i = 0; i < superItem.superItemList.size(); i++)
		{

			if(bottomSupport(xCoord, tempY, item.width, item.depth, superItem.superItemX[i], superItem.superItemX[i] + superItem.superItemWidth[i], 
				superItem.superItemY[i], superItem.superItemY[i] + superItem.superItemDepth[i]))
			{
				int zLevel =  superItem.superItemZ[i] + superItem.superItemHeight[i];

				if(zLevel >= tempZ && zLevel <= zCoord)
					tempZ = zLevel;
			}
		}

		superItem.superItemDimension(item.width, item.depth, item.height);
		superItem.superItemPosition(xCoord - item.width, tempY, tempZ);
		superItem.superItemList.push_back(item);
		superItem.nbItemsInSILayer.back()++;

		if(tempZ + item.height > superItem.height)
			superItem.height = item.height;
		
	}

	returnVector[0] = superItem.superItemX.back();

	if(superItem.superItemY.back() + superItem.superItemDepth.back() <= BinDepth)
		returnVector[1] = superItem.superItemY.back() + superItem.superItemDepth.back();
	else
		returnVector[1] = -1;

	if(superItem.superItemZ.back() + superItem.superItemHeight.back() <= BinHeight - normalLayerHeight)
		returnVector[2] = superItem.superItemZ.back() + superItem.superItemHeight.back();
	else
		returnVector[2] = -1;
	
	return returnVector;
}

Item placeItemsSShape(Item &superItem, vector<Item> &sequence)
{
	int currentX = 0, currentY = 0, currentZ = 0;
	int rightLeftSwitch = 1; // 1 means go right, 2 means go left
	int frontBackSwitch = 1; // 1 means go back, 2 means go front
	int maxYForRow = 0;
	int minYForRow = BinDepth;
	int maxZForLayer = 0;
	bool feasible = true;
	bool goUp = false;
	int layerCounter = 1;
	vector<Item> seqCopy = sequence;
	sort(seqCopy.begin(), seqCopy.end(), &sortDescHeight);

	while(seqCopy.size() > 0)
	{
		Item currentItem = seqCopy[0];
		
		vector<int> placementVector(3);

		if(rightLeftSwitch == 1 && currentX + currentItem.width <= BinWidth && goUp == false)
		{
			placementVector = putRight(currentItem, superItem, currentX, currentY, currentZ);
			
			if(placementVector[1] == -1)
			{
				goUp = true;
				superItem.removeLastSIItem();
				layerCounter++;
			}

			if(placementVector[2] == -1)
			{
				feasible = false;
				superItem.removeLastSIItem();
				if(currentX == 0 && currentY == 0)
				{
					superItem.superItemOnTop--;
					superItem.nbItemsInSILayer.pop_back();
				}

				break;
			}

			if(goUp == false)
			{
				currentX = placementVector[0];

				if(placementVector[1] > maxYForRow)
					maxYForRow = placementVector[1];

				if(placementVector[2] > maxZForLayer)
					maxZForLayer = placementVector[2];

				seqCopy.erase(seqCopy.begin());
				if(placementVector[2] > superItem.height)
					superItem.height = placementVector[2];
			}
		}

		else if(rightLeftSwitch == 1 && currentX + currentItem.width > BinWidth && goUp == false) 
		{
			currentX = BinWidth;
			currentY = maxYForRow;
			rightLeftSwitch = 2;
		}

		else if(rightLeftSwitch == 2 && currentX - currentItem.width >= 0 && goUp == false)
		{
			placementVector = putLeft(currentItem, superItem, currentX, currentY, currentZ);

			if(placementVector[1] == -1)
			{
				goUp = true;
				superItem.removeLastSIItem();
				layerCounter++;
			}

			if(placementVector[2] == -1)
			{
				feasible = false;
				superItem.removeLastSIItem();
				break;
			}

			if(goUp == false)
			{
				currentX = placementVector[0];

				if(placementVector[1] > maxYForRow)
					maxYForRow = placementVector[1];

				if(placementVector[2] > maxZForLayer)
					maxZForLayer = placementVector[2];

				seqCopy.erase(seqCopy.begin());
				if(placementVector[2] > superItem.height)
					superItem.height = placementVector[2];
			}
		}

		else if(rightLeftSwitch == 2 && currentX - currentItem.width < 0 && goUp == false)
		{
			currentX = 0;
			currentY = maxYForRow;
			rightLeftSwitch = 1;
		}

		if(goUp == true)
		{
			currentX = 0;
			currentY = 0;
			maxYForRow = 0;
			currentZ = maxZForLayer;
			rightLeftSwitch = 1;
			goUp = false;
			seqCopy.erase(seqCopy.begin());
			superItem.superItemOnTop++;
			superItem.nbItemsInSILayer.push_back(0);

			if(layerCounter % 2 == 1)
				sort(seqCopy.begin(), seqCopy.end(), &sortDescHeight);
			else
				sort(seqCopy.begin(), seqCopy.end(), &sortAscHeight);
		}

	}//for (int i = 0; i < sequence.size(); i++)


	sequence = seqCopy;
	return superItem;
}