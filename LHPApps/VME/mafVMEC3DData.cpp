/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafVMEC3DData.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:57 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni - porting Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2005 
  CINECA - Interuniversity Consortium (www.cineca.it)
  ULB    - University Libre de Bruxelles
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafVMEC3DData.h"


#include "mafVMELandmarkCloud.h"  
#include "mafVMEItem.h"
#include "mafTagArray.h"

#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "itkRawMotionImporterUtility.h"

#include <vcl_fstream.h>
#include <vcl_string.h>
#include <vnl\vnl_matrix.h>

#include <iostream>
#include <string>


//----------------------------------------------------------------------------
// constant
//----------------------------------------------------------------------------
//If there is no dictionary or if the dictionary
//does not define a NOT_USED value I use
//the value -9999.00 to identify landmarks that
//are not visible at a given timestamp.
const double CONST_NOT_USED_IOR_MAL = -9999.00;

//-------------------------------------------------------------------------
mafCxxTypeMacro(mafVMEC3DData)
//-------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafVMEC3DData::mafVMEC3DData()
//----------------------------------------------------------------------------
{
  this->C3D_DLCloud = mafVMELandmarkCloud::New();
  C3D_DLCloud->SetRadius(15.0);
  C3D_DLCloud->SetDefaultVisibility(0);  //modified by Marco. 3-10-2003


  num_markers       = new unsigned short int;
  num_channels      = new unsigned short int;
  first_frame       = new unsigned short int; //first frame
  prefered_freq     = 0;
  last_frame        = new unsigned short int;  //last frame
  scale_factor      = new float;
  start_record_num  = new unsigned short int;
  frames_per_field  = new unsigned short int;
  video_rate        = new float;	

  this->m_Dictionary = 0;
}
//----------------------------------------------------------------------------
mafVMEC3DData::~mafVMEC3DData()
//----------------------------------------------------------------------------
{
  this->C3D_DLCloud->Delete();

  delete num_markers;
  num_markers = NULL;
  delete num_channels;
  num_channels = NULL;
  delete first_frame;
  first_frame = NULL;
  delete last_frame;
  last_frame = NULL;
  delete scale_factor;
  scale_factor = NULL;
  delete start_record_num;
  start_record_num = NULL;
  delete frames_per_field ;
  frames_per_field = NULL;
  delete video_rate;
  video_rate = NULL;
}

//----------------------------------------------------------------------------
void mafVMEC3DData::SetDictionaryFileName(const char *name)
//----------------------------------------------------------------------------
{
  this->m_DictionaryFileName = name;
  this->DictionaryOn();
  this->Modified();
}

//----------------------------------------------------------------------------
void mafVMEC3DData::SetFileName(const char *name)
//----------------------------------------------------------------------------
{
  this->m_FileName = name;
  this->Modified();
}

//----------------------------------------------------------------------------
int mafVMEC3DData::Read()
//-----------------------------------------------------------------------
{

  c3DLMNamesTagArray = new std::vector<mafString>;

  //root = this->GetOutput();
  //root->CleanTree();


  pC3DFile = fopen(this->m_FileName, "rb");

  //Parse C3D file header
  this->Read_C3D_Header(num_markers, 
    num_channels, 
    first_frame,
    last_frame,
    scale_factor,
    start_record_num,
    frames_per_field,
    video_rate,
    pC3DFile);

  //Read C3D Parameters section
  this->Read_C3D_Parameters(c3DLMNamesTagArray, pC3DFile);

  //Read C3D Data
  this->Read_C3D_Data(
    *num_markers,				// This is the total # markers 
    *num_channels,		// Number of analog channels
    *first_frame,				// Start field #
    *last_frame,					// End field #
    *start_record_num,					// Where data starts
    *frames_per_field,
    *scale_factor,
    c3DLMNamesTagArray,
    pC3DFile);




  delete c3DLMNamesTagArray;
  c3DLMNamesTagArray = NULL;
  fclose(pC3DFile);
  return 0;				
}

