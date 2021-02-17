﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

//a set of helpers to parse rich text
namespace LGUIRichTextParser
{
	enum class SupOrSubMode
	{
		None, Sup, Sub,
	};
	enum class CustomTagMode
	{
		None, Start, End,
	};
	struct RichTextParseResult
	{
		bool bold = false;
		bool italic = false;
		bool underline = false;
		bool strikethrough = false;

		float size = 0;

		FColor color = FColor::Black;

		SupOrSubMode supOrSubMode = SupOrSubMode::None;

		CustomTagMode customTagMode = CustomTagMode::None;
		FName customTag;
	};
	
	struct RichTextParser
	{
	private:
		int						boldCount = 0;
		int						italicCount = 0;
		int						underlineCount = 0;
		int						strikethroughCount = 0;
		TArray<float>			sizeArray;
		TArray<FColor>			colorArray;
		TArray<SupOrSubMode>	supOrSubArray;
		TArray<FName>			customTagArray;

		int originSize;
		FColor originColor;
		bool originBold;
		bool originItalic;
	public:
		void Prepare(float inOriginSize, FColor inOriginColor, bool inBold, bool inItalic, RichTextParseResult& result)
		{
			originSize = inOriginSize;
			originColor = inOriginColor;
			originBold = inBold;
			originItalic = inItalic;

			result.bold = inBold;
			result.italic = inItalic;
			result.size = inOriginSize;
			result.color = inOriginColor;
		}
		void Clear()
		{
			boldCount = 0;
			italicCount = 0;
			underlineCount = 0;
			strikethroughCount = 0;
			sizeArray.Reset();
			colorArray.Reset();
			supOrSubArray.Reset();
			customTagArray.Reset();
		}
		bool Parse(const FString& text, const int& textLength, int& startIndex, RichTextParseResult& parseResult)
		{
			bool haveSymbol = false;
			int charIndex = startIndex;
			if (text[charIndex] == '<')
			{
				if (charIndex + 2 < textLength && text[charIndex + 2] == '>')
				{
					if (text[charIndex + 1] == 'b')//begin bold
					{
						startIndex += 3;
						boldCount++;
						haveSymbol = true;
					}
					else if (text[charIndex + 1] == 'i')//begin italic
					{
						startIndex += 3;
						italicCount++;
						haveSymbol = true;
					}
					else if (text[charIndex + 1] == 'u')//begin underline
					{
						startIndex += 3;
						underlineCount++;
						haveSymbol = true;
					}
					else if (text[charIndex + 1] == 's')//begin strikethough
					{
						startIndex += 3;
						strikethroughCount++;
						haveSymbol = true;
					}
				}
				else if (charIndex + 5 < textLength
					&& text[charIndex + 1] == 's'
					&& text[charIndex + 2] == 'i'
					&& text[charIndex + 3] == 'z'
					&& text[charIndex + 4] == 'e'
					&& text[charIndex + 5] == '='
					)//being size=
				{
					int charEndIndex;
					float parsedSize;
					bool absoluteOrAdditional;
					if (GetSize(text, textLength, charIndex + 6, charEndIndex, parsedSize, absoluteOrAdditional))
					{
						startIndex += charEndIndex - charIndex + 1;
						if (absoluteOrAdditional)
						{
							sizeArray.Add(parsedSize);
						}
						else
						{
							sizeArray.Add(originSize + parsedSize);
						}
						haveSymbol = true;
					}
				}
				else if (charIndex + 6 < textLength
					&& text[charIndex + 1] == 'c'
					&& text[charIndex + 2] == 'o'
					&& text[charIndex + 3] == 'l'
					&& text[charIndex + 4] == 'o'
					&& text[charIndex + 5] == 'r'
					&& text[charIndex + 6] == '='
					)//begin color=
				{
					int charEndIndex;
					FColor parsedColor;
					if (GetColor(text, textLength, charIndex + 7, charEndIndex, parsedColor))
					{
						startIndex += charEndIndex - charIndex + 1;
						colorArray.Add(parsedColor);
						haveSymbol = true;
					}
				}
				else if (charIndex + 7 < textLength
					&& text[charIndex + 1] == 'c'
					&& text[charIndex + 2] == 'u'
					&& text[charIndex + 3] == 's'
					&& text[charIndex + 4] == 't'
					&& text[charIndex + 5] == 'o'
					&& text[charIndex + 6] == 'm'
					&& text[charIndex + 7] == '='
					)//begin custom=
				{
					int charEndIndex;
					FName tag;
					if (GetCustomTag(text, textLength, charIndex + 8, charEndIndex, tag))
					{
						startIndex += charEndIndex - charIndex + 1;
						customTagArray.Add(tag);
						haveSymbol = true;
						parseResult.customTag = tag;
						parseResult.customTagMode = CustomTagMode::Start;
					}
				}
				else if (charIndex + 4 < textLength
					&& text[charIndex + 1] == 's'
					&& text[charIndex + 2] == 'u'
					&& text[charIndex + 3] == 'p'
					&& text[charIndex + 4] == '>'
					)//begin sup
				{
					startIndex += 5;
					supOrSubArray.Add(SupOrSubMode::Sup);
					haveSymbol = true;
				}
				else if (charIndex + 4 < textLength
					&& text[charIndex + 1] == 's'
					&& text[charIndex + 2] == 'u'
					&& text[charIndex + 3] == 'b'
					&& text[charIndex + 4] == '>'
					)//begin sub
				{
					startIndex += 5;
					supOrSubArray.Add(SupOrSubMode::Sub);
					haveSymbol = true;
				}
				//end
				else if (charIndex + 1 < textLength && text[charIndex + 1] == '/')
				{
					if (charIndex + 3 < textLength && text[charIndex + 3] == '>')
					{
						if (text[charIndex + 2] == 'b' && boldCount > 0)//end bold
						{
							startIndex += 4;
							boldCount--;
							haveSymbol = true;
						}
						else if (text[charIndex + 2] == 'i' && italicCount > 0)//end italic
						{
							startIndex += 4;
							italicCount--;
							haveSymbol = true;
						}
						else if (text[charIndex + 2] == 'u' && underlineCount > 0)//end underline
						{
							startIndex += 4;
							underlineCount--;
							haveSymbol = true;
						}
						else if (text[charIndex + 2] == 's' && strikethroughCount > 0)//end strikethough
						{
							startIndex += 4;
							strikethroughCount--;
							haveSymbol = true;
						}
					}
					else if (charIndex + 6 < textLength
						&& text[charIndex + 2] == 's'
						&& text[charIndex + 3] == 'i'
						&& text[charIndex + 4] == 'z'
						&& text[charIndex + 5] == 'e'
						&& text[charIndex + 6] == '>'
						&& sizeArray.Num() > 0
						)//end size
					{
						startIndex += 7;
						sizeArray.RemoveAt(sizeArray.Num() - 1);
						haveSymbol = true;
					}
					else if (charIndex + 7 < textLength
						&& text[charIndex + 2] == 'c'
						&& text[charIndex + 3] == 'o'
						&& text[charIndex + 4] == 'l'
						&& text[charIndex + 5] == 'o'
						&& text[charIndex + 6] == 'r'
						&& text[charIndex + 7] == '>'
						&& colorArray.Num() > 0
						)//end color
					{
						startIndex += 8;
						colorArray.RemoveAt(colorArray.Num() - 1);
						haveSymbol = true;
					}
					else if (charIndex + 8 < textLength
						&& text[charIndex + 2] == 'c'
						&& text[charIndex + 3] == 'u'
						&& text[charIndex + 4] == 's'
						&& text[charIndex + 5] == 't'
						&& text[charIndex + 6] == 'o'
						&& text[charIndex + 7] == 'm'
						&& text[charIndex + 8] == '>'
						&& customTagArray.Num() > 0
						)//end custom tag
					{
						startIndex += 9;
						auto tag = customTagArray.Last();
						customTagArray.RemoveAt(customTagArray.Num() - 1);
						parseResult.customTag = tag;
						parseResult.customTagMode = CustomTagMode::End;
						haveSymbol = true;
					}
					else if (charIndex + 5 < textLength
						&& text[charIndex + 2] == 's'
						&& text[charIndex + 3] == 'u'
						&& text[charIndex + 4] == 'p'
						&& text[charIndex + 5] == '>'
						&& supOrSubArray.Num() > 0
						)//end sup
					{
						startIndex += 6;
						supOrSubArray.RemoveAt(supOrSubArray.Num() - 1);
						haveSymbol = true;
					}
					else if (charIndex + 5 < textLength
						&& text[charIndex + 2] == 's'
						&& text[charIndex + 3] == 'u'
						&& text[charIndex + 4] == 'b'
						&& text[charIndex + 5] == '>'
						&& supOrSubArray.Num() > 0
						)//end sub
					{
						startIndex += 6;
						supOrSubArray.RemoveAt(supOrSubArray.Num() - 1);
						haveSymbol = true;
					}
				}
			}
			if (haveSymbol)
			{
				parseResult.bold = boldCount > 0 || originBold;
				parseResult.italic = italicCount > 0 || originItalic;
				parseResult.underline = underlineCount > 0;
				parseResult.strikethrough = strikethroughCount > 0;
				parseResult.size = sizeArray.Num() > 0 ? sizeArray[sizeArray.Num() - 1] : originSize;
				parseResult.size = FMath::Clamp(parseResult.size, 2.0f, 200.0f);
				parseResult.color = colorArray.Num() > 0 ? colorArray[colorArray.Num() - 1] : originColor;
				parseResult.supOrSubMode = supOrSubArray.Num() > 0 ? supOrSubArray[supOrSubArray.Num() - 1] : SupOrSubMode::None;
				if (parseResult.supOrSubMode != SupOrSubMode::None)
				{
					parseResult.size *= 0.8f;//sup or sub size
				}
			}
			return haveSymbol;
		}
	private:
		//get size from 'size=' or 'size=+' or 'size=-', end with '>'
		//return true if is valid
		static bool GetSize(const FString& text, int textLength, int startIndex, int& outEndIndex, float& outSize, bool& outAbsoluteOrAdditional)
		{
			int endIndex = -1;
			for (int i = startIndex; i < textLength; i++)
			{
				if (text[i] == '>')
				{
					endIndex = i;
					break;
				}
			}
			if (endIndex != -1 && endIndex > startIndex)//found end
			{
				FString sizeStr = text.Mid(startIndex, endIndex - startIndex);
				outAbsoluteOrAdditional = sizeStr[0] != '+' && sizeStr[0] != '-';
				if (sizeStr.IsNumeric())
				{
					outSize = FCString::Atof(*sizeStr);
					outEndIndex = endIndex;
					return true;
				}
			}
			return false;
		}
		static bool GetCustomTag(const FString& text, int textLength, int startIndex, int& outEndIndex, FName& outTag)
		{
			int endIndex = -1;
			for (int i = startIndex; i < textLength; i++)
			{
				if (text[i] == '>')
				{
					endIndex = i;
					break;
				}
			}
			if (endIndex != -1 && endIndex > startIndex)//found end
			{
				outTag = FName(*text.Mid(startIndex, endIndex - startIndex));
				outEndIndex = endIndex;
				return true;
			}
			return false;
		}
		//get color from 'color=red' or 'color=#ffffff', end with '>'
		//return true if is valid
		static bool GetColor(const FString& text, int textLength, int startIndex, int& outEndIndex, FColor& outColor)
		{
			int endIndex = -1;
			for (int i = startIndex; i < textLength; i++)
			{
				if (text[i] == '>')
				{
					endIndex = i;
					break;
				}
			}
			if (endIndex != -1 && endIndex > startIndex)//found end
			{
				outEndIndex = endIndex;
				FString colorString = text.Mid(startIndex, endIndex - startIndex);
				if (colorString == TEXT("black"))
				{
					outColor = FColor::Black; return true;
				}
				else if (colorString == TEXT("blue"))
				{
					outColor = FColor::Blue; return true;
				}
				else if (colorString == TEXT("green"))
				{
					outColor = FColor::Green; return true;
				}
				else if (colorString == TEXT("orange"))
				{
					outColor = FColor::Orange; return true;
				}
				else if (colorString == TEXT("purple"))
				{
					outColor = FColor::Purple; return true;
				}
				else if (colorString == TEXT("red"))
				{
					outColor = FColor::Red; return true;
				}
				else if (colorString == TEXT("white"))
				{
					outColor = FColor::White; return true;
				}
				else if (colorString == TEXT("yellow"))
				{
					outColor = FColor::Yellow; return true;
				}
				else if (colorString[0] == '#')
				{
					const static TArray<TCHAR> hexTable = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
					if (colorString.Len() == 7 || colorString.Len() == 9)//#ffffff/#ffffff00
					{
						outColor.A = 255;
						int colorStringLength = colorString.Len();
						for (int i = 1; i < colorStringLength; i += 2)
						{
							int firstIndex = hexTable.IndexOfByKey(colorString[i]);
							int secondIndex = hexTable.IndexOfByKey(colorString[i + 1]);
							if (firstIndex != -1 && secondIndex != -1)//valid
							{
								uint8 value = (uint8)(firstIndex * 16 + secondIndex);
								switch (i)
								{
								case 1:outColor.R = value; break;
								case 3:outColor.G = value; break;
								case 5:outColor.B = value; break;
								case 7:outColor.A = value; break;
								}
							}
							else
							{
								return false;
							}
						}
						return true;
					}
				}
			}
			return false;
		}
	};
}