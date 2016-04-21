/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkLHPOpenGLVolumeTextureMapper3D.cpp,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPOpenGLVolumeTextureMapper3D.cpp,v $
Language:  C++
Date:      $Date: 2010-04-14 16:27:46 $
Version:   $Revision: 1.1.2.3 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#include "mafDefines.h" 

//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "vtkLHPOpenGLVolumeTextureMapper3D.h"

#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkVolumeProperty.h"
#include "vtkPlaneCollection.h"
#include "vtkTimerLog.h"
#include "vtkCamera.h"
#include "vtkTransform.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkMath.h"

// Copied and adapted from VTK 5.4 vtkOpenGLVolumeTextureMapper3D
// using glew to work on OpenGL extensions

vtkCxxRevisionMacro(vtkLHPOpenGLVolumeTextureMapper3D, "$Revision: 1.1.2.3 $");
vtkStandardNewMacro(vtkLHPOpenGLVolumeTextureMapper3D);


//----------------------------------------------------------------------------
vtkLHPOpenGLVolumeTextureMapper3D::vtkLHPOpenGLVolumeTextureMapper3D()
//----------------------------------------------------------------------------
{
	this->Initialized                  =  0;
	this->Volume1Index                 =  0;
	this->Volume2Index                 =  0;
	this->Volume3Index                 =  0;
	this->ColorLookupIndex             =  0;
	this->AlphaLookupIndex             =  0;
	this->RenderWindow                 = NULL;
}


//----------------------------------------------------------------------------
vtkLHPOpenGLVolumeTextureMapper3D::~vtkLHPOpenGLVolumeTextureMapper3D()
//----------------------------------------------------------------------------
{
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::PrintSelf(ostream & os, vtkIndent indent)
//----------------------------------------------------------------------------
{
	this->Superclass::PrintSelf(os, indent);
}


//----------------------------------------------------------------------------
int vtkLHPOpenGLVolumeTextureMapper3D::IsRenderSupported(vtkVolumeProperty * property)
//----------------------------------------------------------------------------
{

	if ( !this->Initialized )
	{
		this->Initialize();
	}

	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NO_METHOD )
	{
		return 0;
	}


	if ( !this->GetInput() )
	{
		return 0;
	}

	if ( this->GetInput()->GetNumberOfScalarComponents() > 1 &&
		property->GetIndependentComponents() )
	{
		return 0;
	}

	return 1;
}

//----------------------------------------------------------------------------
// use Glew instead of vtkOpenGLExtensionManager to test support of 
// OpenGL extensions
void vtkLHPOpenGLVolumeTextureMapper3D::Initialize()
//----------------------------------------------------------------------------
{
	this->Initialized = 1;
	
	GLenum error = glewInit();
	if( error != GLEW_OK ) 
	{
		vtkErrorMacro(<<"Glew init failed.");
		return;
	}

	// check if various extensions are supported

	int supports_texture3D = glewIsSupported( "GL_EXT_texture3D" );
	int supports_multitexture = glewIsSupported( "GL_ARB_multitexture" );
	
	int supports_GL_NV_texture_shader2     = glewIsSupported( "GL_NV_texture_shader2" );
	int supports_GL_NV_register_combiners2 = glewIsSupported( "GL_NV_register_combiners2" );
	int supports_GL_ATI_fragment_shader    = glewIsSupported( "GL_ATI_fragment_shader" );
	int supports_GL_ARB_fragment_program   = glewIsSupported( "GL_ARB_fragment_program" );
	int supports_GL_ARB_vertex_program     = glewIsSupported( "GL_ARB_vertex_program" );
	int supports_GL_NV_register_combiners  = glewIsSupported( "GL_NV_register_combiners" );

	int canDoFP = 0;
	int canDoNV = 0;

	if ( supports_texture3D          &&
		supports_multitexture       &&
		supports_GL_ARB_fragment_program   &&
		supports_GL_ARB_vertex_program     &&
		glTexImage3D               &&
		glActiveTexture            &&
		glMultiTexCoord3fv         &&
		glGenProgramsARB              &&
		glDeleteProgramsARB           &&
		glBindProgramARB              &&
		glProgramStringARB            &&
		glProgramLocalParameter4fARB )
	{    
		canDoFP = 1;
	}

	if ( supports_texture3D          &&
		supports_multitexture       &&
		supports_GL_NV_texture_shader2     &&
		supports_GL_NV_register_combiners2 &&
		supports_GL_NV_register_combiners  &&
		glTexImage3D               &&
		glActiveTexture            &&
		glMultiTexCoord3fv         &&
		glCombinerParameteriNV        &&
		glCombinerStageParameterfvNV  &&
		glCombinerInputNV             &&
		glCombinerOutputNV            &&
		glFinalCombinerInputNV )
	{
		canDoNV = 1;
	}

	// can't do either
	if ( !canDoFP && !canDoNV )
	{
		this->RenderMethod = vtkLHPVolumeTextureMapper3D::NO_METHOD;
	}
	// can only do FragmentProgram
	else if ( canDoFP && !canDoNV )
	{
		//this->RenderMethod = vtkVolumeTextureMapper3D::FRAGMENT_PROGRAM_METHOD;  
		
		// Youbing: fragment currently not supported
		this->RenderMethod = vtkLHPVolumeTextureMapper3D::NO_METHOD;
	}
	// can only do NVidia method
	else if ( !canDoFP && canDoNV )
	{
		this->RenderMethod = vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD;
	}
	// can do both - pick the preferred one
	else
	{
		//this->RenderMethod = this->PreferredRenderMethod;

		// as only NV code was migrated we can only support NV
		this->RenderMethod = vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD;
	}

}


