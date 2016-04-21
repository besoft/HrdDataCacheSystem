/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: lhpInteractorPERBrushFeedback.h,v $
Language:  C++
Date:      $Date: 2010-06-24 12:37:26 $
Version:   $Revision: 1.1.2.5 $
Authors:   Eleonora Mambrini
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __lhpInteractorPERBrushFeedback_h
#define __lhpInteractorPERBrushFeedback_h

#include "mafInteractorPER.h"

class vtkActor2D;
class vtkImageMapper;
class vtkPolyDataMapper2D;
class vtkCoordinate;
class vtkLineSource;
class vtkCubeSource;
class vtkSphereSource;

//----------------------------------------------------------------------------
// forward declarations
//----------------------------------------------------------------------------


/** Segmentation Editing Interactor
*/
class lhpInteractorPERBrushFeedback : public mafInteractorPER
{
public: 
  
  enum BRUSH_SHAPES
  {
    CIRCLE_BRUSH_SHAPE = 0,
    SQUARE_BRUSH_SHAPE,
  };

  mafTypeMacro(lhpInteractorPERBrushFeedback,mafInteractorPER);

  virtual void OnEvent(mafEventBase *event);

  /** Remove actor from render */
  void RemoveActor();

  /** Add actor to render */
  void AddActor();

  /** Enable/disable brush drawing */
  void EnableDrawing(bool enable) { m_EnableDrawing = enable; };

  /** Set the radius */
  void SetRadius(double radius);

  /***/
  void SetBrushShape(int shape);

protected:
  lhpInteractorPERBrushFeedback();
  virtual ~lhpInteractorPERBrushFeedback();
  
  void DrawEllipse(double x, double y);
  void DrawBox(double x, double y);

  int m_Radius;
  int m_CurrentShape;

  vtkCoordinate *m_Coordinate;
  vtkSphereSource *m_SphereSource;
  vtkCubeSource *m_CubeSource;
  vtkPolyDataMapper2D *m_BrushMapper;
  vtkActor2D *m_BrushActor;

  bool m_IsActorAdded;
  bool m_EnableDrawing;

  
private:
  lhpInteractorPERBrushFeedback(const lhpInteractorPERBrushFeedback&);  // Not implemented.
  void operator=(const lhpInteractorPERBrushFeedback&);  // Not implemented.
};
#endif 
