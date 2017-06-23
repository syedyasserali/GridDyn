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

#include "sensor.h"
#include "Block.h"
#include "Link.h"  //some special features for links
#include "comms/Communicator.h"
#include "comms/controlMessage.h"
#include "core/coreExceptions.h"
#include "core/coreObjectTemplates.hpp"
#include "events/Event.h"
#include "measurement/Condition.h"
#include "measurement/grabberSet.h"
#include "utilities/matrixDataSparse.hpp"
#include "utilities/matrixDataTranslate.hpp"
#include "utilities/stringConversion.h"
#include "utilities/timeSeries.hpp"

#include <boost/format.hpp>

namespace griddyn
{
using namespace gridUnits;
using namespace stringOps;

sensor::sensor (const std::string &objName) : Relay (objName)
{
    opFlags.set (continuous_flag);
    opFlags.set (late_b_initialize);
    opFlags.reset (no_dyn_states);
}

coreObject *sensor::clone (coreObject *obj) const
{
    auto nobj = cloneBase<sensor, Relay> (this, obj);
    if (nobj == nullptr)
    {
        return obj;
    }

    nobj->m_terminal = m_terminal;
    nobj->inputStrings = inputStrings;
    nobj->outputNames = outputNames;
    nobj->blockInputs = blockInputs;
    nobj->outputs = outputs;
    nobj->outputNames = outputNames;
    nobj->outputMode = outputMode;
    nobj->processStatus = processStatus;
    // clone the dataSources

    for (index_t kk = 0; kk < static_cast<index_t> (dataSources.size ()); ++kk)
    {
        if (dataSources[kk])
        {
            if (static_cast<index_t> (nobj->dataSources.size ()) > kk)
            {
                nobj->dataSources[kk] = dataSources[kk]->clone (nobj->dataSources[kk]);
            }
            else
            {
                nobj->dataSources.push_back (dataSources[kk]->clone ());
            }
        }
        else
        {
            if (static_cast<index_t> (nobj->dataSources.size ()) <= kk)
            {
                nobj->dataSources.push_back (nullptr);
            }
        }
    }

    for (index_t kk = 0; kk < static_cast<index_t> (outGrabbers.size ()); ++kk)
    {
        if (outGrabbers[kk])
        {
            if (static_cast<index_t> (nobj->outGrabbers.size ()) > kk)
            {
                nobj->outGrabbers[kk] = outGrabbers[kk]->clone (nobj->outGrabbers[kk]);
                nobj->outGrabbers[kk]->updateObject (nobj);
            }
            else
            {
                nobj->outGrabbers.push_back (outGrabbers[kk]->clone ());
                nobj->outGrabbers[kk]->updateObject (nobj);
            }
        }
        else
        {
            if (static_cast<index_t> (nobj->outGrabbers.size ()) <= kk)
            {
                nobj->outGrabbers.push_back (nullptr);
            }
        }
    }

    return nobj;
}

void sensor::add (coreObject *obj)
{
    if (dynamic_cast<Block *> (obj)!=nullptr)
    {
        add (static_cast<Block *> (obj));
    }
    else
    {
        Relay::add (obj);
    }
}

void sensor::add (Block *blk)
{
    if (blk->locIndex != kNullLocation)
    {
        ensureSizeAtLeast (filterBlocks, blk->locIndex + 1, static_cast<Block *>(nullptr));
        filterBlocks[blk->locIndex] = blk;
    }
    else
    {
        filterBlocks.push_back (blk);
    }
    ensureSizeAtLeast (blockInputs, filterBlocks.size (), -1);
    addSubObject (blk);
}

void sensor::add (std::shared_ptr<grabberSet> dGr)
{
	if (dGr)
	{
		auto cnum = inputStrings.size();
		dataSources.resize(cnum + 1);
		inputStrings.emplace_back("#");

		if (!(dGr->stateCapable()))
		{
			opFlags.set(continuous_flag, false);
		}
		dataSources[cnum] = std::move(dGr);
	}
    
}

void sensor::add (std::shared_ptr<gridGrabber> dGr)
{
    add (std::make_shared<grabberSet> (std::move (dGr), nullptr));
}

std::shared_ptr<grabberSet> sensor::getGrabberSet (index_t grabberNum)
{
    if (grabberNum < static_cast<index_t> (dataSources.size ()))
    {
        return dataSources[grabberNum];
    }
    throw (std::out_of_range ("invalid index"));
}

void sensor::setFlag (const std::string &flag, bool val)
{
    if ((flag == "direct_io") || (flag == "direct"))
    {
        opFlags.set (direct_IO, val);
    }
    else if (flag == "sampled")
    {
        opFlags[sampled_only] = val;
        opFlags[continuous_flag] = !val;
    }
    else if (flag == "continuous")
    {
        opFlags[sampled_only] = !val;
        opFlags[continuous_flag] = val;
    }
    else
    {
        Relay::setFlag (flag, val);
    }
}

void sensor::set (const std::string &param, const std::string &val)
{
    std::string iparam;
    int num = trailingStringInt (param, iparam);
    if (iparam == "input")
    {
        auto sp = splitline (val);
        trim (sp);
        if (num >= 0)
        {
            if (num + sp.size () - 1 >= inputStrings.size ())
            {
                inputStrings.resize (num + sp.size ());
            }
            for (auto &istr : sp)
            {
                inputStrings[num] = istr + '_';
                ++num;
            }
        }
        else
        {
            inputStrings.reserve (inputStrings.size () + sp.size ());
            for (auto &istr : sp)
            {
                inputStrings.push_back (istr);
            }
        }
        if (opFlags[direct_IO])
        {
            if (num >= 0)
            {
                if (num + sp.size () - 1 >= outputNames.size ())
                {
                    outputNames.resize (num + sp.size ());
                }
                for (auto &istr : sp)
                {
                    outputNames[num] = getTailString (istr, ':');
                    ++num;
                }
            }
            else
            {
                outputNames.reserve (outputNames.size () + sp.size ());
                for (auto &istr : sp)
                {
                    outputNames.push_back (getTailString (istr, ':'));
                }
            }
        }
    }
    else if (param == "condition")
    {
        add (std::shared_ptr<Condition> (make_condition (val, this)));
    }
    else if (iparam == "filter")
    {
        auto blk = make_block (val);
        if (blk)
        {
            if (num >= 0)
            {
                blk->locIndex = num;
            }
            add (blk.release ());
        }
        else
        {
            throw (invalidParameterValue (param));
        }
    }
    else if ((iparam == "outputname") || (param == "outputnames"))
    {
        if (num >= 0)
        {
            if (num >= static_cast<int> (outputs.size ()))
            {
                outputNames.resize (num + 1);
            }
            outputNames[num] = val;
        }
        else
        {
            auto sep = splitline (val);
            trim (sep);
            if (outputNames.empty ())
            {
                outputNames = sep;
            }
            else
            {
                for (auto &nm : sep)
                {
                    outputNames.push_back (nm);
                }
            }
        }
    }
    else if ((iparam == "output") || (param == "outputs"))
    {
        auto el = val.find_first_of ('=');
        std::string v2 = (el != std::string::npos) ? val.substr (0, el) : val;
        std::string sval = (el != std::string::npos) ? val.substr (el + 1) : "";

        if (num >= 0)
        {
            if (num >= static_cast<int> (outputs.size ()))
            {
                outputs.resize (num + 1, -1);
                outputNames.resize (num + 1);

                outputMode.resize (num + 1, outputMode_t::block);
                outGrabbers.resize (num + 1);
            }
            if (!sval.empty ())
            {
                outputNames[num] = sval;
            }
            auto outputValue = numeric_conversionComplete<int> (v2, -1);
            if (outputValue >= 0)
            {
                outputs[num] = outputValue;
                outputMode[num] = outputMode_t::block;
            }
            else
            {
                int knum = trailingStringInt (v2, sval, 0);
                if (sval == "input")
                {
                    outputs[num] = knum;
                    outputMode[num] = outputMode_t::direct;
                }
                else if (sval == "block")
                {
                    outputs[num] = knum;
                    outputMode[num] = outputMode_t::block;
                }
                else
                {
                    outGrabbers[num] = std::make_shared<grabberSet> (v2, this);

                    outputMode[num] = outputMode_t::processed;
                }
            }
        }
        else
        {
            auto sep = splitline (val);
            trim (sep);
            auto ns = sep.size ();
            outputs.resize (ns, -1);
            outputMode.resize (ns, outputMode_t::block);
            outGrabbers.resize (ns);
            int pp = 0;
            for (auto &v : sep)
            {
                auto outNum = numeric_conversion<int> (v, -1);
                if (outNum >= 0)
                {
                    outputs[pp] = outNum;
                    outputMode[pp] = outputMode_t::block;
                }
                else
                {
                    int knum = trailingStringInt (v, sval, 0);
                    if (sval == "input")
                    {
                        outputs[pp] = knum;
                        outputMode[pp] = outputMode_t::direct;
                    }
                    else if (sval == "block")
                    {
                        outputs[pp] = knum;
                        outputMode[pp] = outputMode_t::block;
                    }
                    else
                    {
                        outGrabbers[pp] = std::make_shared<grabberSet> (v, this);

                        outputMode[pp] = outputMode_t::processed;
                    }
                }
                ++pp;
            }
        }
    }
    else if ((iparam == "blockinput") || (iparam == "process"))
    {
        auto seq = str2vector<int> (val, -1, ",: ");
        if (num >= 0)
        {
            if (seq.size () == 2u)
            {
                ensureSizeAtLeast (blockInputs, seq[1] + 1, -1);
                blockInputs[seq[1]] = seq[0];
            }
            else
            {
                throw (invalidParameterValue (param));
            }
        }
        else
        {
            ensureSizeAtLeast (blockInputs, seq.size (), -1);
            for (index_t kk = 0; kk < static_cast<index_t> (seq.size ()); ++kk)
            {
                blockInputs[kk] = seq[kk];
            }
        }
    }
    else
    {
        Relay::set (param, val);
    }
}

void sensor::set (const std::string &param, double val, gridUnits::units_t unitType)
{
    std::string iparam;
    int num = trailingStringInt (param, iparam, -1);
    if (param == "terminal")
    {
        m_terminal = static_cast<int> (val);
    }
    else if ((iparam == "blockinput") || (iparam == "process"))
    {
        if (num < 0)
        {
            blockInputs.push_back (static_cast<int> (val));
        }
        else
        {
            ensureSizeAtLeast (blockInputs, num + 1, -1);
            blockInputs[num] = static_cast<int> (val);
        }
    }
    else if (param == "direct")
    {
        opFlags.set (direct_IO, (val > 0));
    }
    else if (iparam == "output")
    {
        if (static_cast<int> (val) < 0)
        {
            throw (invalidParameterValue (param));
        }
        else
        {
            if (num < 0)
            {
                outputs.push_back (static_cast<int> (val));
            }
            else
            {
                if (num < static_cast<int> (outputs.size ()))
                {
                    outputs[num] = static_cast<int> (val);
                }
            }
        }
    }
    else
    {
        Relay::set (param, val, unitType);
    }
}

double sensor::get (const std::string &param, gridUnits::units_t unitType) const
{
    std::string iparam;
    int num = trailingStringInt (param, iparam, -1) - 1;
    double ret = kNullVal;
    if (iparam == "output")
    {
        if (num < 0)
        {
            num = 0;
        }
		
        if (num < static_cast<int> (outputs.size ()))
        {
            switch (outputMode[num])
            {
            case outputMode_t::block:
                if (outputs[num] >= 0)
                {
                    ret = filterBlocks[outputs[num]]->getOutput ();
                }
                break;
            case outputMode_t::processed:
                if (outGrabbers[num])
                {
                    ret = outGrabbers[num]->grabData ();
                }
                break;
            case outputMode_t::direct:
                if (outputs[num]>=0)
                {
                    ret = dataSources[outputs[num]]->grabData ();
                }
                break;
            default:
                ret = kNullVal;
                break;
            }
        }

        return ret;
    }

        for (size_t kk = 0; kk < outputNames.size (); ++kk)
        {
            if (param == outputNames[kk])
            {
                if (kk < outputMode.size ())
                {
                    switch (outputMode[kk])
                    {
                    case outputMode_t::block:
                        if (outputs[kk] >= 0)
                        {
                            ret = filterBlocks[outputs[kk]]->getOutput ();
                        }
                        break;
                    case outputMode_t::processed:
                        if (outGrabbers[kk])
                        {
                            ret = outGrabbers[kk]->grabData ();
                        }
                        break;
                    case outputMode_t::direct:
                        if (outputs[kk] >= 0)
                        {
                            ret = dataSources[outputs[kk]]->grabData ();
                        }
                        break;
                    default:
                        ret = kNullVal;
                        break;
                    }
                }
                return ret;
            }
        }
        if (param == "terminal")
        {
            ret = static_cast<double> (m_terminal);
        }
        else
        {
            ret = Relay::get (param, unitType);
        }

    return ret;
}

void sensor::updateObject (coreObject *obj, object_update_mode mode)
{
    for (auto &ds : dataSources)
    {
        if (ds)
        {
            ds->updateObject (obj, mode);
        }
    }

    Relay::updateObject (obj, mode);
}

void sensor::getObjects (std::vector<coreObject *> &objects) const
{
    for (auto &ds : dataSources)
    {
        if (ds)
        {
            ds->getObjects (objects);
        }
    }

    Relay::getObjects (objects);
}

void sensor::generateInputGrabbers ()
{
    auto iSize = static_cast<int> (inputStrings.size ());
    ensureSizeAtLeast (dataSources, iSize);
    for (int ii = 0; ii < iSize; ++ii)
    {
        if (inputStrings[ii][0] == '#')  // escape hatch for previously loaded grabbers
        {
            continue;
        }
        auto istr = inputStrings[ii];
        if (istr.back () == '_')
        {
            istr.pop_back ();
        }
        auto cloc = istr.find_first_of (':');

        if (cloc == std::string::npos)
        {  // if there is a colon assume the input is fully specified
            if ((opFlags[link_type_source]) && (isdigit (istr.back ())==0))
            {
                if (m_terminal > 0)
                {
                    appendInteger (istr, m_terminal);
                }
            }
        }
        coreObject *target_obj = (m_sourceObject!=nullptr) ? m_sourceObject : getParent ();

        dataSources[ii] = std::make_shared<grabberSet> (istr, target_obj, opFlags[sampled_only]);
    }
}

void sensor::receiveMessage (std::uint64_t sourceID, std::shared_ptr<commMessage> message)
{
    using namespace comms;
    std::shared_ptr<controlMessage> m = std::dynamic_pointer_cast<controlMessage> (message);

    double val;

    switch (m->getMessageType ())
    {
    case controlMessage::SET:
        // only local set
        try
        {
            set (convertToLowerCase (m->m_field), m->m_value, gridUnits::getUnits (m->m_units));
            if (!opFlags[no_message_reply])  // unless told not to respond return with the
            {
                auto gres = std::make_unique<controlMessage> (controlMessage::SET_SUCCESS);
                gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
                commLink->transmit (sourceID, std::move (gres));
            }
        }
        catch (const std::invalid_argument &)
        {
            if (!opFlags[no_message_reply])  // unless told not to respond return with the
            {
                auto gres = std::make_unique<controlMessage> (controlMessage::SET_FAIL);
                gres->m_actionID = (m->m_actionID > 0) ? m->m_actionID : instructionCounter;
                commLink->transmit (sourceID, std::move (gres));
            }
        }

        break;
    case controlMessage::GET:
    {
        val = get (convertToLowerCase (m->m_field), gridUnits::getUnits (m->m_units));
        auto reply = std::make_unique<controlMessage> (controlMessage::GET_RESULT);
        reply->m_field = m->m_field;
        reply->m_value = val;
        reply->m_time = prevTime;
        commLink->transmit (sourceID, std::move (reply));

        break;
    }
    case controlMessage::SET_SUCCESS:
    case controlMessage::SET_FAIL:
    case controlMessage::GET_RESULT:
    case controlMessage::SET_SCHEDULED:
    case controlMessage::GET_SCHEDULED:
    case controlMessage::CANCEL_FAIL:
    case controlMessage::CANCEL_SUCCESS:
    case controlMessage::GET_RESULT_MULTIPLE:
        break;
    case controlMessage::CANCEL:

        break;
    case controlMessage::GET_MULTIPLE:
    {
        auto reply = std::make_unique<controlMessage> (controlMessage::GET_RESULT_MULTIPLE);
        reply->multiValues.resize (0);
        reply->multiFields = m->multiFields;
        for (const auto &fieldName : m->multiFields)
        {
            val = get (convertToLowerCase (fieldName), gridUnits::getUnits (m->m_units));
            m->multiValues.push_back (val);
        }
        reply->m_time = prevTime;
        commLink->transmit (sourceID, std::move (reply));
        break;
    }
    case controlMessage::GET_PERIODIC:
        break;
    default:
        break;
    }
}

const static IOdata kNullVec;

double sensor::getBlockOutput (const stateData &sD, const solverMode &sMode, index_t blockNumber) const
{
    double ret = kNullVal;
    if (isLocal (sMode))
    {
		if (isValidIndex(blockNumber,filterBlocks))
        {
            ret = filterBlocks[blockNumber]->getOutput ();
        }
    }
    else
    {
		if (isValidIndex(blockNumber, filterBlocks))
        {
            ret = filterBlocks[blockNumber]->getOutput (kNullVec, sD, sMode);
        }
    }
    return ret;
}

double sensor::getInput (const stateData &sD, const solverMode &sMode, index_t inputNumber) const
{
    double ret = kNullVal;
    if (isLocal (sMode))
    {

		if (isValidIndex(inputNumber,dataSources))
        {
            ret = dataSources[inputNumber]->grabData ();
        }
    }
    else
    {
		if (isValidIndex(inputNumber, dataSources))
        {
            ret = dataSources[inputNumber]->grabData (sD, sMode);
        }
    }
    return ret;
}

void sensor::dynObjectInitializeA (coreTime time0, std::uint32_t flags)
{
    for (auto fb : filterBlocks)
    {
        if (fb!=nullptr)
        {
            fb->dynInitializeA (time0, flags);
        }
    }
    if (dynamic_cast<Link *> (m_sourceObject)!=nullptr)
    {
        opFlags.set (link_type_source);
    }

    if (dynamic_cast<Link *> (m_sinkObject)!=nullptr)
    {
        opFlags.set (link_type_sink);
    }
    generateInputGrabbers ();
    // check if we need to go to sampled mode
    if (opFlags[continuous_flag])
    {
        for (auto &dgs : dataSources)
        {
            if ((dgs) && (!dgs->stateCapable ()))
            {
                // TODO::PT Make it so it can partially support continuous sensors
                opFlags.set (continuous_flag, false);
                LOG_WARNING ("not all data sources support continuous operation , reverting to sampled mode");
                break;
            }
        }
    }

    if (!opFlags[continuous_flag])
    {
        opFlags.set (sampled_only);
        for (auto &fb : filterBlocks)
        {
            fb->setFlag ("sampled_only", true);
        }
    }

    return Relay::dynObjectInitializeA (time0, flags);
}

void sensor::dynObjectInitializeB (const IOdata & /*inputs*/,
                                   const IOdata & /*desiredOutput*/,
                                   IOdata & /*fieldSet*/)
{
    if (filterBlocks.empty ())  // no filter blocks, use direct output
    {
        opFlags.set (direct_IO);
    }
    for (size_t kk = 0; kk < filterBlocks.size (); ++kk)
    {
        if (blockInputs[kk] < 0)
        {
            if (dataSources.size () > kk)
            {
                blockInputs[kk] = static_cast<int> (kk);
            }
            else if (dataSources.size () == 1)
            {
                blockInputs[kk] = 0;
            }
            else
            {
                LOG_WARNING ("unable to uniquely determine filter block input");
            }
        }
    }

    if (outputs.empty ())
    {
        if (opFlags[direct_IO])
        {
            for (int kk = 0; kk < static_cast<int> (dataSources.size ()); ++kk)
            {
                outputs.push_back (kk);
                outputMode.push_back (outputMode_t::direct);
                outGrabbers.push_back (nullptr);
            }
        }
        else
        {
            for (int kk = 0; kk < static_cast<int> (filterBlocks.size ()); ++kk)
            {
                outputs.push_back (kk);
                outputMode.push_back (outputMode_t::block);
                outGrabbers.push_back (nullptr);
            }
        }
    }

    // initialization of the blocks
    for (size_t kk = 0; kk < filterBlocks.size (); ++kk)
    {
        double cv = dataSources[blockInputs[kk]]->grabData ();
        // make sure the process can be handled in states

        IOdata out (1);
        IOdata in{cv};
        IOdata outset;
        filterBlocks[kk]->dynInitializeB (in, outset, out);
    }
    // TODO:: PT fill in the field vector
}

void sensor::updateA (coreTime time)
{
    if (time >= m_nextSampleTime)
    {
        auto blks = static_cast<index_t> (filterBlocks.size ());
        for (index_t kk = 0; kk < blks; ++kk)
        {
            double inputFB = dataSources[blockInputs[kk]]->grabData ();
            // make sure the process can be handled in states
            // printf("%f block %d input=%f\n ", static_cast<double>(time), static_cast<int>(kk), inputFB);
            filterBlocks[kk]->step (time, inputFB);
        }
    }
    Relay::updateA (time);
}

void sensor::timestep (coreTime time, const IOdata &inputs, const solverMode &sMode)
{
    auto blks = static_cast<index_t> (filterBlocks.size ());
    for (index_t kk = 0; kk < blks; ++kk)
    {
        double inputFB = dataSources[blockInputs[kk]]->grabData ();
        // make sure the process can be handled in states
        filterBlocks[kk]->step (time, inputFB);
    }
    Relay::timestep (time, inputs, sMode);
}

void sensor::jacobianElements (const IOdata & /*inputs*/,
                               const stateData &sD,
                               matrixData<double> &md,
                               const IOlocs & /*inputLocs*/,
                               const solverMode &sMode)
{
    if (stateSize (sMode) > 0)
    {
        matrixDataSparse<double> d2, dp;
        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            // TODO::this needs some help for performance and organization
            double inputFB = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            d2.clear ();
            if (dataSources[blockInputs[kk]]->hasJacobian ())
            {
                dataSources[blockInputs[kk]]->outputPartialDerivatives (sD, d2, sMode);
            }

            // make sure the process can be handled in states

            if (d2.size () == 0)
            {
                filterBlocks[kk]->jacElements (inputFB, 0, sD, md, kNullLocation, sMode);
            }
            else
            {
                filterBlocks[kk]->jacElements (inputFB, 0, sD, dp, 0, sMode);
                dp.cascade (d2, 0);
                md.merge (dp);
            }
        }
    }
}

