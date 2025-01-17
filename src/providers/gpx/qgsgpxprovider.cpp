/***************************************************************************
      qgsgpxprovider.cpp  -  Data provider for GPS eXchange files
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

    Partly based on qgsdelimitedtextprovider.cpp, (C) 2004 Gary E. Sherman
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <algorithm>
#include <limits>
#include <cstring>
#include <cmath>

// Changed #include <qapp.h> to <qapplication.h>. Apparently some
// debian distros do not include the qapp.h wrapper and the compilation
// fails. [gsherman]
#include <QApplication>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QObject>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsrectangle.h"

#include "qgsgpxfeatureiterator.h"
#include "qgsgpxprovider.h"
#include "gpsdata.h"

const char *QgsGPXProvider::ATTR[] = { "name", "elevation", "symbol", "number",
                                       "comment", "description", "source",
                                       "url", "url name"
                                     };
QVariant::Type QgsGPXProvider::attrType[] = { QVariant::String, QVariant::Double, QVariant::String, QVariant::Int,
                                              QVariant::String, QVariant::String, QVariant::String,
                                              QVariant::String, QVariant::String
                                            };
QgsGPXProvider::DataType QgsGPXProvider::attrUsed[] =
{
  QgsGPXProvider::AllType, QgsGPXProvider::WaypointType, QgsGPXProvider::TrkRteType, QgsGPXProvider::TrkRteType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType
};

const int QgsGPXProvider::ATTR_COUNT = sizeof( QgsGPXProvider::ATTR ) / sizeof( const char * );

const QString GPX_KEY = QStringLiteral( "gpx" );

const QString GPX_DESCRIPTION = QObject::tr( "GPS eXchange format provider" );


QgsGPXProvider::QgsGPXProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  // we always use UTF-8
  setEncoding( QStringLiteral( "utf8" ) );

  // get the file name and the type parameter from the URI
  int fileNameEnd = uri.indexOf( '?' );
  if ( fileNameEnd == -1 || uri.mid( fileNameEnd + 1, 5 ) != QLatin1String( "type=" ) )
  {
    QgsLogger::warning( tr( "Bad URI - you need to specify the feature type." ) );
    return;
  }
  QString typeStr = uri.mid( fileNameEnd + 6 );
  mFeatureType = ( typeStr == QLatin1String( "waypoint" ) ? WaypointType :
                   ( typeStr == QLatin1String( "route" ) ? RouteType : TrackType ) );

  // set up the attributes and the geometry type depending on the feature type
  for ( int i = 0; i < ATTR_COUNT; ++i )
  {
    if ( attrUsed[i] & mFeatureType )
    {
      QString attrTypeName = ( attrType[i] == QVariant::Int ? "int" : ( attrType[i] == QVariant::Double ? "double" : "text" ) );
      attributeFields.append( QgsField( ATTR[i], attrType[i], attrTypeName ) );
      indexToAttr.append( i );
    }
  }

  mFileName = uri.left( fileNameEnd );

  // parse the file
  data = QgsGpsData::getData( mFileName );
  if ( !data )
    return;

  mValid = true;
}


QgsGPXProvider::~QgsGPXProvider()
{
  QgsGpsData::releaseData( mFileName );
}

QgsAbstractFeatureSource *QgsGPXProvider::featureSource() const
{
  return new QgsGPXFeatureSource( this );
}


QString QgsGPXProvider::storageType() const
{
  return tr( "GPS eXchange file" );
}

QgsVectorDataProvider::Capabilities QgsGPXProvider::capabilities() const
{
  return QgsVectorDataProvider::AddFeatures |
         QgsVectorDataProvider::DeleteFeatures |
         QgsVectorDataProvider::ChangeAttributeValues;
}

QgsRectangle QgsGPXProvider::extent() const
{
  if ( data )
    return data->getExtent();
  return QgsRectangle();
}

QgsWkbTypes::Type QgsGPXProvider::wkbType() const
{
  if ( mFeatureType == WaypointType )
    return QgsWkbTypes::Point;

  if ( mFeatureType == RouteType || mFeatureType == TrackType )
    return QgsWkbTypes::LineString;

  return QgsWkbTypes::Unknown;
}

long long QgsGPXProvider::featureCount() const
{
  if ( !data )
    return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );

  if ( mFeatureType == WaypointType )
    return data->getNumberOfWaypoints();
  if ( mFeatureType == RouteType )
    return data->getNumberOfRoutes();
  if ( mFeatureType == TrackType )
    return data->getNumberOfTracks();
  return 0;
}

QgsFields QgsGPXProvider::fields() const
{
  return attributeFields;
}

bool QgsGPXProvider::isValid() const
{
  return mValid;
}


QgsFeatureIterator QgsGPXProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsGPXFeatureIterator( new QgsGPXFeatureSource( this ), true, request ) );
}


bool QgsGPXProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( !data )
    return false;

  // add all the features
  for ( QgsFeatureList::iterator iter = flist.begin();
        iter != flist.end(); ++iter )
  {
    if ( !addFeature( *iter, flags ) )
      return false;
  }

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  data->writeXml( ostr );
  return true;
}

bool QgsGPXProvider::addFeature( QgsFeature &f, Flags )
{
  if ( !data )
    return false;

  QByteArray wkb( f.geometry().asWkb() );
  const char *geo = wkb.constData();
  QgsWkbTypes::Type wkbType = f.geometry().wkbType();
  bool success = false;
  QgsGpsObject *obj = nullptr;
  QgsAttributes attrs = f.attributes();

  // is it a waypoint?
  if ( mFeatureType == WaypointType && geo && wkbType == QgsWkbTypes::Point )
  {

    // add geometry
    QgsWaypoint wpt;
    std::memcpy( &wpt.lon, geo + 5, sizeof( double ) );
    std::memcpy( &wpt.lat, geo + 13, sizeof( double ) );

    // add waypoint-specific attributes
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr.at( i ) == EleAttr )
      {
        bool eleIsOK;
        double ele = attrs.at( i ).toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt.ele = ele;
      }
      else if ( indexToAttr.at( i ) == SymAttr )
      {
        wpt.sym = attrs.at( i ).toString();
      }
    }

    QgsGpsData::WaypointIterator iter = data->addWaypoint( wpt );
    success = true;
    obj = &( *iter );
  }

  // is it a route?
  if ( mFeatureType == RouteType && geo && wkbType == QgsWkbTypes::LineString )
  {

    QgsRoute rte;

    // reset bounds
    rte.xMin = std::numeric_limits<double>::max();
    rte.xMax = -std::numeric_limits<double>::max();
    rte.yMin = std::numeric_limits<double>::max();
    rte.yMax = -std::numeric_limits<double>::max();

    // add geometry
    int nPoints;
    std::memcpy( &nPoints, geo + 5, 4 );
    for ( int i = 0; i < nPoints; ++i )
    {
      double lat, lon;
      std::memcpy( &lon, geo + 9 + 16 * i, sizeof( double ) );
      std::memcpy( &lat, geo + 9 + 16 * i + 8, sizeof( double ) );
      QgsRoutepoint rtept;
      rtept.lat = lat;
      rtept.lon = lon;
      rte.points.push_back( rtept );
      rte.xMin = rte.xMin < lon ? rte.xMin : lon;
      rte.xMax = rte.xMax > lon ? rte.xMax : lon;
      rte.yMin = rte.yMin < lat ? rte.yMin : lat;
      rte.yMax = rte.yMax > lat ? rte.yMax : lat;
    }

    // add route-specific attributes
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        long num = attrs.at( i ).toInt( &numIsOK );
        if ( numIsOK )
          rte.number = num;
      }
    }

    QgsGpsData::RouteIterator iter = data->addRoute( rte );
    success = true;
    obj = &( *iter );
  }

  // is it a track?
  if ( mFeatureType == TrackType && geo && wkbType == QgsWkbTypes::LineString )
  {

    QgsTrack trk;
    QgsTrackSegment trkseg;

    // reset bounds
    trk.xMin = std::numeric_limits<double>::max();
    trk.xMax = -std::numeric_limits<double>::max();
    trk.yMin = std::numeric_limits<double>::max();
    trk.yMax = -std::numeric_limits<double>::max();

    // add geometry
    int nPoints;
    std::memcpy( &nPoints, geo + 5, 4 );
    for ( int i = 0; i < nPoints; ++i )
    {
      double lat, lon;
      std::memcpy( &lon, geo + 9 + 16 * i, sizeof( double ) );
      std::memcpy( &lat, geo + 9 + 16 * i + 8, sizeof( double ) );
      QgsTrackpoint trkpt;
      trkpt.lat = lat;
      trkpt.lon = lon;
      trkseg.points.push_back( trkpt );
      trk.xMin = trk.xMin < lon ? trk.xMin : lon;
      trk.xMax = trk.xMax > lon ? trk.xMax : lon;
      trk.yMin = trk.yMin < lat ? trk.yMin : lat;
      trk.yMax = trk.yMax > lat ? trk.yMax : lat;
    }

    // add track-specific attributes
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        long num = attrs.at( i ).toInt( &numIsOK );
        if ( numIsOK )
          trk.number = num;
      }
    }

    trk.segments.push_back( trkseg );
    QgsGpsData::TrackIterator iter = data->addTrack( trk );
    success = true;
    obj = &( *iter );
  }


  // add common attributes
  if ( obj )
  {
    for ( int i = 0; i < attrs.count(); ++i )
    {
      switch ( indexToAttr.at( i ) )
      {
        case NameAttr:
          obj->name    = attrs.at( i ).toString();
          break;
        case CmtAttr:
          obj->cmt     = attrs.at( i ).toString();
          break;
        case DscAttr:
          obj->desc    = attrs.at( i ).toString();
          break;
        case SrcAttr:
          obj->src     = attrs.at( i ).toString();
          break;
        case URLAttr:
          obj->url     = attrs.at( i ).toString();
          break;
        case URLNameAttr:
          obj->urlname = attrs.at( i ).toString();
          break;
      }
    }
  }

  return success;
}


bool QgsGPXProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( !data )
    return false;

  if ( mFeatureType == WaypointType )
    data->removeWaypoints( id );
  else if ( mFeatureType == RouteType )
    data->removeRoutes( id );
  else if ( mFeatureType == TrackType )
    data->removeTracks( id );

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  data->writeXml( ostr );
  return true;
}


bool QgsGPXProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !data )
    return false;

  QgsChangedAttributesMap::const_iterator aIter = attr_map.begin();
  if ( mFeatureType == WaypointType )
  {
    QgsGpsData::WaypointIterator wIter = data->waypointsBegin();
    for ( ; wIter != data->waypointsEnd() && aIter != attr_map.end(); ++wIter )
    {
      if ( wIter->id == aIter.key() )
      {
        changeAttributeValues( *wIter, aIter.value() );
        ++aIter;
      }
    }
  }
  else if ( mFeatureType == RouteType )
  {
    QgsGpsData::RouteIterator rIter = data->routesBegin();
    for ( ; rIter != data->routesEnd() && aIter != attr_map.end(); ++rIter )
    {
      if ( rIter->id == aIter.key() )
      {
        changeAttributeValues( *rIter, aIter.value() );
        ++aIter;
      }
    }
  }
  if ( mFeatureType == TrackType )
  {
    QgsGpsData::TrackIterator tIter = data->tracksBegin();
    for ( ; tIter != data->tracksEnd() && aIter != attr_map.end(); ++tIter )
    {
      if ( tIter->id == aIter.key() )
      {
        changeAttributeValues( *tIter, aIter.value() );
        ++aIter;
      }
    }
  }

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  data->writeXml( ostr );
  return true;
}


void QgsGPXProvider::changeAttributeValues( QgsGpsObject &obj, const QgsAttributeMap &attrs )
{

  QgsWaypoint *wpt = dynamic_cast<QgsWaypoint *>( &obj );
  QgsGpsExtended *ext = dynamic_cast<QgsGpsExtended *>( &obj );

  QgsAttributeMap::const_iterator aIter = attrs.begin();
  for ( ; aIter != attrs.end(); ++aIter )
  {
    int i = aIter.key();
    QVariant v = aIter.value();

    // common attributes
    switch ( indexToAttr.at( i ) )
    {
      case NameAttr:
        obj.name    = v.toString();
        break;
      case CmtAttr:
        obj.cmt     = v.toString();
        break;
      case DscAttr:
        obj.desc    = v.toString();
        break;
      case SrcAttr:
        obj.src     = v.toString();
        break;
      case URLAttr:
        obj.url     = v.toString();
        break;
      case URLNameAttr:
        obj.urlname = v.toString();
        break;
    }

    // waypoint-specific attributes
    if ( wpt )
    {
      if ( indexToAttr.at( i ) == SymAttr )
        wpt->sym = v.toString();
      else if ( indexToAttr.at( i ) == EleAttr )
      {
        bool eleIsOK;
        double ele = v.toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt->ele = ele;
      }
    }

    // route- and track-specific attributes
    if ( ext )
    {
      if ( indexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        int num = v.toInt( &numIsOK );
        if ( numIsOK )
          ext->number = num;
      }
    }

  }

}


QVariant QgsGPXProvider::defaultValue( int fieldId ) const
{
  if ( fieldId == SrcAttr )
    return tr( "Digitized in QGIS" );
  return QVariant();
}


QString QgsGPXProvider::name() const
{
  return GPX_KEY;
} // QgsGPXProvider::name()



QString QgsGPXProvider::description() const
{
  return GPX_DESCRIPTION;
} // QgsGPXProvider::description()

QgsCoordinateReferenceSystem QgsGPXProvider::crs() const
{
  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
}

QgsDataProvider *QgsGpxProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsGPXProvider( uri, options, flags );
}


QgsGpxProviderMetadata::QgsGpxProviderMetadata():
  QgsProviderMetadata( GPX_KEY, GPX_DESCRIPTION )
{
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGpxProviderMetadata();
}