//----------------------------------------------------------------------------
int mafVMEC3DData::PreRead()
//-----------------------------------------------------------------------
{
  pC3DFile = fopen(this->m_FileName, "rb");

  //Parse C3D file header
  this->Read_C3D_Header(num_markers, 
    num_channels, 
    first_frame,
    last_frame,
    scale_factor,
    start_record_num,
    frames_per_field,
    video_rate,
    pC3DFile);
  fclose(pC3DFile);
  return 0;				
}


//----------------------------------------------------------------------------
void mafVMEC3DData::Read_C3D_Header(
                                    unsigned short int	*num_markers, 
                                    unsigned short int	*num_channels,
                                    unsigned short int	*first_frame,
                                    unsigned short int	*last_frame,
                                    float				*scale_factor,
                                    unsigned short int	*start_record_num,
                                    unsigned short int	*frames_per_field,
                                    float				*video_rate,
                                    FILE				*infile)
//----------------------------------------------------------------------------

{ 

  unsigned short int		key1,max_gap;
  char					DEC_float[4];

  /* Read in Header */
  // Key1, byte = 1,2; word = 1
  fread(&key1, sizeof key1, 1, infile); 

  // Number of 3D points per field, byte = 3,4; word = 2
  fread(num_markers, sizeof *num_markers, 1, infile);
  //printf("num_markers = %d\n",*num_markers);

  // Number of analog channels per field byte = 5,6; word = 3
  fread(num_channels, sizeof *num_channels, 1, infile); 
  //printf("num_channels = %d\n",*num_channels);

  // Field number of first field of video data, byte = 7,8; word = 4
  fread(first_frame, sizeof *first_frame, 1, infile); 
  //printf("first_frame = %d\n",*first_frame);

  // Field number of last field of video data, byte = 9,10; word = 5	
  fread(last_frame, sizeof *last_frame, 1, infile); 
  //  if((*last_frame) > (*first_frame) + 100)
  //    *last_frame = (*first_frame) + 100;
  //printf("last_frame = %d\n",*last_frame);

  // Maximum interpolation gap in fields, byte = 11,12; word = 6
  fread(&max_gap, sizeof max_gap, 1, infile); 
  //printf("max_gap = %d\n",max_gap);

  // Scaling Factor, bytes = 13,14,15,16; word = 7,8
  fread(&DEC_float[0], sizeof(char), 1, infile); // 1st byte
  fread(&DEC_float[1], sizeof(char), 1, infile); // 2nd byte
  fread(&DEC_float[2], sizeof(char), 1, infile); // 3rd byte
  fread(&DEC_float[3], sizeof(char), 1, infile); // 4th byte

  //*scale_factor = ConvertDecToFloat(DEC_float); 
  *scale_factor = ConvertDecToFloat(DEC_float);

  // Starting record number, byte = 17,18; word = 9
  fread(start_record_num, sizeof *start_record_num, 1, infile);

  // Number of analog frames per video field, byte = 19,20; word = 10
  fread(frames_per_field, sizeof *frames_per_field, 1, infile);

  // Analog channels sampled
  if (*frames_per_field != 0) 
    *num_channels /= *frames_per_field;

  // Video rate in Hz, bytes = 21,22; word = 11
  fread(&DEC_float[0], sizeof DEC_float[0], 1, infile); // 1st byte
  fread(&DEC_float[1], sizeof DEC_float[0], 1, infile); // 2nd byte
  DEC_float[2] = 0;
  DEC_float[3] = 0;

  *video_rate = ConvertDecToFloat(DEC_float); 

} 

//----------------------------------------------------------------------------
int mafVMEC3DData::Read_C3D_Data(unsigned short	num_markers,				// number of marker trajectories
                                 unsigned short	num_analog_channels,		// number of analog channels
                                 unsigned short	first_field,				// first frame to read
                                 unsigned short	last_field,					// last frame to read
                                 unsigned short	start_byte,					// starting record number
                                 unsigned short	analog_frames_per_field,// analog samples/ video frame	
                                 float	 scale_factor,
                                 std::vector<mafString> *c3DLMNamesTagArray,
                                 FILE			*infile)
