/*=========================================================================
Program:   LHP
Module:    $RCSfile: lhpMeshFeBioWriter.h,v $
Language:  C++
Date:      $Date: 2010-12-15 15:22:24 $
Version:   $Revision: 1.1.2.1 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2007
SCS s.r.l. - BioComputing Competence Centre (www.scsolutions.it - www.b3c.it)
=========================================================================*/

#ifndef __lhpMeshFeBioWriter_H__
#define __lhpMeshFeBioWriter_H__

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
// lhpMeshFeBioWriter :
//----------------------------------------------------------------------------
/** Class "lhpMeshFeBioWriter": write the XML file. */

class lhpMeshFeBioWriter
{
public:
  /** Class constructor */
  lhpMeshFeBioWriter();
  /** Class destructor */
  ~lhpMeshFeBioWriter();

  /** Write XML file*/
  void Write(wxString file);

  /** Set geometry points coordinates*/
  void SetPointCoords(int id, double coords[3]);
  /** Set number of points in geometry. */
  void SetNumberOfPoints(int n){m_NumberOfPoints = n;};

  /** Add a hex-cell with ID number "id"*/
  void SetHex8PointsId(int id, int pointsId[8]);
  /** Set number of hexahedron cells. */
  //int SetNumberOfHex8(int n);

  /** Add a penta-cell*/
  void SetPenta6PointsId(int id, int pointsId[6]);
  /** Set number of pentahedron cells.*/
  //int SetNumberOfPenta6(int n);

  /** Add a tetra-cell*/
  void SetTet4PointsId(int id, int pointsId[4]);
  /** Set number of tetrahedron cells.*/
  //int SetNumberOfTeta4(int n);

  /** Add a quad-cell*/
  void SetQuad4PointsId(int id, int pointsId[4]);
  /** Set number of quad cells.*/
  //int SetNumberOfQuad4(int n);

  /** Add a triangle cell*/
  void SetTri3PointsId(int id, int pointsId[3]);
  /** Set number of triangle cells.*/
  //int SetNumberOfTri3(int n);

private:

  void WriteXML(xercesc::DOMDocument* pmyDOMDocument, const wxChar * FullFilePath);
  void CreateDOMDocument();

  void CreateNodeElements(DOMElement *parent);
  /** Create DOM element for hexahedron cells. */
  void CreateHex8Elements(DOMElement *parent);
  /** Create DOM element for wedge cells. */
  void CreatePenta6Elements(DOMElement *parent);
  /** Create DOM element for tetrahedron cells. */
  void CreateTet4Elements(DOMElement *parent);
  /** Create DOM element for quad cells. */
  void CreateQuad4Elements(DOMElement *parent);
  /** Create DOM element for triangle cells. */
  void CreateTri3Elements(DOMElement *parent);

  xercesc::XercesDOMParser *m_ConfigFileParser;
  xercesc::DOMDocument* m_DOMDocument;
  const wxChar * m_FullFilePath;

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
  XMLCh* ATTR_ID;
  XMLCh* ATTR_cellMat;

  int m_NumberOfPoints;

  std::vector<FEBIO_POINT *> m_Points;
  std::vector<FEBIO_HEX8 *> m_Hex8;
  std::vector<FEBIO_PENTA6 *> m_Penta6;
  std::vector<FEBIO_TET4 *> m_Tet4;
  std::vector<FEBIO_QUAD4 *> m_Quad4;
  std::vector<FEBIO_TRI3 *> m_Tri3;



};
#endif