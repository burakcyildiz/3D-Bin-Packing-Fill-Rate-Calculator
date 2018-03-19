#include "Common.h"
#include "LayerGenerator.h"
#include <cstdio>
#include <iostream>

typedef IloArray<IloNumArray> Float2Matrix;
typedef IloArray<Float2Matrix> Float3Matrix;
typedef IloArray<Float3Matrix> Float4Matrix;

typedef IloArray<IloNumVarArray> Var2Matrix;
typedef IloArray<Var2Matrix> Var3Matrix;
typedef IloArray<Var3Matrix> Var4Matrix;

typedef IloArray<IloRangeArray> Range2Matrix;
typedef IloArray<Range2Matrix> Range3Matrix;
typedef IloArray<Range3Matrix> Range4Matrix;

void buildModelByColumn(IloModel mod,
						IloNumVarArray alpha,
						const IloInt nUniqueItems,
						const IloInt nNormalColumns,
						const vector<Layer> normalLayers,
						IloNumVar::Type type,
						IloRangeArray range,
						IloObjective cost)
{
	IloEnv env = mod.getEnv();
	IloInt i, k;
	mod.add(range);



	for (int k = 0; k < nNormalColumns; k++)
	{
		IloNumColumn col;
		//if(layers[k].itemList.size() == 1 && layers[k].itemList[0] < nItems)
		//	col = cost(10000);
		//else
		col = cost(normalLayers[k].layerHeight);

		//int itemID = 0;
		for (int i = 0; i < nUniqueItems; i++)
		{
			//if(i != layers[k].itemList[itemID])
			col += range[i](0);
			/*
			else
			{
			col += range[i](1);
			if(itemID + 1 < layers[k].itemList.size())
			itemID++;
			}
			*/
		}

		alpha.add(IloNumVar(col, 0, 1, type));
		col.end();

		for (int i = 0; i < normalLayers[k].itemList.size(); i++)
		{
			if(normalLayers[k].itemList[i] < nItems)
				range[normalLayers[k].itemList[i]].setLinearCoef(alpha[k], 1);
			else
			{
				for (int j = 0; j < superItems[normalLayers[k].itemList[i] - nItems].superItemList.size(); j++)
				{
					range[superItems[normalLayers[k].itemList[i] - nItems].superItemList[j].ID].setLinearCoef(alpha[k], 1);
				}
			}
		}
	}

	/*
	for (int k = 0; k < nTopColumns; k++)
	{
	IloNumColumn col;

	col = cost(topLayers[k].layerHeight);

	for (int i = 0; i < nUniqueItems; i++)
	{
	col += range[i](0);
	}

	col += range2(1 - topLayers[k].layerHeight / BinHeight);

	beta.add(IloNumVar(col, 0, 1, type));
	col.end();

	for (int i = 0; i < topLayers[k].itemList.size(); i++)
	{
	if(topLayers[k].itemList[i] < nItems)
	range[topLayers[k].itemList[i]].setLinearCoef(beta[k], 1);
	else
	{
	for (int j = 0; j < superItems[topLayers[k].itemList[i] - nItems].superItemList.size(); j++)
	{
	range[superItems[topLayers[k].itemList[i] - nItems].superItemList[j].ID].setLinearCoef(beta[k], 1);
	}
	}
	}
	}
	*/
}

void buildModelByRow(IloModel mod,
					 Var2Matrix c,
					 vector<vector<vector<int>>> zPar,
					 const vector<int> w,
					 const vector<int> d,
					 int nbItems,
					 Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	
	IloNumVarArray a(env, nbItems);
	IloNumVarArray b(env, nbItems);

	vector<vector<int>> xStrips;
	vector<vector<int>> yStrips;

	for (int i = 0; i < nbItems; i++)
	{
		int xStartCoord = layer.itemXCoords[i];
		int xEndCoord = layer.itemXCoords[i] + layer.itemWidths[i];
		int yStartCoord = layer.itemYCoords[i];
		int yEndCoord = yStartCoord + layer.itemDepths[i];

		vector<int> singleXStrip;
		vector<int> singleYStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				int nXStartCoord = layer.itemXCoords[j];
				int nXEndCoord = nXStartCoord + layer.itemWidths[j];

				if((nXStartCoord >= xStartCoord && nXStartCoord <= xEndCoord) || (nXEndCoord >= xStartCoord && nXEndCoord <= xEndCoord)
					|| (nXStartCoord <= xStartCoord && nXEndCoord >= xEndCoord) || (nXStartCoord >= xStartCoord && nXEndCoord <= xEndCoord))
				{
					singleYStrip.push_back(j);
				}

				int nYStartCoord = layer.itemYCoords[j];
				int nYEndCoord = layer.itemDepths[j] + nYStartCoord;

				if((nYStartCoord >= yStartCoord && nYStartCoord <= yEndCoord) || (nYEndCoord >= yStartCoord && nYEndCoord <= yEndCoord)
					|| (nYStartCoord <= yStartCoord && nYEndCoord >= yEndCoord) || (nYStartCoord >= yStartCoord && nYEndCoord <= yEndCoord))
				{
					singleXStrip.push_back(j);
				}
			}
		}

		singleYStrip.push_back(i);
		singleXStrip.push_back(i);

		sort(singleYStrip.begin(), singleYStrip.end());
		sort(singleXStrip.begin(), singleXStrip.end());
		bool xCheck = false;
		bool yCheck = false;

		for (int j = 0; j < xStrips.size(); j++)
		{
			if(singleXStrip == xStrips[j])
			{
				xCheck = true;
				break;
			}
		}
		for (int j = 0; j < yStrips.size(); j++)
		{
			if(singleYStrip == yStrips[j])
			{
				yCheck = true;
				break;
			}
		}

		if(xCheck == false)
			xStrips.push_back(singleXStrip);

		if(yCheck == false)
			yStrips.push_back(singleYStrip);
	}
	
	/*
	for (int s = 0; s < 2; s++)
	{
		for (int i = 0; i < nbItems; i++)
		{
			for (int j = 0; j < nbItems; j++)
			{
				if(i != j && zPar[i][j][s] == 1)
				{
					//cout << s << "\t" << i << "\t" << j << endl;
					if((s == 0 && layer.itemXCoords[j] - (layer.itemXCoords[i] + layer.itemWidths[i]) <= 20) || (s == 1 && layer.itemYCoords[j] - (layer.itemYCoords[i] + layer.itemDepths[i]) <= 20))
					{
						sum += c[j][s];
						sum -= (c[i][s] + w[i]);
					}
				}
			}
		}
	}
	*/

	IloNumVar aI(env, -IloInfinity, BinWidth, ILOFLOAT);
	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);

	
	for (int i = 0; i < nbItems; i++)
	{
		a[i] = IloNumVar(env, -IloInfinity, BinWidth, ILOFLOAT);
		b[i] = IloNumVar(env, -IloInfinity, BinDepth, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += a[i] + b[i];
	}
	
	
	/*
	for (int i = 0; i < nbItems; i++)
	{
		if(layer.itemXCoords[i] >= BinWidth / 2)
			sum += c[i][0];

		if(layer.itemYCoords[i] >= BinDepth / 2)
			sum += c[i][1];
	}
	*/
	/*
	for (int i = 0; i < nbItems; i++)
	{
		for (int s = 0; s < 2; s++)
		{
			sum += c[i][s];
		}
	}
	*/

	sum += 1000 * (aI + bI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();
				
				if(zPar[i][j][0] == 1)
				{
					IloExpr exprX(env);
					exprX = aI - (c[j][0] - (c[i][0] + w[i]));
					mod.add(exprX <= 0);
					exprX.end();
				}

				if(zPar[i][j][1] == 1)
				{
					IloExpr exprY(env);
					exprY = bI - (c[j][1] - (c[i][1] + d[i]));
					mod.add(exprY <= 0);
					exprY.end();
				}
				
			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();

		/*
		for (int j = 0; j < xStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][0] == 1)
			{
				IloExpr exprXs(env);
				exprXs = a[i] - (c[j][0] - (c[i][0] + w[i]));
				mod.add(exprXs <= 0);
				exprXs.end();
			}
		}	
		for (int j = 0; j < yStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][1] == 1)
			{
				IloExpr exprYs(env);
				exprYs = b[i] - (c[j][1] - (c[i][1] + d[i]));
				mod.add(exprYs <= 0);
				exprYs.end();
			}
		}	
		*/
	}

	for (int j = 0; j < xStrips.size(); j++)
	{
		for (int i = 0; i < xStrips[j].size(); i++)
		{
			for (int k = 0; k < xStrips[j].size(); k++)
			{
				if(xStrips[j][i] != xStrips[j][k] && zPar[xStrips[j][i]][xStrips[j][k]][0] == 1)
				{
					IloExpr exprXs(env);
					exprXs = a[xStrips[j][i]] - (c[xStrips[j][k]][0] - (c[xStrips[j][i]][0] + w[xStrips[j][i]]));
					mod.add(exprXs <= 0);
					exprXs.end();
				}
			}
		}
	}	
	for (int j = 0; j < yStrips.size(); j++)
	{
		for (int i = 0; i < yStrips[j].size(); i++)
		{
			for (int k = 0; k < yStrips[j].size(); k++)
			{
				if(yStrips[j][i] != yStrips[j][k] && zPar[yStrips[j][i]][yStrips[j][k]][1] == 1)
				{
					IloExpr exprYs(env);
					exprYs = b[yStrips[j][i]] - (c[yStrips[j][k]][1] - (c[yStrips[j][i]][1] + d[yStrips[j][i]]));
					mod.add(exprYs <= 0);
					exprYs.end();
				}
			}
		}
	}	
}

