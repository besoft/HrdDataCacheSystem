/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpMeshFeBioWriter.cpp,v $
Language:  C++
Date:      $Date: 2010-12-15 15:22:24 $
Version:   $Revision: 1.1.2.1 $
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

#include "lhpMeshFeBioWriter.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <errno.h>

//#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMWriter.hpp>


using namespace xercesc;
using namespace std;

//----------------------------------------------------------------------------
lhpMeshFeBioWriter::lhpMeshFeBioWriter()
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

  ATTR_ID = XMLString::transcode("id");
  ATTR_cellMat = XMLString::transcode("mat");

  m_ConfigFileParser = new XercesDOMParser;

  m_FullFilePath = NULL;
}

/**
*  Class destructor frees memory used to hold the XML tag and 
*  attribute definitions. It als terminates use of the xerces-C
*  framework.
*/

//----------------------------------------------------------------------------
lhpMeshFeBioWriter::~lhpMeshFeBioWriter()
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

    XMLString::release( &ATTR_ID );
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
*  - Writes and extracts the pertinent information from the XML config file.
*
*  @param in configFile The text string name of the HLA configuration file.
*/

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::Write(wxString file)
//----------------------------------------------------------------------------
//throw( std::runtime_error )
{
  m_FullFilePath = file.c_str();
  CreateDOMDocument();
  WriteXML(m_DOMDocument, m_FullFilePath);

}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetPointCoords(int id, double coords[3])
//----------------------------------------------------------------------------
{
  FEBIO_POINT *point = new FEBIO_POINT();
  point->Id = id;
  for(int i=0;i<3;i++)
    point->point[i] = coords[i];

  m_Points.push_back(point);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetHex8PointsId(int id, int pointsId[8])
//----------------------------------------------------------------------------
{
  FEBIO_HEX8 *cell = new FEBIO_HEX8();
  cell->Id = id;
  for(int i=0;i<8;i++)
    cell->points[i] = pointsId[i];

  m_Hex8.push_back(cell);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetPenta6PointsId(int id, int pointsId[6])
//----------------------------------------------------------------------------
{
  FEBIO_PENTA6 *cell = new FEBIO_PENTA6();
  cell->Id = id;
  for(int i=0;i<6;i++)
    cell->points[i] = pointsId[i];

  m_Penta6.push_back(cell);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetTet4PointsId(int id, int pointsId[4])
//----------------------------------------------------------------------------
{
  FEBIO_TET4 *cell = new FEBIO_TET4();
  cell->Id = id;
  for(int i=0;i<4;i++)
    cell->points[i] = pointsId[i];

  m_Tet4.push_back(cell);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetQuad4PointsId(int id, int pointsId[4])
//----------------------------------------------------------------------------
{
  FEBIO_QUAD4 *cell = new FEBIO_QUAD4();
  cell->Id = id;
  for(int i=0;i<4;i++)
    cell->points[i] = pointsId[i];

  m_Quad4.push_back(cell);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::SetTri3PointsId(int id, int pointsId[3])
//----------------------------------------------------------------------------
{
  FEBIO_TRI3 *cell = new FEBIO_TRI3();
  cell->Id = id;
  for(int i=0;i<3;i++)
    cell->points[i] = pointsId[i];

  m_Tri3.push_back(cell);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::WriteXML(xercesc::DOMDocument* pmyDOMDocument, const wxChar * FullFilePath)
//----------------------------------------------------------------------------
{
  DOMImplementation    *pImplement        = NULL;
  DOMWriter            *pSerializer    = NULL;
  XMLFormatTarget        *pTarget        = NULL;

  //a DOM implementation that has the LS feature... or Load/Save.
  pImplement = DOMImplementationRegistry::getDOMImplementation(L"LS");

  /*
  From the DOMImplementation, create a DOMWriter.
  DOMWriters are used to serialize a DOM tree [back] into an XML document.
  */
  pSerializer = ((DOMImplementationLS*)pImplement)->createDOMWriter();


  //make the output more human-readable by inserting line-feeds 
  pSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);

  pTarget = new LocalFileFormatTarget(FullFilePath);


  // Write the serialized output to the target.
  pSerializer->writeNode(pTarget, *m_DOMDocument);
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateDOMDocument()
//----------------------------------------------------------------------------
{

  // Pointer to our DOMImplementation.
  DOMImplementation * pDOMImplementation = NULL;

  // Get the DOM Implementation (used for creating DOMDocuments).
  pDOMImplementation = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("core"));

  // Pointer to a DOMDocument.
  m_DOMDocument = NULL;

  // Create an empty DOMDocument.
  m_DOMDocument = pDOMImplementation->createDocument(0, TAG_root, 0);


  /*
  Note:Nodes are the base class, 
  and Elements are the specilizations.
  */
  DOMElement * pRootElement = NULL;
  pRootElement = m_DOMDocument->getDocumentElement();

  pRootElement->setAttribute(L"version", L"1.1");


  // Create Geometry Node
  DOMElement *geometryElement = NULL;
  geometryElement = m_DOMDocument->createElement(TAG_Geometry);

  // create Nodes
  DOMElement *nodesElement = NULL;
  nodesElement = m_DOMDocument->createElement(TAG_Nodes);

  CreateNodeElements(nodesElement);

  // create Elements
  DOMElement *elementsElement = NULL;
  elementsElement = m_DOMDocument->createElement(TAG_Elements);

  CreateHex8Elements(elementsElement);

  geometryElement->appendChild(nodesElement);
  geometryElement->appendChild(elementsElement);

  pRootElement->appendChild(geometryElement);

  
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateNodeElements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Points.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_node);

    char idString[10], valueString[100];
    sprintf(idString, "%d", m_Points[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    sprintf(valueString, "%e,%e,%e", m_Points[i]->point[0], m_Points[i]->point[1], m_Points[i]->point[2] );
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateHex8Elements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Hex8.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_hex8);

    char idString[10], materialString[10], valueString[100];

    sprintf(idString, "%d", m_Hex8[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    //sprintf(idString, "%d", m_Hex8[i]->material);
    //nodeElement->setAttribute(ATTR_material, XMLString::transcode(materialString));

    sprintf(valueString, "%d,%d,%d,%d,%d,%d,%d,%d", m_Hex8[i]->points[0], m_Hex8[i]->points[1], m_Hex8[i]->points[2],  m_Hex8[i]->points[3],
      m_Hex8[i]->points[4],  m_Hex8[i]->points[5], m_Hex8[i]->points[6], m_Hex8[i]->points[7]);
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreatePenta6Elements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Penta6.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_penta6);

    char idString[10], materialString[10], valueString[100];

    sprintf(idString, "%d", m_Penta6[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    //sprintf(idString, "%d", m_Hex8[i]->material);
    //nodeElement->setAttribute(ATTR_material, XMLString::transcode(materialString));

    sprintf(valueString, "%d,%d,%d,%d,%d,%d", m_Penta6[i]->points[0], m_Penta6[i]->points[1], m_Penta6[i]->points[2],
      m_Penta6[i]->points[3],  m_Penta6[i]->points[4], m_Penta6[i]->points[5]);
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateTet4Elements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Tet4.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_tet4);

    char idString[10], materialString[10], valueString[100];

    sprintf(idString, "%d", m_Tet4[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    //sprintf(idString, "%d", m_Hex8[i]->material);
    //nodeElement->setAttribute(ATTR_material, XMLString::transcode(materialString));

    sprintf(valueString, "%d,%d,%d,%d", m_Tet4[i]->points[0], m_Tet4[i]->points[1], m_Tet4[i]->points[2], m_Tet4[i]->points[3]);
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateQuad4Elements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Quad4.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_quad4);

    char idString[10], materialString[10], valueString[100];

    sprintf(idString, "%d", m_Quad4[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    //sprintf(idString, "%d", m_Hex8[i]->material);
    //nodeElement->setAttribute(ATTR_material, XMLString::transcode(materialString));

    sprintf(valueString, "%d,%d,%d,%d", m_Quad4[i]->points[0], m_Quad4[i]->points[1], m_Quad4[i]->points[2], m_Quad4[i]->points[3]);
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}

//----------------------------------------------------------------------------
void lhpMeshFeBioWriter::CreateTri3Elements(DOMElement *parent)
//----------------------------------------------------------------------------
{
  for(int i=0;i<m_Tri3.size();i++)
  {
    DOMElement *nodeElement  = NULL;
    nodeElement = m_DOMDocument->createElement(TAG_tri3);

    char idString[10], materialString[10], valueString[100];

    sprintf(idString, "%d", m_Tri3[i]->Id);
    nodeElement->setAttribute(ATTR_ID, XMLString::transcode(idString));

    //sprintf(idString, "%d", m_Hex8[i]->material);
    //nodeElement->setAttribute(ATTR_material, XMLString::transcode(materialString));

    sprintf(valueString, "%d,%d,%d", m_Tri3[i]->points[0], m_Tri3[i]->points[1], m_Tri3[i]->points[2]);
    nodeElement->setTextContent(XMLString::transcode(valueString));

    parent->appendChild(nodeElement);
  }
}