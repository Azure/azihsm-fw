// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//-----------------------------------------------------------------------------
//!
//! @file
//! @brief Mathematics interface
//!
//=============================================================================
#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include "ArmIntrinsics.h"
#include "SysTypes.h"
#include "SystemTypes.h"
#include "MemIo.h"
#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------

#define cCelsiusToKelvinsOffset     (273)   ///< Celsius to Kelvins offset

//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------

/**
 *  @brief returns a random number
 *
 *  @param[in] minimum value of the random number
 *  @param[in] maximum value of the random number
 *  @returns a random number between minimum and maximum
 */
int32_t GetRandomNumber(int32_t minimum, int32_t maximum);

/**
 *  @brief Get the number of elements of an array
 *
 *  @param[in] array        The array
 *  @returns   The number of elements
 */
#define GetNumberOfArrayElements(array)     (sizeof(array) / sizeof(array[0]))
#define NUM_ELEMENTS(a)                     GetNumberOfArrayElements(a)
#define ARRAY_SIZE(a)                       GetNumberOfArrayElements(a)

/**
 *  @brief Get index position of an element in an array
 *
 *  @param[in] pElement     Reference to an array element
 *  @param[in] array        The array
 *  @returns   The array index position of the element.
 */
#define GetArrayElementIndex(pElement, array) ((((uint32_t)pElement) - ((uint32_t)&(array[0]))) / sizeof(array[0]))

/**
 *  @brief Converts a Qx value to an unsigned long value.
 *
 *  @param[in] x      Qx factor
 *  @param[in] value  Qx Value
 *  @returns   U32 value
 */
#define ConvertQxToU32(x, value) ((uint32_t)(value) / (1 << x))

/**
 *  @brief Converts an unsigned long value to a Qx value.
 *
 *  @param[in] x      Qx factor
 *  @param[in] value  U32 Value
 *  @returns   Qx value
 */
#define ConvertU32ToQx(x, value) ((uint32_t)(value) * (1 << x))

/**
 *  @brief Sets a specified number of the least significant bits to 1
 *
 *  @param[in] number of bits to set
 *  @returns   the bitmask
 */
#define BitMask(numberOfBits)   ((numberOfBits > 31) ? 0xffffffff : ((1UL << (numberOfBits)) - 1))

/// Offset (in bytes) of a member within a given struct
#define MEMBER_OFFSET(struct_type, member)  ((int)(void*)&((struct_type*)0)->member)

/// Size (in bytes) of a member within a given struct
#define MEMBER_SIZE(struct_type, member)    (sizeof(((struct_type*)0)->member))

//-----------------------------------------------------------------------------
//  Public Interface Functions:
//-----------------------------------------------------------------------------

/**
 *  @brief Calculates an 8 bit CRC number from a given data buffer
 *
 *  @param[in] pData             The data buffer
 *  @param[in] dataBufferLength  The data buffer length
 *  @returns   The Crc number calculated
 */
uint8_t CalculateCrc(const uint8_t* pData, uint32_t dataBufferLength);

/**
 *  @brief Calculates the highest common denominator of two values
 *
 *  @param[in] value 1
 *  @param[in] value 2
 *  @returns   highest common denominator
 */
uint32_t CalculateHighestCommonDenominator(uint32_t value1, uint32_t value2);

/**
 *  @brief Calculates least common multiple
 *
 *  @param[in] value 1
 *  @param[in] value 2
 *  @returns   least common multiple
 */
uint32_t CalculateLeastCommonMultiple(uint32_t value1, uint32_t value2);

/**
 *  @brief Copy data and calculates an 8 bit CRC number from a given data buffer
 *
 *  @param[in] pDestination      The data buffer
 *  @param[in] pSource           The data buffer to copy to
 *  @param[in] dataBufferLength  The data buffer length
 *  @returns   The Crc number calculated
 */
uint8_t CopyAndCalculateCrc(uint8_t* pDestination, uint8_t* pSource, uint32_t length);

/**
 *  @brief Copy data and calculates an 8 bit Checksum number from a given data buffer
 *
 *  @param[in] pDestination      The data buffer
 *  @param[in] pSource           The data buffer to copy to
 *  @param[in] dataBufferLength  The data buffer length
 *  @returns   The checksum number calculated
 */
uint8_t CopyAndCalculateChecksum(uint8_t* pDestination, uint8_t* pSource, uint32_t length);