void buildModelByRowX(IloModel mod,
					 Var2Matrix c,
					 vector<vector<vector<int>>> zPar,
					 const vector<int> w,
					 const vector<int> d,
					 int nbItems,
					 Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	
	IloNumVarArray a(env, nbItems);

	vector<vector<int>> xStrips;
	vector<int> yCoords;

	for (int i = 0; i < nbItems; i++)
	{
		int yStartCoord = layer.itemYCoords[i];
		int yEndCoord = yStartCoord + layer.itemDepths[i];

		vector<int> singleXStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				int nYStartCoord = layer.itemYCoords[j];
				int nYEndCoord = layer.itemDepths[j] + nYStartCoord;

				if((nYStartCoord >= yStartCoord && nYStartCoord <= yEndCoord) || (nYEndCoord >= yStartCoord && nYEndCoord <= yEndCoord)
					|| (nYStartCoord <= yStartCoord && nYEndCoord >= yEndCoord) || (nYStartCoord >= yStartCoord && nYEndCoord <= yEndCoord))
				{
					singleXStrip.push_back(j);
				}
			}
		}

		singleXStrip.push_back(i);

		std::sort(singleXStrip.begin(), singleXStrip.end());
		bool xCheck = false;

		for (int j = 0; j < xStrips.size(); j++)
		{
			if(singleXStrip == xStrips[j])
			{
				xCheck = true;
				break;
			}
		}

		if(xCheck == false)
			xStrips.push_back(singleXStrip);

	}
	
	/*
	for (int s = 0; s < 2; s++)
	{
		for (int i = 0; i < nbItems; i++)
		{
			for (int j = 0; j < nbItems; j++)
			{
				if(i != j && zPar[i][j][s] == 1)
				{
					//cout << s << "\t" << i << "\t" << j << endl;
					if((s == 0 && layer.itemXCoords[j] - (layer.itemXCoords[i] + layer.itemWidths[i]) <= 20) || (s == 1 && layer.itemYCoords[j] - (layer.itemYCoords[i] + layer.itemDepths[i]) <= 20))
					{
						sum += c[j][s];
						sum -= (c[i][s] + w[i]);
					}
				}
			}
		}
	}
	*/

	IloNumVar aI(env, -IloInfinity, BinWidth, ILOFLOAT);
	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);
	
	for (int i = 0; i < nbItems; i++)
	{
		a[i] = IloNumVar(env, -IloInfinity, BinWidth, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += a[i];
	}
	
	
	/*
	for (int i = 0; i < nbItems; i++)
	{
		if(layer.itemXCoords[i] >= BinWidth / 2)
			sum += c[i][0];

		if(layer.itemYCoords[i] >= BinDepth / 2)
			sum += c[i][1];
	}
	*/
	/*
	for (int i = 0; i < nbItems; i++)
	{
		for (int s = 0; s < 2; s++)
		{
			sum += c[i][s];
		}
	}
	*/

	sum += 1000 * (aI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();
				
				if(zPar[i][j][0] == 1)
				{
					IloExpr exprX(env);
					exprX = aI - (c[j][0] - (c[i][0] + w[i]));
					mod.add(exprX <= 0);
					exprX.end();
				}
				
			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();

		/*
		for (int j = 0; j < xStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][0] == 1)
			{
				IloExpr exprXs(env);
				exprXs = a[i] - (c[j][0] - (c[i][0] + w[i]));
				mod.add(exprXs <= 0);
				exprXs.end();
			}
		}	
		for (int j = 0; j < yStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][1] == 1)
			{
				IloExpr exprYs(env);
				exprYs = b[i] - (c[j][1] - (c[i][1] + d[i]));
				mod.add(exprYs <= 0);
				exprYs.end();
			}
		}	
		*/
	}

	for (int j = 0; j < xStrips.size(); j++)
	{
		for (int i = 0; i < xStrips[j].size(); i++)
		{
			for (int k = 0; k < xStrips[j].size(); k++)
			{
				if(xStrips[j][i] != xStrips[j][k] && zPar[xStrips[j][i]][xStrips[j][k]][0] == 1)
				{
					IloExpr exprXs(env);
					exprXs = a[xStrips[j][i]] - (c[xStrips[j][k]][0] - (c[xStrips[j][i]][0] + w[xStrips[j][i]]));
					mod.add(exprXs <= 0);
					exprXs.end();
				}
			}
		}
	}	
}

