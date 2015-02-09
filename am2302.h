#pragma once
#include "gpio.h"
#include "delay.h"
#include "timers.h"
#include "utils.h"
#include <stdint.h>

namespace Mcucpp
{
namespace Sensors
{
	enum
	{
		Start,
		GetResponse,
		Reading,
		Timeout
	} state;

	template<typename Timer, typename Pin>
	class Am2302
	{
	private:
		enum {
			PollPeriod = 5000,	//ms
			TimerStep = 4,		//us
			ResponseTimeMin = 72 / TimerStep,
			ResponseTimeMax = 84 / TimerStep,
			Threshold = 50 / TimerStep,
		};  
		static uint8_t index, bitcount, value;
		FORCEINLINE
		static void DetectResponse()
		{
			static uint8_t stagecount;
			uint8_t timer = HardTimer::ReadCounter();
			HardTimer::Clear();
			switch (stagecount)
			{
				case 0: case 1: ++stagecount;
					break;
				case 2: case 3: if (timer >= ResponseTimeMin && timer <= ResponseTimeMax)
				{
					if (Pin::IsSet()) ++stagecount;
					else if (stagecount == 3)
					{
						stagecount = 0;
						index = bitcount = value = 0;
						state = Reading;
					}
				}
				else stagecount = 0;
					break;
			}
		}
		FORCEINLINE
		static void ReadingProcess()
		{
			uint8_t timer = HardTimer::ReadCounter();
			HardTimer::Clear();
 			if (!Pin::IsSet())
			{
				value <<= 1;
				if (timer > Threshold) value |= 0x01;
				bitcount++;
			}
			if (bitcount >= 8)
			{
				bitcount = 0;
				if (index == 4)
				{
					uint8_t sum = valueArray[0] + valueArray[1] + valueArray[2] + valueArray[3];
					if (sum != value)
						valueArray[0] = valueArray[1] = valueArray[2] = valueArray[3] = 0;
					else
						state = Timeout;
						index = 0;
				}
				else valueArray[index++] = value;
			}
		}
		static uint8_t valueArray[4];
	public:
		FORCEINLINE
		static void Init()
		{
			Exti::SetExtIntMode<Pin::Port, Exti::RisingFallingEdge>();
//			Pin::template SetConfig<GpioBase::In_Pullup_int>();
			HardTimer::Init<T4::Div8, T4::ARPE>();			//clock 4us
			HardTimer::EnableInterrupt();
			HardTimer::Enable();
			Pin::Clear();
			Pin::template SetConfig<GpioBase::Out_OpenDrain_fast>();
			state = Start;
		}
		
		FORCEINLINE
		static const uint8_t* GetValues()
		{
			return valueArray;
		}

		FORCEINLINE
		static void DeInit()
		{
			state = Start;
			HardTimer::Disable();
			HardTimer::DisableInterrupt();
			Pin::template SetConfig<GpioBase::In_Pullup>();
		}

		FORCEINLINE
		static void ExtInt()
		{
			switch (state)
			{
			case GetResponse: DetectResponse();
				break;
			case Reading: ReadingProcess();
				break;
			default:
				break;
			}
		}

		FORCEINLINE
		static void TimerInt()
		{
			HardTimer::ClearIntFlag();
			static uint16_t timeout;
			if (state == Start)
			{
				Pin::template SetConfig<GpioBase::In_Pullup_int>();
				state = GetResponse;
			}
			else if (timeout++ == PollPeriod)
			{
				Pin::template SetConfig<GpioBase::Out_OpenDrain_fast>();
				timeout = 0;
				state = Start;
 			}
 		}
	};

	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::valueArray[];
	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::bitcount;
	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::index;
	template<typename Timer, typename Pin>
	uint8_t Am2302<Timer, Pin>::value;

}//AM2302
}//Mcucpp

