/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2016, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef GRIDCORELIST_H_

#define GRIDCORELIST_H_
//include the autogenerated config file

#include "gridCore.h"

//library for printf debug statements
//#include <cstdio>
//#include <fstream>
//#include <set>
/*
#include <unordered_map>
#include <map>
*/
/** @brief list class that facilitates adding or searching for pointers to gridCoreObjects by name or id
* a list class that stores a list of the objects contained in an unordered map to faciliate searching for objects
*/
/*
class gridCoreList
{
private:
  typedef std::unordered_map<std::string, gridCoreObject *> obMapStr;            //!< typedef for the map
  typedef std::unordered_map<index_t, gridCoreObject *> obMapOID;            //!< typedef for the map
  typedef std::multimap<index_t, gridCoreObject *> obMapUID;            //!< typedef for the map of userIDS
  obMapStr m_oblistStr;            //!<the object map structure
  obMapOID m_oblistID;             //!<the object map by id
  obMapUID m_oblistUID;             //!<the object map by user id
public:
*/
/** @brief default constructor*/
// gridCoreList () const;
/**
* @brief function to insert an object into the class
* @param[in] obj the object to insert
* @param[in] replace an optional indicator telling whether to replace the object or nor
* @return a bool indicating successful insertion
*/
//  bool insert (gridCoreObject *obj, bool replace = false);
/** @brief remove an object
* function to remove an object from the container
* @param[in] obj the object to insert
* @return a bool indicating successful removal (0 on success, -1 on failure) const
*/
//  bool remove (gridCoreObject *obj);
/** @brief remove an object by name
* function to remove an object from the container
* @param[in] obj the object to insert
* @return a bool indicating successful removal (0 on success, -1 on failure) const
*/
//  bool remove (const std::string &objname);

/** @brief find object by name
* function to find an object by name
* @param[in] objname the name of the object to search for
* @return a pointer to the object if found otherwise nullptr
*/
//  gridCoreObject * find (const std::string &objname) const;

/** @brief find object by id
* function to find an object by user id code
* @param[in] objname the name of the object to search for
* @return a vector of objects with the appropriate searchID
*/
//  std::vector<gridCoreObject *> find(index_t searchID) const;

/** @brief check if an object is already contained
* function to find an object by id
* @param[in] obj to check
* @return a bool indicating if the object is a member or not
*/
//  bool isMember (gridCoreObject *obj);

/**
* @brief deletes all object pointed to by the list
  calls the condDel
*/
// void deleteAll (gridCoreObject *parent) const;

/** @brief get the size of the list
@param[in] the parent object which is doing the deleting
* @return the size of the list
*/
// count_t size () const
//  {
//    return static_cast<count_t> (m_oblistID.size ());
//  }

/**
* @brief update a single object with a name change or index change
@param[in] obj the object with the change
*/
// void updateObject(gridCoreObject *obj);
//};


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace boost::multi_index;
using boost::multi_index_container;

struct name {};
struct id {};
struct uid {};

typedef multi_index_container<
    gridCoreObject *,
    indexed_by<
      ordered_unique<tag<id>, const_mem_fun<gridCoreObject, index_t, &gridCoreObject::getID> >,
      ordered_unique<tag<name>,const_mem_fun<gridCoreObject, const std::string &, &gridCoreObject::getName> >,
      ordered_non_unique<tag<uid>, const_mem_fun<gridCoreObject, index_t, &gridCoreObject::getUserID> >
      >
    > objectIndex;

/** @brief list class that facilitates adding or searching for pointers to gridCoreObjects by name or id
* a list class that stores a list of the objects contained in an unordered map to faciliate searching for objects
*/
class gridCoreList
{
private:
  objectIndex m_objects;                  //!<the object map structur
public:
  /** @brief default constructor*/
  gridCoreList ();
  /**
  * @brief function to insert an object into the class
  * @param[in] obj the object to insert
  * @param[in] replace an optional indicator telling whether to replace the object or nor
  * @return a bool indicating successful insertion
  */
  bool insert (gridCoreObject *obj, bool replace = false);
  /** @brief remove an object
  * function to remove an object from the container
  * @param[in] obj the object to insert
  * @return a bool indicating successful removal (0 on success, -1 on failure) const
  */
  bool remove (gridCoreObject *obj);
  /** @brief remove an object by name
  * function to remove an object from the container
  * @param[in] obj the object to insert
  * @return a bool indicating successful removal (0 on success, -1 on failure) const
  */
  bool remove (const std::string &objname);

  /** @brief find object by name
  * function to find an object by name
  * @param[in] objname the name of the object to search for
  * @return a pointer to the object if found otherwise nullptr
  */
  gridCoreObject * find (const std::string &objname) const;

  /** @brief find object by id
  * function to find an object by user id code
  * @param[in] objname the name of the object to search for
  * @return a vector of objects with the appropriate searchID
  */
  std::vector<gridCoreObject *> find (index_t searchID) const;

  /** @brief check if an object is already contained
  * function to find an object by id
  * @param[in] obj to check
  * @return a bool indicating if the object is a member or not
  */
  bool isMember (gridCoreObject *obj) const;

  /**
  * @brief deletes all object pointed to by the list
  calls the condDel
  */
  void deleteAll (gridCoreObject *parent);

  /** @brief get the size of the list
  @param[in] the parent object which is doing the deleting
  * @return the size of the list
  */
  count_t size () const
  {
    return static_cast<count_t> (m_objects.size ());
  }

  /**
  * @brief update a single object with a name change or index change
  @param[in] obj the object with the change
  */
  void updateObject (gridCoreObject *obj);
};
#endif