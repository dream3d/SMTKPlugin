/*
 * Your License or Copyright can go here
 */

#include "SMTKPluginPlugin.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <QtCore/QTextStream>

#include "SIMPLib/Filtering/FilterFactory.hpp"

#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/Filtering/IFilterFactory.hpp"

#include "SMTKPlugin/SMTKPluginConstants.h"

// Include the MOC generated file for this class
#include "moc_SMTKPluginPlugin.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SMTKPluginPlugin::SMTKPluginPlugin()
: m_Version(SMTKPlugin::Version::Package())
, m_CompatibilityVersion(SMTKPlugin::Version::Package())
, m_Vendor("Open-Source")
, // Initialize SMTKPlugin's Vendor Name Here
    m_URL("http://www.github.com/bluequartzsoftware/SMTKPlugin")
, // Initialize Company URL Here
    m_Location("")
, // Initialize SMTKPlugin library Location Here
    m_Description("")
, // Initialize SMTKPlugin's Description Here
    m_Copyright("")
, // Initialize SMTKPlugin's Copyright Here
    m_Filters(QList<QString>())
, // Initialize SMTKPlugin's List of Dependencies Here
    m_DidLoad(false)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SMTKPluginPlugin::~SMTKPluginPlugin() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getPluginFileName()
{
  return SMTKPluginConstants::SMTKPluginPluginFile;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getPluginDisplayName()
{
  return SMTKPluginConstants::SMTKPluginPluginDisplayName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getPluginBaseName()
{
  return SMTKPluginConstants::SMTKPluginBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getVersion()
{
  return m_Version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getCompatibilityVersion()
{
  return m_CompatibilityVersion;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getVendor()
{
  return m_Vendor;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getURL()
{
  return m_URL;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getLocation()
{
  return m_Location;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getDescription()
{
  /* PLEASE UPDATE YOUR PLUGIN'S DESCRIPTION FILE.
  It is located at SMTKPlugin/Resources/SMTKPlugin/SMTKPluginDescription.txt */

  QFile licenseFile(":/SMTKPlugin/SMTKPluginDescription.txt");
  QFileInfo licenseFileInfo(licenseFile);
  QString text = "<<--Description was not read-->>";

  if(licenseFileInfo.exists())
  {
    if(licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&licenseFile);
      text = in.readAll();
    }
  }
  return text;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getCopyright()
{
  return m_Copyright;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SMTKPluginPlugin::getLicense()
{
  /* PLEASE UPDATE YOUR PLUGIN'S LICENSE FILE.
  It is located at SMTKPlugin/Resources/SMTKPlugin/SMTKPluginLicense.txt */

  QFile licenseFile(":/SMTKPlugin/SMTKPluginLicense.txt");
  QFileInfo licenseFileInfo(licenseFile);
  QString text = "<<--License was not read-->>";

  if(licenseFileInfo.exists())
  {
    if(licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&licenseFile);
      text = in.readAll();
    }
  }
  return text;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMap<QString, QString> SMTKPluginPlugin::getThirdPartyLicenses()
{
  QMap<QString, QString> licenseMap;
  QList<QString> fileStrList;
  fileStrList.push_back(":/ThirdParty/HDF5.txt");
  fileStrList.push_back(":/ThirdParty/Boost.txt");
  fileStrList.push_back(":/ThirdParty/Qt.txt");
  fileStrList.push_back(":/ThirdParty/Qwt.txt");

  for(QList<QString>::iterator iter = fileStrList.begin(); iter != fileStrList.end(); iter++)
  {
    QFile file(*iter);
    QFileInfo licenseFileInfo(file);

    if(licenseFileInfo.exists())
    {
      if(file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        licenseMap.insert(licenseFileInfo.baseName(), in.readAll());
      }
    }
  }

  return licenseMap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SMTKPluginPlugin::getDidLoad()
{
  return m_DidLoad;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SMTKPluginPlugin::setDidLoad(bool didLoad)
{
  m_DidLoad = didLoad;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SMTKPluginPlugin::setLocation(QString filePath)
{
  m_Location = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SMTKPluginPlugin::writeSettings(QSettings& prefs)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SMTKPluginPlugin::readSettings(QSettings& prefs)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SMTKPluginPlugin::registerFilterWidgets(FilterWidgetManager* fwm)
{
}

#include "SMTKPluginFilters/RegisterKnownFilters.cpp"