/**
 *  @brief Count the number of set bits in an array
 *
 *  @param[in] array of words
 *  @param[in] number of entrees in the array
 *  @returns The number of bits set.
 */
uint32_t CountNumberOfOnesInArray(const uint32_t pData[], uint32_t numberOfWordsInArray);

/**
 *  @brief Checksum calculation
 *
 *  This function is used to calculate the 32-bit checksum
 *
 *  @param[in]  pContent            Pointer to the content to calculate
 *  @param[in]  length              The size of the input content
 *  @param[in]  skipUnalignedData   Ignore unaligned data
 *  @return     the checksum of the content
 */
uint32_t CheckSum32(void* pContent, uint32_t length, bool skipUnalignedData);

/**
 *  @brief Checksum calculation
 *
 *  This function is used to calculate the 8-bit checksum
 *
 *  @param[in]  pContent    Pointer to the content to calculate
 *  @param[in]  length      The size of the input content
 *  @return     the checksum of the content
 */
uint8_t CheckSum8(uint8_t* pContent, uint32_t length);

/**
 *  @brief Rounds up value to the next power of 2 unless it's already a power of 2.
 *
 *  @param[in]   value   Value
 *  @return              Rounded up value if applicable.
 */
__inline static uint32_t RoundUpToNextPowerOfTwo(uint32_t value);

/**
 *  @brief Round down (Value) to the previous (Multiple) boundary
 *
 *  @param[in] value        Value
 *  @param[in] multiple     Multiple
 *  @returns   Value' rounded down to the previous multiple of 'Multiple'
 */
__inline static uint32_t RoundDownToPreviousMultipleBoundary(uint32_t value, uint32_t multiple);

/**
 *  @brief round up V(alue) to the next M(ultiple) boundary
 *
 *  @param[in] value        Value
 *  @param[in] multiple     Multiple
 *  @returns   'Value' rounded up to the next multiple of 'Multiple'
 */
__inline static uint64_t RoundUp64BitsToNextMultipleBoundary(uint64_t value, uint64_t multiple);

/**
 *  @brief Count the number of set bits
 *
 *  @param[in] value        Value
 *  @returns   The number of bits set.
 */
__inline static int CountNumberOfOnes(uint32_t value);

/**
 *  @brief Count the number of leading zeros.
 *
 *  @param[in] value        Value
 *  @returns   The number of leading zeros.
 */
__inline static uint8_t CountLeadingZeros(uint32_t value);


/**
 *  @brief Finds first set or Find first one in a range of specific bits from the start bit.
 *
 *  @param[in]  value       the value of data
 *  @param[in]  highBit     the MSB of the bits range
 *  @param[in]  lowBit      the LSB of the bits range
 *  @param[in]  startBit    the start bit to search one, which should be between highBit and lowBit.
 *  @return     bitPos    0 to 31: the first bit position which is set from LSB.\n
 *                        0xFFFF:  no bits which is set.
 */
__inline static uint16_t GetFirstSetOfRange(uint32_t value, uint32_t highBit, uint32_t startBit, uint32_t lowBit);

/**
 *  @brief Finds first set in a range of specific bits from the start bit in reverse direction.
 *
 *  @param[in]  value       the value of data
 *  @param[in]  highBit     the MSB of the bits range
 *  @param[in]  lowBit      the LSB of the bits range
 *  @param[in]  startBit    the start bit to search one, which should be between highBit and lowBit.
 *  @return     bitPos    0 to 31: the first bit position which is set from LSB.\n
 *                        0xFFFF:  no bits which is set.
 */
__inline static uint16_t GetFirstMsbSetOfRange(uint32_t value, uint32_t highBit, uint32_t startBit, uint32_t lowBit);

/**
 *  @brief Calculates the minimum required number of bits to represent the given value
 *
 *  @param[in] value        Value
 *  @returns   minimum number of bits required
 */
__inline static uint32_t CalculateMinimumNumberOfRequiredBits(uint32_t value);


/**
 *  @brief Decrement a circular value.
 *
 *  @param[in] value    value, to decrement by one
 *  @param[in] maximum  maximum value
 *  @returns   Decremented value
 */
__inline static uint32_t CircularDecrement(uint32_t value, uint32_t maximum);

