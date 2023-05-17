// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Utils/LGUIUtils.h"
#include "LGUITextData.h"

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
		bool hasColor = false;

		SupOrSubMode supOrSubMode = SupOrSubMode::None;

		CustomTagMode customTagMode = CustomTagMode::None;
		FName customTag;
		FName imageTag;

		int charIndex = 0;
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
		FName imageTag = NAME_None;

		int originSize;
		FColor originColor;
		uint8 originCanvasGroupAlpha;
		bool originBold;
		bool originItalic;

		bool enableBold, enableItalic, enableUnderline, enableStrikethrough, enableSize, enableColor, enableSuperscript, enableSubscript, enableCustomTag, enableImage;
	public:
		void ClearImageTag()
		{
			imageTag = NAME_None;
		}
		void Prepare(float inOriginSize, FColor inOriginColor, uint8 inCanvasGroupAlpha, bool inBold, bool inItalic, int32 inFlags, RichTextParseResult& result)
		{
			originSize = inOriginSize;
			originColor = inOriginColor;
			originCanvasGroupAlpha = inCanvasGroupAlpha;
			originBold = inBold;
			originItalic = inItalic;

			result.bold = inBold;
			result.italic = inItalic;
			result.size = inOriginSize;
			result.color = inOriginColor;

			enableBold = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Bold);
			enableItalic = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Italic);
			enableUnderline = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Underline);
			enableStrikethrough = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Strikethrough);
			enableSize = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Size);
			enableColor = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Color);
			enableSuperscript = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Superscript);
			enableSubscript = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Subscript);
			enableCustomTag = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::CustomTag);
			enableImage = inFlags & (1 << (int)EUIText_RichTextTagFilterFlags::Image);
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
			imageTag = NAME_None;
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
						if (enableBold)
						{
							startIndex += 3;
							boldCount++;
							haveSymbol = true;
						}
					}
					else if (text[charIndex + 1] == 'i')//begin italic
					{
						if (enableItalic)
						{
							startIndex += 3;
							italicCount++;
							haveSymbol = true;
						}
					}
					else if (text[charIndex + 1] == 'u')//begin underline
					{
						if (enableUnderline)
						{
							startIndex += 3;
							underlineCount++;
							haveSymbol = true;
						}
					}
					else if (text[charIndex + 1] == 's')//begin strikethough
					{
						if (enableStrikethrough)
						{
							startIndex += 3;
							strikethroughCount++;
							haveSymbol = true;
						}
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
					if (enableSize)
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
					if (enableColor)
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
				}
				else if (charIndex + 4 < textLength
					&& text[charIndex + 1] == 's'
					&& text[charIndex + 2] == 'u'
					&& text[charIndex + 3] == 'p'
					&& text[charIndex + 4] == '>'
					)//begin sup
				{
					if (enableSuperscript)
					{
						startIndex += 5;
						supOrSubArray.Add(SupOrSubMode::Sup);
						haveSymbol = true;
					}
				}
				else if (charIndex + 4 < textLength
					&& text[charIndex + 1] == 's'
					&& text[charIndex + 2] == 'u'
					&& text[charIndex + 3] == 'b'
					&& text[charIndex + 4] == '>'
					)//begin sub
				{
					if (enableSubscript)
					{
						startIndex += 5;
						supOrSubArray.Add(SupOrSubMode::Sub);
						haveSymbol = true;
					}
				}
				else if (charIndex + 6 < textLength
					&& text[charIndex + 1] == 'i'
					&& text[charIndex + 2] == 'm'
					&& text[charIndex + 3] == 'g'
					&& text[charIndex + 4] == '='
					)//begin image=
				{
					if (enableImage)
					{
						int charEndIndex;
						if (GetImageTag(text, textLength, charIndex + 5, charEndIndex, imageTag))
						{
							startIndex += charEndIndex - charIndex + 1;
							haveSymbol = true;
						}
					}
				}
				else if (charIndex + 1 < textLength && text[charIndex + 1] == '/')//end
				{
					if (charIndex + 3 < textLength && text[charIndex + 3] == '>')
					{
						if (text[charIndex + 2] == 'b' && boldCount > 0)//end bold
						{
							if (enableBold)
							{
								startIndex += 4;
								boldCount--;
								haveSymbol = true;
							}
						}
						else if (text[charIndex + 2] == 'i' && italicCount > 0)//end italic
						{
							if (enableItalic)
							{
								startIndex += 4;
								italicCount--;
								haveSymbol = true;
							}
						}
						else if (text[charIndex + 2] == 'u' && underlineCount > 0)//end underline
						{
							if (enableUnderline)
							{
								startIndex += 4;
								underlineCount--;
								haveSymbol = true;
							}
						}
						else if (text[charIndex + 2] == 's' && strikethroughCount > 0)//end strikethough
						{
							if (enableStrikethrough)
							{
								startIndex += 4;
								strikethroughCount--;
								haveSymbol = true;
							}
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
						if (enableSize)
						{
							startIndex += 7;
							sizeArray.RemoveAt(sizeArray.Num() - 1);
							haveSymbol = true;
						}
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
						if (enableColor)
						{
							startIndex += 8;
							colorArray.RemoveAt(colorArray.Num() - 1);
							haveSymbol = true;
						}
					}
					else if (charIndex + 5 < textLength
						&& text[charIndex + 2] == 's'
						&& text[charIndex + 3] == 'u'
						&& text[charIndex + 4] == 'p'
						&& text[charIndex + 5] == '>'
						&& supOrSubArray.Num() > 0
						)//end sup
					{
						if (enableSuperscript)
						{
							startIndex += 6;
							supOrSubArray.RemoveAt(supOrSubArray.Num() - 1);
							haveSymbol = true;
						}
					}
					else if (charIndex + 5 < textLength
						&& text[charIndex + 2] == 's'
						&& text[charIndex + 3] == 'u'
						&& text[charIndex + 4] == 'b'
						&& text[charIndex + 5] == '>'
						&& supOrSubArray.Num() > 0
						)//end sub
					{
						if (enableSubscript)
						{
							startIndex += 6;
							supOrSubArray.RemoveAt(supOrSubArray.Num() - 1);
							haveSymbol = true;
						}
					}
					else if (customTagArray.Num() > 0
						)//end custom tag
					{
						if (enableCustomTag)
						{
							int charEndIndex;
							FName tag;
							if (GetCustomTag(text, textLength, charIndex + 2, charEndIndex, tag))
							{
								auto foundIndex = customTagArray.IndexOfByKey(tag);
								if (foundIndex != -1)
								{
									customTagArray.RemoveAt(foundIndex);
									startIndex += charEndIndex - charIndex + 1;
									parseResult.customTag = tag;
									parseResult.customTagMode = CustomTagMode::End;
									haveSymbol = true;
								}
							}
						}
					}
				}
				else if(charIndex + 1 < textLength
					)//check custom tag
				{
					if (enableCustomTag)
					{
						int charEndIndex;
						FName tag;
						if (GetCustomTag(text, textLength, charIndex + 1, charEndIndex, tag))
						{
							auto foundIndex = customTagArray.IndexOfByKey(tag);
							if (foundIndex == -1)
							{
								startIndex += charEndIndex - charIndex + 1;
								customTagArray.Add(tag);
								parseResult.customTag = tag;
								parseResult.customTagMode = CustomTagMode::Start;
								haveSymbol = true;
							}
						}
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
				parseResult.size = FMath::Max(parseResult.size, 0.0f);
				parseResult.hasColor = colorArray.Num() > 0;
				parseResult.color = parseResult.hasColor ? colorArray[colorArray.Num() - 1] : originColor;
				parseResult.supOrSubMode = supOrSubArray.Num() > 0 ? supOrSubArray[supOrSubArray.Num() - 1] : SupOrSubMode::None;
				if (parseResult.supOrSubMode != SupOrSubMode::None)
				{
					parseResult.size *= 0.8f;//sup or sub size
				}
				parseResult.imageTag = imageTag;
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
				else if (text[i] == '<'//reach another tag
					|| text[i] == ' '//reach space
					|| text[i] == '\n'//reach new line
					|| text[i] == '\t'//reach new line
					)
				{
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
				else if (text[i] == '<'//reach another tag
					|| text[i] == ' '//reach space
					|| text[i] == '\n'//reach new line
					|| text[i] == '\t'//reach new line
					)
				{
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
		static bool GetImageTag(const FString& text, int textLength, int startIndex, int& outEndIndex, FName& outTag)
		{
			int endIndex = -1;
			for (int i = startIndex + 1; i < textLength; i++)
			{
				if (text[i] == '>')
				{
					if (text[i - 1] == '/')//image is self-close tag, must end with '/>'
					{
						endIndex = i;
						break;
					}
					else
					{
						break;
					}
				}
				else if (text[i] == '<'//reach another tag
					|| text[i] == ' '//reach space
					|| text[i] == '\n'//reach new line
					|| text[i] == '\t'//reach new line
					)
				{
					break;
				}
			}
			if (endIndex != -1 && endIndex > startIndex)//found end
			{
				outTag = FName(*text.Mid(startIndex, endIndex - startIndex - 1));
				outEndIndex = endIndex;
				return true;
			}
			return false;
		}
		//get color from 'color=red' or 'color=#ffffff', end with '>'
		//return true if is valid
		bool GetColor(const FString& text, int textLength, int startIndex, int& outEndIndex, FColor& outColor)
		{
			int endIndex = -1;
			for (int i = startIndex; i < textLength; i++)
			{
				if (text[i] == '>')
				{
					endIndex = i;
					break;
				}
				else if (text[i] == '<'//reach another tag
					|| text[i] == ' '//reach space
					|| text[i] == '\n'//reach new line
					|| text[i] == '\t'//reach new line
					)
				{
					break;
				}
			}
			if (endIndex != -1 && endIndex > startIndex)//found end
			{
				outEndIndex = endIndex;
				FString colorString = text.Mid(startIndex, endIndex - startIndex);
				if (colorString == TEXT("black"))
				{
					outColor = FColor::Black;
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("white"))
				{
					outColor = FColor::White;
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("gray"))
				{
					outColor = FColor(128, 128, 128);
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("silver"))
				{
					outColor = FColor(192, 192, 192);
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("red"))
				{
					outColor = FColor::Red;
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("green"))
				{
					outColor = FColor::Green;
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("blue"))
				{
					outColor = FColor::Blue;
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("orange"))
				{
					outColor = FColor(255, 165, 0);
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("purple"))
				{
					outColor = FColor(128, 0, 128);
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString == TEXT("yellow"))
				{
					outColor = FColor(255, 255, 0);
					outColor.A = originCanvasGroupAlpha;
					return true;
				}
				else if (colorString[0] == '#')
				{
					const static TArray<TCHAR> hexTable = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
					if (colorString.Len() == 7 || colorString.Len() == 9)//#ffffff/#ffffff00
					{
						outColor.A = originCanvasGroupAlpha;
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
								case 7:
								{
									outColor.A = (uint8)(LGUIUtils::Color255To1_Table[originCanvasGroupAlpha] * value);
								}
								break;
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