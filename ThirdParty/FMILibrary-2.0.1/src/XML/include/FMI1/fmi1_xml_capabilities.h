/*
    Copyright (C) 2012 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

     This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    FMILIB_License.txt file for more details.

    You should have received a copy of the FMILIB_License.txt file
    along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#ifndef FMI1_XML_CAPABILITIES_H
#define FMI1_XML_CAPABILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FMI1/fmi1_xml_model_description.h"
	/**
		\file fmi1_xml_capabilities.h
		Functions to retrieve capability flags.
	*/
	/**
	\addtogroup fmi1_xml
	@{
	\addtogroup fmi1_xml_capabilities Functions to retrieve capability flags.
	The functions accept a pointer to ::fmi1_xml_capabilities_t returned by fmi1_xml_get_capabilities().
	They return the flags as specified by the FMI 1.0 standard. Default values are returned for model-exachange FMUs.
	@}
	\addtogroup fmi1_xml_capabilities
	@{
	*/
/** \brief Retrieve  canHandleVariableCommunicationStepSize flag. */
int fmi1_xml_get_canHandleVariableCommunicationStepSize(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canHandleEvents flag. */
int fmi1_xml_get_canHandleEvents(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canRejectSteps flag. */
int fmi1_xml_get_canRejectSteps(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canInterpolateInputs flag. */
int fmi1_xml_get_canInterpolateInputs(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  maxOutputDerivativeOrder. */
unsigned int fmi1_xml_get_maxOutputDerivativeOrder(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canRunAsynchronuously flag. */
int fmi1_xml_get_canRunAsynchronuously(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canSignalEvents flag. */
int fmi1_xml_get_canSignalEvents(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canBeInstantiatedOnlyOncePerProcess flag. */
int fmi1_xml_get_canBeInstantiatedOnlyOncePerProcess(fmi1_xml_capabilities_t* );
	/** \brief Retrieve  canNotUseMemoryManagementFunctions flag. */
int fmi1_xml_get_canNotUseMemoryManagementFunctions(fmi1_xml_capabilities_t* );

/** 
@}
*/

#ifdef __cplusplus
}
#endif
#endif /* FMI1_XML_CAPABILITIES_H */
