// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Misc/AssertionMacros.h"
#include "HAL/UnrealMemory.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/UnrealString.h"

/** Used to read/write a bit in the static array as a bool. */
template<typename T>
class TStaticBitReference
{
public:

	TStaticBitReference(T& InData,T InMask)
	:	Data(InData)
	,	Mask(InMask)
	{}

	FORCEINLINE operator bool() const
	{
		 return (Data & Mask) != 0;
	}
	FORCEINLINE void operator=(const bool NewValue)
	{
		if(NewValue)
		{
			Data |= Mask;
		}
		else
		{
			Data &= ~Mask;
		}
	}

private:

	T& Data;
	T Mask;
};


/** Used to read a bit in the static array as a bool. */
template<typename T>
class TConstStaticBitReference
{
public:

	TConstStaticBitReference(const T& InData,T InMask)
		: Data(InData)
		, Mask(InMask)
	{ }

	FORCEINLINE operator bool() const
	{
		 return (Data & Mask) != 0;
	}

private:

	const T& Data;
	T Mask;
};


/**
 * A statically sized bit array.
 */
template<uint32 NumBits>
class TStaticBitArray
{
	typedef uint64 WordType;

	struct FBoolType;
	typedef int32* FBoolType::* UnspecifiedBoolType;
	typedef float* FBoolType::* UnspecifiedZeroType;

public:

	/** Minimal initialization constructor */
	FORCEINLINE TStaticBitArray()
	{
		Clear_();
	}

	/** Constructor that allows initializing by assignment from 0 */
	UE_DEPRECATED(5.4, "Implicitly constructing a TStaticBitArray from 0 has been deprecated - please use the default constructor instead")
	FORCEINLINE TStaticBitArray(UnspecifiedZeroType)
	{
		Clear_();
	}

	/**
	 * Constructor to initialize to a single bit
	 */
	FORCEINLINE TStaticBitArray(bool, uint32 InBitIndex)
	{
		/***********************************************************************

		 If this line fails to compile you are attempting to construct a bit
		 array with an out-of bounds bit index. Follow the compiler errors to
		 the initialization point

		***********************************************************************/
//		static_assert(InBitIndex >= 0 && InBitIndex < NumBits, "Invalid bit value.");

		check((NumBits > 0) ? (InBitIndex<NumBits):1);

		uint32 DestWordIndex = InBitIndex / NumBitsPerWord;
		WordType Word = (WordType)1 << (InBitIndex & (NumBitsPerWord - 1));

		for(int32 WordIndex = 0; WordIndex < NumWords; ++WordIndex)
		{
			Words[WordIndex] = WordIndex == DestWordIndex ? Word : (WordType)0;
		}
	}

	/**
	 * Constructor to initialize from string
	 */
	explicit TStaticBitArray(const FString& Str)
	{
		int32 Length = Str.Len();

		// Trim count to length of bit array
		if(NumBits < Length)
		{
			Length = NumBits;
		}
		Clear_();

		int32 Pos = Length;
		for(int32 Index = 0; Index < Length; ++Index)
		{
			const TCHAR ch = Str[--Pos];
			if(ch == TEXT('1'))
			{
				operator[](Index) = true;
			}
			else if(ch != TEXT('0'))
			{
				ErrorInvalid_();
			}
		}
	}

	FORCEINLINE bool HasAnyBitsSet() const
	{
		WordType And = 0;
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			And |= Words[Index];
		}