//----------------------------------------------------------------------------
{

  short filterStep = 1;
  unsigned int finalFreq;
  if(prefered_freq != 0)
    filterStep = (int)(*video_rate / prefered_freq);
  if (filterStep <= 0)
    filterStep = 1;
  // fill C3DMatrix for easy data access
  short numberOfFrames = (last_field - first_field + 1) / filterStep;
  if ((last_field - first_field + 1) % filterStep != 0)
    numberOfFrames++;
  vnl_matrix<double> C3DMatrix(numberOfFrames, num_markers * 3);
  if(filterStep == 1)
    finalFreq = *video_rate;
  else
    finalFreq = prefered_freq;
  if(finalFreq == 0)
    finalFreq = 1;

  //unsigned char	residual, num_cam;
  unsigned short	frame, marker, sample, channel, added;

  // Startbyte is the starting location of tvd/adc data
  start_byte = 512 * (start_byte - 1);

  // Position file pointer to start of data record
  fseek(infile,start_byte,0);

  added = 0;
  // For each frame
  for (frame = first_field; frame <= last_field; frame++)
  {
    // Read Video data
    // For each marker

    short x,y,z;
    for (marker = 1; marker <= num_markers; marker ++)
    {
      //printf("%d ",marker);
      // Read X,Y,Z positions
      fread(&x, sizeof(short), 1, infile);
      fread(&y, sizeof(short), 1, infile);
      fread(&z, sizeof(short), 1, infile);

      if((frame - first_field) % filterStep == 0)
      {
        C3DMatrix(added, (marker - 1) * 3) = x*scale_factor; 
        C3DMatrix(added, (marker - 1) * 3 + 1) = y*scale_factor;
        C3DMatrix(added, (marker - 1) * 3 + 2) = z*scale_factor;
      }

      //skip residual and camera section
      short a,b;
      // Read residual value and # cameras (1 byte each)
      fread(&a, sizeof(char), 1, infile);
      //printf("%d ",residual[marker][frame]);
      fread(&b, sizeof(char), 1, infile);
    } // Next marker
    if((frame - first_field) % filterStep == 0)
      added++;

    // Read Analog Data

    // skip analog section
    short pippo;
    // For each analog sample / frame
    for (sample = 1; sample <= analog_frames_per_field; sample ++)
    {
      // For each channel of analog data
      for (channel = 1; channel<= num_analog_channels; channel ++)
        fread(&pippo, sizeof(short), 1, infile);
    } // Next sample

  }//end for each frame

  // end fill C3DMatrix

  //vcl_ofstream mtrx("C:\\m2", std::ios::out);
  //mtrx << C3DMatrix;

  vcl_string landmarkName, segmentName;

  // dictionaryTagArray holds lm_name <-> segment name association:
  // for example:
  // SACR	IPE
  // LASI	IPE
  // LTHI	LSH
  // LKNE LSH...
  // where SACR, LASI, LTHI and LKNE are landmark names 
  // while IPE and LSH are segment names
  mafTagArray *dictionaryTagArray = mafTagArray::New();

  // dictionary tag item
  mafTagItem	dictTI;
  dictTI.SetNumberOfComponents(3);

  //If a dictionary exist read data from dictionary
  if (this->GetDictionary())
  {	//if dict exist
    vcl_ifstream dictionaryInputStream(this->m_DictionaryFileName, std::ios::in);

    if(dictionaryInputStream.is_open() != 0)
    {

      while(dictionaryInputStream >> landmarkName)	
      {
        dictTI.SetName(landmarkName.c_str());

        dictionaryInputStream >> segmentName;			
        dictTI.SetComponent(segmentName.c_str(), 0);

        dictionaryTagArray->SetTag(dictTI);
      }
    }
    else
    {
      vcl_cout << "Dictionary file does not exist!\n";
      dictionaryTagArray->Delete();
      return 1;
    }

    int currLMId;

    // For every landmarks in c3DLMNamesTagArray:

    for (int i = 0; i < c3DLMNamesTagArray->size(); i++)
    {// for

      // c3DLMNamesTagArray(i) is in the dictionary?
      mafVMELandmarkCloud *currentCloud;
      const char *newSegName;

      // searching in dictionary for c3DLMNamesTagArray(i)...
      if (dictionaryTagArray->IsTagPresent(c3DLMNamesTagArray->at(i)))
        newSegName = dictionaryTagArray->GetTag(c3DLMNamesTagArray->at(i))->GetComponent(0);
      else
        newSegName = "NOT_IN_DICTIONARY";
      {// (tag found)

        // ok, the landmark exist in the dictionary; does the parent segment already exist?		

        if ((currentCloud = (mafVMELandmarkCloud::SafeDownCast(this->FindInTreeByName(newSegName)))) == NULL)
        {
          // parent not exist => create the new cloud

          //new segment name
          mafVMELandmarkCloud *newVmeSegment = mafVMELandmarkCloud::New();
          newVmeSegment->SetRadius(15);
          newVmeSegment->SetDefaultVisibility(0);
          newVmeSegment->SetName(newSegName);
          currentCloud = newVmeSegment;

          //Add new cloud to vme tree
          this->AddChild(newVmeSegment);

          currentCloud = mafVMELandmarkCloud::SafeDownCast(this->FindInTreeByName(newSegName));
          //newVmeSegment->Delete();
          //newVmeSegment = NULL;
        }			

        currLMId = currentCloud->AppendLandmark(c3DLMNamesTagArray->at(i));	

      }

      // Set ith landmark coordinates for every timestamp

      for (int k = 0; k < C3DMatrix.rows(); k++)
      {		
        currentCloud->SetLandmark /*ForTimeFrame*/(currLMId,
          C3DMatrix( k, i * 3), 
          C3DMatrix( k, i * 3 + 1), 
          C3DMatrix( k, i * 3 + 2),
          1.0 * k / finalFreq/*,k + (int)(*this->first_frame)*/);
      }

    }//end for

  }//end (if dict exist)

  else
  {// if dict does not exist:

    double not_used_identifier = 9999;
    // create only one cloud and append all the landmark to it
    for (int i = 0;  i < c3DLMNamesTagArray->size() ; i++)
    {
      // Create num_markers landmarks named from c3DLMNamesTagArray;				  {
      this->C3D_DLCloud->AppendLandmark(c3DLMNamesTagArray->at(i));
      this->C3D_DLCloud->SetName("dummy");
      // set coordinates for each landmark for each time stamp
      for (int k = 0; k < C3DMatrix.rows(); k++)
      {
        // if landmark position is outside valid range, ignore it
        if (fabs(C3DMatrix(k, i * 3)) > not_used_identifier || 
          fabs(C3DMatrix(k, i * 3 + 1)) > not_used_identifier ||
          fabs(C3DMatrix(k, i * 3 + 2)) > not_used_identifier)
          continue;
        this->C3D_DLCloud->SetLandmark/*ForTimeFrame*/(i, C3DMatrix(k, i * 3),
          C3DMatrix(k, i * 3 + 1),
          C3DMatrix(k, i * 3 + 2), 1.0 * k / finalFreq/*,
          k + (int)(*this->first_frame)*/);
      }
    }// end for lm_id
    this->AddChild(C3D_DLCloud);
  } 

  //Clean up
  dictionaryTagArray->Delete();
  return 0; 

}

