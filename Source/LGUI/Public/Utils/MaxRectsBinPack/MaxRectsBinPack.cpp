// Copyright 2019-2022 LexLiu. All Rights Reserved.

/** @file MaxRectsBinPack.cpp
	@author Jukka Jylänki

	@brief Implements different bin packer algorithms that use the MAXRECTS data structure.

	This work is released to Public Domain, do whatever you want with it.
*/

/*
Modified by lexliu:
	Support ue4's native classes, remove std stuff
	Add function to expend bin pack size, so I can add more rects and keep origin rects

	This version is also public domain - do whatever you want with it.
*/

#include "MaxRectsBinPack.h"

namespace rbp {

	using namespace std;

	int min(int a, int b)
	{
		return a > b ? b : a;
	}
	int max(int a, int b)
	{
		return a > b ? a : b;
	}
	int abs(int a)
	{
		return a > 0 ? a : -a;
	}

	MaxRectsBinPack::MaxRectsBinPack()
		:binWidth(0),
		binHeight(0)
	{
	}

	MaxRectsBinPack::MaxRectsBinPack(int width, int height, bool allowFlip)
	{
		Init(width, height, allowFlip);
	}

	void MaxRectsBinPack::Init(int width, int height, bool allowFlip)
	{
		binAllowFlip = allowFlip;
		binWidth = width;
		binHeight = height;

		Rect n;
		n.x = 0;
		n.y = 0;
		n.width = width;
		n.height = height;

		usedRectangles.Reset();

		freeRectangles.Reset();
		freeRectangles.Add(n);
	}

	void MaxRectsBinPack::ExpendSize(int newWidth, int newHeight)
	{
		if (binWidth > newWidth || binHeight > newHeight)//new size is smaller
			return;

		//for freeRectangles, we must find which rect's edge touch the original binPack's edge, and expend these rects
		int extraWidth = newWidth - binWidth;
		int extraHeight = newHeight - binHeight;
		int count = freeRectangles.Num();
		for (int i = 0; i < count; i++)
		{
			auto& rectItem = freeRectangles[i];
			if (rectItem.x + rectItem.width == binWidth)//horizontal touch edge
			{
				rectItem.width += extraWidth;
			}
			if (rectItem.y + rectItem.height == binHeight)//vertical touch edge
			{
				rectItem.height += extraHeight;
			}
		}
		//and for usedRectangles, we must find which rect's edge touch the original binPack's edge, and build a new free rect from that edge
		//incase any usedRect is just fit on right top corner, we must build rect for that
		count = usedRectangles.Num();
		for (int i = 0; i < count; i++)
		{
			auto& rectItem = usedRectangles[i];
			bool horizontalTouchEdge = rectItem.x + rectItem.width == binWidth;
			bool verticalTouchEdge = rectItem.y + rectItem.height == binHeight;
			if (horizontalTouchEdge && verticalTouchEdge)//right top corner
			{
				if (rectItem.width > rectItem.height)
				{
					Rect newRect;
					newRect.x = binWidth;
					newRect.y = rectItem.y;
					newRect.width = extraWidth;
					newRect.height = rectItem.height;
					freeRectangles.Add(newRect);

					newRect.x = rectItem.x;
					newRect.y = binHeight;
					newRect.width = rectItem.width + extraWidth;
					newRect.height = extraHeight;
					freeRectangles.Add(newRect);
				}
				else
				{
					Rect newRect;
					newRect.x = rectItem.x;
					newRect.y = binHeight;
					newRect.width = rectItem.width;
					newRect.height = extraHeight;
					freeRectangles.Add(newRect);

					newRect.x = binWidth;
					newRect.y = rectItem.y;
					newRect.width = extraWidth;
					newRect.height = rectItem.height + extraHeight;
					freeRectangles.Add(newRect);
				}
			}
			else//edge touch edge
			{
				if (horizontalTouchEdge)
				{
					Rect newRect;
					newRect.x = binWidth;
					newRect.y = rectItem.y;
					newRect.width = extraWidth;
					newRect.height = rectItem.height;
					freeRectangles.Add(newRect);
				}
				if (verticalTouchEdge)
				{
					Rect newRect;
					newRect.x = rectItem.x;
					newRect.y = binHeight;
					newRect.width = rectItem.width;
					newRect.height = extraHeight;
					freeRectangles.Add(newRect);
				}
			}
		}

		binWidth = newWidth;
		binHeight = newHeight;
	}
	void MaxRectsBinPack::PrepareExpendSizeForText(int newWidth, int newHeight, TArray<Rect>& outFreeRectangles, int cellSize, bool resetFreeAndUsedRects)
	{
		if (binWidth > newWidth || binHeight > newHeight)//new size is smaller
			return;

		if (resetFreeAndUsedRects)
		{
			freeRectangles.Reset();
			usedRectangles.Reset();
		}
#if 1
		for (int x = newWidth - cellSize; x >= binWidth; x -= cellSize)
		{
			for (int y = newHeight - cellSize; y >= 0; y -= cellSize)
			{
				Rect item;
				item.width = cellSize;
				item.height = cellSize;
				item.x = x;
				item.y = y;
				outFreeRectangles.Add(item);
			}
		}
		for (int x = binWidth - cellSize; x >= 0; x -= cellSize)
		{
			for (int y = newHeight - cellSize; y >= binHeight; y -= cellSize)
			{
				Rect item;
				item.width = cellSize;
				item.height = cellSize;
				item.x = x;
				item.y = y;
				outFreeRectangles.Add(item);
			}
		}
#else
		for (int x = binWidth; x < newWidth; x += cellSize)
		{
			for (int y = 0; y < newHeight; y += cellSize)
			{
				Rect item;
				item.width = cellSize;
				item.height = cellSize;
				item.x = x;
				item.y = y;
				outFreeRectangles.Add(item);
			}
		}
		for (int x = 0; x < binWidth; x += cellSize)
		{
			for (int y = binHeight; y < newHeight; y += cellSize)
			{
				Rect item;
				item.width = cellSize;
				item.height = cellSize;
				item.x = x;
				item.y = y;
				outFreeRectangles.Add(item);
			}
		}
#endif

		binWidth = newWidth;
		binHeight = newHeight;
	}
	void MaxRectsBinPack::DoExpendSizeForText(Rect rect)
	{
		usedRectangles.Reset();
		freeRectangles.Reset();
		freeRectangles.Add(rect);
	}
#define MAXINT32       2147483647
	Rect MaxRectsBinPack::Insert(int width, int height, FreeRectChoiceHeuristic method)
	{
		Rect newNode;
		// Unused in this function. We don't need to know the score after finding the position.
		int score1 = MAXINT32;
		int score2 = MAXINT32;
		switch (method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1); break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}