		return And != 0;
	}

	// Explicit conversion to bool
	FORCEINLINE explicit operator bool() const
	{
		return this->HasAnyBitsSet();
	}

	// Accessors.
	FORCEINLINE static int32 Num() { return NumBits; }
	FORCEINLINE TStaticBitReference<WordType> operator[](int32 Index)
	{
		check(Index>=0 && Index<NumBits);
		return TStaticBitReference<WordType>(
			Words[Index / NumBitsPerWord],
			(WordType)1 << (Index & (NumBitsPerWord - 1))
			);
	}
	FORCEINLINE const TConstStaticBitReference<WordType> operator[](int32 Index) const
	{
		check(Index>=0 && Index<NumBits);
		return TConstStaticBitReference<WordType>(
			Words[Index / NumBitsPerWord],
			(WordType)1 << (Index & (NumBitsPerWord - 1))
			);
	}

	// Modifiers.
	FORCEINLINE TStaticBitArray& operator|=(const TStaticBitArray& Other)
	{
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			Words[Index] |= Other.Words[Index];
		}
		return *this;
	}
	FORCEINLINE TStaticBitArray& operator&=(const TStaticBitArray& Other)
	{
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			Words[Index] &= Other.Words[Index];
		}
		return *this;
	}
	FORCEINLINE TStaticBitArray& operator^=(const TStaticBitArray& Other )
	{
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			Words[Index] ^= Other.Words[Index];
		}
		return *this;
	}
	friend FORCEINLINE TStaticBitArray<NumBits> operator~(const TStaticBitArray<NumBits>& A)
	{
		TStaticBitArray Result;
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			Result.Words[Index] = ~A.Words[Index];
		}
		Result.Trim_();
		return Result;
	}
	friend FORCEINLINE TStaticBitArray<NumBits> operator|(const TStaticBitArray<NumBits>& A, const TStaticBitArray<NumBits>& B)
	{
		// is not calling |= because doing it in here has less LoadHitStores and is therefore faster.
		TStaticBitArray Results(0);

		const WordType* RESTRICT APtr = (const WordType* RESTRICT)A.Words;
		const WordType* RESTRICT BPtr = (const WordType* RESTRICT)B.Words;
		WordType* RESTRICT ResultsPtr = (WordType* RESTRICT)Results.Words;
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			ResultsPtr[Index] = APtr[Index] | BPtr[Index];
		}

		return Results;
	}
	friend FORCEINLINE TStaticBitArray<NumBits> operator&(const TStaticBitArray<NumBits>& A, const TStaticBitArray<NumBits>& B)
	{
		// is not calling &= because doing it in here has less LoadHitStores and is therefore faster.
		TStaticBitArray Results(0);

		const WordType* RESTRICT APtr = (const WordType* RESTRICT)A.Words;
		const WordType* RESTRICT BPtr = (const WordType* RESTRICT)B.Words;
		WordType* RESTRICT ResultsPtr = (WordType* RESTRICT)Results.Words;
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			ResultsPtr[Index] = APtr[Index] & BPtr[Index];
		}

		return Results;
	}
	friend FORCEINLINE TStaticBitArray<NumBits> operator^(const TStaticBitArray<NumBits>& A, const TStaticBitArray<NumBits>& B)
	{
		TStaticBitArray Results(A);
		Results ^= B;
		return Results;
	}
	FORCEINLINE bool operator==(const TStaticBitArray<NumBits>& B) const
	{
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			if(Words[Index] != B.Words[Index])
			{
				return false;
			}
		}
		return true;
	}
	/** This operator only exists to disambiguate == in statements of the form (flags == 0) */
	UE_DEPRECATED(5.4, "Comparing a TStaticBitArray to zero has been deprecated - please use !BitArray.HasAnyBitsSet() instead")
	friend FORCEINLINE bool operator==(const TStaticBitArray<NumBits>& A, UnspecifiedBoolType Value)
	{
		return (UnspecifiedBoolType)A == Value;
	}
	/** != simple maps to == */
	FORCEINLINE bool operator!=(const TStaticBitArray<NumBits>& B) const
	{
		return !(*this == B);
	}
	/** != simple maps to == */
	UE_DEPRECATED(5.4, "Comparing a TStaticBitArray to zero has been deprecated - please use BitArray.HasAnyBitsSet() instead")
	friend FORCEINLINE bool operator!=(const TStaticBitArray<NumBits>& A, UnspecifiedBoolType Value)
	{
		return !(A == Value);
	}
	
	/**
	 * Finds the first clear bit in the array and returns the bit index.
	 * If there isn't one, INDEX_NONE is returned.
	 */
	int32 FindFirstClearBit() const
	{
		const int32 LocalNumBits = NumBits;

		int32 WordIndex = 0;
		// Iterate over the array until we see a word with a unset bit.
		while (WordIndex < NumWords && Words[WordIndex] == WordType(-1))
		{
			++WordIndex;
		}

		if (WordIndex < NumWords)
		{
			// Flip the bits, then we only need to find the first one bit -- easy.
			const WordType Bits = ~(Words[WordIndex]);
			UE_ASSUME(Bits != 0);

			const int32 LowestBitIndex = FMath::CountTrailingZeros64(Bits) + (WordIndex << NumBitsPerWordLog2);
			if (LowestBitIndex < LocalNumBits)
			{
				return LowestBitIndex;
			}
		}

		return INDEX_NONE;
	}

	/**
	 * Finds the first set bit in the array and returns it's index.
	 * If there isn't one, INDEX_NONE is returned.
	 */
	int32 FindFirstSetBit() const
	{
		const int32 LocalNumBits = NumBits;

		int32 WordIndex = 0;
		// Iterate over the array until we see a word with a set bit.
		while (WordIndex < NumWords && Words[WordIndex] == WordType(0))
		{
			++WordIndex;
		}

		if (WordIndex < NumWords)
		{
			const WordType Bits = Words[WordIndex];
			UE_ASSUME(Bits != 0);

			const int32 LowestBitIndex = (int32)FMath::CountTrailingZeros64(Bits) + (WordIndex << NumBitsPerWordLog2);
			if (LowestBitIndex < LocalNumBits)
			{
				return LowestBitIndex;
			}
		}

		return INDEX_NONE;
	}

	/**
	 * Converts the bitarray to a string representing the binary representation of the array
	 */
	FString ToString() const
	{
		FString Str;
		Str.Empty(NumBits);

		for(int32 Index = NumBits - 1; Index >= 0; --Index)
		{
			Str += operator[](Index) ? TEXT('1') : TEXT('0');
		}

		return Str;
	}

	static constexpr uint32 NumOfBits = NumBits;

