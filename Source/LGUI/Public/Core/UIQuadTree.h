// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

/** A datastruct for efficiently check rectangle overlapping */
namespace UIQuadTree
{
	struct Rectangle
	{
		Rectangle() {}
		Rectangle(FVector2D InMin, FVector2D InMax)
		{
			this->Min = InMin;
			this->Max = InMax;
		}
		FVector2D Min, Max;
		FVector2D GetCenter()const
		{
			return (Max - Min) * 0.5f + Min;
		}

		bool Contains(Rectangle InRect)
		{
			return this->Min.X <= InRect.Min.X
				&& this->Max.X >= InRect.Max.X
				&& this->Min.Y <= InRect.Min.Y
				&& this->Max.Y >= InRect.Max.Y
				;
		}
		bool Intersects(Rectangle InRect)
		{
			return !(this->Min.X >= InRect.Max.X
				|| this->Max.X <= InRect.Min.X
				|| this->Max.Y <= InRect.Min.Y
				|| this->Min.Y >= InRect.Max.Y
				);
		}
	};
	struct Node
	{
	private:
		//The rectangle covered by this node
		Rectangle NodeRect;
		FVector2D Center;
		//All rects present in this node. Several rects can overlap a single rect without ever overlapping each other.
		TArray<Rectangle> ValueArray;

		Node* TopLeft = nullptr;
		Node* TopRight = nullptr;
		Node* BottomLeft = nullptr;
		Node* BottomRight = nullptr;

		//insert a rect, and potentially create sub areas
		void InsertWithSplit(Rectangle InRect)
		{
			//the rect overlap on more than one sub area of this node, means it can't divide into any single child node
			if (
				(InRect.Min.X < Center.X && InRect.Max.X > Center.X)
				|| (InRect.Min.Y < Center.Y && InRect.Max.Y > Center.Y)
				)
			{
				this->ValueArray.Add(InRect);
			}
			//can insert into child node
			else
			{
				//create sub nodes
				if (TopLeft == nullptr)
				{
					BottomLeft = new Node(Rectangle(
						NodeRect.Min, Center
					));
					BottomRight = new Node(Rectangle(
						FVector2D(Center.X, NodeRect.Min.Y), FVector2D(NodeRect.Max.X, Center.Y)
					));
					TopLeft = new Node(Rectangle(
						FVector2D(NodeRect.Min.X, Center.Y), FVector2D(Center.X, NodeRect.Max.Y)
					));
					TopRight = new Node(Rectangle(
						Center, NodeRect.Max
					));
				}
				//try insert to sub node
				if (TopLeft->NodeRect.Contains(InRect))
				{
					TopLeft->Insert(InRect);
				}
				else if (TopRight->NodeRect.Contains(InRect))
				{
					TopRight->Insert(InRect);
				}
				else if (BottomLeft->NodeRect.Contains(InRect))
				{
					BottomLeft->Insert(InRect);
				}
				else if (BottomRight->NodeRect.Contains(InRect))
				{
					BottomRight->Insert(InRect);
				}
				else//not contains in all area, could be out side this rect, just put it to ValueArray
				{
					this->ValueArray.Add(InRect);
				}
			}
		}
	public:
		Node(Rectangle InRect)
		{
			this->NodeRect = InRect;
			this->Center = this->NodeRect.GetCenter();
		}
		~Node()
		{
			if (TopLeft != nullptr)
			{
				delete TopLeft;
				delete TopRight;
				delete BottomLeft;
				delete BottomRight;
				TopLeft = nullptr;
				TopRight = nullptr;
				BottomLeft = nullptr;
				BottomRight = nullptr;
			}
		}
	public:
		bool Overlap(Rectangle InRect)
		{
			//empty node
			if (this->ValueArray.Num() == 0 && TopLeft == nullptr)
			{
				return false;
			}
			if (this->NodeRect.Intersects(InRect))
			{
				//check ValueArray
				for (auto& ItemRect : ValueArray)
				{
					if (ItemRect.Intersects(InRect))
					{
						return true;
					}
				}
				//check sub node
				if (TopLeft != nullptr)
				{
					if (TopLeft->Overlap(InRect))
					{
						return true;
					}
					if (TopRight->Overlap(InRect))
					{
						return true;
					}
					if (BottomLeft->Overlap(InRect))
					{
						return true;
					}
					if (BottomRight->Overlap(InRect))
					{
						return true;
					}
				}
			}
			return false;
		}
		void Insert(Rectangle InRect)
		{
			//not contains sub node
			if (TopLeft == nullptr)
			{
				//empty rect, just add to ValueArray
				if (ValueArray.Num() == 0)
				{
					ValueArray.Add(InRect);
				}
				//only have one rect, because prev is empty, so we need to use InsertWithSplit to potentially split area
				else if (ValueArray.Num() == 1)
				{
					auto First = ValueArray[0];
					ValueArray.RemoveAt(0);
					InsertWithSplit(First);
					InsertWithSplit(InRect);
				}
				//sub area could still be null because all prev areas is added to ValueArray
				else
				{
					InsertWithSplit(InRect);
				}
			}
			//already contains sub node
			else
			{
				InsertWithSplit(InRect);
			}
		}
	};
};