		if (newNode.height == 0)
			return newNode;

		int numRectanglesToProcess = freeRectangles.Num();
		for (int i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], newNode))
			{
				freeRectangles.RemoveAt(i);
				--i;
				--numRectanglesToProcess;
			}
		}

		PruneFreeList();

		usedRectangles.Add(newNode);
		return newNode;
	}

	void MaxRectsBinPack::Insert(TArray<RectSize> &rects, TArray<Rect> &dst, FreeRectChoiceHeuristic method)
	{
		dst.Empty();

		while (rects.Num() > 0)
		{
			int bestScore1 = MAXINT32;
			int bestScore2 = MAXINT32;
			int bestRectIndex = -1;
			Rect bestNode;

			for (int i = 0; i < rects.Num(); ++i)
			{
				int score1;
				int score2;
				Rect newNode = ScoreRect(rects[i].width, rects[i].height, method, score1, score2);

				if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
				{
					bestScore1 = score1;
					bestScore2 = score2;
					bestNode = newNode;
					bestRectIndex = i;
				}
			}

			if (bestRectIndex == -1)
				return;

			PlaceRect(bestNode);
			dst.Add(bestNode);
			rects.RemoveAt(bestRectIndex);
		}
	}

	void MaxRectsBinPack::PlaceRect(const Rect &node)
	{
		int numRectanglesToProcess = freeRectangles.Num();
		for (int i = 0; i < numRectanglesToProcess; ++i)
		{
			if (SplitFreeNode(freeRectangles[i], node))
			{
				freeRectangles.RemoveAt(i);
				--i;
				--numRectanglesToProcess;
			}
		}

		PruneFreeList();

		usedRectangles.Add(node);
		//		dst.Add(bestNode); ///\todo Refactor so that this compiles.
	}

	Rect MaxRectsBinPack::ScoreRect(int width, int height, FreeRectChoiceHeuristic method, int &score1, int &score2) const
	{
		Rect newNode;
		score1 = MAXINT32;
		score2 = MAXINT32;
		switch (method)
		{
		case RectBestShortSideFit: newNode = FindPositionForNewNodeBestShortSideFit(width, height, score1, score2); break;
		case RectBottomLeftRule: newNode = FindPositionForNewNodeBottomLeft(width, height, score1, score2); break;
		case RectContactPointRule: newNode = FindPositionForNewNodeContactPoint(width, height, score1);
			score1 = -score1; // Reverse since we are minimizing, but for contact point score bigger is better.
			break;
		case RectBestLongSideFit: newNode = FindPositionForNewNodeBestLongSideFit(width, height, score2, score1); break;
		case RectBestAreaFit: newNode = FindPositionForNewNodeBestAreaFit(width, height, score1, score2); break;
		}

		// Cannot fit the current rectangle.
		if (newNode.height == 0)
		{
			score1 = MAXINT32;
			score2 = MAXINT32;
		}

		return newNode;
	}

	/// Computes the ratio of used surface area.
	float MaxRectsBinPack::Occupancy() const
	{
		unsigned long usedSurfaceArea = 0;
		for (int i = 0; i < usedRectangles.Num(); ++i)
			usedSurfaceArea += usedRectangles[i].width * usedRectangles[i].height;

		return (float)usedSurfaceArea / (binWidth * binHeight);
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const
	{
		Rect bestNode;
		FMemory::Memset(&bestNode, 0, sizeof(Rect));

		bestY = MAXINT32;
		bestX = MAXINT32;

		for (int i = 0; i < freeRectangles.Num(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int topSideY = freeRectangles[i].y + height;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int topSideY = freeRectangles[i].y + width;
				if (topSideY < bestY || (topSideY == bestY && freeRectangles[i].x < bestX))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestY = topSideY;
					bestX = freeRectangles[i].x;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestShortSideFit(int width, int height,
		int &bestShortSideFit, int &bestLongSideFit) const
	{
		Rect bestNode;
		FMemory::Memset(&bestNode, 0, sizeof(Rect));

		bestShortSideFit = MAXINT32;
		bestLongSideFit = MAXINT32;

		for (int i = 0; i < freeRectangles.Num(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (shortSideFit < bestShortSideFit || (shortSideFit == bestShortSideFit && longSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int flippedLeftoverHoriz = abs(freeRectangles[i].width - height);
				int flippedLeftoverVert = abs(freeRectangles[i].height - width);
				int flippedShortSideFit = min(flippedLeftoverHoriz, flippedLeftoverVert);
				int flippedLongSideFit = max(flippedLeftoverHoriz, flippedLeftoverVert);

				if (flippedShortSideFit < bestShortSideFit || (flippedShortSideFit == bestShortSideFit && flippedLongSideFit < bestLongSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = flippedShortSideFit;
					bestLongSideFit = flippedLongSideFit;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestLongSideFit(int width, int height,
		int &bestShortSideFit, int &bestLongSideFit) const
	{
		Rect bestNode;
		FMemory::Memset(&bestNode, 0, sizeof(Rect));

		bestShortSideFit = MAXINT32;
		bestLongSideFit = MAXINT32;

		for (int i = 0; i < freeRectangles.Num(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}

			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - height);
				int leftoverVert = abs(freeRectangles[i].height - width);
				int shortSideFit = min(leftoverHoriz, leftoverVert);
				int longSideFit = max(leftoverHoriz, leftoverVert);

				if (longSideFit < bestLongSideFit || (longSideFit == bestLongSideFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestLongSideFit = longSideFit;
				}
			}
		}
		return bestNode;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeBestAreaFit(int width, int height,
		int &bestAreaFit, int &bestShortSideFit) const
	{
		Rect bestNode;
		FMemory::Memset(&bestNode, 0, sizeof(Rect));

		bestAreaFit = MAXINT32;
		bestShortSideFit = MAXINT32;

		for (int i = 0; i < freeRectangles.Num(); ++i)
		{
			int areaFit = freeRectangles[i].width * freeRectangles[i].height - width * height;

			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - width);
				int leftoverVert = abs(freeRectangles[i].height - height);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}
			
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int leftoverHoriz = abs(freeRectangles[i].width - height);
				int leftoverVert = abs(freeRectangles[i].height - width);
				int shortSideFit = min(leftoverHoriz, leftoverVert);

				if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestShortSideFit = shortSideFit;
					bestAreaFit = areaFit;
				}
			}
		}
		return bestNode;
	}

	/// Returns 0 if the two intervals i1 and i2 are disjoint, or the length of their overlap otherwise.
	int CommonIntervalLength(int i1start, int i1end, int i2start, int i2end)
	{
		if (i1end < i2start || i2end < i1start)
			return 0;
		return min(i1end, i2end) - max(i1start, i2start);
	}

	int MaxRectsBinPack::ContactPointScoreNode(int x, int y, int width, int height) const
	{
		int score = 0;

		if (x == 0 || x + width == binWidth)
			score += height;
		if (y == 0 || y + height == binHeight)
			score += width;

		for (int i = 0; i < usedRectangles.Num(); ++i)
		{
			if (usedRectangles[i].x == x + width || usedRectangles[i].x + usedRectangles[i].width == x)
				score += CommonIntervalLength(usedRectangles[i].y, usedRectangles[i].y + usedRectangles[i].height, y, y + height);
			if (usedRectangles[i].y == y + height || usedRectangles[i].y + usedRectangles[i].height == y)
				score += CommonIntervalLength(usedRectangles[i].x, usedRectangles[i].x + usedRectangles[i].width, x, x + width);
		}
		return score;
	}

	Rect MaxRectsBinPack::FindPositionForNewNodeContactPoint(int width, int height, int &bestContactScore) const
	{
		Rect bestNode;
		FMemory::Memset(&bestNode, 0, sizeof(Rect));

		bestContactScore = -1;

		for (int i = 0; i < freeRectangles.Num(); ++i)
		{
			// Try to place the rectangle in upright (non-flipped) orientation.
			if (freeRectangles[i].width >= width && freeRectangles[i].height >= height)
			{
				int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, width, height);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = width;
					bestNode.height = height;
					bestContactScore = score;
				}
			}
			if (binAllowFlip && freeRectangles[i].width >= height && freeRectangles[i].height >= width)
			{
				int score = ContactPointScoreNode(freeRectangles[i].x, freeRectangles[i].y, height, width);
				if (score > bestContactScore)
				{
					bestNode.x = freeRectangles[i].x;
					bestNode.y = freeRectangles[i].y;
					bestNode.width = height;
					bestNode.height = width;
					bestContactScore = score;
				}
			}
		}
		return bestNode;
	}

	bool MaxRectsBinPack::SplitFreeNode(Rect freeNode, const Rect &usedNode)
	{
		// Test with SAT if the rectangles even intersect.
		if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x ||
			usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
			return false;

		if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
		{
			// New node at the top side of the used node.
			if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
			{
				Rect newNode = freeNode;
				newNode.height = usedNode.y - newNode.y;
				freeRectangles.Add(newNode);
			}

			// New node at the bottom side of the used node.
			if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
			{
				Rect newNode = freeNode;
				newNode.y = usedNode.y + usedNode.height;
				newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
				freeRectangles.Add(newNode);
			}
		}

		if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
		{
			// New node at the left side of the used node.
			if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
			{
				Rect newNode = freeNode;
				newNode.width = usedNode.x - newNode.x;
				freeRectangles.Add(newNode);
			}

			// New node at the right side of the used node.
			if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
			{
				Rect newNode = freeNode;
				newNode.x = usedNode.x + usedNode.width;
				newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
				freeRectangles.Add(newNode);
			}
		}

		return true;
	}

	void MaxRectsBinPack::PruneFreeList()
	{
		/*
		///  Would be nice to do something like this, to avoid a Theta(n^2) loop through each pair.
		///  But unfortunately it doesn't quite cut it, since we also want to detect containment.
		///  Perhaps there's another way to do this faster than Theta(n^2).

		if (freeRectangles.size() > 0)
			clb::sort::QuickSort(&freeRectangles[0], freeRectangles.size(), NodeSortCmp);

		for(size_t i = 0; i < freeRectangles.size()-1; ++i)
			if (freeRectangles[i].x == freeRectangles[i+1].x &&
				freeRectangles[i].y == freeRectangles[i+1].y &&
				freeRectangles[i].width == freeRectangles[i+1].width &&
				freeRectangles[i].height == freeRectangles[i+1].height)
			{
				freeRectangles.erase(freeRectangles.begin() + i);
				--i;
			}
		*/

		/// Go through each pair and remove any rectangle that is redundant.
		for (int i = 0; i < freeRectangles.Num(); ++i)
			for (int j = i + 1; j < freeRectangles.Num(); ++j)
			{
				if (IsContainedIn(freeRectangles[i], freeRectangles[j]))
				{
					freeRectangles.RemoveAt(i);
					--i;
					break;
				}
				if (IsContainedIn(freeRectangles[j], freeRectangles[i]))
				{
					freeRectangles.RemoveAt(j);
					--j;
				}
			}
	}

}
