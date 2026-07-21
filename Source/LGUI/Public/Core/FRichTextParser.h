// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Containers/StringView.h"
#include "Utils/LexUIUtils.h"
#include "LexUITextData.h"

//a set of helpers to parse rich text
namespace LexUIRichTextParser
{
	enum class ESupOrSubMode
	{
		None, Sup, Sub,
	};
	enum class ECustomTagMode
	{
		None, Start, End,
	};
	struct FRichTextParseResult
	{
		bool Bold = false;
		bool Italic = false;
		bool Underline = false;
		bool Strikethrough = false;

		float Size = 0;

		FColor Color = FColor::Black;
		bool HasColor = false;

		ESupOrSubMode SupOrSubMode = ESupOrSubMode::None;

		ECustomTagMode CustomTagMode = ECustomTagMode::None;
		FName CustomTag;
		FName ImageTag;

		int CharIndex = 0;
	};
	
	struct FRichTextParser
	{
	private:
		int						BoldCount = 0;
		int						ItalicCount = 0;
		int						UnderlineCount = 0;
		int						StrikethroughCount = 0;
		TArray<float>			SizeArray;
		TArray<FColor>			ColorArray;
		TArray<ESupOrSubMode>	SupOrSubArray;
		TArray<FName>			CustomTagArray;
		FName ImageTag = NAME_None;

		int OriginSize = 0;
		FColor OriginColor = FColor::White;
		uint8 OriginRenderOpacity = 255;
		bool OriginBold = false;
		bool OriginItalic = false;

