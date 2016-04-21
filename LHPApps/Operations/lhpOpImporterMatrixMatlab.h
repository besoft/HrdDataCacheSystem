/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: lhpOpImporterMatrixMatlab.h,v $
  Language:  C++
  Date:      $Date: 2012-03-08 15:51:57 $
  Version:   $Revision: 1.1.2.1 $
  Authors:   Youbing Zhao
==========================================================================
  Copyright (c) 2002/2012
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpImporterMatrixMatlab_H__
#define __lhpImporterMatrixMatlab_H__

#include "mafOp.h"
#include "lhpOperationsDefines.h"
#include <string>

#include "lhpOpExporterMatrixOctave.h"
//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafEvent;

//----------------------------------------------------------------------------
//  lhpOpImporterMatrixMatlab : Importing octave matrix to NMSBuilder
//----------------------------------------------------------------------------
/** read an octave matrix and apply it to a VME node */
class LHP_OPERATIONS_EXPORT lhpOpImporterMatrixMatlab : public mafOp
{
public:
	/** Class constructor */
	lhpOpImporterMatrixMatlab(const wxString &label = "Octave Matrix Importer");
	/** Class destructor */
	~ lhpOpImporterMatrixMatlab(); 

	/**  Operation copy */
	mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode* node); 

	/** Builds operation's interface. */
	void OpRun();

	/** event handling */
	virtual void OnEvent(mafEventBase *maf_event);



protected:

	class OctaveImportException : public std::exception {
	public:
		OctaveImportException(const char * str) : std::exception(str) { }
	};

	// header information, including name and type
	struct HeaderOctave {
		std::string  name;
		std::string  type;
		bool valid() { 	return  ( (! name.empty()) && (! type.empty()) ); 	}
		bool IsPoseMatrix() { return   std::string::npos != name.find(lhpOctaveMatrixType::matPoseNameSfx); }
		bool IsPointList() { return  std::string::npos != name.find(lhpOctaveMatrixType::pointListNameSfx) ; }
		bool IsFaceList() {  return  std::string::npos != name.find(lhpOctaveMatrixType::faceNameSfx) ; }
	};

	class MatrixOctave {
	public:
		MatrixOctave():name(""), mrows(0), ncols(0),bElemValid(false) 
		{
		}

		/*
		MatrixOctave(MatrixOctave & mat):name(mat.name), mrows(mat.mrows), ncols(mat.ncols),bElemValid(mat.bElemValid) 
		{
			elem = mat.elem;
		}
		*/

		bool IsTypeInfoReady() {
			return  ( (!name.empty()) && (mrows > 0) && (ncols > 0) );
		}

		void AllocateIfReady() {
			if (IsTypeInfoReady()) {
				elem.resize(mrows * ncols);
			}
		}

		void SetElement(int r, int c, double e) {
			int matSize = mrows * ncols;
			if ( matSize > elem.size() )
				AllocateIfReady();

			int idx = r * ncols + c;
			if ((idx < 0)||  (idx >= matSize) )
				return;

			elem[idx] = e; 
		}

		double GetElement(int r, int c) {
			int idx = r * ncols + c;
			int matSize = mrows * ncols;
			if ((idx < 0)||  (idx >= matSize) )
				return 0;

			return elem[idx];
		}

		void SetNumOfRows(int n)
		{
			if (n > 0) 	{
				mrows = n;
				AllocateIfReady();
			}
		}

		void SetNumOfColumns(int n)
		{
			if (n > 0)	{
				ncols = n;
				AllocateIfReady();
			}
		}

		int GetNumOfRows() { return mrows; }
		int GetNumOfColumns() { return ncols; }

		std::string GetName() { return name; }
		void SetName(std::string nm) { name = nm; }
		void SetElemValid(bool b) { bElemValid = b; }
		bool GetElemValid() { 	return bElemValid; 	}

		bool IsPoseMatrix() { return   std::string::npos != name.find(lhpOctaveMatrixType::matPoseNameSfx); }
		bool IsPointList() { return  std::string::npos != name.find(lhpOctaveMatrixType::pointListNameSfx) ; }
		bool IsFaceList() {  return  std::string::npos != name.find(lhpOctaveMatrixType::faceNameSfx) ; }
	
	protected:
		/** event ids */
		enum 
		{
			//ID_USE_POSEMATRIX = MINID,
			//ID_USE_POINTLIST,
		};

		std::string  name;
		unsigned int mrows;
		unsigned int ncols;
		bool bElemValid;
		
		// matrix elements
		std::vector<double> elem;

	} ;

	/** Create the dialog interface for the importer. */
	virtual void CreateGui();  

	int OnOK();

	void OnChooseVme();

	/** Read the Octave file*/
	void Read() throw (OctaveImportException);
	/** import pose matrix to the vme */
	void ImportPoseMatrix();
	/** import point data to the vme */
	void ImportPointList();

	void CreateVMELandmarkCloud();
	void CreateVMESurface();
	void SetPoseMatrix(mafVME * pVme);

	/** import file directory */
	wxString m_FileDir;
	/** import file */
	wxString m_File;

	/** vme to be imported to */
	mafVME * m_pVme;
	/** name of the vme */
	mafString  m_NameVme;

	/** import pose matrix */
	int m_bUsePoseMatrix;
	/** import point data */
	int m_bUsePointList;
	/** vme type to be generated after import */
	int m_VmeType;

	/* current processing line number */
	int m_nCurrLine;
	/** all the lines contained in the file */
	std::vector<std::string> m_Lines;

	/** the imported pose matrix */
	MatrixOctave m_matPoseMatrix;
	/** the imported  point list  */
	MatrixOctave m_matPointList;
	/** the imported  face list  */
	MatrixOctave m_matFaceList;

};


#endif