private:
	static constexpr uint32 NumBitsPerWord = sizeof(WordType) * 8;
	static constexpr uint32 NumBitsPerWordLog2 = 6;
	static_assert(NumBitsPerWord == (1u << NumBitsPerWordLog2), "Update NumBitsPerWordLog2 to reflect WordType");
	static constexpr uint32 NumWords = ((NumBits + NumBitsPerWord - 1) & ~(NumBitsPerWord - 1)) / NumBitsPerWord;
	WordType Words[NumWords];

	// Helper class for bool conversion
	struct FBoolType
	{
		int32* Valid;
	};

	/**
	 * Resets the bit array to a 0 value
	 */
	FORCEINLINE void Clear_()
	{
		for(int32 Index = 0; Index < NumWords; ++Index)
		{
			Words[Index] = 0;
		}
	}

	/**
	 * Clears any trailing bits in the last word
	 */
	void Trim_()
	{
		constexpr uint32 NumOverflowBits = NumBits % NumBitsPerWord; //-V1064 The 'NumBits' operand of the modulo operation is less than the 'NumBitsPerWord' operand. The result is always equal to the left operand.
		if constexpr (NumOverflowBits != 0)
		{
			Words[NumWords-1] &= (WordType(1) << NumOverflowBits) - 1;
		}
	}

	/**
	 * Reports an invalid string element in the bitset conversion
	 */
	void ErrorInvalid_() const
	{
		LowLevelFatalError(TEXT("invalid TStaticBitArray<NumBits> character"));
	}
};

/**
	* Serializer.
	*/
template<uint32 NumBits>
FArchive& operator<<(FArchive& Ar, TStaticBitArray<NumBits>& BitArray)
{
	uint32 ArchivedNumWords = BitArray.NumWords;
	Ar << ArchivedNumWords;

	if(Ar.IsLoading())
	{
		FMemory::Memset(BitArray.Words, 0, sizeof(BitArray.Words));
		ArchivedNumWords = FMath::Min(BitArray.NumWords, ArchivedNumWords);
	}

	Ar.Serialize(BitArray.Words, ArchivedNumWords * sizeof(BitArray.Words[0]));
	return Ar;
}