//----------------------------------------------------------------------------
int vtkLHPOpenGLVolumeTextureMapper3D::IsTextureSizeSupported( int size[3] )
//----------------------------------------------------------------------------
{
	
	if ( this->GetInput()->GetNumberOfScalarComponents() < 4 )
	{
		if ( size[0]*size[1]*size[2] > 128*256*256 )
		{
			return 0;
		}

		glTexImage3D(GL_PROXY_TEXTURE_3D, 0, GL_RGBA8, size[0]*2,
			size[1]*2, size[2], 0, GL_RGBA, GL_UNSIGNED_BYTE,
			this->Volume2 );
	}
	else
	{
		if ( size[0]*size[1]*size[2] > 128*128*128 )
		{
			return 0;
		}

		glTexImage3D( GL_PROXY_TEXTURE_3D, 0, GL_RGBA8, size[0]*2,
			size[1]*2, size[2]*2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			this->Volume2 );
	}


	GLint params[1];
	glGetTexLevelParameteriv ( GL_PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH,
		params );

	if ( params[0] != 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}

//----------------------------------------------------------------------------
// Release the graphics resources used by this texture.
void vtkLHPOpenGLVolumeTextureMapper3D::ReleaseGraphicsResources(vtkWindow
															  *renWin)
//----------------------------------------------------------------------------
{
	if (( this->Volume1Index || this->Volume2Index ||
		this->Volume3Index || this->ColorLookupIndex) && renWin)
	{
		static_cast<vtkRenderWindow *>(renWin)->MakeCurrent();
#ifdef GL_VERSION_1_1
		// free any textures
		this->DeleteTextureIndex( &this->Volume1Index );
		this->DeleteTextureIndex( &this->Volume2Index );
		this->DeleteTextureIndex( &this->Volume3Index );
		this->DeleteTextureIndex( &this->ColorLookupIndex );
		this->DeleteTextureIndex( &this->AlphaLookupIndex );
#endif
	}
	this->Volume1Index     = 0;
	this->Volume2Index     = 0;
	this->Volume3Index     = 0;
	this->ColorLookupIndex = 0;
	this->RenderWindow     = NULL;
	this->Modified();
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::Render(vtkRenderer * ren, vtkVolume * vol)
//----------------------------------------------------------------------------
{

	ren->GetRenderWindow()->MakeCurrent();

	if ( !this->Initialized )
	{
		this->Initialize();
	}

	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NO_METHOD )
	{
		vtkErrorMacro( "required extensions not supported" );
		return;
	}

	vtkMatrix4x4       *matrix = vtkMatrix4x4::New();
	vtkPlaneCollection *clipPlanes;
	vtkPlane           *plane;
	int                numClipPlanes = 0;
	double             planeEquation[4];


	// build transformation
	vol->GetMatrix(matrix);
	matrix->Transpose();

	glPushAttrib(GL_ENABLE_BIT   |
		GL_COLOR_BUFFER_BIT   |
		GL_STENCIL_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT   |
		GL_POLYGON_BIT        |
		GL_TEXTURE_BIT);

	int i;

	// Use the OpenGL clip planes
	clipPlanes = this->ClippingPlanes;
	if ( clipPlanes )
	{
		numClipPlanes = clipPlanes->GetNumberOfItems();
		if (numClipPlanes > 6)
		{
			vtkErrorMacro(<< "OpenGL guarantees only 6 additional clipping planes");
		}

		for (i = 0; i < numClipPlanes; i++)
		{
			glEnable(static_cast<GLenum>(GL_CLIP_PLANE0+i));

			plane = static_cast<vtkPlane *>(clipPlanes->GetItemAsObject(i));

			planeEquation[0] = plane->GetNormal()[0];
			planeEquation[1] = plane->GetNormal()[1];
			planeEquation[2] = plane->GetNormal()[2];
			planeEquation[3] = -(planeEquation[0]*plane->GetOrigin()[0]+
				planeEquation[1]*plane->GetOrigin()[1]+
				planeEquation[2]*plane->GetOrigin()[2]);
			glClipPlane(static_cast<GLenum>(GL_CLIP_PLANE0+i),planeEquation);
		}
	}

	// insert model transformation
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMultMatrixd(matrix->Element[0]);

	glColor4f( 1.0, 1.0, 1.0, 1.0 );

	// Turn lighting off - the polygon textures already have illumination
	glDisable( GL_LIGHTING );

	//vtkGraphicErrorMacro(ren->GetRenderWindow(),"Before actual render method");
	switch ( this->RenderMethod )
	{
		case vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD:
			this->RenderNV(ren,vol);
			break;
		//case vtkLHPVolumeTextureMapper3D::FRAGMENT_PROGRAM_METHOD:
		//	this->RenderFP(ren,vol);
		//	break;
	}

	// pop transformation matrix
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	matrix->Delete();
	glPopAttrib();


	glFlush();
	glFinish();

	
	this->Timer->StopTimer();

	this->TimeToDraw = static_cast<float>(this->Timer->GetElapsedTime());

	// If the timer is not accurate enough, set it to a small
	// time so that it is not zero
	if ( this->TimeToDraw == 0.0 )
	{
		this->TimeToDraw = 0.0001;
	}
	
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderNV( vtkRenderer *ren, vtkVolume *vol )
//----------------------------------------------------------------------------
{
	glAlphaFunc (GL_GREATER, static_cast<GLclampf>(0));
	glEnable (GL_ALPHA_TEST);

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	int components = this->GetInput()->GetNumberOfScalarComponents();
	switch ( components )
	{
	case 1:
		if ( !vol->GetProperty()->GetShade() )
		{
			this->RenderOneIndependentNoShadeNV(ren,vol);
		}
		else
		{
			this->RenderOneIndependentShadeNV(ren,vol);
		}
		break;
	
	case 2:
		if ( !vol->GetProperty()->GetShade() )
		{
			this->RenderTwoDependentNoShadeNV(ren,vol);
		}
		else
		{
			this->RenderTwoDependentShadeNV(ren,vol);
		}
		break;

	case 3:
	case 4:
		if ( !vol->GetProperty()->GetShade() )
		{
			this->RenderFourDependentNoShadeNV(ren,vol);
		}
		else
		{
			this->RenderTwoDependentShadeNV(ren,vol);
		}
		break;
		

	}

	glActiveTexture( GL_TEXTURE2 );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );

	glActiveTexture( GL_TEXTURE1 );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );

	glActiveTexture( GL_TEXTURE0 );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );

	glDisable( GL_TEXTURE_SHADER_NV );

	glDisable( GL_REGISTER_COMBINERS_NV);
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderOneIndependentNoShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupOneIndependentTextures( ren, vol );

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersNoShadeNV( ren, vol, 1 );

	int stages[4] = {1,0,0,0};
	this->RenderPolygons( ren, vol, stages );
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderOneIndependentShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupOneIndependentTextures( ren, vol );

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersShadeNV( ren, vol, 1 );

	int stages[4] = {1,0,1,0};
	this->RenderPolygons( ren, vol, stages );
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::DeleteTextureIndex( GLuint *index )
//----------------------------------------------------------------------------
{
	if (glIsTexture(*index))
	{
		GLuint tempIndex;
		tempIndex = *index;
		glDeleteTextures(1, &tempIndex);
		*index = 0;
	}
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::CreateTextureIndex( GLuint *index )
//----------------------------------------------------------------------------
{
	GLuint tempIndex=0;
	glGenTextures(1, &tempIndex);
	*index = static_cast<long>(tempIndex);
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderPolygons( vtkRenderer *ren,
													vtkVolume *vol,
													int stages[4] )
//----------------------------------------------------------------------------
{
	vtkRenderWindow *renWin = ren->GetRenderWindow();

	if ( renWin->CheckAbortStatus() )
	{
		return;
	}

	double bounds[27][6];
	float distance2[27];

	int   numIterations;
	int i, j, k;

	// No cropping case - render the whole thing
	if ( !this->Cropping )
	{
		// Use the input data bounds - we'll take care of the volume's
		// matrix during rendering
		this->GetInput()->GetBounds(bounds[0]);
		numIterations = 1;
	}
	// Simple cropping case - render the subvolume
	else if ( this->CroppingRegionFlags == 0x2000 )
	{
		this->GetCroppingRegionPlanes(bounds[0]);
		numIterations = 1;
	}
	// Complex cropping case - render each region in back-to-front order
	else
	{
		// Get the camera position
		double camPos[4];
		ren->GetActiveCamera()->GetPosition(camPos);

		double volBounds[6];
		this->GetInput()->GetBounds(volBounds);

		// Pass camera through inverse volume matrix
		// so that we are in the same coordinate system
		vtkMatrix4x4 *volMatrix = vtkMatrix4x4::New();
		vol->GetMatrix( volMatrix );
		camPos[3] = 1.0;
		volMatrix->Invert();
		volMatrix->MultiplyPoint( camPos, camPos );
		volMatrix->Delete();
		if ( camPos[3] )
		{
			camPos[0] /= camPos[3];
			camPos[1] /= camPos[3];
			camPos[2] /= camPos[3];
		}

		// These are the region limits for x (first four), y (next four) and
		// z (last four). The first region limit is the lower bound for
		// that axis, the next two are the region planes along that axis, and
		// the final one in the upper bound for that axis.
		float limit[12];
		for ( i = 0; i < 3; i++ )
		{
			limit[i*4  ] = volBounds[i*2];
			limit[i*4+1] = this->CroppingRegionPlanes[i*2];
			limit[i*4+2] = this->CroppingRegionPlanes[i*2+1];
			limit[i*4+3] = volBounds[i*2+1];
		}

		// For each of the 27 possible regions, find out if it is enabled,
		// and if so, compute the bounds and the distance from the camera
		// to the center of the region.
		int numRegions = 0;
		int region;
		for ( region = 0; region < 27; region++ )
		{
			int regionFlag = 1<<region;

			if ( this->CroppingRegionFlags & regionFlag )
			{
				// what is the coordinate in the 3x3x3 grid
				int loc[3];
				loc[0] = region%3;
				loc[1] = (region/3)%3;
				loc[2] = (region/9)%3;

				// compute the bounds and center
				float center[3];
				for ( i = 0; i < 3; i++ )
				{
					bounds[numRegions][i*2  ] = limit[4*i+loc[i]];
					bounds[numRegions][i*2+1] = limit[4*i+loc[i]+1];
					center[i] =
						(bounds[numRegions][i*2  ] +
						bounds[numRegions][i*2+1])/2.0;
				}

				// compute the distance squared to the center
				distance2[numRegions] =
					(camPos[0]-center[0])*(camPos[0]-center[0]) +
					(camPos[1]-center[1])*(camPos[1]-center[1]) +
					(camPos[2]-center[2])*(camPos[2]-center[2]);

				// we've added one region
				numRegions++;
			}
		}
		// Do a quick bubble sort on distance
		for ( i = 1; i < numRegions; i++ )
		{
			for ( j = i; j > 0 && distance2[j] > distance2[j-1]; j-- )
			{
				float tmpBounds[6];
				float tmpDistance2;

				for ( k = 0; k < 6; k++ )
				{
					tmpBounds[k] = bounds[j][k];
				}
				tmpDistance2 = distance2[j];

				for ( k = 0; k < 6; k++ )
				{
					bounds[j][k] = bounds[j-1][k];
				}
				distance2[j] = distance2[j-1];

				for ( k = 0; k < 6; k++ )
				{
					bounds[j-1][k] = tmpBounds[k];
				}
				distance2[j-1] = tmpDistance2;

			}
		}

		numIterations = numRegions;
	}

	// loop over all regions we need to render
	for ( int loop = 0;
		loop < numIterations;
		loop++ )
	{
		// Compute the set of polygons for this region
		// according to the bounds
		this->ComputePolygons( ren, vol, bounds[loop] );

		// Loop over the polygons
		for ( i = 0; i < this->NumberOfPolygons; i++ )
		{
			if ( i%64 == 1 )
			{
				glFlush();
				glFinish();
			}

			if ( renWin->CheckAbortStatus() )
			{
				return;
			}

			float *ptr = this->PolygonBuffer + 36*i;

			glBegin( GL_TRIANGLE_FAN );

			for ( j = 0; j < 6; j++ )
			{
				if ( ptr[0] < 0.0 )
				{
					break;
				}

				for ( k = 0; k < 4; k++ )
				{
					if ( stages[k] )
					{
						glMultiTexCoord3fv( GL_TEXTURE0 + k, ptr );
					}
				}
				glVertex3fv( ptr+3 );

				ptr += 6;
			}
			glEnd();
		}
	}
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::Setup3DTextureParameters( vtkVolumeProperty *property )
//----------------------------------------------------------------------------
{
	if ( property->GetInterpolationType() == VTK_NEAREST_INTERPOLATION )
	{
		glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	else
	{
		glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::SetupRegisterCombinersNoShadeNV( vtkRenderer *vtkNotUsed(ren),
																	 vtkVolume *vtkNotUsed(vol),
																	 int components )
//----------------------------------------------------------------------------
{
	if ( components < 3 )
	{
		glActiveTexture(GL_TEXTURE2);
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);

		if ( components == 1 )
		{
			glActiveTexture(GL_TEXTURE3);
			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
		}
	}


	glEnable(GL_REGISTER_COMBINERS_NV);
	glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
	glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, GL_TRUE);

	glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	if ( components < 3 )
	{
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE1, GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	}
	else
	{
		glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE0, GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	}

	if ( components == 1 )
	{
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE1, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
	}
	else
	{
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE3, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
	}
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::SetupRegisterCombinersShadeNV( vtkRenderer *ren,
																   vtkVolume *vol,
																   int components )
//----------------------------------------------------------------------------
{
	if ( components == 1 )
	{
		glActiveTexture(GL_TEXTURE3);
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_NONE);
	}

	GLfloat white[4] = {1,1,1,1};

	GLfloat lightDirection[2][4];
	GLfloat lightDiffuseColor[2][4];
	GLfloat lightSpecularColor[2][4];
	GLfloat halfwayVector[2][4];
	GLfloat ambientColor[4];

	// Gather information about the light sources. Although we gather info for multiple light sources,
	// in this case we will only use the first one, and will duplicate it (in opposite direction) to
	// approximate two-sided lighting.
	this->GetLightInformation( ren, vol, lightDirection, lightDiffuseColor, 
		lightSpecularColor, halfwayVector, ambientColor );

	float specularPower = vol->GetProperty()->GetSpecularPower();

	glEnable(GL_REGISTER_COMBINERS_NV);    
	glEnable( GL_PER_STAGE_CONSTANTS_NV );
	glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 8);
	glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, GL_TRUE);

	// Stage 0
	//
	//  N dot L is computed into GL_SPARE0_NV
	// -N dot L is computed into GL_SPARE1_NV
	//
	glCombinerStageParameterfvNV( GL_COMBINER0_NV, GL_CONSTANT_COLOR0_NV, lightDirection[0] );

	glCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_CONSTANT_COLOR0_NV, GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE2,       GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_CONSTANT_COLOR0_NV, GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_TEXTURE2,       GL_EXPAND_NEGATE_NV, GL_RGB );

	glCombinerOutputNV( GL_COMBINER0_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, GL_DISCARD_NV, 
		GL_NONE, GL_NONE, GL_TRUE, GL_TRUE, GL_FALSE );

	// Stage 1
	//
	// lightColor * max( 0, (N dot L)) + lightColor * max( 0, (-N dot L)) is computed into GL_SPARE0_NV
	// 
	glCombinerStageParameterfvNV( GL_COMBINER1_NV, GL_CONSTANT_COLOR0_NV, lightDiffuseColor[0] );

	glCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV,          GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_SPARE1_NV,          GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );

	glCombinerOutputNV( GL_COMBINER1_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_SPARE0_NV, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// Stage 2
	//
	// result from Stage 1 is added to the ambient color and stored in GL_PRIMARY_COLOR_NV
	//
	glCombinerStageParameterfvNV( GL_COMBINER2_NV, GL_CONSTANT_COLOR0_NV, white );
	glCombinerStageParameterfvNV( GL_COMBINER2_NV, GL_CONSTANT_COLOR1_NV, ambientColor );    

	glCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV,          GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_CONSTANT_COLOR1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );

	glCombinerOutputNV( GL_COMBINER2_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, 
		GL_PRIMARY_COLOR_NV, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// Stage 3
	// 
	//  N dot H is computed into GL_SPARE0_NV
	// -N dot H is computed into GL_SPARE1_NV
	//
	glCombinerStageParameterfvNV( GL_COMBINER3_NV, GL_CONSTANT_COLOR0_NV, halfwayVector[0] );

	glCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_CONSTANT_COLOR0_NV, GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE2, GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_CONSTANT_COLOR0_NV, GL_EXPAND_NORMAL_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_TEXTURE2, GL_EXPAND_NEGATE_NV, GL_RGB );

	glCombinerOutputNV( GL_COMBINER3_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, 
		GL_DISCARD_NV, GL_NONE, GL_NONE, GL_TRUE, GL_TRUE, GL_FALSE );

	// Stage 4
	//
	// if the specular power is greater than 1, then
	//
	//  N dot H squared is computed into GL_SPARE0_NV
	// -N dot H squared is computed into GL_SPARE1_NV
	//
	// otherwise these registers are simply multiplied by white
	glCombinerStageParameterfvNV( GL_COMBINER4_NV, GL_CONSTANT_COLOR0_NV, white );

	glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	if ( specularPower > 1.0 )
	{
		glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}
	else
	{
		glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER4_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}

	glCombinerOutputNV( GL_COMBINER4_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, GL_DISCARD_NV, 
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// Stage 5
	//
	// if the specular power is greater than 3, then
	//
	//  N dot H to the fourth is computed into GL_SPARE0_NV
	// -N dot H to the fourth is computed into GL_SPARE1_NV
	//
	// otherwise these registers are simply multiplied by white
	glCombinerStageParameterfvNV( GL_COMBINER5_NV, GL_CONSTANT_COLOR0_NV, white );

	glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV,  GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_SPARE1_NV,  GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	if ( specularPower > 3.0 )
	{
		glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}
	else
	{
		glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER5_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}

	glCombinerOutputNV( GL_COMBINER5_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, GL_DISCARD_NV, 
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// Stage 6
	//
	// if the specular power is greater than 6, then
	//
	//  N dot H to the eighth is computed into GL_SPARE0_NV
	// -N dot H to the eighth is computed into GL_SPARE1_NV
	//
	// otherwise these registers are simply multiplied by white
	glCombinerStageParameterfvNV( GL_COMBINER6_NV, GL_CONSTANT_COLOR0_NV, white );

	glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );

	if ( specularPower > 6.0 )
	{
		glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}
	else
	{
		glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		glCombinerInputNV( GL_COMBINER6_NV, GL_RGB, GL_VARIABLE_D_NV, 
			GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	}

	glCombinerOutputNV( GL_COMBINER6_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, 
		GL_DISCARD_NV, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );


	// Stage 7
	//
	// Add the two specular contributions and multiply each by the specular color.
	glCombinerStageParameterfvNV( GL_COMBINER7_NV, GL_CONSTANT_COLOR0_NV, lightSpecularColor[0] );
	glCombinerStageParameterfvNV( GL_COMBINER7_NV, GL_CONSTANT_COLOR1_NV, lightSpecularColor[1] );    

	glCombinerInputNV( GL_COMBINER7_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER7_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER7_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	glCombinerInputNV( GL_COMBINER7_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_CONSTANT_COLOR1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );

	glCombinerOutputNV( GL_COMBINER7_NV, GL_RGB, GL_DISCARD_NV, 
		GL_DISCARD_NV, GL_SPARE0_NV, 
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_PRIMARY_COLOR_NV, 
		GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	if ( components < 3 )
	{
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_TEXTURE1, 
			GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	}
	else
	{
		glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_TEXTURE0, 
			GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	}
	glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, 
		GL_UNSIGNED_IDENTITY_NV, GL_RGB  );
	glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SPARE0_NV, 
		GL_UNSIGNED_IDENTITY_NV, GL_RGB  );

	if ( components == 1 )
	{
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE1, 
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
	}
	else
	{
		glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE3, 
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
	}

}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::SetupOneIndependentTextures( vtkRenderer *vtkNotUsed(ren),
																 vtkVolume *vol )
//----------------------------------------------------------------------------
{
	glActiveTexture( GL_TEXTURE0 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );
		glTexEnvi( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	glActiveTexture( GL_TEXTURE2 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );
		glTexEnvi( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	// Update the volume containing the 2 byte scalar / gradient magnitude
	if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index )
	{
		int dim[3];
		this->GetVolumeDimensions(dim);
		this->DeleteTextureIndex(&this->Volume3Index);

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume1Index);
		this->CreateTextureIndex(&this->Volume1Index);
		glBindTexture( GL_TEXTURE_3D, this->Volume1Index);
		glTexImage3D( GL_TEXTURE_3D, 0, GL_LUMINANCE8_ALPHA8, dim[0], dim[1], dim[2], 0,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, this->Volume1 );

		glActiveTexture( GL_TEXTURE2 );
		glBindTexture( GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume2Index);
		this->CreateTextureIndex(&this->Volume2Index);
		glBindTexture( GL_TEXTURE_3D, this->Volume2Index);
		glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->Volume2 );
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, this->Volume1Index);
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_3D, this->Volume2Index);
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glTexEnvf ( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV,
			GL_DEPENDENT_AR_TEXTURE_2D_NV );
		glTexEnvi( GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
	}

	// Update the dependent 2D color table mapping scalar value and
	// gradient magnitude to RGBA
	if ( this->UpdateColorLookup( vol ) || !this->ColorLookupIndex )
	{
		this->DeleteTextureIndex( &this->ColorLookupIndex );
		this->DeleteTextureIndex( &this->AlphaLookupIndex );

		this->CreateTextureIndex( &this->ColorLookupIndex );
		glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, this->ColorLookup );
	}

	glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::SetupTwoDependentTextures(
	vtkRenderer *vtkNotUsed(ren),
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	glActiveTexture( GL_TEXTURE0 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	glActiveTexture( GL_TEXTURE2 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );  
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	// Update the volume containing the 3 byte scalars / gradient magnitude
	if ( this->UpdateVolumes( vol ) || !this->Volume1Index || !this->Volume2Index )
	{    
		int dim[3];
		this->GetVolumeDimensions(dim);
		this->DeleteTextureIndex(&this->Volume3Index);

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume1Index);
		this->CreateTextureIndex(&this->Volume1Index);
		glBindTexture(GL_TEXTURE_3D, this->Volume1Index);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, dim[0], dim[1], dim[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->Volume1 );

		glActiveTexture( GL_TEXTURE2 );
		glBindTexture(GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume2Index);
		this->CreateTextureIndex(&this->Volume2Index);
		glBindTexture(GL_TEXTURE_3D, this->Volume2Index);
		glTexImage3D(GL_TEXTURE_3D,0, GL_RGBA8, dim[0], dim[1], dim[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->Volume2 );
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_3D, this->Volume1Index);   
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE2 );
	glBindTexture(GL_TEXTURE_3D, this->Volume2Index);   
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glTexEnvf ( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, 
			GL_DEPENDENT_AR_TEXTURE_2D_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
	}

	glActiveTexture( GL_TEXTURE3 );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glTexEnvf ( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, 
			GL_DEPENDENT_GB_TEXTURE_2D_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
	}

	// Update the dependent 2D color table mapping scalar value and
	// gradient magnitude to RGBA
	if ( this->UpdateColorLookup( vol ) || 
		!this->ColorLookupIndex || !this->AlphaLookupIndex )
	{    
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture(GL_TEXTURE_2D,0);
		this->DeleteTextureIndex(&this->ColorLookupIndex);
		this->CreateTextureIndex(&this->ColorLookupIndex);
		glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->ColorLookup );    

		glActiveTexture( GL_TEXTURE3 );
		glBindTexture(GL_TEXTURE_2D,0);
		this->DeleteTextureIndex(&this->AlphaLookupIndex);
		this->CreateTextureIndex(&this->AlphaLookupIndex);
		glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, 256, 256, 0,
			GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
	}

	glActiveTexture( GL_TEXTURE1 );
	glBindTexture(GL_TEXTURE_2D, this->ColorLookupIndex);   

	glActiveTexture( GL_TEXTURE3 );
	glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
}

