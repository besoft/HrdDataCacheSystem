/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpMeshFeBioReader.h,v $
Language:  C++
Date:      $Date: 2010-12-15 15:27:39 $
Version:   $Revision: 1.1.2.4 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpMeshFeBioReader_H__
#define __lhpMeshFeBioReader_H__

/**
*  @file
*  
*  @version 1.0
*/

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <string>
#include <stdexcept>
#include <vector>

#include "wx/string.h"

using namespace xercesc;

// Error codes

enum {
  ERROR_ARGS = 1, 
  ERROR_XERCES_INIT,
  ERROR_PARSE,
  ERROR_EMPTY_DOCUMENT
};

struct FEBIO_POINT
{        
  unsigned char Id;
  double point[3];
};

struct FEBIO_HEX8
{        
  unsigned char Id;
  int points[8];
};

struct FEBIO_PENTA6
{        
  unsigned char Id;
  int points[6];
};

struct FEBIO_TET4
{        
  unsigned char Id;
  int points[4];
};

struct FEBIO_QUAD4
{        
  unsigned char Id;
  int points[4];
};

struct FEBIO_TRI3
{        
  unsigned char Id;
  int points[3];
};

//----------------------------------------------------------------------------
// lhpMeshFeBioReader :
//----------------------------------------------------------------------------
/** Class "lhpMeshFeBioReader": read the XML data. */

class lhpMeshFeBioReader
{
public:
  /** Class constructor */
  lhpMeshFeBioReader();
  /** Class destructor */
  ~lhpMeshFeBioReader();

  /** Read XML file*/
  void ReadConfigFile(wxString file);// throw(std::runtime_error);

  /** Retrieve geometry points coordinates*/
  void GetPointCoords(int id, double coords[3]);
  /** Get number of points in geometry. */
  int GetNumberOfPoints() {return m_Points.size();};

  /** Get points ids for hex-cell with ID number "id"*/
  void GetHex8PointsId(int id, int pointsId[8]);
  /** Get number of hexahedron cells. */
  int GetNumberOfHex8() {return m_Hex8.size();};

  /** Get points ids for penta-cell with ID number "id"*/
  void GetPenta6PointsId(int id, int pointsId[6]);
  /** Get number of pentahedron cells.*/
  int GetNumberOfPenta6() {return m_Penta6.size();};

  /** Get points ids for tetra-cell with ID number "id"*/
  void GetTet4PointsId(int id, int pointsId[4]);
  /** Get number of tetrahedron cells.*/
  int GetNumberOfTeta4() {return m_Tet4.size();};

  /** Get points ids for quad-cell with ID number "id"*/
  void GetQuad4PointsId(int id, int pointsId[4]);
  /** Get number of quad cells.*/
  int GetNumberOfQuad4() {return m_Quad4.size();};

  /** Get points ids for triangle-cell with ID number "id"*/
  void GetTri3PointsId(int id, int pointsId[3]);
  /** Get number of triangle cells.*/
  int GetNumberOfTri3() {return m_Tri3.size();};

private:

  /** Retrieve point coordinates from XML file. */
  void ReadPoint(DOMElement *element);
  /** Retrieve hex cells info from XML file. */
  void ReadHex8(DOMElement *element);
  /** Retrieve penta cells info from XML file. */
  void ReadPenta6(DOMElement *element);
  /** Retrieve tetra cells info from XML  file. */
  void ReadTet4(DOMElement *element);
  /** Retrieve quad cells info from XML file. */
  void ReadQuad4(DOMElement *element);
  /** Retrieve triangle cells info from XML file. */
  void ReadTri3(DOMElement *element);

  xercesc::XercesDOMParser *m_ConfigFileParser;

  // Internal class use only. Hold Xerces data in UTF-16 SMLCh type.

  XMLCh* TAG_root;

  XMLCh* TAG_Geometry;
  XMLCh* TAG_Nodes;
  XMLCh* TAG_node;
  XMLCh* TAG_Elements;
  XMLCh* TAG_hex8;
  XMLCh* TAG_penta6;
  XMLCh* TAG_tet4;
  XMLCh* TAG_quad4;
  XMLCh* TAG_tri3;


  // could use just ATTR_ID
  XMLCh* ATTR_nodeID;
  XMLCh* ATTR_cellID;
  XMLCh* ATTR_cellMat;

  std::vector<FEBIO_POINT *> m_Points;
  std::vector<FEBIO_HEX8 *> m_Hex8;
  std::vector<FEBIO_PENTA6 *> m_Penta6;
  std::vector<FEBIO_TET4 *> m_Tet4;
  std::vector<FEBIO_QUAD4 *> m_Quad4;
  std::vector<FEBIO_TRI3 *> m_Tri3;



};
#endif