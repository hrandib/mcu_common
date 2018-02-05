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
#ifndef NOKIA1XXX_LCD_H
#define NOKIA1XXX_LCD_H

#include "spi.h"
#include "delay.h"
#include "fonts.h"
namespace Mcucpp {
	namespace Nokia {
		enum Texture
		{
			Clear = 0,
			Solid = 0xFF
		};

		namespace Internal {
			struct _1100_base
			{
				constexpr static Spis::BaseConfig config = Spis::DefaultCfg;
				enum InstructionSet
				{
					Reset = 0xE2,
					DisplayOn = 0xAF,
					DisplayOff = 0xAE,
					AllPixelsModeOff = 0xA4,
					AllPixelsModeOn = 0xA5,
					VregOff = 0x28,
					VregOn = 0x2F,
					NormalView = 0xA6,
					InvertedView = 0xA7,
					PageAddress = 0xB0,
					ColumnUpper = 0x10,
					ColumnLower = 0,
					NormalX = 0xA0,
					MirrorX = 0xA1,
					NormalY = 0xC0,
					MirrorY = 0xCF
				};
			};
		}

//------=== Lcd types ===-------

		struct _1200 : public Internal::_1100_base
		{
			constexpr static uint8_t Max_X = 95;
			constexpr static uint8_t Max_Y = 72;
		};

//------=== Main Class ===-----------------------

		template<uint32_t SpiBaseAddr = SPI1_BASE, bool useCs = false, typename LcdType = _1200>
		class Lcd
		{
		protected:
			using Spi = Spis::Spi<SpiBaseAddr, Spis::SendOnly, Spis::Framewidth<Spis::Bits::_9>>;
			struct _Spi : public Spi
			{
				using Cs = typename Spi::MisoPin;
			};
			using Cs = typename std::conditional<useCs, typename _Spi::Cs, Nullpin>::type;
			constexpr static uint16_t dataFlag = 0x0100;

			static uint16_t xPosition_, yPosition_;
			static const Font* defaultFont_;
			static void IncrementPosition(uint16_t x = 1)
			{
				xPosition_ += x;
				while (xPosition_ > LcdType::Max_X)
				{
					xPosition_ -= LcdType::Max_X + 1;
					if(++yPosition_ == LcdType::Max_Y / 8)
					{
						xPosition_ = yPosition_ = 0;
					}
				}
			}
			template<typename T>
			static void WaitForComplete(T)
			{
				while(!Spi::Complete())
					;
			}
			static void WaitForComplete(Nullpin){ }
			static void Send(uint16_t data)
			{
				Cs::Clear();
				Spi::Send(data);
				WaitForComplete(Cs{});
				Cs::Set();
			}
			static void SendData(uint8_t data)
			{
				Send((uint16_t)data | dataFlag);
			}
			static void SendCommand(uint8_t cmd)
			{
				Send(cmd);
			}
		public:
			static void Init()
			{
			// Peripheral;
				Cs::Clear();
				Cs::template SetConfig<Gpio::OutputFast, Gpio::PushPull>();
				using namespace Spis;
				Spi::template Init<LcdType::config, Div::_8>();
			// Display
				delay_ms<10>();
				SendCommand(LcdType::Reset);
				delay_ms<10>();
				SendCommand(LcdType::AllPixelsModeOff); //also PowerSaver Off
				SendCommand(LcdType::VregOn);
				SendCommand(LcdType::DisplayOn);
				Clear();
			}
			static void On()
			{
				SendCommand(LcdType::AllPixelsModeOff);
				SendCommand(LcdType::DisplayOn);
			}
			static void Off()
			{
				SendCommand(LcdType::DisplayOff);
				SendCommand(LcdType::AllPixelsModeOn);
			}

