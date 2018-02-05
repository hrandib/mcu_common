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
#ifndef STREAMS_H
#define STREAMS_H

#include <type_traits>
#include <stdint.h>
namespace Mcucpp {
	namespace Io {

		uint8_t* to_lower(uint8_t* str);

		uint8_t* utoa(uint32_t value, uint8_t* bufferEnd, uint8_t base = 10);	//ptr points to the end of buf
		uint8_t* itoa(int32_t value, uint8_t* result, uint8_t base = 10);
		template<typename T>
		inline T* itoa(int32_t value, T* result, uint8_t base = 10)
		{
			static_assert(sizeof(T) == 1, "itoa pointer data type error");
			return (T*)itoa(value, (char*)result, base);
		}

		uint8_t* InsertDot(uint32_t value, uint8_t position, uint8_t* buf);

		enum class System
		{
			hex = 16,
			dec = 10
		};
		enum Adjust
		{
			left, center, right
		};

		enum flags_t
		{

		};
		constexpr flags_t operator|(flags_t f1, flags_t f2)
		{
			return static_cast<flags_t>((uint32_t)f1 | (uint32_t)f2);
		}

		using putfunc_t = void(&)(uint8_t);

		class Ostream;

		template<typename T>
		using pManip_t = Ostream&(*)(Ostream&, T);

		template<typename T>
		struct omanip
		{
		private:
			pManip_t<T> manip_;
			T value_;
		public:
			omanip(pManip_t<T> manip, T value) : manip_(manip), value_(value)
			{	}
			Ostream& operator()(Ostream& os)
			{
				return manip_(os, value_);
			}

		};

		class Ostream
		{
		private:
			using Self = Ostream;

			putfunc_t Put_;
			System numsystem_ = System::dec;
			Adjust adj_ = left;
			uint8_t tabw_ = 4;
			uint8_t fieldw_ = 16;
			uint8_t pointpos_ = 0;
			void Putc(uint8_t ch)
			{
				Put_(ch);
			}
			void Puts(const uint8_t* str)
			{
				while(*str)
					Put_(*str++);
			}
			void Puts(const char* str)
			{
				Puts((const uint8_t*)str);
			}

			void Put(int32_t value)
			{
				constexpr uint8_t maxPrefixSize = 3;
				uint8_t prefix[maxPrefixSize];
				uint8_t* prefixPtr = prefix + maxPrefixSize;
				*--prefixPtr = '\0';
				if(numsystem_ == System::dec)
				{
					if(value < 0)
					{
						value = -value;
						*--prefixPtr = '-';
					}
				}
				else //hex
				{
					*--prefixPtr = 'x';
					*--prefixPtr = '0';
				}
				constexpr uint8_t bufSize = 11;
				uint8_t buf[bufSize];
				uint8_t* str = utoa(value, buf + bufSize, static_cast<uint8_t>(numsystem_));
				uint32_t outputLength = buf + bufSize - str + prefix + maxPrefixSize - prefixPtr - 1;
				if(adj_ == right) Fill(fieldw_ - outputLength);
				if(adj_ == center) Fill((fieldw_ - outputLength) >> 1);
				Puts(prefixPtr);
				Puts(str);
				if(adj_ == center) Fill((fieldw_ - outputLength) >> 1);
				if(adj_ == left) Fill(fieldw_ - outputLength);
			}

		public:
			Ostream(putfunc_t Put) : Put_(Put)
			{	}
			Self& operator<<(Self&(*pf)(Self&))
			{
				return pf(*this);
			}
			template<typename T>
			Self& operator<<(const omanip<T>& manip)
			{
				return manip(*this);
			}
			template<typename T>
			Self& operator<<(T obj)
			{
				Put(obj);
				return *this;
			}
			template<typename T>
			Self& operator<<(T* obj)
			{
				Puts(obj);
				return *this;
			}
			void Fill(uint16_t n, uint8_t sym = ' ')
			{
				while(n--)
					Put_(sym);
			}
			void endl()
			{
				Puts("\r\n");
			}
			void ends()
			{
				Put_('\0');
			}
			void tab()
			{
				uint8_t n = tabw_;
				while(n--)
				{
					Put_(' ');
				}
			}
			void SetTabWidth(uint8_t w)
			{
				tabw_ = w;
			}
			void SetFieldWidth(uint8_t w)
			{
				fieldw_ = w;
			}
			void SetSystem(System ns)
			{
				numsystem_ = ns;
			}
			void AdjustField(Adjust adj)
			{
				adj_ = adj;
			}
		};

		inline Ostream& endl(Ostream& os)
		{
			os.endl();
			return os;
		}
		inline Ostream& ends(Ostream& os)
		{
			os.ends();
			return os;
		}
		inline Ostream& hex(Ostream& os)
		{
			os.SetSystem(System::hex);
			return os;
		}
		inline Ostream& dec(Ostream& os)
		{
			os.SetSystem(System::dec);
			return os;
		}

		inline Ostream& setw(Ostream& os, uint8_t w)
		{
			os.SetFieldWidth(w);
			return os;
		}
		inline omanip<uint8_t> setw(uint8_t w)
		{
			return omanip<uint8_t>(setw, w);
		}
		inline Ostream& setTabw(Ostream& os, uint8_t w)
		{
			os.SetTabWidth(w);
			return os;
		}
		inline omanip<uint8_t> setTabw(uint8_t w)
		{
			return omanip<uint8_t>(setTabw, w);
		}

		namespace Internal {
			uint8_t* utoa(uint32_t value, uint8_t* ptr, uint32_t base = 10);
		}

	}//IO
}//Mcucpp


#endif // STREAMS_H
