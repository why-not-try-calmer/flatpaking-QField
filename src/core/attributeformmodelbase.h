/***************************************************************************
  attributeformmodelbase.h - AttributeFormModelBase

 ---------------------
 begin                : 16.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ATTRIBUTEFORMMODELBASE_H
#define ATTRIBUTEFORMMODELBASE_H

#include <QStandardItemModel>
#include<QStack>

#include <qgseditformconfig.h>
#include <qgsexpressioncontext.h>

#include "featuremodel.h"

class AttributeFormModelBase : public QStandardItemModel
{
    Q_OBJECT

    Q_PROPERTY( FeatureModel *featureModel READ featureModel WRITE setFeatureModel NOTIFY featureModelChanged )
    Q_PROPERTY( bool hasTabs READ hasTabs WRITE setHasTabs NOTIFY hasTabsChanged )
    Q_PROPERTY( bool constraintsHardValid READ constraintsHardValid NOTIFY constraintsHardValidChanged )
    Q_PROPERTY( bool constraintsSoftValid READ constraintsSoftValid NOTIFY constraintsSoftValidChanged )

  public:
    explicit AttributeFormModelBase( QObject *parent = nullptr );
    ~AttributeFormModelBase();

    QHash<int, QByteArray> roleNames() const override;

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    FeatureModel *featureModel() const;
    void setFeatureModel( FeatureModel *featureModel );

    bool hasTabs() const;
    void setHasTabs( bool hasTabs );

    void save();

    bool create();

    bool deleteFeature();

    bool constraintsHardValid() const;

    bool constraintsSoftValid() const;

    QVariant attribute( const QString &name );

  signals:
    void featureModelChanged();
    void hasTabsChanged();
    void featureChanged();
    void constraintsHardValidChanged();
    void constraintsSoftValidChanged();

  private slots:
    void onLayerChanged();
    void onFeatureChanged();

  private:
    /**
     * Generates a root container for autogenerated layouts, so we can just use the same
     * form logic to deal with them.
     */
    QgsAttributeEditorContainer *generateRootContainer() const;

    QgsAttributeEditorContainer *invisibleRootContainer() const;

    void updateAttributeValue( QStandardItem *item );

    void flatten( QgsAttributeEditorContainer *container, QStandardItem *parent, const QString &parentVisibilityExpressions, QVector<QStandardItem *> &items );

    void updateVisibility( int fieldIndex = -1 );

    void setConstraintsHardValid( bool constraintsHardValid );

    void setConstraintsSoftValid( bool constraintsSoftValid );

    /**
     * finds the best widget type regarding to the field type or the configured widget setup
     * \param fieldIndex to get the field
     * \returns widget setup containing the best widget type
     */
    QgsEditorWidgetSetup findBest( int fieldIndex );

    FeatureModel *mFeatureModel = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsAttributeEditorContainer *mTemporaryContainer = nullptr;
    bool mHasTabs;

    typedef QPair<QgsExpression, QVector<QStandardItem *> > VisibilityExpression;
    QList<VisibilityExpression> mVisibilityExpressions;
    QMap<QStandardItem *, QgsFieldConstraints> mConstraints;

    QgsExpressionContext mExpressionContext;
    bool mConstraintsHardValid = true;
    bool mConstraintsSoftValid = true;
};

#endif // ATTRIBUTEFORMMODELBASE_H
