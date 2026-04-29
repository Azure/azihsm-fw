// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//----------------------------------------------------
//
//! @file  M7FibersBitmap.h
//! @brief Header file for M7FibersBitmap class
//!
//----------------------------------------------------
#ifndef FP3CORE_SYSTEM_FIBER_M7FIBERSBITMAP_H_
#define FP3CORE_SYSTEM_FIBER_M7FIBERSBITMAP_H_
#pragma once

//----------------------------------------------------
//  Dependencies
//----------------------------------------------------

#include "Mathematics.h"

/**
 * Fibers bitmap class
 *
 * Fibers bitmap is a proxy class that helps to implement functionality of  bitmap manipulating.
 *
 */
class M7FibersBitmap {
public:
    // Constuctors
    M7FibersBitmap() = default;

    // Assignment operator.
    M7FibersBitmap& operator=(const M7FibersBitmap& bitmap);

    // Get the bitmap a value
    uint32_t GetValue() const;

    // Disable fiber in the bitmap.
    void ClearFiberId(uint16_t id);

    // Disable fiber in the bitmap.
    void SetFiberId(uint16_t id);

    // Checks if bitmap is empty.
    bool IsEmpty() const;

    // Checks if fiber bit setted in bitmap.
    bool IsFiberIdSet(uint16_t id) const;

    // Get tralling number of zeros
    uint16_t NextFiberId() const;

    // Get number of Ones in bitmap
    uint32_t GetFibersCount() const;
    static constexpr uint32_t _maxFiberCount = 8;

private:
    // Non default constructor
    explicit M7FibersBitmap(uint32_t val) : _value(val)
    {
    }
    uint32_t _value;
};

/**
 * @brief Assign new bitmap value
 *
 * @param[in] bitmap: The argument bitmap to assignment from
 * @return bitmap
 */
inline M7FibersBitmap& \
    M7FibersBitmap::operator= (const M7FibersBitmap& bitmap)
{
    if (this != &bitmap)
    {
        _value = bitmap._value;
    }

    return *this;
}

/**
 * @brief Get value of this bitmap
 *
 * @return bitmap
 */
inline uint32_t M7FibersBitmap::GetValue() const
{
    return _value;
}

/**
 * @brief Clear bit in this bitmap by fiber Id
 *
 * @param[in] id: Fiber Id
 * @return nothing
 */
inline void M7FibersBitmap::ClearFiberId(uint16_t id)
{
    _value &= ~(1UL << id);
}

/**
 * @brief Set bit in this bitmap by fiber Id
 *
 * @param[in] id: Fiber Id
 * @return nothing
 */
inline void M7FibersBitmap::SetFiberId(uint16_t id)
{
    _value |= (1UL << id);
}

/**
 * @brief Check if no fibers are set
 *
 * @return true if bitmap has all ones
 */
inline bool M7FibersBitmap::IsEmpty() const
{
    return ((_value == 0) ? true : false);
}

/**
 * @brief Checks if fiber ID is set in bitmap.
 *
 * @param[in] id: Fiber Id
 * @return true if fiber is one
 */
inline bool M7FibersBitmap::IsFiberIdSet(uint16_t id) const
{
    return ((_value & (1UL << id)) != 0);
}

/**
 * @brief Returns next fiber ID in the bitmap
 *
 * @return next fiber id
 */
inline uint16_t M7FibersBitmap::NextFiberId() const
{
    // Next Fiber ID in the bitmap
    // is equal to a number of trailing zeroes in the bitmap
    //return CountTrailingZeros(_value) % 32;
    return (CountTrailingZeros(_value) & 0x1fUL);
}

/**
 * @brief Calculates number of fibers in the bitmap
 *
 * @return number of fibers
 */
inline uint32_t M7FibersBitmap::GetFibersCount() const
{
    return (uint32_t)CountNumberOfOnes(_value);
}
#endif  // FP3CORE_SYSTEM_FIBER_M7FIBERSBITMAP_H_
