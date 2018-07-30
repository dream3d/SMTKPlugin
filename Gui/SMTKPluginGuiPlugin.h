#pragma once

#include "SMTKPlugin/SMTKPluginPlugin.h"

class SMTKPluginGuiPlugin : public SMTKPluginPlugin
{
  Q_OBJECT
  Q_INTERFACES(ISIMPLibPlugin)
  Q_PLUGIN_METADATA(IID "net.bluequartz.dream3d.SMTKPluginGuiPlugin")

public:
  SMTKPluginGuiPlugin();
   ~SMTKPluginGuiPlugin() override;
  
  /**
   * @brief Register all the filters with the FilterWidgetFactory
   */
  void registerFilterWidgets(FilterWidgetManager* fwm) override;
  

public:
  SMTKPluginGuiPlugin(const SMTKPluginGuiPlugin&) = delete;            // Copy Constructor Not Implemented
  SMTKPluginGuiPlugin(SMTKPluginGuiPlugin&&) = delete;                 // Move Constructor Not Implemented
  SMTKPluginGuiPlugin& operator=(const SMTKPluginGuiPlugin&) = delete; // Copy Assignment Not Implemented
  SMTKPluginGuiPlugin& operator=(SMTKPluginGuiPlugin&&) = delete;      // Move Assignment Not Implemented
};
