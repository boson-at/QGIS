/***************************************************************************
                          qgsgpsplugingui.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSPLUGINGUI_H
#define QGSGPSPLUGINGUI_H

#include "ui_qgsgpspluginguibase.h"
#include "qgsbabelformat.h"
#include "qgsbabelgpsdevice.h"
#include "qgshelp.h"

#include <vector>

#include <QString>

class QgsVectorLayer;


/**
*/
class QgsGpsPluginGui : public QDialog, private Ui::QgsGpsPluginGuiBase
{
    Q_OBJECT

  public:
    QgsGpsPluginGui( const std::map<QString, QgsAbstractBabelFormat *> &importers,
                     std::map<QString, QgsBabelGpsDeviceFormat *> &devices,
                     const std::vector<QgsVectorLayer *> &gpxMapLayers,
                     QWidget *parent, Qt::WindowFlags );
    ~QgsGpsPluginGui() override;

  public slots:

    void openDeviceEditor();
    void devicesUpdated();
    void enableRelevantControls();

    void on_pbnGPXSelectFile_clicked();

    void pbnIMPInput_clicked();
    void pbnIMPOutput_clicked();

    void pbnDLOutput_clicked();

  private:
    void populateDeviceComboBox();
    void populateULLayerComboBox();
    void populateIMPBabelFormats();
    void populatePortComboBoxes();

    void saveState();
    void restoreState();
    void showHelp();

#if 0
    void populateLoadDialog();
    void populateDLDialog();
    void populateULDialog();
    void populateIMPDialog();
#endif

  private slots:

    void pbnRefresh_clicked();
    void buttonBox_accepted();
    void buttonBox_rejected();

  signals:
    void drawRasterLayer( const QString & );
    void drawVectorLayer( const QString &, const QString &, const QString & );
    void loadGPXFile( const QString &fileName, bool showWaypoints, bool showRoutes,
                      bool showTracks );
    void importGPSFile( const QString &inputFileName, QgsAbstractBabelFormat *importer,
                        Qgis::GpsFeatureType type, const QString &outputFileName,
                        const QString &layerName );
    void downloadFromGPS( const QString &device, const QString &port, Qgis::GpsFeatureType type,
                          const QString &outputFileName, const QString &layerName );
    void uploadToGPS( QgsVectorLayer *gpxLayer, const QString &device, const QString &port );

  private:

    std::vector<QgsVectorLayer *> mGPXLayers;
    const std::map<QString, QgsAbstractBabelFormat *> &mImporters;
    std::map<QString, QgsBabelGpsDeviceFormat *> &mDevices;
    QString mBabelFilter;
    QString mImpFormat;
    QAbstractButton *pbnOK = nullptr;
};

#endif