//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderTwoDependentNoShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupTwoDependentTextures(ren, vol);

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersNoShadeNV( ren, vol, 2 );

	int stages[4] = {1,0,0,0};
	this->RenderPolygons( ren, vol, stages );  
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderTwoDependentShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupTwoDependentTextures( ren, vol );

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersShadeNV( ren, vol, 2 );

	int stages[4] = {1,0,1,0};
	this->RenderPolygons( ren, vol, stages );  
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::SetupFourDependentTextures(
	vtkRenderer *vtkNotUsed(ren),
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	glActiveTexture( GL_TEXTURE0 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	glActiveTexture( GL_TEXTURE1 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	glActiveTexture( GL_TEXTURE2 );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glEnable( GL_TEXTURE_SHADER_NV );  
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_3D);
	}

	// Update the volume containing the 3 byte scalars / gradient magnitude
	if ( this->UpdateVolumes( vol ) || !this->Volume1Index || 
		!this->Volume2Index || !this->Volume3Index )
	{    
		int dim[3];
		this->GetVolumeDimensions(dim);

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture(GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume1Index);
		this->CreateTextureIndex(&this->Volume1Index);
		glBindTexture(GL_TEXTURE_3D, this->Volume1Index);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, dim[0], dim[1], dim[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->Volume1 );

		glActiveTexture( GL_TEXTURE1 );
		glBindTexture(GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume2Index);
		this->CreateTextureIndex(&this->Volume2Index);
		glBindTexture(GL_TEXTURE_3D, this->Volume2Index);   
		glTexImage3D(GL_TEXTURE_3D,0,GL_LUMINANCE8_ALPHA8,dim[0],dim[1],
			dim[2], 0,GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
			this->Volume2 );

		glActiveTexture( GL_TEXTURE2 );
		glBindTexture(GL_TEXTURE_3D,0);
		this->DeleteTextureIndex(&this->Volume3Index);
		this->CreateTextureIndex(&this->Volume3Index);
		glBindTexture(GL_TEXTURE_3D, this->Volume3Index);
		glTexImage3D( GL_TEXTURE_3D,0, GL_RGB8, dim[0], dim[1], dim[2], 0,
			GL_RGB, GL_UNSIGNED_BYTE, this->Volume3 );
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_3D, this->Volume1Index);
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE1 );
	glBindTexture(GL_TEXTURE_3D, this->Volume2Index);   
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE2 );
	glBindTexture(GL_TEXTURE_3D_EXT, this->Volume3Index);   
	this->Setup3DTextureParameters( vol->GetProperty() );

	glActiveTexture( GL_TEXTURE3 );
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_3D );
	if ( this->RenderMethod == vtkLHPVolumeTextureMapper3D::NVIDIA_METHOD )
	{
		glTexEnvf ( GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, 
			GL_DEPENDENT_AR_TEXTURE_2D_NV );
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV,
			GL_TEXTURE1);
	}

	// Update the dependent 2D table mapping scalar value and
	// gradient magnitude to opacity
	if ( this->UpdateColorLookup( vol ) || !this->AlphaLookupIndex )
	{    
		this->DeleteTextureIndex(&this->ColorLookupIndex);

		glActiveTexture( GL_TEXTURE3 );
		glBindTexture(GL_TEXTURE_2D,0);
		this->DeleteTextureIndex(&this->AlphaLookupIndex);
		this->CreateTextureIndex(&this->AlphaLookupIndex);
		glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   

		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, 256, 256, 0,
			GL_ALPHA, GL_UNSIGNED_BYTE, this->AlphaLookup );      
	}

	glActiveTexture( GL_TEXTURE3 );
	glBindTexture(GL_TEXTURE_2D, this->AlphaLookupIndex);   
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderFourDependentNoShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupFourDependentTextures(ren, vol);

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersNoShadeNV( ren, vol, 4 );

	int stages[4] = {1,1,0,0};
	this->RenderPolygons( ren, vol, stages );  
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::RenderFourDependentShadeNV(
	vtkRenderer *ren,
	vtkVolume *vol )
