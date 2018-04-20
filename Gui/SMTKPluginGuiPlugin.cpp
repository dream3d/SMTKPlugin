

#include "SMTKPluginGuiPlugin.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SMTKPluginGuiPlugin::SMTKPluginGuiPlugin()
: SMTKPluginPlugin()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SMTKPluginGuiPlugin::~SMTKPluginGuiPlugin() = default;

#include "SMTKPlugin/Gui/FilterParameterWidgets/RegisterKnownFilterParameterWidgets.cpp"
