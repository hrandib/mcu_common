#include "streams.h"
#include <algorithm>

namespace Mcucpp {
	namespace IO {

		uint8_t* to_lower(uint8_t* str)
		{
			while(*str)
			{
				*str = std::tolower(*str);
				str++;
			}
			return str;
		}
namespace Internal
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
}
		uint8_t* itoa(int32_t value, uint8_t* result, uint8_t base)
		{
			// check that the base if valid
			if (base < 2 || base > 16) { *result = 0; return result; }

			uint8_t* out = result;
			//int quotient = abs(value);
			uint32_t quotient = value < 0 ? -value : value;
			do {
				const int tmp = quotient / base;
				*out = (tmp < 10 ? '0' : 'a' - 10) + quotient - (tmp*base);
				++out;
				quotient = tmp;
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
	}//IO
}//Mcucpp
