/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMeshFeBioReader.cpp,v $
Language:  C++
Date:      $Date: 2010-12-15 15:27:39 $
Version:   $Revision: 1.1.2.5 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2007
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "lhpMeshFeBioReader.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <errno.h>

using namespace xercesc;
using namespace std;

//----------------------------------------------------------------------------
lhpMeshFeBioReader::lhpMeshFeBioReader()
//----------------------------------------------------------------------------
{
  /**
  *  Constructor initializes xerces-C libraries.
  *  The XML tags and attributes which we seek are defined.
  *  The xerces-C DOM parser infrastructure is initialized.
  */

  try
  {
    XMLPlatformUtils::Initialize();  // Initialize Xerces infrastructure
  }
  catch( XMLException& e )
  {
    char* message = XMLString::transcode( e.getMessage() );
    cerr << "XML toolkit initialization error: " << message << endl;
    XMLString::release( &message );
    // throw exception here to return ERROR_XERCES_INIT
  }

  // Tags and attributes used in XML file.
  // Can't call transcode till after Xerces Initialize()
  TAG_root        = XMLString::transcode("febio_spec");
  TAG_Geometry = XMLString::transcode("Geometry");
  TAG_Nodes = XMLString::transcode("Nodes");
  TAG_node = XMLString::transcode("node");
  TAG_Elements = XMLString::transcode("Elements");
  TAG_hex8 = XMLString::transcode("hex8");

  ATTR_nodeID = XMLString::transcode("id");
  ATTR_cellID = XMLString::transcode("id");
  ATTR_cellMat = XMLString::transcode("mat");

  m_ConfigFileParser = new XercesDOMParser;
}

/**
*  Class destructor frees memory used to hold the XML tag and 
*  attribute definitions. It als terminates use of the xerces-C
*  framework.
*/

//----------------------------------------------------------------------------
lhpMeshFeBioReader::~lhpMeshFeBioReader()
//----------------------------------------------------------------------------
{
  // Free memory

  delete m_ConfigFileParser;

  try
  {
    XMLString::release( &TAG_root );

    XMLString::release( &TAG_Geometry );
    XMLString::release( &TAG_Nodes );
    XMLString::release( &TAG_node );
    XMLString::release( &TAG_Elements );
    XMLString::release( &TAG_hex8 );

    XMLString::release( &ATTR_nodeID );
    XMLString::release( &ATTR_cellID );
    XMLString::release( &ATTR_cellMat );
  }
  catch( ... )
  {
    cerr << "Unknown exception encountered in TagNamesdtor" << endl;
  }

  for(int i=0;i<m_Hex8.size();i++)
    cppDEL(m_Hex8[i]);

  for(int i=0;i<m_Penta6.size();i++)
    cppDEL(m_Penta6[i]);

  for(int i=0;i<m_Tet4.size();i++)
    cppDEL(m_Tet4[i]);

  for(int i=0;i<m_Quad4.size();i++)
    cppDEL(m_Quad4[i]);

  for(int i=0;i<m_Tri3.size();i++)
    cppDEL(m_Tri3[i]);


  // Terminate Xerces
  try
  {
    XMLPlatformUtils::Terminate();  // Terminate after release of memory
  }
  catch( xercesc::XMLException& e )
  {
    char* message = xercesc::XMLString::transcode( e.getMessage() );

    cerr << "XML toolkit teardown error: " << message << endl;
    XMLString::release( &message );
  }
}