//----------------------------------------------------------------------------
void mafVMEC3DData::Read_C3D_Parameters(std::vector<mafString> *mlabels, FILE *infile)
//----------------------------------------------------------------------------
{ 

  char				cdum,dim,type,gname[25],pname[25];
  unsigned char	nrow,ncol,i;
  // char string[256];
  int				offset;
  fpos_t			gbyte;

  /* Read in Header */

  // Start at Byte 512 of infile

  fseek(infile,512,0);

  // First four bytes specified

  // bytes 1 and 2 are part of ID key (1 and 80)
  fread(&cdum,sizeof(char),1,infile);		// byte 1
  fread(&cdum,sizeof(char),1,infile);		// byte 2
  // byte 3 holds # of parameter records to follow
  // but is not set correctly by 370. Ignore.
  fread(&cdum,sizeof(char),1,infile);		// byte 3
  // byte 4 is processor type, Vicon uses DEC (type 2)
  fread(&cdum,sizeof(char),1,infile);		// byte 3
  if (cdum-83 != 2) 
  {
    printf("Non DEC processor type specified\n");
    //	getch();
  }

  // Because it is unknown how many groups and how many
  // paremeters/group are stored, must scan for each variable 
  // of interest.
  // Note: Only data of interest passed back from this routine
  // is read. Program can be modified to read other data values.

  // 1st scan for group POINT
  // Parameters stored in POINT are:
  //		1. LABELS
  //		2. DESCRIPTIONS
  //		3. USED
  //		4. UNITS
  //		5. SCALE
  //		6. RATE
  //		7. DATA_START
  //		8. FAMES
  //		9. INITIAL_COMMAND
  //		10.X_SCREEN
  //		11.Y_SCREEN

  printf("searching for group POINT ... ");
  while (strncmp(gname,"POINT",5) != 0)
  {
    fread(&gname,sizeof(char),5,infile);
    fseek(infile,-4*(long)sizeof(char),SEEK_CUR);
  }
  fseek(infile,4*sizeof(char),SEEK_CUR);

  // Record this file position to go back to for each parameter in group
  fgetpos(infile, &gbyte);
  printf("found group POINT\n");

  // Scan for each parameter of interest in group POINT
  while (strncmp(pname,"LABELS",6) != 0)
  {
    fread(&pname,sizeof(char),6,infile);
    fseek(infile,-5*(long)sizeof(char),SEEK_CUR);
  }
  // reposition to end of LABELS
  fseek(infile,5*sizeof(char),SEEK_CUR);
  printf("\tfound parameter LABELS\n");

  RPF(&offset, &type, &dim, infile);	// dim should be 2 for a 2D array of labels[np][4]
  // read in array dimensions: should be 4 x np
  fread(&ncol, sizeof(char),1,infile);
  fread(&nrow, sizeof(char),1,infile);
  //mlabels = cmatrix(1,nrow,1,ncol);

  for (i=1;i<=nrow;i++) 
  {
    mafString lmname;
    char tmpstr[100];
    fgets(tmpstr, ncol+1, infile);
    mafString lm_name = this->strrtrim(tmpstr);
    mlabels->push_back(lm_name);
  }
} 

