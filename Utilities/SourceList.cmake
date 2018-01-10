
set(${PLUGIN_NAME}_Utilities_HDRS "")
set(${PLUGIN_NAME}_Utilities_SRCS "")
set(${PLUGIN_NAME}_Utilities_UIS "")


set(${PLUGIN_NAME}_Utilities_HDRS
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/SIMPLVtkBridge.h
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkEdgeGeom.h
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkQuadGeom.h
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkTetrahedralGeom.h
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkTriangleGeom.h
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkVertexGeom.h
)

set(${PLUGIN_NAME}_Utilities_SRCS
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/SIMPLVtkBridge.cpp
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkEdgeGeom.cpp
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkQuadGeom.cpp
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkTetrahedralGeom.cpp
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkTriangleGeom.cpp
  ${${PLUGIN_NAME}_SOURCE_DIR}/Utilities/VtkVertexGeom.cpp
)

# Organize the Source files for things like Visual Studio and Xcode
cmp_IDE_SOURCE_PROPERTIES( "${PLUGIN_NAME}/Utilities" "${${PLUGIN_NAME}_Utilities_HDRS}" "${${PLUGIN_NAME}_Utilities_SRCS}" "0")

# --------------------------------------------------------------------
# We are using CMake's AuotMoc feature so we do not need to wrap our .cpp files with a specific call to 'moc'

# These generated moc files will be #include in the FilterWidget source file that
# are generated so we need to tell the build system to NOT compile these files
set_source_files_properties( ${${PLUGIN_NAME}_Utilities_Generated_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)

# --------------------------------------------------------------------
# -- Run UIC on the necessary files
QT5_WRAP_UI( ${PLUGIN_NAME}_Utilities_Generated_UI_HDRS ${${PLUGIN_NAME}_Utilities_UIS} )

# --------------------------------------------------------------------
#-- Put the Qt generated files into their own group for IDEs
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Moc" "" "${${PLUGIN_NAME}_Utilities_Generated_MOC_SRCS}" "0")

# --------------------------------------------------------------------
# If you are doing more advanced Qt programming where you are including resources you will have to enable this section
# with your own cmake codes to include your resource file (.qrc) and any other needed files
# QT5_ADD_RESOURCES( ${PLUGIN_NAME}_Generated_RC_SRCS ""  )
# cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${${PLUGIN_NAME}_Generated_RC_SRCS}" "" "0")




