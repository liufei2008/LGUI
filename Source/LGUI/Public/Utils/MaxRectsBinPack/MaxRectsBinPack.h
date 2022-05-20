// Copyright 2019-2022 LexLiu. All Rights Reserved.

/** @file MaxRectsBinPack.h
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

#include "CoreMinimal.h"
#pragma once


namespace rbp {

	struct RectSize
	{
		int width;
		int height;
	};

	struct Rect
	{
		Rect() {}
		int x;
		int y;
		int width;
		int height;
	};



	/** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
		use this structure. */
	class LGUI_API MaxRectsBinPack
	{
	public:
		/// Instantiates a bin of size (0,0). Call Init to create a new bin.
		MaxRectsBinPack();

		/// Instantiates a bin of the given size.
		MaxRectsBinPack(int width, int height, bool allowFlip = false);

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void Init(int width, int height, bool allowFlip = false);

		/// add by lexliu to expend the binpack size
		void ExpendSize(int newWidth, int newHeight);
		/// add by lexliu to expend the binpack size, for uitext, prevent too many small rects
		void PrepareExpendSizeForText(int newWidth, int newHeight, TArray<Rect>& outFreeRectangles, int cellSize, bool resetFreeAndUsedRects = true);
		void DoExpendSizeForText(Rect rect);

		/// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
		enum FreeRectChoiceHeuristic
		{
			RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
			RectBestLongSideFit, ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
			RectBestAreaFit, ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
			RectBottomLeftRule, ///< -BL: Does the Tetris placement.
			RectContactPointRule ///< -CP: Choosest the placement where the rectangle touches other rects as much as possible.
		};

		/// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
		/// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
		/// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
		/// @param method The rectangle placement rule to use when packing.
		void Insert(TArray<RectSize> &rects, TArray<Rect> &dst, FreeRectChoiceHeuristic method);

		/// Inserts a single rectangle into the bin, possibly rotated.
		Rect Insert(int width, int height, FreeRectChoiceHeuristic method);

		/// Computes the ratio of used surface area to the total bin area.
		float Occupancy() const;

		int GetBinWidth()const { return binWidth; }
		int GetBinHeight()const { return binHeight; }
	private:
		int binWidth = 0;
		int binHeight = 0;

		bool binAllowFlip = false;

		TArray<Rect> usedRectangles;
		TArray<Rect> freeRectangles;

		/// Computes the placement score for placing the given rectangle with the given method.
		/// @param score1 [out] The primary placement score will be outputted here.
		/// @param score2 [out] The secondary placement score will be outputted here. This isu sed to break ties.
		/// @return This struct identifies where the rectangle would be placed if it were placed.
		Rect ScoreRect(int width, int height, FreeRectChoiceHeuristic method, int &score1, int &score2) const;

		/// Places the given rectangle into the bin.
		void PlaceRect(const Rect &node);

		/// Computes the placement score for the -CP variant.
		int ContactPointScoreNode(int x, int y, int width, int height) const;

		Rect FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const;
		Rect FindPositionForNewNodeBestShortSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
		Rect FindPositionForNewNodeBestLongSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
		Rect FindPositionForNewNodeBestAreaFit(int width, int height, int &bestAreaFit, int &bestShortSideFit) const;
		Rect FindPositionForNewNodeContactPoint(int width, int height, int &contactScore) const;

		/// @return True if the free node was split.
		bool SplitFreeNode(Rect freeNode, const Rect &usedNode);

		/// Goes through the free rectangle list and removes any redundant entries.
		void PruneFreeList();

		/// Returns true if a is contained in b.
		static bool IsContainedIn(const Rect &a, const Rect &b)
		{
			return a.x >= b.x && a.y >= b.y
				&& a.x + a.width <= b.x + b.width
				&& a.y + a.height <= b.y + b.height;
		}
	};

}