//----------------------------------------------------------------------------
void mafVMEC3DData::RPF(int *offset, char *type, char *dim, FILE *infile)
//----------------------------------------------------------------------------
{
  char	offset_low,offset_high;
  // Read Parameter Format for the variable following the 
  // parameter name
  //		offset = number of bytes to start of next parameter (2 Bytes, reversed order: low, high)
  //		T = parameter type: -1 = char, 1 = boolean, 2 = int, 3 = float
  //		D = total size of array storage (incorrect, ignore)
  //		d = number of dimensions for an array parameter (d=0 for single value)
  //		dlen = length of data
  fread(&offset_low,sizeof(char),1,infile);		// byte 1
  fread(&offset_high,sizeof(char),1,infile);	// byte 2
  *offset = 256*offset_high + offset_low;
  fread(type, sizeof(char),1,infile);				// byte 3
  fread(dim, sizeof(char),1,infile);				// byte 4
}

//----------------------------------------------------------------------------
float mafVMEC3DData::ConvertDecToFloat(char bytes[4])
//----------------------------------------------------------------------------
{
  char p[4];
  p[0] = bytes[2];
  p[1] = bytes[3];
  p[2] = bytes[0];
  p[3] = bytes[1];
  if (p[0] || p[1] || p[2] || p[3])
    --p[3];          // adjust exponent

  return *(float*)p;
}

//----------------------------------------------------------------------------
void mafVMEC3DData::ConvertFloatToDec(float f, char* bytes)
//----------------------------------------------------------------------------
{
  char* p = (char*)&f;
  bytes[0] = p[2];
  bytes[1] = p[3];
  bytes[2] = p[0];
  bytes[3] = p[1];
  if (bytes[0] || bytes[1] || bytes[2] || bytes[3])
    ++bytes[1];      // adjust exponent
}

//----------------------------------------------------------------------------
char* mafVMEC3DData::strrtrim( char* s)
//----------------------------------------------------------------------------
{
  int i;

  if (s) {
    i = strlen(s); while ((--i)>0 && isspace(s[i]) ) s[i]=0;
  }
  return s;
}
