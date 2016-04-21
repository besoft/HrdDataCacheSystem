/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEC3DData.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - porting Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
  ULB    - University Libre de Bruxelles
=========================================================================*/
#ifndef __mafVMEC3DData_h
#define __mafVMEC3DData_h
//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVMEGroup.h"
#include "lhpVMEDefines.h"
#include <fstream>

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafVMELandmarkCloud;

/** mafVMEC3DData - 
*/
class LHP_VME_EXPORT  mafVMEC3DData : public mafVMEGroup  
{
public:

  mafTypeMacro(mafVMEC3DData,mafVMEGroup);
  
  /** Set RAW motion data  Dictionary file name*/
  void SetDictionaryFileName(const char *name);
  
  /** Get RAW motion data  Dictionary file name*/
  const char *GetDictionaryName() {return this->m_DictionaryFileName.GetCStr();}

  /** Set RAW motion data file name*/
  void mafVMEC3DData::SetFileName(const char *name);
  
  /** Get RAW motion data file name*/
  const char *GetFileName() {return this->m_FileName.GetCStr(); }	
  
  /** Parse C3D motion data file and fill VME Tree*/
  int Read();

	/**
	Use DictionaryOn() to use a dictionary otherwise
	only one segment will be created and all the landmarks 
	will be appended to it.*/
	void SetDictionary(int Dict){m_Dictionary = Dict;}
	int GetDictionary(){return m_Dictionary;}

	void DictionaryOn () { this->SetDictionary((int)1);}
	void DictionaryOff () { this->SetDictionary((int)0);}

  /**
  //C3D Header access function*/
  unsigned short int  GetNumberOfMarkers() {return *(this->num_markers);};

  /**
  //C3D Header access function*/
  unsigned short int  GetNumberOfChannels() {return *(this->num_channels);};

  /**
  //C3D Header access function*/
  unsigned short int  GetFirstFrame() {return *(this->first_frame);};

  /**
  //C3D Header access function*/
  unsigned short int  GetLastFrame() {return *(this->last_frame);};

  /**
  //C3D Header access function*/
  unsigned short int  GetScaleFactor() {return *(this->scale_factor);};

  /**
  //C3D Header access function*/
  unsigned short int  GetStartRecord() {return *(this->start_record_num);};

  /**
  //C3D Header access function*/
  unsigned short int  GetFramesPerField() {return *(this->frames_per_field);};

  /**
  //C3D Header access function*/
  unsigned short int  GetVideoRate() {return *(this->video_rate);};

  //prefered freq access function*/
  void SetPrefFreq(unsigned short int value) {prefered_freq = value;};

  /**
  Initial reading data*/
  int PreRead();

protected:

  mafVMEC3DData();
  virtual ~mafVMEC3DData();

  // By default, UpdateInformation calls this method. Subclasses should fill
  // the output structure information.
  //virtual int ExecuteInformation();

  //virtual int ExecuteData(mafVMEItem *item); //virtual int ExecuteData(mflVMEItem *item);

  mafString m_DictionaryFileName; 
  mafString m_FileName; 
  int m_Dictionary;
  mafVMELandmarkCloud *m_CurrentDlc;
	mafVMELandmarkCloud *m_Dlc;

  mafVMELandmarkCloud *C3D_DLCloud;
  std::vector<mafString> *c3DLMNamesTagArray;
  FILE* pC3DFile;

  unsigned short int	*num_markers;
  unsigned short int	*num_channels;
  unsigned short int	*first_frame; //first frame

  unsigned short int	*last_frame;  //last frame
  float			          *scale_factor;
  unsigned short int	*start_record_num;
  unsigned short int	*frames_per_field;
  float			          *video_rate;
  unsigned int        prefered_freq;

  void Read_C3D_Header(
    unsigned short int	*num_markers, 
    unsigned short int	*num_channels,
    unsigned short int	*first_field,
    unsigned short int	*last_field,
    float				*scale_factor,
    unsigned short int	*start_record_num,
    unsigned short int	*frames_per_field,
    float				*video_rate,
    FILE				*infile);


  int Read_C3D_Data(
    unsigned short	num_markers,				// number of marker trajectories
    unsigned short	num_analog_channels,		// number of analog channels
    unsigned short	first_field,				// first frame to read
    unsigned short	last_field,					// last frame to read
    unsigned short	start_byte,					// starting record number
    unsigned short	analog_frames_per_field,    // analog samples/ video frame
    float scale_factor,
    std::vector<mafString> *lm_names_tagarray,
    FILE			*infile);

  void Read_C3D_Parameters(std::vector<mafString> *mlabels,   FILE	*infile);


  float ConvertDecToFloat(char bytes[4]);
  void  ConvertFloatToDec(float f, char* bytes);

  /**
  Read Parameter Format for the variable following the 
  parameter name*/
  void RPF(int *offset, char *type, char *dim, FILE *infile);

private:
  mafVMEC3DData(const mafVMEC3DData&);  // Not implemented.
  void operator=(const mafVMEC3DData&);  // Not implemented.

  /**
  //Removes trailing spaces from the end of a string*/
  char* strrtrim( char* s);
  //mafVMERoot *root;
  int Dictionary;
  //wxString DictionaryFileName;
};
#endif