		bool
		bEnableBold = false
		, bEnableItalic = false
		, bEnableUnderline = false
		, bEnableStrikethrough = false
		, bEnableSize = false
		, bEnableColor = false
		, bEnableSuperscript = false
		, bEnableSubscript = false
		, bEnableCustomTag = false
		, bEnableImage = false
		;
	public:
		void ClearImageTag()
		{
			ImageTag = NAME_None;
		}
		void Prepare(float inOriginSize, FColor inOriginColor, uint8 inRenderOpacity, bool inBold, bool inItalic, int32 inFlags, FRichTextParseResult& result)
		{
			OriginSize = inOriginSize;
			OriginColor = inOriginColor;
			OriginRenderOpacity = inRenderOpacity;
			OriginBold = inBold;
			OriginItalic = inItalic;

			result.Bold = inBold;
			result.Italic = inItalic;
			result.Size = inOriginSize;
			result.Color = inOriginColor;

			bEnableBold = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Bold);
			bEnableItalic = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Italic);
			bEnableUnderline = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Underline);
			bEnableStrikethrough = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Strikethrough);
			bEnableSize = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Size);
			bEnableColor = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Color);
			bEnableSuperscript = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Superscript);
			bEnableSubscript = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Subscript);
			bEnableCustomTag = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::CustomTag);
			bEnableImage = inFlags & (1 << (int)ELexUIText_RichTextTagFilterFlags::Image);
		}
		void Clear()
		{
			BoldCount = 0;
			ItalicCount = 0;
			UnderlineCount = 0;
			StrikethroughCount = 0;
			SizeArray.Reset();
			ColorArray.Reset();
			SupOrSubArray.Reset();
			CustomTagArray.Reset();
			ImageTag = NAME_None;
		}
		bool Parse(const FString& Text, int TextLength, int& InOutStartIndex, FRichTextParseResult& ParseResult)
		{
			bool bHaveSymbol = false;
			int CharIndex = InOutStartIndex;
			if (Text[CharIndex] == '<')
			{
				if (CharIndex + 2 < TextLength && Text[CharIndex + 2] == '>')
				{
					if (Text[CharIndex + 1] == 'b')//begin bold
					{
						if (bEnableBold)
						{
							InOutStartIndex += 3;
							BoldCount++;
							bHaveSymbol = true;
						}
					}
					else if (Text[CharIndex + 1] == 'i')//begin italic
					{
						if (bEnableItalic)
						{
							InOutStartIndex += 3;
							ItalicCount++;
							bHaveSymbol = true;
						}
					}
					else if (Text[CharIndex + 1] == 'u')//begin underline
					{
						if (bEnableUnderline)
						{
							InOutStartIndex += 3;
							UnderlineCount++;
							bHaveSymbol = true;
						}
					}
					else if (Text[CharIndex + 1] == 's')//begin strikethough
					{
						if (bEnableStrikethrough)
						{
							InOutStartIndex += 3;
							StrikethroughCount++;
							bHaveSymbol = true;
						}
					}
				}
				else if (CharIndex + 5 < TextLength
					&& Text[CharIndex + 1] == 's'
					&& Text[CharIndex + 2] == 'i'
					&& Text[CharIndex + 3] == 'z'
					&& Text[CharIndex + 4] == 'e'
					&& Text[CharIndex + 5] == '='
					)//being size=
				{
					if (bEnableSize)
					{
						int charEndIndex;
						float parsedSize;
						bool absoluteOrAdditional;
						if (GetSize(Text, TextLength, CharIndex + 6, charEndIndex, parsedSize, absoluteOrAdditional))
						{
							InOutStartIndex += charEndIndex - CharIndex + 1;
							if (absoluteOrAdditional)
							{
								SizeArray.Add(parsedSize);
							}
							else
							{
								SizeArray.Add(OriginSize + parsedSize);
							}
							bHaveSymbol = true;
						}
					}
				}
				else if (CharIndex + 6 < TextLength
					&& Text[CharIndex + 1] == 'c'
					&& Text[CharIndex + 2] == 'o'
					&& Text[CharIndex + 3] == 'l'
					&& Text[CharIndex + 4] == 'o'
					&& Text[CharIndex + 5] == 'r'
					&& Text[CharIndex + 6] == '='
					)//begin color=
				{
					if (bEnableColor)
					{
						int charEndIndex;
						FColor parsedColor;
						if (GetColor(Text, TextLength, CharIndex + 7, charEndIndex, parsedColor))
						{
							InOutStartIndex += charEndIndex - CharIndex + 1;
							ColorArray.Add(parsedColor);
							bHaveSymbol = true;
						}
					}
				}
				else if (CharIndex + 4 < TextLength
					&& Text[CharIndex + 1] == 's'
					&& Text[CharIndex + 2] == 'u'
					&& Text[CharIndex + 3] == 'p'
					&& Text[CharIndex + 4] == '>'
					)//begin sup
				{
					if (bEnableSuperscript)
					{
						InOutStartIndex += 5;
						SupOrSubArray.Add(ESupOrSubMode::Sup);
						bHaveSymbol = true;
					}
				}
				else if (CharIndex + 4 < TextLength
					&& Text[CharIndex + 1] == 's'
					&& Text[CharIndex + 2] == 'u'
					&& Text[CharIndex + 3] == 'b'
					&& Text[CharIndex + 4] == '>'
					)//begin sub
				{
					if (bEnableSubscript)
					{
						InOutStartIndex += 5;
						SupOrSubArray.Add(ESupOrSubMode::Sub);
						bHaveSymbol = true;
					}
				}
				else if (CharIndex + 6 < TextLength
					&& Text[CharIndex + 1] == 'i'
					&& Text[CharIndex + 2] == 'm'
					&& Text[CharIndex + 3] == 'g'
					&& Text[CharIndex + 4] == '='
					)//begin image=
				{
					if (bEnableImage)
					{
						int charEndIndex;
						if (GetImageTag(Text, TextLength, CharIndex + 5, charEndIndex, ImageTag))
						{
							InOutStartIndex += charEndIndex - CharIndex + 1;
							bHaveSymbol = true;
						}
					}
				}
				else if (CharIndex + 1 < TextLength && Text[CharIndex + 1] == '/')//end
				{
					if (CharIndex + 3 < TextLength && Text[CharIndex + 3] == '>')
					{
						if (Text[CharIndex + 2] == 'b' && BoldCount > 0)//end bold
						{
							if (bEnableBold)
							{
								InOutStartIndex += 4;
								BoldCount--;
								bHaveSymbol = true;
							}
						}
						else if (Text[CharIndex + 2] == 'i' && ItalicCount > 0)//end italic
						{
							if (bEnableItalic)
							{
								InOutStartIndex += 4;
								ItalicCount--;
								bHaveSymbol = true;
							}
						}
						else if (Text[CharIndex + 2] == 'u' && UnderlineCount > 0)//end underline
						{
							if (bEnableUnderline)
							{
								InOutStartIndex += 4;
								UnderlineCount--;
								bHaveSymbol = true;
							}
						}
						else if (Text[CharIndex + 2] == 's' && StrikethroughCount > 0)//end strikethough
						{
							if (bEnableStrikethrough)
							{
								InOutStartIndex += 4;
								StrikethroughCount--;
								bHaveSymbol = true;
							}
						}
					}
					else if (CharIndex + 6 < TextLength
						&& Text[CharIndex + 2] == 's'
						&& Text[CharIndex + 3] == 'i'
						&& Text[CharIndex + 4] == 'z'
						&& Text[CharIndex + 5] == 'e'
						&& Text[CharIndex + 6] == '>'
						&& SizeArray.Num() > 0
						)//end size
					{
						if (bEnableSize)
						{
							InOutStartIndex += 7;
							SizeArray.Pop();
							bHaveSymbol = true;
						}
					}
					else if (CharIndex + 7 < TextLength
						&& Text[CharIndex + 2] == 'c'
						&& Text[CharIndex + 3] == 'o'
						&& Text[CharIndex + 4] == 'l'
						&& Text[CharIndex + 5] == 'o'
						&& Text[CharIndex + 6] == 'r'
						&& Text[CharIndex + 7] == '>'
						&& ColorArray.Num() > 0
						)//end color
					{
						if (bEnableColor)
						{
							InOutStartIndex += 8;
							ColorArray.Pop();
							bHaveSymbol = true;
						}
					}
					else if (CharIndex + 5 < TextLength
						&& Text[CharIndex + 2] == 's'
						&& Text[CharIndex + 3] == 'u'
						&& Text[CharIndex + 4] == 'p'
						&& Text[CharIndex + 5] == '>'
						&& SupOrSubArray.Num() > 0
						)//end sup
					{
						if (bEnableSuperscript)
						{
							InOutStartIndex += 6;
							SupOrSubArray.Pop();
							bHaveSymbol = true;
						}
					}
					else if (CharIndex + 5 < TextLength
						&& Text[CharIndex + 2] == 's'
						&& Text[CharIndex + 3] == 'u'
						&& Text[CharIndex + 4] == 'b'
						&& Text[CharIndex + 5] == '>'
						&& SupOrSubArray.Num() > 0
						)//end sub
					{
						if (bEnableSubscript)
						{
							InOutStartIndex += 6;
							SupOrSubArray.Pop();
							bHaveSymbol = true;
						}
					}
					else if (CustomTagArray.Num() > 0
						)//end custom tag
					{
						if (bEnableCustomTag)
						{
							int charEndIndex;
							FName tag;
							if (GetCustomTag(Text, TextLength, CharIndex + 2, charEndIndex, tag))
							{
								auto foundIndex = CustomTagArray.IndexOfByKey(tag);
								if (foundIndex != -1)
								{
									CustomTagArray.RemoveAt(foundIndex);
									InOutStartIndex += charEndIndex - CharIndex + 1;
									ParseResult.CustomTag = tag;
									ParseResult.CustomTagMode = ECustomTagMode::End;
									bHaveSymbol = true;
								}
							}
						}
					}
				}
				else if(CharIndex + 1 < TextLength
					)//check custom tag
				{
					if (bEnableCustomTag)
					{
						int charEndIndex;
						FName tag;
						if (GetCustomTag(Text, TextLength, CharIndex + 1, charEndIndex, tag))
						{
							auto foundIndex = CustomTagArray.IndexOfByKey(tag);
							if (foundIndex == -1)
							{
								InOutStartIndex += charEndIndex - CharIndex + 1;
								CustomTagArray.Add(tag);
								ParseResult.CustomTag = tag;
								ParseResult.CustomTagMode = ECustomTagMode::Start;
								bHaveSymbol = true;
							}
						}
					}
				}
			}
			if (bHaveSymbol)
			{
				ParseResult.Bold = BoldCount > 0 || OriginBold;
				ParseResult.Italic = ItalicCount > 0 || OriginItalic;
				ParseResult.Underline = UnderlineCount > 0;
				ParseResult.Strikethrough = StrikethroughCount > 0;
				ParseResult.Size = SizeArray.Num() > 0 ? SizeArray[SizeArray.Num() - 1] : OriginSize;
				ParseResult.Size = FMath::Max(ParseResult.Size, 0.0f);
				ParseResult.HasColor = ColorArray.Num() > 0;
				ParseResult.Color = ParseResult.HasColor ? ColorArray[ColorArray.Num() - 1] : OriginColor;
				ParseResult.SupOrSubMode = SupOrSubArray.Num() > 0 ? SupOrSubArray[SupOrSubArray.Num() - 1] : ESupOrSubMode::None;
				if (ParseResult.SupOrSubMode != ESupOrSubMode::None)
				{
					ParseResult.Size *= 0.8f;//sup or sub size
				}
				ParseResult.ImageTag = ImageTag;
			}
			return bHaveSymbol;
		}
	private:
		//scan from StartIndex for the first tag-value terminator ('>', '<', space, '\n' or '\t').
		//returns its index, or -1 if none found before TextLength.
		static int FindTokenEnd(const FString& Text, int TextLength, int StartIndex)
		{
			for (int i = StartIndex; i < TextLength; i++)
			{
				const TCHAR c = Text[i];
				if (c == '>' || c == '<' || c == ' ' || c == '\n' || c == '\t')
				{
					return i;
				}
			}
			return -1;
		}
		//parse a float (optional leading +/-, digits, at most one '.') straight from a TCHAR range.
		//matches FString::IsNumeric semantics so behaviour stays identical to the old Mid+IsNumeric+Atof path.
		static bool ParseFloat(const TCHAR* Str, int Len, float& OutValue)
		{
			if (Len <= 0)
			{
				return false;
			}
			int i = 0;
			bool bNegative = false;
			if (Str[0] == '+')
			{
				i = 1;
			}
			else if (Str[0] == '-')
			{
				i = 1;
				bNegative = true;
			}
			//note: a lone sign (e.g. "+"/"-") is treated as 0, matching FString::IsNumeric + FCString::Atof
			bool bHasDot = false;
			double IntegerPart = 0.0;
			double FractionPart = 0.0;
			double FractionScale = 0.1;
			for (; i < Len; i++)
			{
				const TCHAR c = Str[i];
				if (c == '.')
				{
					if (bHasDot)
					{
						return false;
					}
					bHasDot = true;
				}
				else if (c >= '0' && c <= '9')
				{
					const int32 Digit = c - '0';
					if (bHasDot)
					{
						FractionPart += Digit * FractionScale;
						FractionScale *= 0.1;
					}
					else
					{
						IntegerPart = IntegerPart * 10.0 + Digit;
					}
				}
				else
				{
					return false;
				}
			}
			const double Result = IntegerPart + FractionPart;
			OutValue = bNegative ? -(float)Result : (float)Result;
			return true;
		}
		//get size from 'size=' or 'size=+' or 'size=-', end with '>'
		//return true if is valid
		static bool GetSize(const FString& Text, int TextLength, int StartIndex, int& OutEndIndex, float& OutSize, bool& OutAbsoluteOrAdditional)
		{
			int EndIndex = FindTokenEnd(Text, TextLength, StartIndex);
			if (EndIndex != -1 && EndIndex > StartIndex && Text[EndIndex] == '>')//found end
			{
				const TCHAR* TokenPtr = Text.GetCharArray().GetData() + StartIndex;
				const int TokenLen = EndIndex - StartIndex;
				OutAbsoluteOrAdditional = TokenPtr[0] != '+' && TokenPtr[0] != '-';
				if (ParseFloat(TokenPtr, TokenLen, OutSize))
				{
					OutEndIndex = EndIndex;
					return true;
				}
			}
			return false;
		}
		static bool GetCustomTag(const FString& Text, int TextLength, int StartIndex, int& OutEndIndex, FName& OutTag)
		{
			int EndIndex = FindTokenEnd(Text, TextLength, StartIndex);
			if (EndIndex != -1 && EndIndex > StartIndex && Text[EndIndex] == '>')//found end
			{
				OutTag = FName(FStringView(Text.GetCharArray().GetData() + StartIndex, EndIndex - StartIndex));
				OutEndIndex = EndIndex;
				return true;
			}
			return false;
		}
		static bool GetImageTag(const FString& Text, int TextLength, int StartIndex, int& OutEndIndex, FName& OutTag)
		{
			//image is a self-closing tag, must end with '/>'; scan from the char after the first tag char
			int EndIndex = FindTokenEnd(Text, TextLength, StartIndex + 1);
			if (EndIndex != -1 && EndIndex > StartIndex && Text[EndIndex] == '>' && Text[EndIndex - 1] == '/')//found end
			{
				OutTag = FName(FStringView(Text.GetCharArray().GetData() + StartIndex, EndIndex - StartIndex - 1));
				OutEndIndex = EndIndex;
				return true;
			}
			return false;
		}
		//get color from 'color=red' or 'color=#ffffff', end with '>'
		//return true if is valid
		bool GetColor(const FString& Text, int TextLength, int StartIndex, int& OutEndIndex, FColor& OutColor)
		{
			int EndIndex = FindTokenEnd(Text, TextLength, StartIndex);
			if (EndIndex == -1 || EndIndex <= StartIndex || Text[EndIndex] != '>')//no valid end
			{
				return false;
			}
			OutEndIndex = EndIndex;
			const TCHAR* TokenPtr = Text.GetCharArray().GetData() + StartIndex;
			const int TokenLen = EndIndex - StartIndex;

			auto EqualsIC = [&TokenPtr, &TokenLen](const TCHAR* Lit) -> bool
			{
				const int LitLen = (int)FCString::Strlen(Lit);
				return TokenLen == LitLen && FCString::Strnicmp(TokenPtr, Lit, LitLen) == 0;
			};

			if (EqualsIC(TEXT("black")))
			{
				OutColor = FColor::Black;
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("white")))
			{
				OutColor = FColor::White;
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("gray")))
			{
				OutColor = FColor(128, 128, 128);
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("silver")))
			{
				OutColor = FColor(192, 192, 192);
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("red")))
			{
				OutColor = FColor::Red;
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("green")))
			{
				OutColor = FColor::Green;
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("blue")))
			{
				OutColor = FColor::Blue;
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("orange")))
			{
				OutColor = FColor(255, 165, 0);
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("purple")))
			{
				OutColor = FColor(128, 0, 128);
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (EqualsIC(TEXT("yellow")))
			{
				OutColor = FColor(255, 255, 0);
				OutColor.A = OriginRenderOpacity;
				return true;
			}
			else if (TokenPtr[0] == '#')
			{
				auto HexValue = [](TCHAR c) -> int
				{
					if (c >= '0' && c <= '9')return c - '0';
					if (c >= 'a' && c <= 'f')return c - 'a' + 10;
					if (c >= 'A' && c <= 'F')return c - 'A' + 10;
					return -1;
				};
				if (TokenLen == 7 || TokenLen == 9)//#ffffff/#ffffff00
				{
					OutColor.A = OriginRenderOpacity;
					for (int i = 1; i < TokenLen; i += 2)
					{
						int FirstIndex = HexValue(TokenPtr[i]);
						int SecondIndex = HexValue(TokenPtr[i + 1]);
						if (FirstIndex != -1 && SecondIndex != -1)//valid
						{
							uint8 value = (uint8)(FirstIndex * 16 + SecondIndex);
							switch (i)
							{
							case 1:OutColor.R = value; break;
							case 3:OutColor.G = value; break;
							case 5:OutColor.B = value; break;
							case 7:
							{
								OutColor.A = (uint8)(FLexUIUtils::ByteToFloat01(OriginRenderOpacity) * value);
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
			return false;
		}
	};
}