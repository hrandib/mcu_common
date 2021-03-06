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
#ifndef HD44780_H
#define HD44780_H

#include "pinlist.h"
#include "delay.h"

namespace Mcucpp {
	enum class LcdType
	{
		KS0066,
		HD44780
	};

/*	const unsigned char CyrTable[64]= {
	0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3, 0xA4, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8,
	0x50, 0x43, 0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0xAD, 0xAF, 0xB0, 0xB1,
	0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0x6F, 0xBE,
	0x70, 0x63, 0xBF, 0x79, 0xE4, 0x78, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7};
*/
	enum class CursorMode
	{
		Off = 0x00,
		On = 0x02,
		Blink = 0x01
	};
	constexpr CursorMode operator|(CursorMode c1, CursorMode c2)
	{
		return static_cast<CursorMode>((uint32_t)c1 | (uint32_t)c2);
	}

	template<typename Databus, typename Rs, typename E, uint32_t lines = 2, LcdType type = LcdType::KS0066>
	class Hd44780
	{
	private:
		enum CommandSet
		{
//			RESET = 0x30, // Команда сброса ЖКД
			BUS_4BIT = 0x02, // 4-х разрядная шина данных
			DISPLAY_ON = 0x04, // Включить дисплей. Комб. с CURSOR_ON и CURSOR_BLINK
			ONE_LINE_MODE = 0x00, // Одна строка
			TWO_LINES_MODE = 0x08, // Две строки
			SET_CURSOR_MODE = 0x08,
			INCREMENT_MODE = 0x06,
			RETURN_HOME = 0x02,
			DISPLAY_CLEAR = 0x01, // Очистка дисплея
			NEXT_LINE = 0x40, // Адрес новой строки
			CGRAM_ADDR = 0x40, // Установка адреса CGRAM
			DDRAM_ADDR = 0x80 // Установка адреса DDRAM

//			DISPLAY_RIGHT = 0x1C, // Сдвинуть дисплей вправо
//			DISPLAY_LEFT = 0x18, // Сдвинуть дисплей влево
//			DISPLAY_SHIFT = 0x05, // Комб. с SHIFTDIR_RIGHT и SHIFTDIR_LEFT
//			CURSOR_RIGHT = 0x14, // Сдвинуть курсор вправо
//			CURSOR_LEFT = 0x10, // Сдвинуть курсор влево
//			SHIFT_RIGHT = 0x06, // Комб. с DISPLAY_SHIFT
//			SHIFT_LEFT = 0x04, // Комб. с DISPLAY_SHIFT
//			BUSY_FLAG = 0x80, // Флаг занятости
		};
		enum { CmdDelay = (type == LcdType::KS0066 ? 50 : 100)}; //in us

		static void Write(uint8_t data)
		{
			Databus::Write(data);
			E::Set();
			delay_us<CmdDelay>();
			E::Clear();
		}
		static void WriteCommand(uint8_t data)
		{
			Rs::Clear();
			Write(data >> 4);
			Write(data);
		}
		static void WriteData(uint8_t data)
		{
			Rs::Set();
			Write(data >> 4);
			Write(data);
		}
		static void InitPins()
		{
			using namespace Gpio;
			Databus::template SetConfig<OutputFast, PushPull>();
			E::template SetConfig<OutputFast, PushPull>();
			Rs::template SetConfig<OutputFast, PushPull>();
			Rs::Clear();
		}
		static void InitDisplay(CursorMode mode)
		{
			if(type == LcdType::HD44780)
			{
				Write(BUS_4BIT | 0x01);
				delay_ms<5>();
				Write(BUS_4BIT | 0x01);
				Write(BUS_4BIT | 0x01);
			}
			else if(type == LcdType::KS0066)
			{
				Write(BUS_4BIT);
			}
			Write(BUS_4BIT);
			Write(lines == 2 ? TWO_LINES_MODE : ONE_LINE_MODE);
			WriteCommand(DISPLAY_ON | SET_CURSOR_MODE | static_cast<uint8_t>(mode));
			Clear();
			WriteCommand(INCREMENT_MODE);
		}
	public:
		static void Init(CursorMode mode = CursorMode::Off)
		{
			InitPins();
			delay_ms<50>();
			InitDisplay(mode);
		}
		static void BuildCustomChar(uint8_t location, const uint8_t* ptr)
		{
			WriteCommand(0x40 + (location * 8));
			for(uint8_t i = 0; i < 8; ++i)
			{
				WriteData(ptr[i]);
			}
		}
		static void PrepareToShutdown()
		{
			Databus::Write(0);
			Rs::Clear();
			E::Clear();
		}

		static void SetPosition(uint8_t x, uint8_t y)
		{
			WriteCommand(DDRAM_ADDR | (y ? NEXT_LINE : 0) | x);
		}
		static void SetPosition(uint8_t x)
		{
			WriteCommand(DDRAM_ADDR | x);
		}
		static void Home()
		{
			WriteCommand(RETURN_HOME);
			delay_ms<2>();
		}
		static void Clear()
		{
			WriteCommand(DISPLAY_CLEAR);
			delay_ms<2>();
		}
		template<typename T>
		static void Putch(T ch)
		{
			static_assert(sizeof(T) == 1, "Data type is not compatible with PutChar func.");
			WriteData((const uint8_t)ch);
		}
		template<typename T>
		static void Puts(const T* s)
		{
			while(*s) Putch(*s++);
		}
	};

	template<typename Lcd, typename T, uint8_t x_dim = 16, uint8_t y_dim = 2>
	static void LcdPutch(T ch)
	{
		static uint8_t cur_x, cur_y;
		if(ch == '\r') return;
		if(ch == '\n' || cur_x >= x_dim)
		{
			cur_x = 0;
			if(y_dim - 1) cur_y = !cur_y ? 1 : 0;
			Lcd::SetPosition(cur_x, cur_y);
		}
		if(ch != '\n')
		{
			Lcd::Putch(ch);
			++cur_x;
		}
	}
	template<typename Lcd, typename T, uint8_t x_dim = 16, uint8_t y_dim = 2>
	static void LcdPuts(const T* s)
	{
		while(*s) LcdPutch<Lcd, T, x_dim, y_dim>(*s++);
	}

}

#endif // HD44780_H
