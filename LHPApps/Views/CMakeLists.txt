#
# Copyright and description to be placed here
#

PROJECT(lhpViews)

# Set your list of sources here.
SET(PROJECT_SRCS
mafGraphIDDesc.cpp
mafGraphIDDesc.h
mafMemGraph.cpp
mafMemGraph.h
mafStringSet.cpp
mafStringSet.h
mafViewIntGraph.cpp
mafViewIntGraph.h
mafViewIntGraphWindow.cpp
mafViewIntGraphWindow.h
lhpViewAnalogGraphXY.cpp
lhpViewAnalogGraphXY.h
lhpViewSurfacePlot.cpp
lhpViewSurfacePlot.h
lhpViewOrthoSlice.cpp
lhpViewOrthoSlice.h
lhpViewArbitrarySlice.cpp	
lhpViewArbitrarySlice.h
)

# List libraries that are needed by this project.
# SET(PROJECT_LIBS mafVME mafInteraction mafCore)

# SET(PROJECT_LIBS ${PROJECT_LIBS} vtkMED)

# Create the library.
ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PROJECT_LIBS})