void sensor::residual (const IOdata & /*inputs*/, const stateData &sD, double resid[], const solverMode &sMode)
{
    if (stateSize (sMode) > 0)
    {
        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            double inputFB = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            // make sure the process can be handled in states
            filterBlocks[kk]->residElements (inputFB, 0, sD, resid, sMode);
        }
    }
}

void sensor::algebraicUpdate (const IOdata & /*inputs*/,
                              const stateData &sD,
                              double update[],
                              const solverMode &sMode,
                              double /*alpha*/)
{
    if (algSize (sMode) > 0)
    {
        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            double inputFB = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            // make sure the process can be handled in states
            filterBlocks[kk]->algElements (inputFB, sD, update, sMode);
        }
    }
}

void sensor::derivative (const IOdata & /*inputs*/, const stateData &sD, double deriv[], const solverMode &sMode)
{
    if (diffSize (sMode) > 0)
    {
        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            double inputFB = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            // make sure the process can be handled in states
            filterBlocks[kk]->derivElements (inputFB, 0, sD, deriv, sMode);
        }
    }
}

index_t sensor::lookupOutput (const std::string &outName) const
{
    for (index_t kk = 0; kk < static_cast<index_t> (outputNames.size ()); ++kk)
    {
        if (outName == outputNames[kk])
        {
            return kk;
        }
    }
    return kNullLocation;
}

