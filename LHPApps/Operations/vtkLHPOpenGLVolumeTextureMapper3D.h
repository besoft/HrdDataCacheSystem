/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkLHPOpenGLVolumeTextureMapper3D.h,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*=========================================================================
Program:   LHP Builder
Module:    $RCSfile: vtkLHPOpenGLVolumeTextureMapper3D.h,v $
Language:  C++
Date:      $Date: 2010-04-14 16:27:46 $
Version:   $Revision: 1.1.2.3 $
Authors:   Youbing Zhao
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/
#ifndef __lhpOpenGLVolumeTextureMapper3D_H__
#define __lhpOpenGLVolumeTextureMapper3D_H__

#include "vtkLHPVolumeTextureMapper3D.h"

// from openMAF/GPUAPI/glew
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include "glew.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

//------------------------------------------------------------------------------ 
// Forward declarations 
//------------------------------------------------------------------------------ 
class vtkRenderWindow;
class vtkVolumeProperty;

/**
class for performing 3D texture volume rendering on MAF2.2
as 3D texture volume rendering is not available on VTK4.4.
This class is adpated from VTK5 vtkOpenGLVolumeTextureMapper3D, 
the major difference is 
1. GLEW is used to test OpenGL extentsions instead of using 
vtkOpenGLExtensionManager in VTK 5.4.
2. Fragment program rendering from vtkOpenGLVolumeTextureMapper3D
is not migrated.

This class can be used on the MAF2.2 platform.But the multiple 
component rendering hasn't been tested.

Please refer to VTK5 vtkOpenGLVolumeTextureMapper3D for detailed 
documentation if there is any. 
*/

class vtkLHPOpenGLVolumeTextureMapper3D : public vtkLHPVolumeTextureMapper3D
{
public:
	vtkTypeRevisionMacro(vtkLHPOpenGLVolumeTextureMapper3D, vtkLHPVolumeTextureMapper3D);
	void PrintSelf(ostream& os, vtkIndent indent);

	static vtkLHPOpenGLVolumeTextureMapper3D * New();

	/** to test if 3d texture volume rendering is supported
	by current hardware or software implementation
	*/
	int IsRenderSupported(vtkVolumeProperty * property);

	/** Render the volume with OpenGL 3D texture extension */
	virtual void Render(vtkRenderer * ren, vtkVolume * vol);

	/**
	// Initialize when we go to render, or go to answer the
	// IsRenderSupported question. Don't call unless we have
	// a valid OpenGL context!
	*/
	vtkGetMacro( Initialized, int );

	/**
	// Release any graphics resources that are being consumed by this texture.
	// The parameter window could be used to determine which graphic
	// resources to release.
	*/
	void ReleaseGraphicsResources(vtkWindow *);


protected:
	vtkLHPOpenGLVolumeTextureMapper3D();
	~vtkLHPOpenGLVolumeTextureMapper3D();

	int              Initialized;
	GLuint           Volume1Index;
	GLuint           Volume2Index;
	GLuint           Volume3Index;
	GLuint           ColorLookupIndex;
	GLuint           AlphaLookupIndex;
	vtkRenderWindow *RenderWindow;

	void GetLightInformation(vtkRenderer *ren,
		vtkVolume *vol,
		GLfloat lightDirection[2][4],
		GLfloat lightDiffuseColor[2][4],
		GLfloat lightSpecularColor[2][4],
		GLfloat halfwayVector[2][4],
		GLfloat *ambient );

	/** use GLEW to test support of OpenGL extensions */
	void Initialize();

	/** main entry for rendering on NV extension supported platform */
	virtual void RenderNV(vtkRenderer *ren, vtkVolume *vol);

	/** rendering volume data of 1 component, no shading */
	void RenderOneIndependentNoShadeNV( vtkRenderer *ren, vtkVolume *vol );
	/** rendering volume data of 1 component, with shading */
	void RenderOneIndependentShadeNV( vtkRenderer *ren, vtkVolume *vol );
	/** rendering volume data of 2 component, no shading */
	void RenderTwoDependentNoShadeNV( vtkRenderer *ren, vtkVolume *vol );
	/** rendering volume data of 2 component, with shading */
	void RenderTwoDependentShadeNV( vtkRenderer *ren, vtkVolume *vol );
	/** rendering volume data of 3-4 component, no shading */
	void RenderFourDependentNoShadeNV( vtkRenderer *ren, vtkVolume *vol );
	/** rendering volume data of 3-4 component, with shading */
	void RenderFourDependentShadeNV( vtkRenderer *ren, vtkVolume *vol );

	void SetupOneIndependentTextures( vtkRenderer *ren, vtkVolume *vol );
	void SetupTwoDependentTextures( vtkRenderer *ren, vtkVolume *vol );
	void SetupFourDependentTextures( vtkRenderer *ren, vtkVolume *vol );

	void SetupRegisterCombinersNoShadeNV( vtkRenderer *ren,
		vtkVolume *vol,
		int components );

	void SetupRegisterCombinersShadeNV( vtkRenderer *ren,
		vtkVolume *vol,
		int components );

	void DeleteTextureIndex( GLuint *index );
	void CreateTextureIndex( GLuint *index );

	void RenderPolygons( vtkRenderer *ren, vtkVolume *vol,int stages[4] );

	/** Common code for setting up interpolation / clamping on 3D textures */
	void Setup3DTextureParameters( vtkVolumeProperty *property );

	/** Check if the texture size can be supported */
	int IsTextureSizeSupported( int size[3] );


private:
	// copy constructor not implemented
	vtkLHPOpenGLVolumeTextureMapper3D(const vtkLHPOpenGLVolumeTextureMapper3D & );
	void operator=(const vtkLHPOpenGLVolumeTextureMapper3D &);

};


#endif
