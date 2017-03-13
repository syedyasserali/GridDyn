/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#include "gridEvent.h"
#include "utilities/timeSeries.h"
#include "gridDyn.h"
#include "utilities/units.h"
#include "core/objectInterpreter.h"
#include "utilities/stringOps.h"
#include "core/helperTemplates.h"
#include "core/coreExceptions.h"
#include <sstream>


compoundEventPlayer::compoundEventPlayer()
{

}

compoundEventPlayer::compoundEventPlayer(const std::string &eventName):compoundEvent(eventName)
{

}
compoundEventPlayer::compoundEventPlayer(gridEventInfo &gdEI, coreObject *rootObject):compoundEvent(gdEI,rootObject)
{

}

std::shared_ptr<gridEvent> compoundEventPlayer::clone(std::shared_ptr<gridEvent> ggb) const
{
	auto nE = cloneBaseStack<compoundEventPlayer, compoundEvent, gridEvent>(this, ggb);
	if (!nE)
	{
		return ggb;
	}
	
	return nE;
}


void compoundEventPlayer::setTime(coreTime time)
{
	triggerTime = time;
}

void compoundEventPlayer::setTimeValue(coreTime time, double val)
{
	triggerTime = time;
	value = val;
}

void compoundEventPlayer::setTimeValue(const std::vector<coreTime> &, const std::vector<double> &)
{

	

}

void compoundEventPlayer::set(const std::string &param, double val)
{
	if (param[0] == '#')
	{

	}
	else
	{
		compoundEvent::set(param, val);
	}
}

void compoundEventPlayer::set(const std::string &param, const std::string &val)
{
	if (param[0] == '#')
	{

	}
	else
	{
		compoundEvent::set(param, val);
	}
}
void compoundEventPlayer::updateTrigger(coreTime time)
{
	if (currIndex != kNullLocation)             //we have a file operation
	{
		currIndex++;
		if (currIndex >= ts.size())
		{
			if (period > timeZero)                     //if we have a period loop the time series
			{
				if (time - ts.time(currIndex) > period)
				{
					for (index_t kk = 0; kk < ts.size(); ++kk)
					{
						ts.time(kk) += period + triggerTime;
					}
				}
				else
				{
					for (index_t kk = 0; kk < ts.size(); ++kk)
					{
						ts.time(kk) += period;
					}
				}

				currIndex = 0;
				triggerTime = ts.time(currIndex);
				
			}
			else
			{
				armed = false;
			}
		}
		else                   //just proceed to the next trigger and Value
		{
			triggerTime = ts.time(currIndex);
			
		}
	}
	else                //no file so loop if there is a period otherwise disarm
	{
		if (period > timeZero)
		{
			do
			{
				triggerTime = triggerTime + period;
			} while (time >= triggerTime);
		}
		else
		{
			armed = false;
		}
	}
}


void compoundEventPlayer::initialize()
{

}

std::string compoundEventPlayer::toString()
{
	// @time1[,time2,time3,... |+ period] >[rootobj::obj:]field(units) = val1,[val2,val3,...]
	std::stringstream ss;
	if (eFile.empty())
	{
		ss << '@' << triggerTime;
		if (period > timeZero)
		{
			ss << '+' << period << '|';
		}
		else if (ts.size() > 0)
		{
			for (auto nn = currIndex + 1; nn < ts.size(); ++nn)
			{
				ss << ", " << ts.time(nn);
			}
			ss << "| ";
		}
		else
		{
			ss << " | ";
		}
		ss << fullObjectName(m_obj) << ':' << field;
		if (unitType != gridUnits::defUnit)
		{
			ss << '(' << gridUnits::to_string(unitType) << ')';
		}
		ss << " = " << value;
		if (ts.size() > 0)
		{
			for (size_t nn = currIndex + 1; nn < ts.size(); ++nn)
			{
				//ss << ", " << ts.data[nn];
			}
		}
	}
	else
	{
		ss << fullObjectName(m_obj) << ':' << field;
		if (unitType != gridUnits::defUnit)
		{
			ss << '(' << gridUnits::to_string(unitType) << ')';
		}
		ss << " = <" << eFile;
	
		ss << '>';
	}
	return ss.str();
}

change_code compoundEventPlayer::trigger()
{
	try
	{
		m_obj->set(field, value, unitType);
		return change_code::parameter_change;
	}
	catch (const gridDynException &)
	{
		return change_code::execution_failure;
	}
}

change_code compoundEventPlayer::trigger(coreTime time)
{
	change_code ret = change_code::not_triggered;
	if (time+kSmallTime >= triggerTime)
	{
		try
		{
			m_obj->set(field, value, unitType);
			ret = change_code::parameter_change;
		}
		catch (const gridDynException &)
		{
			ret = change_code::execution_failure;
		}
		updateTrigger(time);
	}
	return ret;
}

bool compoundEventPlayer::setTarget(coreObject *gdo, const std::string &var)
{
	if (!var.empty())
	{
		field = var;
	}
	m_obj = gdo;

	if (m_obj)
	{
		setName(m_obj->getName());
		
	}
	armed = checkArmed();
	return armed;
}

void compoundEventPlayer::loadEventFile(const std::string &fname)
{
	eFile = fname;
	ts.loadFile(eFile);
	
	currIndex = 0;
	if (ts.size() > 0)
	{
		triggerTime = ts.time(0);
	}
		
		
	
}