IOdata sensor::getOutputs (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode) const
{
    IOdata out (outputs.size ());
    for (size_t pp = 0; pp < outputs.size (); ++pp)
    {
        switch (outputMode[pp])
        {
        case outputMode_t::block:
            out[pp] = filterBlocks[outputs[pp]]->getOutput (noInputs, sD, sMode);
            break;
        case outputMode_t::processed:
            out[pp] = outGrabbers[pp]->grabData (sD, sMode);
            break;
        case outputMode_t::direct:
            out[pp] = dataSources[outputs[pp]]->grabData ();
            break;
        default:
            out[pp] = kNullVal;
            break;
        }
    }
    return out;
}

double
sensor::getOutput (const IOdata & /*inputs*/, const stateData &sD, const solverMode &sMode, index_t outNum) const
{
    double out = kNullVal;
    if (outNum >= static_cast<index_t> (outputMode.size ()))
    {
        return out;
    }
    switch (outputMode[outNum])
    {
    case outputMode_t::block:
        out = filterBlocks[outputs[outNum]]->getOutput (noInputs, sD, sMode);
        break;
    case outputMode_t::processed:
        out = outGrabbers[outNum]->grabData (sD, sMode);
        break;
    case outputMode_t::direct:
        out = dataSources[outputs[outNum]]->grabData (sD, sMode);
        break;
    default:
        break;
    }

    // printf("sensors %d, output %d val=%f\n", getUserID(),outNum, out);
    return out;
}