void buildModelByRowY(IloModel mod,
					 Var2Matrix c,
					 vector<vector<vector<int>>> zPar,
					 const vector<int> w,
					 const vector<int> d,
					 int nbItems,
					 Layer layer)
{
	IloEnv env = mod.getEnv();

	IloExpr sum(env);

	
	IloNumVarArray b(env, nbItems);
	IloNumVarArray cI(env, nbItems);

	vector<vector<int>> yStrips;

	for (int i = 0; i < nbItems; i++)
	{
		int xStartCoord = layer.itemXCoords[i];
		int xEndCoord = layer.itemXCoords[i] + layer.itemWidths[i];

		vector<int> singleYStrip;
		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				int nXStartCoord = layer.itemXCoords[j];
				int nXEndCoord = nXStartCoord + layer.itemWidths[j];

				if((nXStartCoord >= xStartCoord && nXStartCoord <= xEndCoord) || (nXEndCoord >= xStartCoord && nXEndCoord <= xEndCoord)
					|| (nXStartCoord <= xStartCoord && nXEndCoord >= xEndCoord) || (nXStartCoord >= xStartCoord && nXEndCoord <= xEndCoord))
				{
					singleYStrip.push_back(j);
				}
			}
		}

		singleYStrip.push_back(i);

		sort(singleYStrip.begin(), singleYStrip.end());
		bool yCheck = false;

		for (int j = 0; j < yStrips.size(); j++)
		{
			if(singleYStrip == yStrips[j])
			{
				yCheck = true;
				break;
			}
		}

		if(yCheck == false)
			yStrips.push_back(singleYStrip);
	}
	
	/*
	for (int s = 0; s < 2; s++)
	{
		for (int i = 0; i < nbItems; i++)
		{
			for (int j = 0; j < nbItems; j++)
			{
				if(i != j && zPar[i][j][s] == 1)
				{
					//cout << s << "\t" << i << "\t" << j << endl;
					if((s == 0 && layer.itemXCoords[j] - (layer.itemXCoords[i] + layer.itemWidths[i]) <= 20) || (s == 1 && layer.itemYCoords[j] - (layer.itemYCoords[i] + layer.itemDepths[i]) <= 20))
					{
						sum += c[j][s];
						sum -= (c[i][s] + w[i]);
					}
				}
			}
		}
	}
	*/

	IloNumVar bI(env, -IloInfinity, BinDepth, ILOFLOAT);

	
	for (int i = 0; i < nbItems; i++)
	{
		b[i] = IloNumVar(env, -IloInfinity, BinDepth, ILOFLOAT);
		cI[i] = IloNumVar(env, -IloInfinity, IloInfinity, ILOFLOAT);
	}

	for (int i = 0; i < nbItems; i++)
	{
		sum += b[i];
		sum -= cI[i];
	}
	
	
	/*
	for (int i = 0; i < nbItems; i++)
	{
		if(layer.itemXCoords[i] >= BinWidth / 2)
			sum += c[i][0];

		if(layer.itemYCoords[i] >= BinDepth / 2)
			sum += c[i][1];
	}
	*/
	/*
	for (int i = 0; i < nbItems; i++)
	{
		for (int s = 0; s < 2; s++)
		{
			sum += c[i][s];
		}
	}
	*/

	sum += 1000 * (bI);
	mod.add(IloMaximize(env, sum));

	for (int i = 0; i < nbItems; i++)
	{
		IloExpr exprAbs(env);
		exprAbs = c[i][0] - layer.itemXCoords[i];
		mod.add(cI[i] >= exprAbs);
		exprAbs.end();

		IloExpr exprAbs2(env);
		exprAbs2 = layer.itemXCoords[i] - c[i][0];
		mod.add(cI[i] >= exprAbs2);
		exprAbs2.end();

		for (int j = 0; j < nbItems; j++)
		{
			if(i != j)
			{
				IloExpr expr3(env);
				expr3 = c[i][0] + w[i] - c[j][0] - BinWidth + BinWidth*zPar[i][j][0];
				mod.add(expr3 <= 0);
				expr3.end();

				IloExpr expr4(env);
				expr4 = c[i][1] + d[i] - c[j][1] - BinDepth + BinDepth*zPar[i][j][1];
				mod.add(expr4 <= 0);
				expr4.end();

				if(zPar[i][j][1] == 1)
				{
					IloExpr exprY(env);
					exprY = bI - (c[j][1] - (c[i][1] + d[i]));
					mod.add(exprY <= 0);
					exprY.end();
				}
				
			}
		}

		IloExpr expr8(env);
		expr8 = c[i][0] - (BinWidth - w[i]);
		mod.add(expr8 <= 0);
		expr8.end();

		IloExpr expr9(env);
		expr9 = c[i][1] - (BinDepth - d[i]);
		mod.add(expr9 <= 0);
		expr9.end();

		/*
		for (int j = 0; j < xStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][0] == 1)
			{
				IloExpr exprXs(env);
				exprXs = a[i] - (c[j][0] - (c[i][0] + w[i]));
				mod.add(exprXs <= 0);
				exprXs.end();
			}
		}	
		for (int j = 0; j < yStrips[i].size(); j++)
		{
			if(i != j && zPar[i][j][1] == 1)
			{
				IloExpr exprYs(env);
				exprYs = b[i] - (c[j][1] - (c[i][1] + d[i]));
				mod.add(exprYs <= 0);
				exprYs.end();
			}
		}	
		*/
	}

	for (int j = 0; j < yStrips.size(); j++)
	{
		for (int i = 0; i < yStrips[j].size(); i++)
		{
			for (int k = 0; k < yStrips[j].size(); k++)
			{
				if(yStrips[j][i] != yStrips[j][k] && zPar[yStrips[j][i]][yStrips[j][k]][1] == 1)
				{
					IloExpr exprYs(env);
					exprYs = b[yStrips[j][i]] - (c[yStrips[j][k]][1] - (c[yStrips[j][i]][1] + d[yStrips[j][i]]));
					mod.add(exprYs <= 0);
					exprYs.end();
				}
			}
		}
	}	
}

