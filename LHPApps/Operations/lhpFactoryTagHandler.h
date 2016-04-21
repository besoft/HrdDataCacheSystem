/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpFactoryTagHandler.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:53 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpFactoryTagHandler_h
#define __lhpFactoryTagHandler_h
//----------------------------------------------------------------------------
// includes :
//----------------------------------------------------------------------------
#include "mafObjectFactory.h"

/** to be used internally for plugging default tags handler --- calls a member function directly */
#define lhpPlugTagHandlerMacro(tag_handler_type,descr) \
  RegisterNewTagHandler(tag_handler_type::GetStaticTypeName(), descr, tag_handler_type::NewObject);
  

// to be defined in a separate file. This is just an empty skeleton
// start skeleton
#include "mafObject.h"
#include "lhpTagHandlerContainer.h"
#include "lhpTagHandlerDICOMContainer.h"
#include "mafVME.h"

// end skeleton...

/** Object factory for tag handlers.
  To make a new handler available in the LHDL it must be plugged inside a factory, in particular
  this factory must be of type lhpFactoryTagHandler to be able to retrieve the list of handlers plugged
  in the factory.  */

class lhpFactoryTagHandler : public mafObjectFactory
// need to create LHP_EXPORT symbol
{
public: 
  mafTypeMacro(lhpFactoryTagHandler,mafObjectFactory);
  virtual const char* GetMAFSourceVersion() const;
  virtual const char* GetDescription() const;

  /* Initialize the factory creating and registering a new instance */
  static int Initialize();

  /** return the instance pointer of the factory. return NULL if not initialized yet */
  static lhpFactoryTagHandler *GetInstance() {if (!m_Instance) Initialize(); return m_Instance;}

  /** create an instance of the tag handler give its type name */
  static lhpTagHandler *CreateTagHandlerInstance(const char *type_name);
   
   /**
    This function can be used by Application code to register new Objects's to the mflCoreFactory */
  void RegisterNewTagHandler(const char* tagHandlerName, const char* description, mafCreateObjectFunction createFunction);

  /** return list of names for tag handlers plugged into this factory */
  const static std::vector<std::string> &GetTagHandlerNames() {return m_TagHandlerNames;}

protected:
  lhpFactoryTagHandler();
  ~lhpFactoryTagHandler() { }

  static lhpFactoryTagHandler *m_Instance;
  static std::vector<std::string> m_TagHandlerNames; 
  
private:
  lhpFactoryTagHandler(const lhpFactoryTagHandler&);  // Not implemented.
  void operator=(const lhpFactoryTagHandler&);  // Not implemented.
};

/** Plug  a tag handler in the main tag handlers factory.*/
template <class T>
class lhpPlugTagHandler
// needs to create LHP_EXPORT symbol 
{
  public:
  lhpPlugTagHandler(const char *description);
  
};

//------------------------------------------------------------------------------
/** Plug a new tag handler class into the tag handlers factory.*/
template <class T>
lhpPlugTagHandler<T>::lhpPlugTagHandler(const char *description)
//------------------------------------------------------------------------------
{ 
  lhpFactoryTagHandler *factory=lhpFactoryTagHandler::GetInstance();
  if (factory)
  {
    factory->RegisterNewTagHandler(T::GetStaticTypeName(), description, T::NewObject);
  }
}

#endif
