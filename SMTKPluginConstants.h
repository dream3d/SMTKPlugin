/*
 * Your License should go here
 */
#ifndef _smtkpluginconstants_h_
#define _smtkpluginconstants_h_

#include <QtCore/QString>

/**
* @brief This namespace is used to define some Constants for the plugin itself.
*/
namespace SMTKPluginConstants
{
  const QString SMTKPluginPluginFile("SMTKPluginPlugin");
  const QString SMTKPluginPluginDisplayName("SMTKPluginPlugin");
  const QString SMTKPluginBaseName("SMTKPluginPlugin");

  namespace FilterGroups
  {
  	const QString SMTKPluginFilters("SMTKPlugin");
  }
}

/**
* @brief Use this namespace to define any custom GUI widgets that collect FilterParameters
* for a filter. Do NOT define general reusable widgets here.
*/
namespace FilterParameterWidgetType
{
/* const QString SomeCustomWidget("SomeCustomWidget"); */
}
#endif
