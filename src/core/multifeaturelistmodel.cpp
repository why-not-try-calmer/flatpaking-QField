/***************************************************************************
                            featurelistmodel.cpp
                              -------------------
              begin                : 10.12.2014
              copyright            : (C) 2014 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsproject.h>
#include <qgsgeometry.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsexpressioncontextutils.h>
#include <qgsrelationmanager.h>
#include <qgsmessagelog.h>

#include "multifeaturelistmodel.h"

#include <QDebug>

MultiFeatureListModel::MultiFeatureListModel( QObject *parent )
  :  QAbstractItemModel( parent )
{
  connect( this, &MultiFeatureListModel::modelReset, this, &MultiFeatureListModel::countChanged );
}

void MultiFeatureListModel::setFeatures( const QMap<QgsVectorLayer *, QgsFeatureRequest> requests )
{
  beginResetModel();

  mFeatures.clear();

  QMap<QgsVectorLayer *, QgsFeatureRequest>::ConstIterator it;
  for ( it = requests.constBegin(); it != requests.constEnd(); it++ )
  {
    QgsFeature feat;
    QgsFeatureIterator fit = it.key()->getFeatures( it.value() );
    while ( fit.nextFeature( feat ) )
    {
      mFeatures.append( QPair< QgsVectorLayer *, QgsFeature >( it.key(), feat ) );
      connect( it.key(), &QgsVectorLayer::destroyed, this, &MultiFeatureListModel::layerDeleted, Qt::UniqueConnection );
      connect( it.key(), &QgsVectorLayer::featureDeleted, this, &MultiFeatureListModel::featureDeleted, Qt::UniqueConnection );
      connect( it.key(), &QgsVectorLayer::attributeValueChanged, this, &MultiFeatureListModel::attributeValueChanged, Qt::UniqueConnection );
    }
  }

  endResetModel();
}

void MultiFeatureListModel::appendFeatures( const QList<IdentifyTool::IdentifyResult> &results )
{
  beginInsertRows( QModelIndex(), mFeatures.count(), mFeatures.count() + results.count() - 1 );
  for ( const IdentifyTool::IdentifyResult &result : results )
  {
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( result.layer );
    mFeatures.append( QPair<QgsVectorLayer *, QgsFeature>( layer, result.feature ) );
    connect( layer, &QObject::destroyed, this, &MultiFeatureListModel::layerDeleted, Qt::UniqueConnection );
    connect( layer, &QgsVectorLayer::featureDeleted, this, &MultiFeatureListModel::featureDeleted, Qt::UniqueConnection );
    connect( layer, &QgsVectorLayer::attributeValueChanged, this, &MultiFeatureListModel::attributeValueChanged, Qt::UniqueConnection );
  }
  endInsertRows();
}

void MultiFeatureListModel::clear()
{
  beginResetModel();
  mFeatures.clear();
  endResetModel();
}

QHash<int, QByteArray> MultiFeatureListModel::roleNames() const
{
  QHash<int, QByteArray> roleNames;

  roleNames[Qt::DisplayRole] = "display";
  roleNames[FeatureIdRole] = "featureId";
  roleNames[FeatureRole] = "feature";
  roleNames[LayerNameRole] = "layerName";
  roleNames[LayerRole] = "currentLayer";
  roleNames[GeometryRole] = "geometry";
  roleNames[CrsRole] = "crs";
  roleNames[DeleteFeatureRole] = "deleteFeatureCapability";
  roleNames[EditGeometryRole] = "editGeometryCapability";

  return roleNames;
}

QModelIndex MultiFeatureListModel::index( int row, int column, const QModelIndex &parent ) const
{
  Q_UNUSED( parent )

  if ( row < 0 || row >= mFeatures.size() || column != 0 )
    return QModelIndex();

  return createIndex( row, column, const_cast<QPair< QgsVectorLayer *, QgsFeature >*>( &mFeatures.at( row ) ) );
}

QModelIndex MultiFeatureListModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}

int MultiFeatureListModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeatures.count();
}

int MultiFeatureListModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant MultiFeatureListModel::data( const QModelIndex &index, int role ) const
{
  QPair< QgsVectorLayer *, QgsFeature > *feature = toFeature( index );
  if ( !feature )
    return QVariant();

  switch ( role )
  {
    case FeatureIdRole:
      return feature->second.id();

    case FeatureRole:
      return feature->second;

    case Qt::DisplayRole:
    {
      QgsExpressionContext context = QgsExpressionContext()
                                     << QgsExpressionContextUtils::globalScope()
                                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
                                     << QgsExpressionContextUtils::layerScope( feature->first );
      context.setFeature( feature->second );
      return QgsExpression( feature->first->displayExpression() ).evaluate( &context ).toString();
    }

    case LayerNameRole:
      return feature->first->name();

    case LayerRole:
      return QVariant::fromValue<QgsVectorLayer *>( feature->first );

    case GeometryRole:
      return QVariant::fromValue<QgsGeometry>( feature->second.geometry() );

    case CrsRole:
      return QVariant::fromValue<QgsCoordinateReferenceSystem>( feature->first->crs() );

    case DeleteFeatureRole:
      return !feature->first->readOnly() && ( feature->first->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures );

    case EditGeometryRole:
      return !feature->first->readOnly() && ( feature->first->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeGeometries );
  }

  return QVariant();
}

bool MultiFeatureListModel::removeRows( int row, int count, const QModelIndex &parent = QModelIndex() )
{
  if ( !count )
    return true;

  int i = 0;
  QMutableListIterator<QPair< QgsVectorLayer *, QgsFeature >> it( mFeatures );
  while ( i < row )
  {
    it.next();
    i++;
  }

  int last = row + count - 1;

  beginRemoveRows( parent, row, last );
  while ( i <= last )
  {
    it.next();
    it.remove();
    i++;
  }
  endRemoveRows();

  return true;
}

int MultiFeatureListModel::count() const
{
  return mFeatures.size();
}

bool MultiFeatureListModel::deleteFeature( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !layer )
  {
      QgsMessageLog::logMessage( tr( "Cannot start editing, no layer" ), "QField", Qgis::Warning );
      return false;
  }

  if ( ! layer->startEditing() )
  {
    QgsMessageLog::logMessage( tr( "Cannot start editing" ), "QField", Qgis::Warning );
    return false;
  }

  //delete child features in case of compositions
  const QList<QgsRelation> referencingRelations = QgsProject::instance()->relationManager()->referencedRelations( layer );
  QList<QgsVectorLayer *> childLayersEdited;
  bool isSuccess = true;
  for ( const QgsRelation &referencingRelation : referencingRelations )
  {
    if ( referencingRelation.strength() == QgsRelation::Composition )
    {
      QgsVectorLayer *childLayer = referencingRelation.referencingLayer();

      if ( childLayer->startEditing() )
      {
        QgsFeatureIterator relatedFeaturesIt = referencingRelation.getRelatedFeatures( layer->getFeature( fid ) );
        QgsFeature childFeature;
        while ( relatedFeaturesIt.nextFeature( childFeature ) )
        {
          if ( ! childLayer->deleteFeature( childFeature.id() ) )
          {
            QgsMessageLog::logMessage( tr( "Cannot delete feature from child layer" ), "QField", Qgis::Critical );
            isSuccess = false;
          }
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Cannot start editing child layer" ), "QField", Qgis::Critical );
        isSuccess = false;
        break;
      }

      if ( isSuccess ) 
        childLayersEdited.append( childLayer );
      else
        break;
    }
  }

  // we need to either commit or rollback the child layers that have experienced any modification
  for ( QgsVectorLayer *childLayer : qgis::as_const( childLayersEdited ) )
  {
    // if everything went well so far, we try to commit
    if ( isSuccess )
    {
      if ( ! childLayer->commitChanges() )
      {
        QgsMessageLog::logMessage( tr( "Cannot commit layer changes in layer %1." ).arg( childLayer->name() ), "QField", Qgis::Critical );
        isSuccess = false;
      }
    }

    // if the commit failed, we try to rollback the changes and the rest of the modified layers (parent and children) will be rollbacked
    if ( ! isSuccess )
    {
      if ( ! childLayer->rollBack() )
        QgsMessageLog::logMessage( tr( "Cannot rollback layer changes in layer %1" ).arg( childLayer->name() ), "QField", Qgis::Critical );
    }
  }

  if ( isSuccess )
  {
    //delete parent
    if ( layer->deleteFeature( fid ) )
    {
      // commit parent changes
      if ( ! layer->commitChanges() )
        isSuccess = false;
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Cannot delete feature %1" ).arg( fid ), "QField", Qgis::Warning );
  
      isSuccess = false;
    }
  }

  if ( ! isSuccess )
  {
    if ( ! layer->rollBack() )
      QgsMessageLog::logMessage( tr( "Cannot rollback layer changes in layer %1" ).arg( layer->name() ), "QField", Qgis::Critical );
  }

  return isSuccess;
}

void MultiFeatureListModel::layerDeleted( QObject *object )
{
  int firstRowToRemove = -1;
  int count = 0;
  int currentRow = 0;

  /*
   * Features on the same layer are always subsequent.
   * We therefore can search for the first feature and
   * count all subsequent ones.
   * Once there is a feature of a different layer found
   * we can stop searching.
   */
  for ( auto it = mFeatures.constBegin(); it != mFeatures.constEnd(); it++ )
  {
    if ( it->first == object )
    {
      if ( firstRowToRemove == -1 )
        firstRowToRemove = currentRow;

      count++;
    }
    else if ( firstRowToRemove != -1 )
    {
      break;
    }
    currentRow++;
  }

  removeRows( firstRowToRemove, count );
}

void MultiFeatureListModel::featureDeleted( QgsFeatureId fid )
{
  QgsVectorLayer *l = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( l );

  int i = 0;
  for ( auto it = mFeatures.constBegin(); it != mFeatures.constEnd(); it++ )
  {
    if ( it->first == l && it->second.id() == fid )
    {
      removeRows( i, 1 );
      break;
    }
    ++i;
  }
}

void MultiFeatureListModel::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value )
{
  QgsVectorLayer *l = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( l );

  int i = 0;
  for ( auto it = mFeatures.begin(); it != mFeatures.end(); it++ )
  {
    if ( it->first == l && it->second.id() == fid )
    {
      it->second.setAttribute( idx, value );
      break;
    }
    ++i;
  }
}
