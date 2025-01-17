/***************************************************************************
                          qgsgpsplugin.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPSPLUGIN_H
#define QGSGPSPLUGIN_H
#include "qgsbabelformat.h"
#include "qgsbabelgpsdevice.h"
#include "qgisplugin.h"

#include <QObject>

class QgisInterface;
class QgsVectorLayer;
class QAction;

/**
 * A plugin with various GPS tools.
*/
class QgsGpsPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:

    /**
     * Constructor for a plugin. The QgisInterface pointer
     *  is passed by QGIS when it attempts to instantiate the plugin.
     *  \param interface pointer to the QgisInterface object.
     */
    explicit QgsGpsPlugin( QgisInterface * );


    ~QgsGpsPlugin() override;

  public slots:
    //! init the gui
    void initGui() override;
    //! Show the dialog box
    void run();
    //! Add a vector layer given vectorLayerPath, baseName, providerKey
    void drawVectorLayer( const QString &, const QString &, const QString & );
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( const QString &themeName );

    //! load a GPX file
    void loadGPXFile( const QString &fileName, bool loadWaypoints, bool loadRoutes,
                      bool loadTracks );
    void importGPSFile( const QString &inputFileName, QgsAbstractBabelFormat *importer,
                        Qgis::GpsFeatureType type, const QString &outputFileName,
                        const QString &layerName );
    void downloadFromGPS( const QString &device, const QString &port,
                          Qgis::GpsFeatureType type, const QString &outputFileName,
                          const QString &layerName );
    void uploadToGPS( QgsVectorLayer *gpxLayer, const QString &device,
                      const QString &port );

  signals:

    void closeGui();

  private:

    //! Initializes all variables needed to run GPSBabel.
    void setupBabel();

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisInterface = nullptr;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *mQActionPointer = nullptr;
    //! The path to the GPSBabel program
    QString mBabelPath;
    //! Importers for external GPS data file formats
    std::map<QString, QgsAbstractBabelFormat *> mImporters;
    //! Upload/downloaders for GPS devices
    std::map<QString, QgsBabelGpsDeviceFormat *> mDevices;
};

#endif
