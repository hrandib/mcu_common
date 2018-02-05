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

#include "streams.h"
#include <algorithm>
#include <cstring>

namespace Mcucpp {
	namespace Io {

		uint8_t* to_lower(uint8_t* str)
		{
			while(*str)
			{
				*str = std::tolower(*str);
				str++;
			}
			return str;
		}
/*namespace Internal
{
		uint8_t* utoa_rev(uint32_t value, uint8_t* ptr, uint32_t base)
		{
			uint32_t tmp_value;
			// check that the base if valid
			if(base < 2 || base > 16) { *ptr = '\0'; return nullptr; }
			do {
				tmp_value = value;
				value /= base;
				*ptr++ = "0123456789ABCDEF"[tmp_value - value * base];
			} while(value);
			*ptr-- = '\0';
			return ptr;
		}

		uint8_t* utoa(uint32_t value, uint8_t* ptr1, uint32_t base)
		{
			uint8_t *retptr = ptr1, tmp_char;
			uint8_t* ptr = utoa_rev(value, ptr1, base);
			while(ptr1 < ptr)
			{
				tmp_char = *ptr;
				*ptr-- = *ptr1;
				*ptr1++ = tmp_char;
			}
			return retptr;
		}
}*///Internal
		uint8_t* itoa(int32_t value, uint8_t* result, uint8_t base)
		{
			// check that the base if valid
//			if (base < 2 || base > 16) { *result = 0; return result; }

			uint8_t* out = result;
			uint32_t quotient = value < 0 ? -value : value;
			do {
				const uint32_t q = quotient / base;
				const uint32_t rem = quotient - q * base;
				quotient = q;
				*out++ = (rem < 10 ? '0' : 'a' - 10) + rem;
			} while ( quotient );
		// Apply negative sign
			if ( value < 0) *out++ = '-';
			*out-- = '\0';
			uint8_t tmp_char;
			uint8_t* ptr1 = result;
			while(ptr1 < out)
			{
				tmp_char = *out;
				*out-- = *ptr1;
				*ptr1++ = tmp_char;
			}
			return result;
		}

		uint8_t* utoa(uint32_t value, uint8_t* bufferEnd, uint8_t base)
		{
				uint8_t* ptr = bufferEnd;
				do
				{
					uint32_t q = value / base;
					uint32_t rem = value - q * base;
					value = q;
					*--ptr = (rem < 10 ? '0' : 'a' - 10) + rem;
				} while (value != 0);
				return ptr;
		}

		uint8_t* InsertDot(uint32_t value, uint8_t position, uint8_t* buf)
		{
			auto len = strlen((const char*)Io::itoa(value, buf));
			if(len <= position)
			{
				const uint8_t offset = position + 2 - len;
				memmove(buf + offset, buf, len + 1);
				buf[0] = '0';
				buf[1] = '.';
				for(uint8_t x = 2; x < offset; ++x)
				{
					buf[x] = '0';
				}
			}
			else	//length > position
			{
				memmove(buf + len - position + 1, buf + len - position, position + 1);
				buf[len - position] = '.';
			}
			return buf;
		}
	}//IO
}//Mcucpp
