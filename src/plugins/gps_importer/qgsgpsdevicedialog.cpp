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

#include "qgsgpsdevicedialog.h"
#include "qgsguiutils.h"
#include "qgssettings.h"

#include <QMessageBox>


QgsGpsDeviceDialog::QgsGpsDeviceDialog( std::map < QString,
                                        QgsBabelGpsDeviceFormat * > &devices )
  : QDialog( nullptr, QgsGuiUtils::ModalDialogFlags )
  , mDevices( devices )
{
  setupUi( this );
  connect( pbnNewDevice, &QPushButton::clicked, this, &QgsGpsDeviceDialog::pbnNewDevice_clicked );
  connect( pbnDeleteDevice, &QPushButton::clicked, this, &QgsGpsDeviceDialog::pbnDeleteDevice_clicked );
  connect( pbnUpdateDevice, &QPushButton::clicked, this, &QgsGpsDeviceDialog::pbnUpdateDevice_clicked );
  setAttribute( Qt::WA_DeleteOnClose );
  // Manually set the relative size of the two main parts of the
  // device dialog box.

  QObject::connect( lbDeviceList, &QListWidget::currentItemChanged,
                    this, &QgsGpsDeviceDialog::slotSelectionChanged );
  slotUpdateDeviceList();
}


void QgsGpsDeviceDialog::pbnNewDevice_clicked()
{
  std::map<QString, QgsBabelGpsDeviceFormat *>::const_iterator iter = mDevices.begin();
  QString deviceName = tr( "New device %1" );
  int i = 1;
  for ( ; iter != mDevices.end(); ++i )
    iter = mDevices.find( deviceName.arg( i ) );
  deviceName = deviceName.arg( i - 1 );
  mDevices[deviceName] = new QgsBabelGpsDeviceFormat;
  writeDeviceSettings();
  slotUpdateDeviceList( deviceName );
  emit devicesChanged();
}


void QgsGpsDeviceDialog::pbnDeleteDevice_clicked()
{
  if ( QMessageBox::warning( this, tr( "Delete Device" ),
                             tr( "Are you sure that you want to delete this device?" ),
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
  {

    std::map<QString, QgsBabelGpsDeviceFormat *>::iterator iter =
      mDevices.find( lbDeviceList->currentItem()->text() );
    if ( iter != mDevices.end() )
    {
      delete iter->second;
      mDevices.erase( iter );
      writeDeviceSettings();
      slotUpdateDeviceList();
      emit devicesChanged();
    }
  }
}


void QgsGpsDeviceDialog::pbnUpdateDevice_clicked()
{
  if ( lbDeviceList->count() > 0 )
  {
    std::map<QString, QgsBabelGpsDeviceFormat *>::iterator iter =
      mDevices.find( lbDeviceList->currentItem()->text() );
    if ( iter != mDevices.end() )
    {
      delete iter->second;
      mDevices.erase( iter );
      mDevices[leDeviceName->text()] =
        new QgsBabelGpsDeviceFormat( leWptDown->text(), leWptUp->text(),
                                     leRteDown->text(), leRteUp->text(),
                                     leTrkDown->text(), leTrkUp->text() );
      writeDeviceSettings();
      slotUpdateDeviceList( leDeviceName->text() );
      emit devicesChanged();
    }
  }
}

void QgsGpsDeviceDialog::slotUpdateDeviceList( const QString &selection )
{
  QString selected;
  if ( selection.isEmpty() )
  {
    QListWidgetItem *item = lbDeviceList->currentItem();
    selected = ( item ? item->text() : QString() );
  }
  else
  {
    selected = selection;
  }

  // We're going to be changing the selected item, so disable our
  // notification of that.
  QObject::disconnect( lbDeviceList, &QListWidget::currentItemChanged,
                       this, &QgsGpsDeviceDialog::slotSelectionChanged );

  lbDeviceList->clear();
  std::map<QString, QgsBabelGpsDeviceFormat *>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    QListWidgetItem *item = new QListWidgetItem( iter->first, lbDeviceList );
    if ( iter->first == selected )
    {
      lbDeviceList->setCurrentItem( item );
    }
  }

  if ( !lbDeviceList->currentItem() && lbDeviceList->count() > 0 )
    lbDeviceList->setCurrentRow( 0 );

  // Update the display and reconnect the selection changed signal
  slotSelectionChanged( lbDeviceList->currentItem() );
  QObject::connect( lbDeviceList, &QListWidget::currentItemChanged,
                    this, &QgsGpsDeviceDialog::slotSelectionChanged );
}


void QgsGpsDeviceDialog::slotSelectionChanged( QListWidgetItem *current )
{
  if ( lbDeviceList->count() > 0 )
  {
    QString devName = current->text();
    leDeviceName->setText( devName );
    QgsBabelGpsDeviceFormat *device = mDevices[devName];
    leWptDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
    leWptUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
    leRteDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
    leRteUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
    leTrkDown->setText( device->
                        importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
    leTrkUp->setText( device->
                      exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) ) );
  }
}


void QgsGpsDeviceDialog::writeDeviceSettings()
{
  QStringList deviceNames;
  QgsSettings settings;
  QString devPath = QStringLiteral( "/Plugin-GPS/devices/%1" );
  settings.remove( QStringLiteral( "/Plugin-GPS/devices" ) );

  std::map<QString, QgsBabelGpsDeviceFormat *>::const_iterator iter;
  for ( iter = mDevices.begin(); iter != mDevices.end(); ++iter )
  {
    deviceNames.append( iter->first );
    QString wptDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    QString wptUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Waypoint, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    QString rteDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    QString rteUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Route, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    QString trkDownload =
      iter->second->importCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    QString trkUpload =
      iter->second->exportCommand( QStringLiteral( "%babel" ), Qgis::GpsFeatureType::Track, QStringLiteral( "%in" ), QStringLiteral( "%out" ) ).join( QLatin1Char( ' ' ) );
    settings.setValue( devPath.arg( iter->first ) + "/wptdownload",
                       wptDownload );
    settings.setValue( devPath.arg( iter->first ) + "/wptupload", wptUpload );
    settings.setValue( devPath.arg( iter->first ) + "/rtedownload",
                       rteDownload );
    settings.setValue( devPath.arg( iter->first ) + "/rteupload", rteUpload );
    settings.setValue( devPath.arg( iter->first ) + "/trkdownload",
                       trkDownload );
    settings.setValue( devPath.arg( iter->first ) + "/trkupload", trkUpload );
  }
  settings.setValue( QStringLiteral( "/Plugin-GPS/devicelist" ), deviceNames );
}

void QgsGpsDeviceDialog::on_pbnClose_clicked()
{
  close();
}