double sensor::getOutput (index_t outNum) const
{
    double out = kNullVal;
    if (outNum >= static_cast<index_t> (outputMode.size ()))
    {
        return out;
    }
    switch (outputMode[outNum])
    {
    case outputMode_t::block:
        out = filterBlocks[outputs[outNum]]->getOutput ();
        break;
    case outputMode_t::processed:
        out = outGrabbers[outNum]->grabData ();
        break;
    case outputMode_t::direct:
        out = dataSources[outputs[outNum]]->grabData ();
        break;
    default:
        break;
    }

    // printf("sensors %d, output %d val=%f\n", getUserID(),num, out);
    return out;
}

index_t sensor::getOutputLoc (const solverMode &sMode, index_t outNum) const
{
    if (outNum >= static_cast<index_t> (outputMode.size ()))
    {
        return kNullLocation;
    }
    switch (outputMode[outNum])
    {
    case outputMode_t::block:
        return filterBlocks[outputs[outNum]]->getOutputLoc (sMode);
    case outputMode_t::processed:
    case outputMode_t::direct:
    default:
        return kNullLocation;
    }
}

// TODO:: PT This is a somewhat complicated function still need to work on it
void sensor::outputPartialDerivatives (const IOdata & /*inputs*/,
                                       const stateData &sD,
                                       matrixData<double> &md,
                                       const solverMode &sMode)
{
    matrixDataTranslate<3, double> aDT (md);
    for (index_t pp = 0; pp < static_cast<index_t> (outputs.size ()); ++pp)
    {
        switch (outputMode[pp])
        {
        case outputMode_t::block:
            aDT.setTranslation (0, pp);
            filterBlocks[outputs[pp]]->outputPartialDerivatives (noInputs, sD, aDT, sMode);
            break;
        case outputMode_t::processed:
            // out[pp] = outGrabbers[pp]->grabData(sD, sMode);
            break;
        case outputMode_t::direct:
            // out[pp] = dataSources[outputs[pp]]->grabData();
            break;
        default:
            // out[pp] = kNullVal;
            break;
        }
    }
}