/**
 *  @brief Increment a circular value.
 *
 *  @param[in] value    value, to increase by one
 *  @param[in] maximum  maximum value, will wrap at this value
 *  @returns   Incremented value
 */
__inline static uint32_t CircularIncrement(uint32_t value, uint32_t maximum);

/**
 *  @brief Round up V(alue) after division with M(ultiple) boundary
 *
 *  @param[in] value        Value
 *  @param[in] multiple     Multiple
 *  @returns   rounded up quotient
 */
__inline static uint32_t DivideAndRoundUp(uint32_t value, uint32_t multiple);

/**
 * @brief Reverses the bit order
 *
 * @param[in]   value   Value to reverse.
 * @return              Reversed value.
 */
__inline static uint32_t ReverseBits(uint32_t value);

/**
 *  @brief Count the number of trailing Ones.
 *
 *  @param[in] value        Value
 *  @returns   The number of trailing ones (or: the bitposition of the first least significant bit that is reset)
 */
__inline static uint8_t CountTrailingOnes(uint32_t value);

/**
 *  @brief Count the number of trailing Ones.
 *
 *  @param[in] value        Value
 *  @returns   The number of trailing zeros (or: the bitposition of the first least significant bit that is set)
 */
__inline static uint8_t CountTrailingZeros(uint32_t value);

/**
 *  @brief Determines if the value is a power of 2.
 *
 *  @param[in]  value  The value to check.
 *  @return            True if it's a power of 2, false otherwise.
 */
__inline static bool IsPowerOfTwo(uint32_t value);

/**
 *  @brief Convert temperature from Celsius to Kelvins.
 *
 *  @param[in]  temperature  Temperature in degrees of Celsius
 *  @return                  Temperature in degrees of Kelvins.
 */
__inline static uint32_t ConvertCelsiusToKelvins(int32_t temperature);

/**
 *  @brief Convert temperature from Kelvins to Celisus.
 *
 *  @param[in]  temperature  Temperature in degrees of Kelvins.
 *  @return                  Temperature in degrees of Celsius
 */
__inline static int32_t ConvertKelvinsToCelsius(uint32_t temperature);

/**
 *  @brief Multiply a value by a ratio using a bit shifted multiplier
 *
 *  @param a first value : NOT already shifted
 *  @param b second value : Already shifted
 *  @param bits number of bits shifted
 *  @return resulting value1
 *  @example If we wanted to find 110% of a = 0xa, we could use a value
 *          of bits = 12 and b = 0x1199 (1<<bits * 110%), where result would be 0xb
 */
__inline static uint32_t fixed_mul(uint32_t a, uint32_t b, uint32_t bits);

//-----------------------------------------------------------------------------
//  Inline Functions
//-----------------------------------------------------------------------------

/**
 *  @brief Round up V(alue) to the next M(ultiple) boundary
 *
 *  @param[in] value        Value
 *  @param[in] multiple     Multiple
 *  @returns   Value' rounded up to the next multiple of 'Multiple'
 */
#ifdef __cplusplus
constexpr static uint32_t RoundUpToNextMultipleBoundary(uint32_t value, uint32_t multiple)
{
    return (multiple * ((value + multiple - 1) / multiple));
}
#else
static uint32_t RoundUpToNextMultipleBoundary_C(uint32_t value, uint32_t multiple)
{
    return (multiple * ((value + multiple - 1) / multiple));
}
#endif

__inline static uint32_t RoundUpToNextPowerOfTwo(uint32_t value)
{
    uint32_t log2 = 32 - CountLeadingZeros(value >> 1);
    return (BIT(log2) == value) ? BIT(log2) : BIT(log2 + 1);
}

__inline static uint32_t RoundDownToPreviousMultipleBoundary(uint32_t value, uint32_t multiple)
{
    return (multiple * (value / multiple));
}

__inline static uint64_t RoundUp64BitsToNextMultipleBoundary(uint64_t value, uint64_t multiple)
{
    return (multiple * ((value + multiple - 1) / multiple));
}

__inline static uint32_t RoundDownToPreviousPowerOfTwo(uint32_t value)
{
    if (value == 0)
    {
        return 0;
    }
    else
    {
        uint32_t log2 = 32 - CountLeadingZeros(value >> 1);
        return BIT(log2);
    }
}

