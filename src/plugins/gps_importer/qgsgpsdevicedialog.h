/***************************************************************************
 *   Copyright (C) 2004 by Lars Luthman
 *   larsl@users.sourceforge.net
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSGPSDEVICEDIALOG_H
#define QGSGPSDEVICEDIALOG_H

#include "ui_qgsgpsdevicedialogbase.h"
#include "qgsbabelgpsdevice.h"

#include <QString>


class QgsGpsDeviceDialog : public QDialog, private Ui::QgsGpsDeviceDialogBase
{
    Q_OBJECT
  public:
    explicit QgsGpsDeviceDialog( std::map<QString, QgsBabelGpsDeviceFormat *> &devices );

  public slots:
    void pbnNewDevice_clicked();
    void pbnDeleteDevice_clicked();
    void pbnUpdateDevice_clicked();
    void on_pbnClose_clicked();
    void slotUpdateDeviceList( const QString &selection = "" );
    void slotSelectionChanged( QListWidgetItem *current );

  signals:
    void devicesChanged();

  private:
    void writeDeviceSettings();

    std::map<QString, QgsBabelGpsDeviceFormat *> &mDevices;
};

#endif
