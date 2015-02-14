#pragma once
#include "gpio.h"
#include "delay.h"
#include "timers.h"
#include "utils.h"
#include <stdint.h>
#include <limits>

namespace Mcucpp
{
namespace Sensors
{

	template<typename Timer, typename Pin>
	class Am2302
	{
	private:
		static enum State
		{
			Start,
			GetResponse,
			Reading,
			Timeout
		} state;
		enum {
			PollPeriod = 10000,	//ms
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
			uint8_t timer = Timer::ReadCounter();
			Timer::Clear();
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
			uint8_t timer = Timer::ReadCounter();
			Timer::Clear();
 			if (!Pin::IsSet())
			{
				value <<= 1;
				if (timer > Threshold) value |= 0x01;
				bitcount++;
			}
			if (bitcount >= 8)
			{
				bitcount = 0;
				if (index == 0)
				{
					uint8_t sum = valueArray[0] + valueArray[1] + valueArray[2] + valueArray[3];
					if (sum != value)
						valueArray[0] = valueArray[1] = valueArray[2] = valueArray[3] = 0;
					else
						state = Timeout;
						index = 4;
				}
				else valueArray[--index] = value;
			}
		}
		static uint8_t valueArray[4];
	public:
		FORCEINLINE
		static void Init()
		{
			using namespace Gpio;
			Pin::Exti::EnableIRQ(Trigger::BothEdges);
			Pin::template SetConfig<Input, PullUp>();
			using namespace Timers;
			Timer:: template Init<UpCount, (F_CPU * TimerStep) / 1000000UL, 512>(); //step 4us, cycle 2ms
			Timer::EnableIRQ(UpdateIRQ);
			Timer::Enable();
			Pin::Clear();
			Pin::template SetConfig<Gpio::OutputFast, Gpio::OpenDrain>();
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
			Timer::Disable();
			Timer::DisableIRQ(Timers::UpdateIRQ);
			Pin::template SetConfig<Gpio::Input, Gpio::PullUp>();
		}
		FORCEINLINE
		static void ExtiIRQ()
		{
			Pin::Exti::ClearPending();
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
		static void TimerIRQ()
		{
			using namespace Timers;
			Timer::ClearEvent(UpdateEv);
			static uint16_t timeout;
			if (state == Start)
			{
				Pin::template SetConfig<Gpio::Input, Gpio::PullUp>();
				state = GetResponse;
			}
			else if (timeout++ == PollPeriod / 2)
			{
				Pin::template SetConfig<Gpio::OutputFast, Gpio::OpenDrain>();
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
	template<typename Timer, typename Pin>
	typename Am2302<Timer, Pin>::State Am2302<Timer, Pin>::state;

}//AM2302
}//Mcucpp