__inline static int CountNumberOfOnes(uint32_t value)
{
    value = value - ((value >> 0x1UL) & 0x55555555UL);
    value = (value & 0x33333333UL) + ((value >> 0x2UL) & 0x33333333UL);
    return (((value + (value >> 0x4UL)) & 0x0F0F0F0FUL) * 0x01010101UL) >> 0x18UL;
}

__inline static uint8_t CountLeadingZeros(uint32_t value)
{
    return __clz(value);
}

__inline static uint16_t GetFirstSetOfRange(uint32_t value, uint32_t highBit, uint32_t startBit, uint32_t lowBit)
{
    /// For the performance reason, take over the responsibility of checking startBit to the caller.
    /// if(startBit < lowBit || startBit > highBit) { startBit = lowBit; }

    uint16_t bitPos = FFS_RANGE(value, highBit, startBit); /// 0xFFFF is invalid.
    if ((bitPos == 0xFFFF) && (startBit != 0))
    {
        bitPos = FFS_RANGE(value, startBit - 1, lowBit);
    }

    return bitPos;
}

__inline static uint16_t GetFirstMsbSetOfRange(uint32_t value, uint32_t highBit, uint32_t startBit, uint32_t lowBit)
{
    /// For the performance reason, take over the responsibility of checking startBit to the caller.
    /// if(startBit < lowBit || startBit > highBit) { startBit = lowBit; }

    uint16_t bitPos = FFS_RANGE_RVS(value, startBit, lowBit);   /// 0xFFFF is invalid
    if ((bitPos == 0xFFFF) && (startBit != 7))
    {
        bitPos = FFS_RANGE_RVS(value, highBit, startBit + 1);
    }

    return bitPos;
}

__inline static uint32_t CalculateMinimumNumberOfRequiredBits(uint32_t value)
{
    return (32 - CountLeadingZeros(value));
}

__inline static uint32_t CircularDecrement(uint32_t value, uint32_t maximum)
{
    return (value == 0) ? maximum - 1 : value - 1;
}

__inline static uint32_t CircularIncrement(uint32_t value, uint32_t maximum)
{
    return (value == maximum - 1) ? 0 : value + 1;
}

__inline static uint32_t ReverseBits(uint32_t value)
{
    uint32_t out = 0;
    asm ("RBIT %0, %1" : "=r" (out) : "r" (value));
    return out;
    // uint32_t size = sizeof(value) * 8;
    // uint32_t result = 0;
    /*
       for (uint32_t i = 0; i < size; i++)
       {
        if ((value & (1 << i)))
        {
            result |= (1 << ((size - 1) - i));
        }
       }
       return result;
     */
}


__inline static uint8_t CountTrailingOnes(uint32_t value)
{
    uint32_t reversed = ReverseBits(value);
    return __builtin_clz(~reversed);
}

__inline static uint8_t CountTrailingZeros(uint32_t value)
{
    return __builtin_ctz(value);
}

__inline static bool IsPowerOfTwo(uint32_t value)
{
    return (value & (value - 1)) == 0;
}

__inline static bool IsPowerOfTwo64(uint64_t value)
{
    return (value & (value - 1ULL)) == 0ULL;
}

__inline static uint32_t ConvertCelsiusToKelvins(int32_t temperature)
{
    return (uint32_t)(temperature + cCelsiusToKelvinsOffset);
}

__inline static int32_t ConvertKelvinsToCelsius(uint32_t temperature)
{
    return (int32_t)temperature - cCelsiusToKelvinsOffset;
}
__inline static uint32_t fixed_mul(uint32_t a, uint32_t b, uint32_t bits)
{
    /// Truncation is faster, but loses precision in the fractional bits.
    uint32_t tmp = a * b;
    uint32_t rnd = tmp + ((tmp & 0x1UL << (bits - 0x1UL)) << 0x1UL);
    return rnd >> bits;
}

/**
 *  @brief Obtains the log of the 64-bit value
 *
 *  @param[in] value : value
 *  @return Base-2 Log of parameter
 */
__inline static uint32_t LOG264(uint64_t value)
{
    uint32_t bits;

    if (value >= 0x100000000ULL)
    {
        bits = 32;
        value >>= 32;
    }
    else
    {
        bits = 0;
    }

    bits += LOG2((uint32_t)(value));
    return bits;
}

/**
 *  @brief Optimized divide function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @return the quotient
 */