void sensor::rootTest (const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode)
{
    Relay::rootTest (inputs, sD, roots, sMode);
    if (stateSize (sMode) > 0)
    {
        IOdata localInputs (1);

        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            localInputs[0] = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            // printf("%d root test:%f block %d input=%f\n ", filterBlocks[kk]->getUserID(),
            // static_cast<double>(sD.time), static_cast<int>(kk), localInputs[0]);  make sure the process can be
            // handled in states
            filterBlocks[kk]->rootTest (localInputs, sD, roots, sMode);
        }
    }
}

void sensor::rootTrigger (coreTime time,
                          const IOdata &inputs,
                          const std::vector<int> &rootMask,
                          const solverMode &sMode)
{
    Relay::rootTrigger (time, inputs, rootMask, sMode);
    if (stateSize (sMode) > 0)
    {
        IOdata localInputs (1);

        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            localInputs[0] = dataSources[blockInputs[kk]]->grabData ();
            // make sure the process can be handled in states
            // printf("rootTrigger:%f block %d input=%f\n ", static_cast<double>(time), static_cast<int>(kk),
            // localInputs[0]);
            filterBlocks[kk]->rootTrigger (time, localInputs, rootMask, sMode);
        }
    }
}

change_code
sensor::rootCheck (const IOdata &inputs, const stateData &sD, const solverMode &sMode, check_level_t level)
{
    change_code ret = Relay::rootCheck (inputs, sD, sMode, level);
    if (stateSize (sMode) > 0)
    {
        IOdata localInputs (1);

        auto blks = filterBlocks.size ();
        for (size_t kk = 0; kk < blks; ++kk)
        {
            localInputs[0] = dataSources[blockInputs[kk]]->grabData (sD, sMode);
            // printf("%d root check:%f block %d input=%f\n ", filterBlocks[kk]->getUserID(),
            // static_cast<double>(sD.time), static_cast<int>(kk), localInputs[0]);  make sure the process can be
            // handled in states
            auto iret = filterBlocks[kk]->rootCheck (localInputs, sD, sMode, level);
            ret = std::max (ret, iret);
        }
    }
    return ret;
}
}  // namespace griddyn