			static void Reset()
			{
				SendCommand(LcdType::Reset);
			}
			static void Home()
			{
				SetXY(0, 0);
			}
			static void Clear()
			{
				Fill(0, 0, 864, false);
			}
			static void Fill(uint8_t x, uint8_t y, uint16_t num, uint8_t texture = Texture::Solid)
			{
				SetXY(x, y, true);
				for(; num; --num)
				{
					SendData(texture);
				}
				SetXY(x, y, true);
			}
			static void SetContrast(uint8_t contrast)
			{
				SendCommand(0x80 | (contrast & 0x1F));
			}
			static void InvertedView(bool invert = true)
			{
				SendCommand(invert ? LcdType::InvertedView : LcdType::NormalView);
			}
			static void AllPixelsOn(bool on = true)
			{
				SendCommand(on ? LcdType::AllPixelsModeOn : LcdType::AllPixelsModeOff);
			}
			static void SetXY(uint8_t x, uint8_t y, bool xInPixel = false)
			{
				if(y >= LcdType::Max_Y / 8) y = 0;
				x *= xInPixel ? 1 : 6;
				SendCommand(LcdType::PageAddress | (y & 0x0F));
				SendCommand(LcdType::ColumnUpper | (x >> 4));
				SendCommand(LcdType::ColumnLower | (x  & 0x0F));
				xPosition_ = x;
				yPosition_ = y;
			}
			static void MirrorX(bool mirror = true)
			{
				SendCommand(mirror ? LcdType::MirrorX : LcdType::NormalX);
			}
			static void MirrorY(bool mirror = true)
			{
				SendCommand(mirror ? LcdType::MirrorY : LcdType::NormalY);
			}
			static void SetFont(const Font& font)
			{
				int32_t diff = (defaultFont_->Height() - font.Height()) >> 3;
				SetXY(xPosition_, yPosition_ + diff, true);
				defaultFont_ = &font;
			}

			static void Draw(const Bitmap& bmap, uint8_t x_ = xPosition_, uint8_t y_ = yPosition_)
			{
				uint16_t x = 0;
				for(uint8_t y = 0; y < (bmap.Height() >> 3); ++y)
				{
					SetXY(x_, y_ + y, true);
					for(; x < (uint16_t)bmap.Width() * (y + 1); ++x)
					{
						SendData(bmap[x]);
					}
				}
				IncrementPosition(bmap.Width());
				SetXY(xPosition_, yPosition_ - ((bmap.Height() >> 3) - 1), true);
			}
			static void Putch(uint8_t ch, const Font& font = *defaultFont_)
			{
				if(ch == '\n')
				{
					SetXY(0, yPosition_ + (font.Height() >> 3));
					return;
				}
				if(ch == '\r') return;
			//adjust height offset
				static uint8_t prevHeight = defaultFont_->Height();
				int32_t diff = (prevHeight - font.Height()) >> 3;
				SetXY(xPosition_, yPosition_ + diff, true);
				prevHeight = font.Height();

			//end of line
				if((LcdType::Max_X - xPosition_) < font.Width())
				{
					SetXY(0, yPosition_ + (font.Height() >> 3));
				}

				const uint8_t* p = font[ch];
				Draw(Bitmap{font.Width(), font.Height(), p});
				//Space between chars
				Draw(Bitmap{1, font.Height(), font5x8[' ']});	//Not portable
			}
			static void Puts(const uint8_t* str, const Font& font = *defaultFont_)
			{
				while(*str) Putch(*str++, font);
			}
			static void Puts(const char* str, const Font& font = *defaultFont_)
			{
				Puts((const uint8_t*)str, font);
			}
			static void NewLine()
			{
				Putch('\n');
			}
		};

		template<uint32_t SpiBaseAddr, bool useCs, typename LcdType>
		uint16_t Lcd<SpiBaseAddr, useCs, LcdType>::xPosition_;
		template<uint32_t SpiBaseAddr, bool useCs, typename LcdType>
		uint16_t Lcd<SpiBaseAddr, useCs, LcdType>::yPosition_;
		template<uint32_t SpiBaseAddr, bool useCs, typename LcdType>
		const Font* Lcd<SpiBaseAddr, useCs, LcdType>::defaultFont_ = &font5x8;

	}
}



#endif // NOKIA1XXX_LCD_H