__inline static uint32_t Divide(uint32_t value, uint32_t divisor)
{
    if (IsPowerOfTwo(divisor))
    {
        const uint32_t bitShift = LOG2(divisor);
        return value >> bitShift;
    }
    else
    {
        return value / divisor;
    }
}

/**
 *  @brief Optimized divide function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @return the quotient
 */
__inline static uint64_t Divide64(uint64_t value, uint64_t divisor)
{
    if (IsPowerOfTwo64(divisor))
    {
        const uint32_t bitShift = LOG264(divisor);
        return value >> bitShift;
    }
    else
    {
        return value / divisor;
    }
}

__inline static uint32_t DivideAndRoundUp(uint32_t value, uint32_t multiple)
{
    return Divide(value + multiple - 1, multiple);
}

__inline static uint64_t DivideAndRoundUp64(uint64_t value, uint64_t multiple)
{
    return Divide64(value + multiple - 1ULL, multiple);
}

/**
 *  @brief Optimized modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @return the remainder
 */
__inline static uint32_t Modulo(uint32_t value, uint32_t divisor)
{
    if (IsPowerOfTwo(divisor))
    {
        return value & (divisor - 1);
    }
    else
    {
        return value % divisor;
    }
}

/**
 *  @brief Optimized modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @return the remainder
 */
__inline static uint64_t Modulo64(uint64_t value, uint64_t divisor)
{
    if (IsPowerOfTwo64(divisor))
    {
        return value & (divisor - 1ULL);
    }
    else
    {
        return value % divisor;
    }
}

#ifdef __cplusplus
/*
 *  @brief Optimized divide and modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @param[out] remainder: the remainder value
 *  @return the quotient value
 */
__inline static uint32_t DivideAndModulo(uint32_t value, uint32_t divisor, uint32_t& remainder)
{
    if (IsPowerOfTwo(divisor))
    {
        const uint32_t bitShift = LOG2(divisor);

        remainder = value & (divisor - 1);
        return value >> bitShift;
    }
    else
    {
        remainder = value % divisor;
        return value / divisor;
    }
}
#else
/**
 *  @brief Optimized divide and modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @param[out] remainder: the remainder value
 *  @return the quotient value
 */
__inline static uint32_t DivideAndModulo_C(uint32_t value, uint32_t divisor, uint32_t* remainder)
{
    if (IsPowerOfTwo(divisor))
    {
        const uint32_t bitShift = LOG2(divisor);

        *remainder = value & (divisor - 1);
        return value >> bitShift;
    }
    else
    {
        *remainder = value % divisor;
        return value / divisor;
    }
}
#endif

#ifdef __cplusplus
/**
 *  @brief Optimized divide and modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @param[out] remainder: the remainder value
 *  @return the quotient value
 */
__inline static uint64_t DivideAndModulo64(uint64_t value, uint64_t divisor, uint64_t& remainder)
{
    if (IsPowerOfTwo64(divisor))
    {
        uint32_t bitShift = LOG264(divisor);

        remainder = value & (divisor - 1ULL);
        return value >> bitShift;
    }
    else
    {
        remainder = value % divisor;
        return value / divisor;
    }
}
#else
/**
 *  @brief Optimized divide and modulo function.  Takes advantage if divisor is power of two.
 *
 *  Best to only use this function on divisors that are NOT constant.
 *
 *  @param[in] value     : the dividend value
 *  @param[in] divisor   : the divisor value
 *  @param[out] remainder: the remainder value
 *  @return the quotient value
 */
__inline static uint64_t DivideAndModulo64_C(uint64_t value, uint64_t divisor, uint64_t* remainder)
{
    if (IsPowerOfTwo64(divisor))
    {
        uint32_t bitShift = LOG264(divisor);

        *remainder = value & (divisor - 1ULL);
        return value >> bitShift;
    }
    else
    {
        *remainder = value % divisor;
        return value / divisor;
    }
}
#endif

/**
 *  @brief Gets the absolute difference between the two unsigned integers.
 *
 *  @param[in] value1     : value 1
 *  @param[in] value2     : value 2
 *  @return absolute difference between the two values
 */
__inline static uint32_t AbsDifference(uint32_t value1, uint32_t value2)
{
    if (value1 >= value2)
    {
        return value1 - value2;
    }
    else
    {
        return value2 - value1;
    }
}


#ifdef __cplusplus
}
#endif
