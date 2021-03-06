/*
 * Copyright (c) 2015 Dmytro Shestakov
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef MCUCPP_UTILS
#define MCUCPP_UTILS
	#include <stdint.h>

	#ifndef CONCAT
	#define CONCAT2(First, Second) (First ## Second)
	#define CONCAT(First, Second) CONCAT2(First, Second)
	#endif

	#ifdef __cplusplus

		#if __cplusplus > 199711L // check for c++11
			#define STATIC_ASSERT(expr) static_assert((expr), "Static assertion failed")
		#else
		namespace Mcucpp
		{
			template<bool> struct StaticAssertionFailed;
			template<> struct StaticAssertionFailed<true> {};
			template<int> struct StaticAssertionTest{};
		}
			#define STATIC_ASSERT(expr) typedef ::Mcucpp::StaticAssertionTest<sizeof(::Mcucpp::StaticAssertionFailed<(expr)>)> (CONCAT(static_assert_failed_at_line_, __LINE__))
		#define nullptr NULL
		#endif

	#else
		#define STATIC_ASSERT(expr) typedef char CONCAT(static_assert_failed_at_line_, __LINE__) [(expr) ? 1 : -1]
	#endif
#ifdef __GNUC__
#define FORCEINLINE [[gnu::always_inline]]
#elif __ICCARM__
#define FORCEINLINE _Pragma("inline=forced")
#endif
//Create IRQ function after class template specialization
#define DECLAREIRQ(class, irqname)	extern "C" void irqname##_IRQHandler() { class::Irq(); }

#define USINGBASEFUNC_NORET(baseclass, func, T)	template<T arg> inline static void func() { baseclass:: template func<arg>(); }
#define USINGBASEFUNC(baseclass, ret, func, T) template<T arg> inline static ret func() { return baseclass:: template func<arg>(); }

#include <cstring>
#include <cctype>
#include <algorithm>

namespace Mcucpp
{
	enum class DataFormat
	{
		Bin8,
		Bin16,
		Bin32,
		Hex8,
		Hex16,
		Hex32,
		Dec
	};

	template<uint32_t v>
	struct Int2Type
	{
		enum{ value = v	};
	};

	namespace PrivateUtils
	{
		uint8_t* utoa_rev(uint32_t value, uint8_t* ptr, uint32_t base);

		template<DataFormat format>
		struct UtoaTraits;
		template<> struct UtoaTraits<DataFormat::Bin8>
		{
			enum
			{
				asize = 8 + 1,
				base = 2
			};
		};
		template<> struct UtoaTraits<DataFormat::Bin16>
		{
			enum
			{
				asize = 16 + 1,
				base = 2
			};
		};
		template<> struct UtoaTraits<DataFormat::Bin32>
		{
			enum
			{
				asize = 32,
				base = 2
			};
		};
		template<> struct UtoaTraits<DataFormat::Hex8>
		{
			enum
			{
				asize = 2 + 1,
				base = 16
			};
		};
		template<> struct UtoaTraits<DataFormat::Hex16>
		{
			enum
			{
				asize = 4 + 1,
				base = 16
			};
		};
		template<> struct UtoaTraits<DataFormat::Hex32>
		{
			enum
			{
				asize = 8 + 1,
				base = 16
			};
		};
		template<> struct UtoaTraits<DataFormat::Dec>
		{
			enum
			{
				asize = 10 + 1,
				base = 10
			};
		};
	}

	template<uint32_t mask>
	struct Mask2Position
	{
		enum{ value = Mask2Position<(mask >> 1)>::value + 1 };
	};
	template<>
	struct Mask2Position<0x01>
	{
		enum{ value = 0 };
	};

	uint32_t Populate(uint8_t x);
}//Mcucpp
#endif