//----------------------------------------------------------------------------
{
	this->SetupFourDependentTextures( ren, vol );

	// Start the timer now
	this->Timer->StartTimer();

	this->SetupRegisterCombinersShadeNV( ren, vol, 4 );

	int stages[4] = {1,1,1,0};
	this->RenderPolygons( ren, vol, stages );  
}


//----------------------------------------------------------------------------
void vtkLHPOpenGLVolumeTextureMapper3D::GetLightInformation(
	vtkRenderer *ren,
	vtkVolume *vol,
	GLfloat lightDirection[2][4],
	GLfloat lightDiffuseColor[2][4],
	GLfloat lightSpecularColor[2][4],
	GLfloat halfwayVector[2][4],
	GLfloat ambientColor[4] )
//----------------------------------------------------------------------------
{
	float ambient = vol->GetProperty()->GetAmbient();
	float diffuse  = vol->GetProperty()->GetDiffuse();
	float specular = vol->GetProperty()->GetSpecular();

	vtkTransform *volumeTransform = vtkTransform::New();

	volumeTransform->SetMatrix( vol->GetMatrix() );
	volumeTransform->Inverse();

	vtkLightCollection *lights = ren->GetLights();
	lights->InitTraversal();

	vtkLight *light[2];
	light[0] = lights->GetNextItem();
	light[1] = lights->GetNextItem();

	int lightIndex = 0;

	double cameraPosition[3];
	double cameraFocalPoint[3];

	ren->GetActiveCamera()->GetPosition( cameraPosition );
	ren->GetActiveCamera()->GetFocalPoint( cameraFocalPoint );

	double viewDirection[3];

	volumeTransform->TransformPoint( cameraPosition, cameraPosition );
	volumeTransform->TransformPoint( cameraFocalPoint, cameraFocalPoint );

	viewDirection[0] = cameraFocalPoint[0] - cameraPosition[0];
	viewDirection[1] = cameraFocalPoint[1] - cameraPosition[1];
	viewDirection[2] = cameraFocalPoint[2] - cameraPosition[2];

	vtkMath::Normalize( viewDirection );


	ambientColor[0] = 0.0;
	ambientColor[1] = 0.0;
	ambientColor[2] = 0.0;  
	ambientColor[3] = 0.0;  

	for ( lightIndex = 0; lightIndex < 2; lightIndex++ )
	{
		float dir[3] = {0,0,0};
		float half[3] = {0,0,0};

		if ( light[lightIndex] == NULL ||
			light[lightIndex]->GetSwitch() == 0 )
		{
			lightDiffuseColor[lightIndex][0] = 0.0;
			lightDiffuseColor[lightIndex][1] = 0.0;
			lightDiffuseColor[lightIndex][2] = 0.0;
			lightDiffuseColor[lightIndex][3] = 0.0;

			lightSpecularColor[lightIndex][0] = 0.0;
			lightSpecularColor[lightIndex][1] = 0.0;
			lightSpecularColor[lightIndex][2] = 0.0;
			lightSpecularColor[lightIndex][3] = 0.0;
		}
		else
		{
			float lightIntensity = light[lightIndex]->GetIntensity();
			double lightColor[3];

			light[lightIndex]->GetColor( lightColor );

			double lightPosition[3];
			double lightFocalPoint[3];
			light[lightIndex]->GetTransformedPosition( lightPosition );
			light[lightIndex]->GetTransformedFocalPoint( lightFocalPoint );

			volumeTransform->TransformPoint( lightPosition, lightPosition );
			volumeTransform->TransformPoint( lightFocalPoint, lightFocalPoint );

			dir[0] = lightPosition[0] - lightFocalPoint[0];
			dir[1] = lightPosition[1] - lightFocalPoint[1];
			dir[2] = lightPosition[2] - lightFocalPoint[2];

			vtkMath::Normalize( dir );

			lightDiffuseColor[lightIndex][0] = lightColor[0]*diffuse*lightIntensity;
			lightDiffuseColor[lightIndex][1] = lightColor[1]*diffuse*lightIntensity;
			lightDiffuseColor[lightIndex][2] = lightColor[2]*diffuse*lightIntensity;
			lightDiffuseColor[lightIndex][3] = 1.0;

			lightSpecularColor[lightIndex][0]= lightColor[0]*specular*lightIntensity;
			lightSpecularColor[lightIndex][1]= lightColor[1]*specular*lightIntensity;
			lightSpecularColor[lightIndex][2]= lightColor[2]*specular*lightIntensity;
			lightSpecularColor[lightIndex][3] = 0.0;

			half[0] = dir[0] - viewDirection[0];
			half[1] = dir[1] - viewDirection[1];
			half[2] = dir[2] - viewDirection[2];

			vtkMath::Normalize( half );

			ambientColor[0] += ambient*lightColor[0];
			ambientColor[1] += ambient*lightColor[1];
			ambientColor[2] += ambient*lightColor[2];      
		}

		lightDirection[lightIndex][0] = (dir[0]+1.0)/2.0;
		lightDirection[lightIndex][1] = (dir[1]+1.0)/2.0;
		lightDirection[lightIndex][2] = (dir[2]+1.0)/2.0;
		lightDirection[lightIndex][3] = 0.0;

		halfwayVector[lightIndex][0] = (half[0]+1.0)/2.0;
		halfwayVector[lightIndex][1] = (half[1]+1.0)/2.0;
		halfwayVector[lightIndex][2] = (half[2]+1.0)/2.0;
		halfwayVector[lightIndex][3] = 0.0;    
	}

	volumeTransform->Delete();

}