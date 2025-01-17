/***************************************************************************
     qgsbabelgpsdevice.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:04:15 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QRegularExpression>

#include "qgsbabelgpsdevice.h"


QgsBabelGpsDeviceFormat::QgsBabelGpsDeviceFormat( const QString &waypointDownloadCommand, const QString &waypointUploadCommand,
    const QString &routeDownloadCommand, const QString &routeUploadCommand,
    const QString &trackDownloadCommand, const QString &trackUploadCommand )
{
  const thread_local QRegularExpression whiteSpaceRx( QStringLiteral( "\\s" ) );

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  if ( !waypointDownloadCommand.isEmpty() )
    mWaypointDownloadCommand = waypointDownloadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
  if ( !waypointUploadCommand.isEmpty() )
    mWaypointUploadCommand = waypointUploadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
  if ( !routeDownloadCommand.isEmpty() )
    mRouteDownloadCommand = routeDownloadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
  if ( !routeUploadCommand.isEmpty() )
    mRouteUploadCommand = routeUploadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
  if ( !trackDownloadCommand.isEmpty() )
    mTrackDownloadCommand = trackDownloadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
  if ( !trackUploadCommand.isEmpty() )
    mTrackUploadCommand = trackUploadCommand.split( whiteSpaceRx, QString::SkipEmptyParts );
#else

  if ( !waypointDownloadCommand.isEmpty() )
    mWaypointDownloadCommand = waypointDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
  if ( !waypointUploadCommand.isEmpty() )
    mWaypointUploadCommand = waypointUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
  if ( !routeDownloadCommand.isEmpty() )
    mRouteDownloadCommand = routeDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
  if ( !routeUploadCommand.isEmpty() )
    mRouteUploadCommand = routeUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
  if ( !trackDownloadCommand.isEmpty() )
    mTrackDownloadCommand = trackDownloadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
  if ( !trackUploadCommand.isEmpty() )
    mTrackUploadCommand = trackUploadCommand.split( whiteSpaceRx, Qt::SkipEmptyParts );
#endif
}

QStringList QgsBabelGpsDeviceFormat::importCommand( const QString &babel,
    Qgis::GpsFeatureType type,
    const QString &in,
    const QString &out ) const
{
  QStringList original;

  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      original = mWaypointDownloadCommand;
      break;
    case Qgis::GpsFeatureType::Route:
      original = mRouteDownloadCommand;
      break;
    case Qgis::GpsFeatureType::Track:
      original = mTrackDownloadCommand;
      break;
  }

  QStringList copy;
  copy.reserve( original.size() );
  for ( const QString &iter : std::as_const( original ) )
  {
    if ( iter == QLatin1String( "%babel" ) )
      copy.append( babel );
    else if ( iter == QLatin1String( "%type" ) )
      copy.append( featureTypeToArgument( type ) );
    else if ( iter == QLatin1String( "%in" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( in ) );
    else if ( iter == QLatin1String( "%out" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( out ) );
    else
      copy.append( iter );
  }
  return copy;
}

QStringList QgsBabelGpsDeviceFormat::exportCommand( const QString &babel,
    Qgis::GpsFeatureType type,
    const QString &in,
    const QString &out ) const
{
  QStringList original;
  switch ( type )
  {
    case Qgis::GpsFeatureType::Waypoint:
      original = mWaypointUploadCommand;
      break;
    case Qgis::GpsFeatureType::Route:
      original = mRouteUploadCommand;
      break;
    case Qgis::GpsFeatureType::Track:
      original = mTrackUploadCommand;
      break;
  }

  QStringList copy;
  copy.reserve( original.size() );
  for ( const QString &iter : std::as_const( original ) )
  {
    if ( iter == QLatin1String( "%babel" ) )
      copy.append( babel );
    else if ( iter == QLatin1String( "%type" ) )
      copy.append( featureTypeToArgument( type ) );
    else if ( iter == QLatin1String( "%in" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( in ) );
    else if ( iter == QLatin1String( "%out" ) )
      copy.append( QStringLiteral( "\"%1\"" ).arg( out ) );
    else
      copy.append( iter );
  }
  return copy;
}