int main(int argc, char **argv)
{
	vector<string> filesToRead;

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

	for (int q = 0; q < filesToRead.size(); q++)
	{
		string filename = filesToRead[q];

		//srand(unsigned(time(0)));
		//string filename = "50_Cases_4";
		parseInput(filename);
		//generateSupportSuperItems();

		clock_t start;
		double duration;
		start = clock();

		groupItems();

		int iteration = 1;
		int nbLayers;
		int improvementCounter = 0;
		double objectiveValue = pow(10,15);
		int nUniqueItems = nItems / 6;
		double improvementObj = pow(10,15);
		int nbLayersLast;

		IloEnv env;
		IloNumVar::Type varType = ILOFLOAT;
		IloModel mod(env);
		IloNumVarArray alpha(env);
		IloNumArray constrMin(env, nUniqueItems);
		IloNumArray constrMax(env, nUniqueItems);
		for (IloInt i = 0; i < nUniqueItems; i++)
		{
			constrMin[i] = 1;
			constrMax[i] = IloInfinity;
		}
		IloRangeArray range(env, constrMin, constrMax);
		IloObjective cost = IloAdd(mod, IloMinimize(env));

		clock_t sectStart;
		double sectDuration;

		while(true)
		{
			bool cont = false;

			sectStart = clock();
			if(iteration == 1)
				generateInitialLayers();
			else
				cont = generateLayers("normal");

			cout << "Case " << q << "|| Iteration " << iteration << " layer generation time (in sec): " << (clock() - sectStart) / (double)CLOCKS_PER_SEC << endl;

			if(iteration > 1 && cont == false)
				break;

			if(iteration > 1 && layerList.size() == nbLayers)
				break;

			if(improvementCounter == improvementLimit)
				break;

			if((clock() - start) / (double) CLOCKS_PER_SEC > 1200)
				break;


			nbLayers = layerList.size();

			//int nextIterLayerStart;

			if(iteration == 1)
			{
				for (int i = 0; i < uniqueItemList.size(); i++)
				{
					Layer newLayer;
					newLayer.itemList.push_back(uniqueItemList[i].ID);
					newLayer.spaceList.push_back(uniqueItemList[i].coveredSpace);
					newLayer.itemDepths.push_back(uniqueItemList[i].depth);
					newLayer.itemWidths.push_back(uniqueItemList[i].width);
					newLayer.itemHeights.push_back(uniqueItemList[i].height);
					newLayer.itemXCoords.push_back(0);
					newLayer.itemYCoords.push_back(0);
					newLayer.layerHeight = uniqueItemList[i].height;
					newLayer.layerOccupancy = (uniqueItemList[i].depth * uniqueItemList[i].width * 100.0) / (BinDepth * BinWidth);
					layerList.push_back(newLayer);
				}
			}

			if(iteration == 1)
			{
				sectStart = clock();

				IloInt nColumns = layerList.size();
				IloInt nTopColumns = topLayerList.size();

				buildModelByColumn(mod, alpha, nUniqueItems, nColumns, layerList, varType, range, cost);

				cout << "Case " << q << "|| Iteration " << iteration << " model build time (in sec): " << (clock() - sectStart) / (double)CLOCKS_PER_SEC << endl;
			}
			else
			{
				sectStart = clock();

				for (int k = nbLayersLast; k < layerList.size(); k++)
				{
					IloNumColumn col;
					col = cost(layerList[k].layerHeight);

					for (int i = 0; i < nUniqueItems; i++)
					{
						col += range[i](0);
					}

					alpha.add(IloNumVar(col, 0, 1, varType));
					col.end();

					for (int i = 0; i < layerList[k].itemList.size(); i++)
					{
						if(layerList[k].itemList[i] < nItems)
							range[layerList[k].itemList[i]].setLinearCoef(alpha[k], 1);
						else
						{
							for (int j = 0; j < superItems[layerList[k].itemList[i] - nItems].superItemList.size(); j++)
							{
								range[superItems[layerList[k].itemList[i] - nItems].superItemList[j].ID].setLinearCoef(alpha[k], 1);
							}
						}
					}
				}
				cout << "Case " << q << "|| Iteration " << iteration << " model build time (in sec): " << (clock() - sectStart) / (double)CLOCKS_PER_SEC << endl;
			}
			//nextIterLayerStart = layerList.size();

			sectStart = clock();

			IloCplex cplex(mod);
			//cplex.exportModel("Model.mps");
			cplex.setOut(env.getNullStream());
			cplex.solve();
			cplex.getStatus();

			cout << "Case " << q << "|| Iteration " << iteration << " model solve time (in sec): " << (clock() - sectStart) / (double)CLOCKS_PER_SEC << endl;

			//std::cout << cplex.getObjValue() << "\t" << iteration << "\t" << layerList.size() << endl;
			nbLayersLast = layerList.size();

			if(iteration == 1)
				improvementObj = cplex.getObjValue();

			if((improvementObj - cplex.getObjValue()) / cplex.getObjValue() > 0.01)
			{
				improvementCounter = 0;
				improvementObj = cplex.getObjValue();
			}
			else
				improvementCounter++;

			objectiveValue = cplex.getObjValue();

			sectStart = clock();

			for (int i = 0; i < itemList.size(); i++)
			{
				itemList[i].reducedCost = cplex.getDual(range[itemList[i].ID]);

				if(i < nUniqueItems)
				{
					uniqueItemList[i].reducedCost = cplex.getDual(range[i]);
					//cout << cplex.getDual(range[i]) << endl;
				}
			}

			for (int i = 0; i < superItems.size(); i++)
			{
				superItems[i].reducedCost = 0;
				for (int j = 0; j < superItems[i].superItemList.size(); j++)
				{
					superItems[i].reducedCost += uniqueItemList[superItems[i].superItemList[j].ID].reducedCost;
				}
			}

			cout << "Case " << q << "|| Iteration " << iteration << " lambda values set time (in sec): " << (clock() - sectStart) / (double)CLOCKS_PER_SEC << endl;

			iteration++;
		}//while(true)

		std::cout << "Finished normal layer generation. Solving the set covering formulation one final time." << endl;

		for (int k = nbLayersLast; k < layerList.size(); k++)
		{
			IloNumColumn col;
			col = cost(layerList[k].layerHeight);

			for (int i = 0; i < nUniqueItems; i++)
			{
				col += range[i](0);
			}

			alpha.add(IloNumVar(col, 0, 1, varType));
			col.end();

			for (int i = 0; i < layerList[k].itemList.size(); i++)
			{
				if(layerList[k].itemList[i] < nItems)
					range[layerList[k].itemList[i]].setLinearCoef(alpha[k], 1);
				else
				{
					for (int j = 0; j < superItems[layerList[k].itemList[i] - nItems].superItemList.size(); j++)
					{
						range[superItems[layerList[k].itemList[i] - nItems].superItemList[j].ID].setLinearCoef(alpha[k], 1);
					}
				}
			}
		}

		varType = ILOFLOAT;
		IloInt nColumns = layerList.size();

		//IloCplex cplex2(mod2);
		//cplex.setOut(env.getNullStream());
		//cplex2.setParam(IloCplex::TiLim, 900);
		//cplex2.solve();

		IloCplex cplex(mod);
		//cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::TiLim, 900);
		cplex.solve();
		cplex.getStatus();

		duration = (clock() - start) / (double) CLOCKS_PER_SEC;

		//cplex2.getStatus();

		objectiveValue = cplex.getObjValue();
		/*
		vector<int> importantLayers;

		for (int i = 0; i < layerList.size(); i++)
		{
		if(cplex2.getValue(alpha2[i]) >= 0.4)
		importantLayers.push_back(i);
		}

		varType = ILOINT;
		nColumns = layerList.size();
		IloModel mod2(env);
		IloNumVarArray alpha2(env);
		IloNumArray constrMin2(env, nUniqueItems);
		IloNumArray constrMax2(env, nUniqueItems);
		for (IloInt i = 0; i < nUniqueItems; i++)
		{
		constrMin2[i] = 1;
		constrMax2[i] = 1;
		}
		IloRangeArray range2(env, constrMin2, constrMax2);
		buildModelByColumn2(mod2, alpha2, nUniqueItems, nColumns, layerList, varType, range2, importantLayers);

		IloCplex cplex2(mod2);
		//cplex.setOut(env.getNullStream());
		cplex2.setParam(IloCplex::TiLim, 900);
		cplex2.solve();

		duration = (clock() - start) / (double) CLOCKS_PER_SEC;

		cplex2.getStatus();

		objectiveValue = cplex2.getObjValue();
		*/


		vector<int> itemSelectionChart(nUniqueItems, 0);
		vector<int> actualSelectionChart(nUniqueItems, 0);
		vector<int> layerSelectionChart(layerList.size(), 0);
		vector<int> topLayerSelectionChart(topLayerList.size(), 0);			//TAKE A LOOK
		int nbItemsSelected = 0;
		int nbTopLayersSelected = 0;

		//vector<Layer> layerListCopy;
		//vector<Layer> topLayerListCopy;										//TAKE A LOOK

		//layerListCopy = layerList;
		//topLayerListCopy = topLayerList;

		for (int k = 0; k < nColumns; k++)
		{
			layerList[k].alpha = cplex.getValue(alpha[k]);
		}

		if(sortByAlpha == 1)
			std::sort(layerList.begin(), layerList.end(), &sortLayers);		//Sort by decreasing alpha
		else
			std::sort(layerList.begin(), layerList.end());					//Sort by decreasing density

		std::sort(topLayerList.begin(), topLayerList.end(), &sortLayers);	//NOPE

		//TOP LAYERS WILL BE GENERATED AFTERWARDS
		/*
		for (int l = 0; l < topLayerList.size(); l++)
		{
		int itemCheck = 0;
		int nbItemsInLayer = 0;

		//if(topLayerList[l].layerHeight > 500)
		//	continue;



		//cout << cplex2.getObjValue() / BinHeight << endl;

		for (int i = 0; i < topLayerList[l].itemList.size(); i++)
		{
		if(topLayerList[l].itemList[i] < nItems)
		{
		nbItemsInLayer++;

		if (itemSelectionChart[topLayerList[l].itemList[i]] == 0)
		itemCheck++;
		}
		else
		{
		Item superItem = superItems[topLayerList[l].itemList[i] - nItems];

		for (int j = 0; j < superItem.superItemList.size(); j++)
		{
		nbItemsInLayer++;

		if(itemSelectionChart[superItem.superItemList[j].ID] == 0)
		itemCheck++;
		}
		}
		}//for (int i = 0; i < topLayerList[l].itemList.size(); i++)

		if(itemCheck == nbItemsInLayer)
		{
		for (int i = 0; i < topLayerList[l].itemList.size(); i++)
		{
		if(topLayerList[l].itemList[i] < nItems)
		{
		itemSelectionChart[topLayerList[l].itemList[i]] = 1;
		nbItemsSelected++;
		}
		else
		{
		Item superItem = superItems[topLayerList[l].itemList[i] - nItems];

		for (int j = 0; j < superItem.superItemList.size(); j++)
		{
		itemSelectionChart[superItem.superItemList[j].ID] = 1;
		nbItemsSelected++;
		}
		}


		}//for (int j = 0; j < layerList[i].itemList.size(); j++)

		topLayerSelectionChart[l] = 1;
		nbTopLayersSelected++;

		}//if(itemCheck == nbItemsInLayer)

		if(nbItemsSelected >= nUniqueItems)
		break;

		}//for (int l = 0; l < topLayerList.size(); l++)
		*/

		//Select normal layers
		cout << "Selecting normal layers." << endl;
		for (int i = 0; i < layerList.size(); i++)
		{
			int itemCheck = 0;
			int nbItemsInLayer = 0;

			//if(layerList[i].layerOccupancy < 70)
			//	continue;

			int nbPreviouslySelected = 0;

			layerList[i].calculateLayerOccupancy();
			if(layerList[i].layerOccupancy < 70)
				continue;

			for (int j = 0; j < layerList[i].itemList.size(); j++)
			{
				if(layerList[i].itemList[j] < nItems)
				{
					nbItemsInLayer++;

					if(itemSelectionChart[layerList[i].itemList[j]] < nbTimesItemCovered)
						itemCheck++;

					if(itemSelectionChart[layerList[i].itemList[j]] > 0)
						nbPreviouslySelected++;
				}
				else
				{
					//cout << i << "\t" << j << endl;
					Item superItem = superItems[layerList[i].itemList[j] - nItems];
					//bool selected = false;

					for (int k = 0; k < superItem.superItemList.size(); k++)
					{
						nbItemsInLayer++;

						if(itemSelectionChart[superItem.superItemList[k].ID] == 0)
							itemCheck++;
						/*
						if(itemSelectionChart[superItem.superItemList[k].ID] > 0 && selected == false)
						selected = true;
						*/
					}
					/*
					if(selected == true)
					nbPreviouslySelected++;
					*/
				}
			}//for (int j = 0; j < layerList[i].itemList.size(); j++)

			if(itemCheck == nbItemsInLayer && nbPreviouslySelected <= nbPreviouslySelectedItems)
			{
				vector<int> removedItemIDs;

				for (int j = 0; j < layerList[i].itemList.size(); j++)
				{
					if(layerList[i].itemList[j] < nItems)
					{
						itemSelectionChart[layerList[i].itemList[j]]++;

						if(itemSelectionChart[layerList[i].itemList[j]] == 1)
						{
							nbItemsSelected++;
							actualSelectionChart[layerList[i].itemList[j]] = 1;
						}
						else if(itemSelectionChart[layerList[i].itemList[j]] > 1)
							removedItemIDs.push_back(j);
					}
					else
					{
						Item superItem = superItems[layerList[i].itemList[j] - nItems];
						//bool remove = false;

						for (int k = 0; k < superItem.superItemList.size(); k++)
						{
							itemSelectionChart[superItem.superItemList[k].ID]++;
							nbItemsSelected++;
							actualSelectionChart[superItem.superItemList[k].ID] = 1;

							//if(itemSelectionChart[superItem.superItemList[k].ID] > 1 && remove == false)
							//{
							//	remove = true;
							//	break;
							//}
						}

						/*
						if(remove == true)
						removedItemIDs.push_back(j);

						if(remove == false)
						nbItemsSelected += superItem.superItemList.size();
						*/
					}


				}//for (int j = 0; j < layerList[i].itemList.size(); j++)

				//Remove the item from layer if it is covered before
				for (int j = removedItemIDs.size() - 1; j >= 0; j--)
				{
					layerList[i].deleteItem(removedItemIDs[j]);
					layerList[i].spaceList[removedItemIDs[j]] = 0;
				}

				removedItemIDs.clear();
				//Add the selected layer to the selected layer list
				layerSelectionChart[i] = 1;
				//layerList[i].calculateLayerOccupancy();
				selectedLayerList.push_back(layerList[i]);



			}//if(itemCheck == nbItemsInLayer && nbPreviouslySelected <= nbPreviouslySelectedItems)

			if(nbItemsSelected >= nUniqueItems)
				break;


		}//for (int i = 0; i < layerList.size(); i++)


		//Determine which items are unselected
		for (int i = 0; i < uniqueItemList.size(); i++)
		{
			if(itemSelectionChart[i] == 0)
				unselectedItemList.push_back(i);
		}

		vector<int> applicableSuperItems(superItems.size(), 0);

		//Tag super-items with selected items as non-applicable
		for (int i = 0; i < superItems.size(); i++)
		{
			for (int j = 0; j < superItems[i].superItemList.size(); j++)
			{
				if(actualSelectionChart[superItems[i].superItemList[j].ID] == 1)
				{
					applicableSuperItems[i] = 1;
					break;
				}
			}
		}

		//Place all items again in layers with empty spaces, and try to fit new items in
		for (int i = 0; i < selectedLayerList.size(); i++)
		{
			Layer curLayer = selectedLayerList[i];
			Layer newLayer;
			newLayer.layerHeight = 0;
			rbp::MaxRectsBinPack bin;
			bin.Init(BinWidth, BinDepth);

			rbp::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic;
			//Perform the packing
			if(MAXRECTSHeuristic == 0)
				heuristic = rbp::MaxRectsBinPack::RectContactPointRule;
			else if(MAXRECTSHeuristic == 1)
				heuristic = rbp::MaxRectsBinPack::RectBestLongSideFit;
			else if(MAXRECTSHeuristic == 2)
				heuristic = rbp::MaxRectsBinPack::RectBestShortSideFit;
			else if(MAXRECTSHeuristic == 3)
				heuristic = rbp::MaxRectsBinPack::RectBestAreaFit;
			else
				heuristic = rbp::MaxRectsBinPack::RectBottomLeftRule;

			//Place every item in the layer again
			for (int j = 0; j < curLayer.itemList.size(); j++)
			{
				int itemWidth = curLayer.itemWidths[j];
				int itemDepth = curLayer.itemDepths[j];
				int itemHeight = curLayer.itemHeights[j];

				rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

				if (packedRect.height > 0)
				{
					newLayer.itemList.push_back(curLayer.itemList[j]);

					

					if(curLayer.itemList[j] < nItems)
					{
						newLayer.orientationList.push_back(curLayer.orientationList[j]);
					}
					else
					{
						if(packedRect.width == superItems[curLayer.itemList[j] - nItems].depth)
							newLayer.orientationList.push_back(2);
						else
							newLayer.orientationList.push_back(1);
					}

					newLayer.itemHeights.push_back(itemHeight);

					newLayer.insertRect(packedRect);
					newLayer.spaceList.push_back(curLayer.spaceList[j]);

					if(itemHeight > newLayer.layerHeight)
						newLayer.layerHeight = itemHeight;

					newLayer.calculateLayerOccupancy();
				}//if (packedRect.height > 0)
			}//for (int j = 0; j < curLayer.itemList.size(); j++)

			//Try to place unselected items
			for (int j = 0; j < itemList.size(); j++)
			{
				Item curItem;
				if(actualSelectionChart[itemList[j].ID] == 0)
					curItem = itemList[j];
				else
					continue;

				int itemWidth = curItem.width;
				int itemDepth = curItem.depth;
				int itemHeight = curItem.height;

				if(itemHeight > newLayer.layerHeight)
					continue;
				else if(newLayer.layerHeight - itemHeight > LayerHeightTolerance + 5)
					continue;

				rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

				if (packedRect.height > 0)
				{
					newLayer.itemList.push_back(curItem.ID);
					newLayer.spaceList.push_back(curItem.coveredSpace);
					newLayer.orientationList.push_back(curItem.orientation);
					newLayer.itemHeights.push_back(itemHeight);

					newLayer.insertRect(packedRect);


					if(itemHeight > newLayer.layerHeight)
						newLayer.layerHeight = itemHeight;

					newLayer.calculateLayerOccupancy();

					actualSelectionChart[curItem.ID] = 1;
				}//if (packedRect.height > 0)
			}//for (int j = 0; j < itemList.size(); j++)

			for (int j = 0; j < superItems.size(); j++)
			{
				Item currentSI = superItems[j];

				//See if the super-item is tagged as non-applicable
				if(applicableSuperItems[j] == 1)
					continue;
				//See if the super-item has any previously covered item. If it does, tag it as non-applicable
				else
				{
					bool scrapSI = false;
					for (int l = 0; l < currentSI.superItemList.size(); l++)
					{
						if(actualSelectionChart[currentSI.superItemList[l].ID] == 1)
						{
							applicableSuperItems[j] = 1;
							scrapSI = true;
							break;
						}
					}
					if(scrapSI == true)
						continue;
				}

				int itemWidth = currentSI.width;
				int itemDepth = currentSI.depth;
				int itemHeight = currentSI.height;

				if(itemHeight > newLayer.layerHeight)
					continue;
				else if(newLayer.layerHeight - itemHeight > LayerHeightTolerance + 5)
					continue;

				rbp::Rect packedRect = bin.Insert(itemWidth, itemDepth, heuristic);

				if (packedRect.height > 0)
				{
					newLayer.itemList.push_back(currentSI.ID);
					newLayer.spaceList.push_back(currentSI.coveredSpace);

					if(packedRect.width == itemDepth)
						newLayer.orientationList.push_back(2);
					else if(packedRect.width == itemWidth)
						newLayer.orientationList.push_back(1);

					newLayer.itemHeights.push_back(itemHeight);

					newLayer.insertRect(packedRect);


					if(itemHeight > newLayer.layerHeight)
						newLayer.layerHeight = itemHeight;

					newLayer.calculateLayerOccupancy();

					for (int k = 0; k < currentSI.superItemList.size(); k++)
					{
						actualSelectionChart[currentSI.superItemList[k].ID] = 1;
					}
					applicableSuperItems[j] = 1;
				}//if (packedRect.height > 0)


			}//for (int j = 0; j < superItems.size(); j++)

			selectedLayerList[i] = newLayer;

		}//for (int i = 0; i < selectedLayerList.size(); i++)

		int nbTopLayers = 0;
		improvementCounter = 0;
		objectiveValue = cplex.getObjValue();
		vector<Layer> newLayerList;

		newLayerList = topLayerList;
		topLayerIG.clear();

		for (int i = 0; i < selectedLayerList.size(); i++)
		{
			if(selectedLayerList[i].layerOccupancy < 60)
			{
				for (int j = 0; j < selectedLayerList[i].itemList.size(); j++)
				{
					if(selectedLayerList[i].itemList[j] < nItems)
						actualSelectionChart[selectedLayerList[i].itemList[j]] = 0;
					else
					{
						for (int k = 0; k < superItems[selectedLayerList[i].itemList[j] - nItems].superItemList.size(); k++)
						{
							actualSelectionChart[superItems[selectedLayerList[i].itemList[j] - nItems].superItemList[k].ID] = 0;
						}
					}
				}
				selectedLayerList.erase(selectedLayerList.begin() + i);
				i--;
			}
		}

		vector<Item> leftoverItems;
		vector<Layer> smallLayers;
		vector<int> widths;
		vector<int> depths;
		vector<int> heights;

		for (int i = 0; i < uniqueItemList.size(); i++)
		{
			if(actualSelectionChart[uniqueItemList[i].ID] == 0)
				leftoverItems.push_back(uniqueItemList[i]);
		}
		for (int i = 0; i < leftoverItems.size(); i++)
		{
			Item tempItem = leftoverItems[i];
			tempItem.sortDimensionsWDH();
			leftoverItems[i] = tempItem;
		}
		

		for (int i = 0; i < superItems.size(); i++)
		{
			if(applicableSuperItems[i] == 1)
				continue;
			else
			{
				bool breakFor = false;
				for (int l = 0; l < superItems[i].superItemList.size(); l++)
				{
					if(actualSelectionChart[superItems[i].superItemList[l].ID] == 1)
					{
						applicableSuperItems[l] = 1;
						breakFor = true;
						break;
					}
				}
				if(breakFor == true)
					continue;
			}

			if(applicableSuperItems[i] == 0 && superItems[i].height <= BinHeight - normalLayerHeight)
			{
				topLayerIG.push_back(superItems[i].ID);
				applicableSuperItems[i] = 1;
			}
		}


		//Spacing with mathematical model
		for (int i = 0; i < selectedLayerList.size(); i++)
		{
			Layer newLayer = selectedLayerList[i];
			Layer modifiedLayer;
			modifiedLayer.layerHeight = newLayer.layerHeight;

			for (int j = 0; j < newLayer.itemList.size(); j++)
			{
				if(newLayer.itemList[j] < nItems)
				{
					modifiedLayer.itemList.push_back(newLayer.itemList[j]);
					modifiedLayer.itemXCoords.push_back(newLayer.itemXCoords[j]);
					modifiedLayer.itemYCoords.push_back(newLayer.itemYCoords[j]);
					modifiedLayer.spaceList.push_back(newLayer.spaceList[j]);
					modifiedLayer.orientationList.push_back(newLayer.orientationList[j]);
					modifiedLayer.itemWidths.push_back(newLayer.itemWidths[j]);
					modifiedLayer.itemDepths.push_back(newLayer.itemDepths[j]);
					modifiedLayer.itemHeights.push_back(newLayer.itemHeights[j]);
				}
				else
				{
					if(superItems[newLayer.itemList[j] - nItems].superItemOnTop > 1)
					{
						modifiedLayer.itemList.push_back(newLayer.itemList[j]);
						modifiedLayer.itemXCoords.push_back(newLayer.itemXCoords[j]);
						modifiedLayer.itemYCoords.push_back(newLayer.itemYCoords[j]);
						modifiedLayer.spaceList.push_back(newLayer.spaceList[j]);
						modifiedLayer.orientationList.push_back(newLayer.orientationList[j]);
						modifiedLayer.itemWidths.push_back(newLayer.itemWidths[j]);
						modifiedLayer.itemDepths.push_back(newLayer.itemDepths[j]);
						modifiedLayer.itemHeights.push_back(newLayer.itemHeights[j]);
					}
					else
					{
						if(newLayer.orientationList[j] == 2)
							superItems[newLayer.itemList[j] - nItems].exchangeSIDimensions();

						for (int k = 0; k < superItems[newLayer.itemList[j] - nItems].superItemList.size(); k++)
						{
							modifiedLayer.itemList.push_back(superItems[newLayer.itemList[j] - nItems].superItemList[k].ID);
							modifiedLayer.itemXCoords.push_back(newLayer.itemXCoords[j] + superItems[newLayer.itemList[j] - nItems].superItemX[k]);
							modifiedLayer.itemYCoords.push_back(newLayer.itemYCoords[j] + superItems[newLayer.itemList[j] - nItems].superItemY[k]);
							modifiedLayer.spaceList.push_back(superItems[newLayer.itemList[j] - nItems].superItemWidth[k] * superItems[newLayer.itemList[j] - nItems].superItemDepth[k]);
							modifiedLayer.orientationList.push_back(0);
							modifiedLayer.itemWidths.push_back(superItems[newLayer.itemList[j] - nItems].superItemWidth[k]);
							modifiedLayer.itemDepths.push_back(superItems[newLayer.itemList[j] - nItems].superItemDepth[k]);
							modifiedLayer.itemHeights.push_back(superItems[newLayer.itemList[j] - nItems].superItemHeight[k]);
						}
					}
				}
			}

			selectedLayerList[i] = modifiedLayer;
			newLayer = modifiedLayer;

			int nbItems = newLayer.itemList.size();

			vector<vector<vector<int>>> zPar;
			vector<int> widths(nbItems, 0);
			vector<int> depths(nbItems, 0);

			zPar.resize(nbItems);

			for (int j = 0; j < nbItems; j++)
			{
				zPar[j].resize(nbItems);

				for (int k = 0; k < nbItems; k++)
				{
					zPar[j][k].resize(2);
				}
			}

			for (int j = 0; j < newLayer.itemList.size(); j++)
			{
				for (int k = 0; k < newLayer.itemList.size(); k++)
				{
					if(j != k && newLayer.itemXCoords[j] + newLayer.itemWidths[j] <= newLayer.itemXCoords[k])
					{
						zPar[j][k][0] = 1;
						zPar[k][j][0] = 0;
					}
					else if(j != k && newLayer.itemXCoords[k] + newLayer.itemWidths[k] <= newLayer.itemXCoords[j])
					{
						zPar[j][k][0] = 0;
						zPar[k][j][0] = 1;
					}

					if(j != k && newLayer.itemYCoords[j] + newLayer.itemDepths[j] <= newLayer.itemYCoords[k])
					{
						zPar[j][k][1] = 1;
						zPar[k][j][1] = 0;
					}
					else if(j != k && newLayer.itemYCoords[k] + newLayer.itemDepths[k] <= newLayer.itemYCoords[j])
					{
						zPar[j][k][1] = 0;
						zPar[k][j][1] = 1;
					}
				}
				widths[j] = newLayer.itemWidths[j];
				depths[j] = newLayer.itemDepths[j];
			}

			IloEnv envRP;
			IloModel modRP(envRP);

			//Define vars
			Var2Matrix c(envRP, nbItems);

			//Initialize vars
			for(int j = 0; j < nbItems; j++)
			{
				c[j] = IloNumVarArray(envRP, 2);

				for (int s = 0; s < 2; s++)
				{
					c[j][s] = IloNumVar(envRP, 0, BinHeight, ILOFLOAT);
				}
			}


			buildModelByRowX(modRP, c, zPar, widths, depths, nbItems, newLayer);

			IloCplex cplexRP(modRP);
			//cplex.setOut(env.getNullStream());
			//cplex.setParam(IloCplex::PreInd, 0);
			cplexRP.setParam(IloCplex::TiLim, 900.0);

			cplexRP.solve();

			for (int j = 0; j < nbItems; j++)
			{
				newLayer.itemXCoords[j] = (int)cplexRP.getValue(c[j][0]);
				newLayer.itemYCoords[j] = (int)cplexRP.getValue(c[j][1]);
			}

			selectedLayerList[i] = newLayer;

			zPar.clear();
			widths.clear();
			depths.clear();
		}

		for (int i = 0; i < selectedLayerList.size(); i++)
		{
			Layer newLayer = selectedLayerList[i];

			int nbItems = newLayer.itemList.size();

			vector<vector<vector<int>>> zPar;
			vector<int> widths(nbItems, 0);
			vector<int> depths(nbItems, 0);

			zPar.resize(nbItems);

			for (int j = 0; j < nbItems; j++)
			{
				zPar[j].resize(nbItems);

				for (int k = 0; k < nbItems; k++)
				{
					zPar[j][k].resize(2);
				}
			}

			for (int j = 0; j < newLayer.itemList.size(); j++)
			{
				for (int k = 0; k < newLayer.itemList.size(); k++)
				{
					if(j != k && newLayer.itemXCoords[j] + newLayer.itemWidths[j] <= newLayer.itemXCoords[k])
					{
						zPar[j][k][0] = 1;
						zPar[k][j][0] = 0;
					}
					else if(j != k && newLayer.itemXCoords[k] + newLayer.itemWidths[k] <= newLayer.itemXCoords[j])
					{
						zPar[j][k][0] = 0;
						zPar[k][j][0] = 1;
					}

					if(j != k && newLayer.itemYCoords[j] + newLayer.itemDepths[j] <= newLayer.itemYCoords[k])
					{
						zPar[j][k][1] = 1;
						zPar[k][j][1] = 0;
					}
					else if(j != k && newLayer.itemYCoords[k] + newLayer.itemDepths[k] <= newLayer.itemYCoords[j])
					{
						zPar[j][k][1] = 0;
						zPar[k][j][1] = 1;
					}
				}
				widths[j] = newLayer.itemWidths[j];
				depths[j] = newLayer.itemDepths[j];
			}

			IloEnv envRPY;
			IloModel modRPY(envRPY);

			//Define vars
			Var2Matrix c(envRPY, nbItems);

			//Initialize vars
			for(int j = 0; j < nbItems; j++)
			{
				c[j] = IloNumVarArray(envRPY, 2);

				for (int s = 0; s < 2; s++)
				{
					c[j][s] = IloNumVar(envRPY, 0, BinHeight, ILOFLOAT);
				}
			}


			buildModelByRowY(modRPY, c, zPar, widths, depths, nbItems, newLayer);

			IloCplex cplexRPY(modRPY);
			//cplex.setOut(env.getNullStream());
			//cplex.setParam(IloCplex::PreInd, 0);
			cplexRPY.setParam(IloCplex::TiLim, 900.0);

			cplexRPY.solve();

			for (int j = 0; j < nbItems; j++)
			{
				newLayer.itemXCoords[j] = (int)cplexRPY.getValue(c[j][0]);
				newLayer.itemYCoords[j] = (int)cplexRPY.getValue(c[j][1]);
			}

			selectedLayerList[i] = newLayer;

			zPar.clear();
			widths.clear();
			depths.clear();
		}

		cout << nUniqueItems - nbItemsSelected << " items are unselected. Creating top layers from them." << endl;

		int coverCount = 0;
		for (int i = 0; i < uniqueItemList.size(); i++)
		{
			coverCount += actualSelectionChart[i];
		}
		cout << nUniqueItems - coverCount << " items remanining" << endl;
		while(leftoverItems.size() > 0)
		{
			Item tempSuperItem(0, 0, 0);

			tempSuperItem = placeItemsSShape(tempSuperItem, leftoverItems);
			tempSuperItem.ID = superItems.back().ID + 1;
			int height = 0;
			for (int i = 0; i < tempSuperItem.superItemList.size(); i++)
			{
				if(tempSuperItem.superItemZ[i] + tempSuperItem.superItemHeight[i] > height)
					height = tempSuperItem.superItemZ[i] + tempSuperItem.superItemHeight[i];
			}
			tempSuperItem.height = height;

			superItems.push_back(tempSuperItem);

			Layer newLayer;
			newLayer.insertItem(tempSuperItem.ID, tempSuperItem.width, tempSuperItem.depth, tempSuperItem.height, 0, 0, 1, tempSuperItem.coveredSpace);
			newLayer.layerHeight = tempSuperItem.height;
			newLayer.calculateLayerOccupancy();
			topLayerList.push_back(newLayer);

			for (int i = 0; i < tempSuperItem.superItemList.size(); i++)
			{
				actualSelectionChart[tempSuperItem.superItemList[i].ID] = 1;
			}
		}

		selectedTopLayerList = topLayerList;

		duration = (clock() - start) / (double)CLOCKS_PER_SEC;
		cout << "Finished filling the gaps." << endl;

		ofstream resultingLayers;
		resultingLayers.open("results.txt");

		ofstream computationalResults;
		computationalResults.open("Results/" + filename + "/Computational Results.txt");

		computationalResults << "Nb Layers Generated" << "\t" 
			<< "Total CPU" << "\t"
			<< "Min Density" << "\t"
			<< "Avg Density" << "\t"
			<< "Max Density" << "\t"
			<< "Nb Layers with Density >70%" << "\t"
			<< "Min Items per Layer" << "\t"
			<< "Avg Items per Layer" << "\t"
			<< "Max Items per Layer" << "\t"
			<< "Nb Items not in Layer" << endl;

		double minDensity = 100;
		double maxDensity = 0;
		double avgDensity;
		double totalDensity;
		int layerCounter = 0;
		int nbDensityOver70 = 0;
		int minItemsPerLayer = 5000;
		int maxItemsPerLayer = 0;
		int totalItemsPerLayer = 0;
		double avgItemsPerLayer;
		int nbItemsNotInLayer = 0;

		ofstream var;
		var.open("alpha.txt");

		double upperBound = 0;
		double lowerBound = 0;

		cout << "Calculating statistics." << endl;
		for (int k = 0; k < layerList.size(); k++)
		{
			if(layerList[k].itemList.size() > 1)
				lowerBound += cplex.getValue(alpha[k]) * layerList[k].layerHeight;

			var << cplex.getValue(alpha[k]) << endl;
			layerList[k].alpha = cplex.getValue(alpha[k]);
			if(layerSelectionChart[k] == 1)
			{
				resultingLayers << layerList[k].itemList.size() << "\t" << layerList[k].layerOccupancy << "\t" << layerList[k].layerHeight << endl;

				upperBound += layerList[k].layerHeight;
			}

			if(layerList[k].itemList.size() > 1)
			{
				minDensity = min(minDensity, layerList[k].layerOccupancy);
				maxDensity = max(maxDensity, layerList[k].layerOccupancy);
				totalDensity += layerList[k].layerOccupancy;
				layerCounter++;

				int maxX = 0;
				int maxY = 0;

				if(layerList[k].layerOccupancy > 70)
				{
					nbDensityOver70++;

					for (int i = 0; i < layerList[k].itemList.size(); i++)
					{
						if(layerList[k].itemXCoords[i] + layerList[k].itemWidths[i] > maxX)
							maxX = layerList[k].itemXCoords[i] + layerList[k].itemWidths[i];

						if(layerList[k].itemYCoords[i] + layerList[k].itemDepths[i] > maxY)
							maxY = layerList[k].itemYCoords[i] + layerList[k].itemDepths[i];
					}

					double xRatio = BinWidth / maxX;
					double yRatio = BinDepth / maxY;

					for (int i = 0; i < layerList[k].itemList.size(); i++)
					{
						layerList[k].itemXCoords[i] *= xRatio;
						layerList[k].itemYCoords[i] *= yRatio;
					}
				}

				minItemsPerLayer = min(minItemsPerLayer, (int)layerList[k].itemList.size());
				maxItemsPerLayer = max(maxItemsPerLayer, (int)layerList[k].itemList.size());
				totalItemsPerLayer += layerList[k].itemList.size();
			}
		}

		for (int l = 0; l < selectedTopLayerList.size(); l++)
		{
			upperBound += selectedTopLayerList[l].layerHeight;


			if(selectedTopLayerList[l].itemList.size() > 1)
			{
				minDensity = min(minDensity, selectedTopLayerList[l].layerOccupancy);
				maxDensity = max(maxDensity, selectedTopLayerList[l].layerOccupancy);
				totalDensity += selectedTopLayerList[l].layerOccupancy;
				layerCounter++;

				if(selectedTopLayerList[l].layerOccupancy > 70)
					nbDensityOver70++;

				minItemsPerLayer = min(minItemsPerLayer, (int)selectedTopLayerList[l].itemList.size());
				maxItemsPerLayer = max(maxItemsPerLayer, (int)selectedTopLayerList[l].itemList.size());
				totalItemsPerLayer += selectedTopLayerList[l].itemList.size();
			}
		}

		avgDensity = totalDensity / layerCounter;
		avgItemsPerLayer = totalItemsPerLayer / layerCounter;

		computationalResults << layerCounter << "\t"
			<< duration << "\t"
			<< minDensity << "\t"
			<< avgDensity << "\t"
			<< maxDensity << "\t"
			<< nbDensityOver70 << "\t"
			<< minItemsPerLayer << "\t"
			<< avgItemsPerLayer << "\t"
			<< maxItemsPerLayer << "\t"
			<< nUniqueItems - nbItemsSelected << "\t"
			<< upperBound << "\t"
			<< objectiveValue << endl;

		ofstream resultingBin;

		int currentHeight = 0;
		double totalHeight = 0;
		for (int i = 0; i < selectedLayerList.size(); i++)
		{
			selectedLayerList[i].calculateLayerOccupancy();
			totalHeight += selectedLayerList[i].layerHeight;
		}

		sort(selectedLayerList.begin(), selectedLayerList.end());


		cout << "Generating bins." << endl;

		if(balanced == 1)
			binGeneratorBalanced(selectedLayerList, selectedTopLayerList, totalHeight);
		else
			binGenerator(selectedLayerList, selectedTopLayerList, totalHeight);

		ofstream unselectedItems;
		unselectedItems.open("Results/" + filename + "/unselectedItems.txt");

		ofstream selectedItems;
		selectedItems.open("Results/" + filename + "/selectedItems.txt");

		cout << "Writing results to files." << endl;
		for (int b = 0; b < binList.size(); b++)
		{
			currentHeight = 0;
			char fileName[100];
			sprintf(fileName, "Results/");
			sprintf(fileName + strlen(fileName), filename.c_str());
			sprintf(fileName + strlen(fileName), "/currentbin%d.txt", b);
			resultingBin.open(fileName);
			vector<Layer> binLayers = binList[b].binLayers;

			for (int i = 0; i < binLayers.size(); i++)
			{
				for (int j = 0; j < binLayers[i].itemList.size(); j++)
				{
					if(binLayers[i].itemList[j] < nItems)
					{
						//cout << i << "\t" << j << endl;

						resultingBin << binLayers[i].itemXCoords[j] << " "
							<< binLayers[i].itemYCoords[j] << " "
							<< currentHeight << " "
							<< binLayers[i].itemWidths[j] << " "
							<< binLayers[i].itemDepths[j] << " "
							<< binLayers[i].itemHeights[j] << " "
							<< "1914 11585 2 128650" << endl;
					}
					else
					{
						if(binLayers[i].orientationList[j] == 2)
							superItems[binLayers[i].itemList[j] - nItems].exchangeSIDimensions();

						for (int k = 0; k < superItems[binLayers[i].itemList[j] - nItems].superItemList.size(); k++)
						{
							cout << i << "\t" << j << "\t" << k << endl;

							resultingBin << binLayers[i].itemXCoords[j] + superItems[binLayers[i].itemList[j] - nItems].superItemX[k] << " "
								<< binLayers[i].itemYCoords[j] + superItems[binLayers[i].itemList[j] - nItems].superItemY[k] << " "
								<< currentHeight + superItems[binLayers[i].itemList[j] - nItems].superItemZ[k] << " "
								<< superItems[binLayers[i].itemList[j] - nItems].superItemWidth[k] << " "
								<< superItems[binLayers[i].itemList[j] - nItems].superItemDepth[k] << " "
								<< superItems[binLayers[i].itemList[j] - nItems].superItemHeight[k] << " "
								<< "1914 11585 2 128650" << endl;
						}
					}

				}//for (int j = 0; j < binLayers[i].itemList.size(); j++)

				currentHeight += binLayers[i].layerHeight;
			}//for (int i = 0; i < binLayers.size(); i++)
			resultingBin.close();
		}//for (int i = 0; i < binLayers.size(); i++)

		for (int i = 0; i < binList.size(); i++)
		{
			cout << "For bin " << i << endl;
			for (int j = 0; j < binList[i].binLayers.size(); j++)
			{
				cout << "For layers " << j << endl;
				for (int k = 0; k < binList[i].binLayers[j].itemList.size(); k++)
				{
					if(binList[i].binLayers[j].itemList[k] < nItems)  
						cout << "Item " << binList[i].binLayers[j].itemList[k] << endl;
					else
					{
						for (int l = 0; l < superItems[binList[i].binLayers[j].itemList[k] - nItems].superItemList.size(); l++)
						{
							cout << "Item " << superItems[binList[i].binLayers[j].itemList[k] - nItems].superItemList[l].ID << endl;
						}
					}
				}
			}
		}


		vector<int> placeCounter(nbLines, 0);
		vector<int> pCounter(nbLines, 0);
		for (int i = 0; i < nUniqueItems; i++)
		{
			if(actualSelectionChart[i] == 1)
				placeCounter[uniqueItemList[i].inputPlace]++;
			else
				pCounter[uniqueItemList[i].inputPlace]++;
		}

		for (int i = 0; i < nbLines; i++)
		{
			selectedItems << i << "\t" << placeCounter[i] << endl;
		}



		for (int b = 0; b < binList.size(); b++)
		{
			if(binList[b].binLayers.size() == 1)
			{
				for (int i = 0; i < binList[b].binLayers[0].itemList.size(); i++)
				{
					if(binList[b].binLayers[0].itemList[i] < nItems)
					{
						pCounter[uniqueItemList[binList[b].binLayers[0].itemList[i]].inputPlace]++;
					}
					else
					{
						for (int j = 0; j < superItems[binList[b].binLayers[0].itemList[i] - nItems].superItemList.size(); j++)
						{
							pCounter[superItems[binList[b].binLayers[0].itemList[i] - nItems].superItemList[j].inputPlace]++;
						}
					}
				}
			}
		}

		for (int i = 0; i < nbLines; i++)
		{
			unselectedItems << i << "\t" << pCounter[i] << endl;
		}


		itemList.clear();
		uniqueItemList.clear();
		superItems.clear();
		unselectedItemList.clear();
		layerList.clear();
		topLayerList.clear();
		selectedLayerList.clear();
		selectedTopLayerList.clear();
		binList.clear();

		itemGroups.clear();
		topLayerIG.clear();

		//unordered_map<Layer, int> layerHash;
		layerSet.clear();
	}
}