/**
*  This function:
*  - Tests the access and availability of the XML configuration file.
*  - Configures the xerces-c DOM parser.
*  - Reads and extracts the pertinent information from the XML config file.
*
*  @param in configFile The text string name of the HLA configuration file.
*/

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadConfigFile(wxString file)
//----------------------------------------------------------------------------
//throw( std::runtime_error )
{
  // Test to see if the file is ok.

  struct stat fileStatus;

  int iretStat = stat(file.c_str(), &fileStatus);
  if( iretStat == ENOENT )
  {
    mafLogMessage("Path file_name does not exist, or path is an empty string.");
    return;
  }
  else if( iretStat == ENOTDIR )
  {
    mafLogMessage("A component of the path is not a directory.");
    return;
  }
  else if( iretStat == EACCES )
  {
    mafLogMessage("Permission denied.");
    return;
  }
  else if( iretStat == ENAMETOOLONG )
  {
    mafLogMessage("File can not be read\n");
    return;
  }

  // Configure DOM parser.

  m_ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
  m_ConfigFileParser->setDoNamespaces( false );
  m_ConfigFileParser->setDoSchema( false );
  m_ConfigFileParser->setLoadExternalDTD( false );

  try
  {
    m_ConfigFileParser->parse( file.c_str() );

    // no need to free this pointer - owned by the parent parser object
    xercesc::DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();

    // Get the top-level element: NAme is "root". No attributes for "root"

    DOMElement* elementRoot = xmlDoc->getDocumentElement();
    if( !elementRoot ) throw(std::runtime_error( "empty XML document" ));

    // Parse XML file for tags of interest: "Geometry"

    DOMNodeList*      children = elementRoot->getChildNodes();
    const  XMLSize_t nodeCount = children->getLength();

    // For all nodes, children of "root" in the XML tree.

    for( XMLSize_t xx = 0; xx < nodeCount; ++xx )
    {
      DOMNode* currentNode = children->item(xx);
      if( currentNode->getNodeType() &&  // true is not NULL
        currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
      {
        // Found node which is an Element. Re-cast node as element
        DOMElement* currentElement
          = dynamic_cast< xercesc::DOMElement* >( currentNode );
        if( XMLString::equals(currentElement->getTagName(), TAG_Geometry))
        {
          DOMNodeList *geometryChildren = currentNode->getChildNodes();
          const  XMLSize_t geometryChildrenCount = geometryChildren->getLength();

          for(XMLSize_t i = 0; i < geometryChildrenCount; ++i)
          {
            DOMNode* currentChild = geometryChildren->item(i);
            if( currentChild->getNodeType() &&  // true is not NULL
              currentChild->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
            {
              // Found node which is an Element. Re-cast node as element
              DOMElement* currentChildElement
                = dynamic_cast< xercesc::DOMElement* >( currentChild );
              if( XMLString::equals(currentChildElement->getTagName(), TAG_Nodes) )
              {
                DOMNodeList *nodesChildren = currentChildElement->getChildNodes();
                const  XMLSize_t nodesCount = nodesChildren->getLength();

                for(XMLSize_t i = 0; i < nodesCount; ++i)
                {
                  DOMNode* nodesNode = nodesChildren->item(i);
                  if( nodesNode->getNodeType() &&  // true is not NULL
                    nodesNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
                  {
                    // Found node which is an Element. Re-cast node as element
                    DOMElement* node
                      = dynamic_cast< xercesc::DOMElement* >( nodesNode );
                    if( XMLString::equals(node->getTagName(), TAG_node) )
                    {
                      // retrieve node elements (point coordinates)
                      const XMLCh *text = node->getTextContent();
                      char *textChars = XMLString::transcode(text);

                      double values[3];

                      char *pch;
                      pch = strtok (textChars," ,");
                      //while (pch != NULL)
                      for (int i=0; i<3&& pch!=NULL; i++)
                      {
                        char *value = pch;
                        values[i] = atof(value);
                        pch = strtok (NULL, " ,");
                      }

                      const XMLCh* xmlch_nodeID
                        = node->getAttribute(ATTR_cellID);
                      int id = atoi( XMLString::transcode(xmlch_nodeID) );

                      FEBIO_POINT *point = new FEBIO_POINT();
                      point->Id = id;
                      for(int i=0;i<3;i++)
                        point->point[i] = values[i];

                      m_Points.push_back(point);
                    }
                  }
                }
              }
                 
                    
              else if( XMLString::equals(currentChildElement->getTagName(), TAG_Elements) )
              {

                DOMNodeList *elementsChildren = currentChildElement->getChildNodes();
                const  XMLSize_t elementsCount = elementsChildren->getLength();

                for(XMLSize_t i = 0; i < elementsCount; ++i)
                {
                  DOMNode* elementsNode = elementsChildren->item(i);
                  if( elementsNode->getNodeType() &&  // true is not NULL
                    elementsNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element 
                  {
                    // Found node which is an Element. Re-cast node as element
                    DOMElement* element
                      = dynamic_cast< xercesc::DOMElement* >( elementsNode );
                    
                    const XMLCh *tagName = element->getTagName();

                    if( XMLString::equals(tagName, TAG_hex8) )
                    {
                      ReadHex8(element);
                    }
                    else if( XMLString::equals(tagName, TAG_penta6) )
                    {
                      ReadPenta6(element);
                    }
                    else if( XMLString::equals(tagName, TAG_tet4) )
                    {
                      ReadTet4(element);
                    }
                    else if( XMLString::equals(tagName, TAG_quad4) )
                    {
                      ReadQuad4(element);
                    }
                    else if( XMLString::equals(tagName, TAG_tri3) )
                    {
                      ReadTri3(element);
                    }
                  }
                }
             
              }


            }
          }
        }
      }
    }
  }
  catch( xercesc::XMLException& e )
  {
    char* message = xercesc::XMLString::transcode( e.getMessage() );
    ostringstream errBuf;
    errBuf << "Error parsing file: " << message << flush;
    XMLString::release( &message );
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetPointCoords(int id, double coords[3])
//----------------------------------------------------------------------------
{
  coords[0] = m_Points[id]->point[0];
  coords[1] = m_Points[id]->point[1];
  coords[2] = m_Points[id]->point[2];
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetHex8PointsId(int id, int pointsId[8])
//----------------------------------------------------------------------------
{
  for (int i=0;i<8;i++)
  {
    pointsId[i] = m_Hex8[id]->points[i];
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetPenta6PointsId(int id, int pointsId[6])
//----------------------------------------------------------------------------
{
  for (int i=0;i<6;i++)
  {
    pointsId[i] = m_Penta6[id]->points[i];
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetTet4PointsId(int id, int pointsId[4])
//----------------------------------------------------------------------------
{
  for (int i=0;i<4;i++)
  {
    pointsId[i] = m_Tet4[id]->points[i];
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetQuad4PointsId(int id, int pointsId[4])
//----------------------------------------------------------------------------
{
  for (int i=0;i<4;i++)
  {
    pointsId[i] = m_Quad4[id]->points[i];
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::GetTri3PointsId(int id, int pointsId[3])
//----------------------------------------------------------------------------
{
  for (int i=0;i<3;i++)
  {
    pointsId[i] = m_Tri3[id]->points[i];
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadHex8(DOMElement *element)
//----------------------------------------------------------------------------
{
  const XMLCh *text = element->getTextContent();
  char *textChars = XMLString::transcode(text);

  int values[8];

  char *pch;
  pch = strtok (textChars," ,");
  //while (pch != NULL)
  for (int i=0; i<8&& pch!=NULL; i++)
  {
    char *value = pch;
    values[i] = atoi(value);
    pch = strtok (NULL, " ,");
  }

  const XMLCh* xmlch_nodeID
    = element->getAttribute(ATTR_cellID);

  int id = atoi( XMLString::transcode(xmlch_nodeID) );

  FEBIO_HEX8 *cell = new FEBIO_HEX8();
  cell->Id = id;
  for(int i=0;i<8;i++)
    cell->points[i] = values[i];

  m_Hex8.push_back(cell);

  XMLString::release(&textChars);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadPenta6(DOMElement *element)
//----------------------------------------------------------------------------
{
  const XMLCh *text = element->getTextContent();
  char *textChars = XMLString::transcode(text);

  int values[6];

  char *pch;
  pch = strtok (textChars," ,");
  //while (pch != NULL)
  for (int i=0; i<6&& pch!=NULL; i++)
  {
    char *value = pch;
    values[i] = atoi(value);
    pch = strtok (NULL, " ,");
  }

  const XMLCh* xmlch_nodeID
    = element->getAttribute(ATTR_cellID);
  int id = atoi( XMLString::transcode(xmlch_nodeID) );

  FEBIO_PENTA6 *cell = new FEBIO_PENTA6();
  cell->Id = id;
  for(int i=0;i<6;i++)
    cell->points[i] = values[i];

  m_Penta6.push_back(cell);

  XMLString::release(&textChars);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadTet4(DOMElement *element)
//----------------------------------------------------------------------------
{
  const XMLCh *text = element->getTextContent();
  char *textChars = XMLString::transcode(text);

  int values[4];

  char *pch;
  pch = strtok (textChars," ,");
  //while (pch != NULL)
  for (int i=0; i<4&& pch!=NULL; i++)
  {
    char *value = pch;
    values[i] = atoi(value);
    pch = strtok (NULL, " ,");
  }

  const XMLCh* xmlch_nodeID
    = element->getAttribute(ATTR_cellID);
  int id = atoi( XMLString::transcode(xmlch_nodeID) );

  FEBIO_TET4 *cell = new FEBIO_TET4();
  cell->Id = id;
  for(int i=0;i<4;i++)
    cell->points[i] = values[i];

  m_Tet4.push_back(cell);

  XMLString::release(&textChars);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadQuad4(DOMElement *element)
//----------------------------------------------------------------------------
{
  const XMLCh *text = element->getTextContent();
  char *textChars = XMLString::transcode(text);

  int values[4];

  char *pch;
  pch = strtok (textChars," ,");
  //while (pch != NULL)
  for (int i=0; i<4&& pch!=NULL; i++)
  {
    char *value = pch;
    values[i] = atoi(value);
    pch = strtok (NULL, " ,");
  }

  const XMLCh* xmlch_nodeID
    = element->getAttribute(ATTR_cellID);
  int id = atoi( XMLString::transcode(xmlch_nodeID) );

  FEBIO_QUAD4 *cell = new FEBIO_QUAD4();
  cell->Id = id;
  for(int i=0;i<4;i++)
    cell->points[i] = values[i];

  m_Quad4.push_back(cell);

  XMLString::release(&textChars);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioReader::ReadTri3(DOMElement *element)
//----------------------------------------------------------------------------
{
  const XMLCh *text = element->getTextContent();
  char *textChars = XMLString::transcode(text);

  int values[3];

  char *pch;
  pch = strtok (textChars," ,");
  //while (pch != NULL)
  for (int i=0; i<3&& pch!=NULL; i++)
  {
    char *value = pch;
    values[i] = atoi(value);
    pch = strtok (NULL, " ,");
  }

  const XMLCh* xmlch_nodeID
    = element->getAttribute(ATTR_cellID);
  int id = atoi( XMLString::transcode(xmlch_nodeID) );

  FEBIO_TRI3 *cell = new FEBIO_TRI3();
  cell->Id = id;
  for(int i=0;i<3;i++)
    cell->points[i] = values[i];

  m_Tri3.push_back(cell);

  XMLString::release(&textChars);
}