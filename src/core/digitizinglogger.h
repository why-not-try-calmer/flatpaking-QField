/***************************************************************************
 digitizinglogger.h - DigitizingLogger
  ---------------------
 begin                : 7.6.2021
 copyright            : (C) 2021 by Mathieu Pellerin
 email                : mathieu (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIGITIZINGLOGGER_H
#define DIGITIZINGLOGGER_H

#include "gnsspositioninformation.h"
#include "qfieldcloudconnection.h"
#include "qgsquickmapsettings.h"
#include "snappingresult.h"

#include <QObject>
#include <qgspoint.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>

class DigitizingLogger : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString type READ type WRITE setType NOTIFY typeChanged )
    Q_PROPERTY( QgsProject *project READ project WRITE setProject NOTIFY projectChanged )
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings WRITE setMapSettings NOTIFY mapSettingsChanged )
    Q_PROPERTY( QgsVectorLayer *digitizingLayer READ digitizingLayer WRITE setDigitizingLayer NOTIFY digitizingLayerChanged )
    Q_PROPERTY( GnssPositionInformation positionInformation READ positionInformation WRITE setPositionInformation NOTIFY positionInformationChanged )
    Q_PROPERTY( bool positionLocked READ positionLocked WRITE setPositionLocked NOTIFY positionLockedChanged )
    Q_PROPERTY( SnappingResult topSnappingResult READ topSnappingResult WRITE setTopSnappingResult NOTIFY topSnappingResultChanged )
    Q_PROPERTY( CloudUserInformation cloudUserInformation READ cloudUserInformation WRITE setCloudUserInformation NOTIFY cloudUserInformationChanged );

  public:
    explicit DigitizingLogger();

    //! Returns the digitizing logs type
    QString type() const { return mType; }

    /*
     * Sets the digitizing logs \a type
     * \note if the type is an empty string, digitized vertices will not be logged
     */
    void setType( const QString &type );

    //! Returns the current project from which the digitizing logs will be sought
    QgsProject *project() const { return mProject; }

    //! Sets the \a project used to find the digitizing logs layer
    void setProject( QgsProject *project );

    //! Returns map settings
    QgsQuickMapSettings *mapSettings() const { return mMapSettings; }

    /**
     * Sets map settings
     * \param mapSettings the QgsQuickMapSettings object
     */
    void setMapSettings( QgsQuickMapSettings *mapSettings );

    //! Returns the current vector layer used to digitize features
    QgsVectorLayer *digitizingLayer() const { return mDigitizingLayer; }

    //! Sets the current vector \a layer used to digitze features
    void setDigitizingLayer( QgsVectorLayer *layer );

    /**
     * Returns position information generated by the TransformedPositionSource according to its provider
     */
    GnssPositionInformation positionInformation() const { return mPositionInformation; }

    /**
     * Sets position information generated by the TransformedPositionSource according to its provider
     * \param positionInformation the position information
     */
    void setPositionInformation( const GnssPositionInformation &positionInformation );

    /**
     * Returns whether the position is locked to the GNSS
     */
    bool positionLocked() const { return mPositionLocked; }

    /**
     * Sets whether the position is locked to the GNSS
     * \param positionLocked set to TRUE if the position is locked
     */
    void setPositionLocked( bool positionLocked );

    /**
     * Returns the top snapping result of the coordinate locator
     */
    SnappingResult topSnappingResult() const { return mTopSnappingResult; }

    /**
     * Sets the top snapping result of the coordinate locator
     * \param topSnappingResult the top snapping result object
     */
    void setTopSnappingResult( const SnappingResult &topSnappingResult );

    /**
     * Returns the current cloud user information
     */
    CloudUserInformation cloudUserInformation() const { return mCloudUserInformation; }

    /**
     * Sets the current cloud user information
     * \param cloudUserInformation the cloud user information
     */
    void setCloudUserInformation( const CloudUserInformation &cloudUserInformation );

    /**
     * Adds a \a point into the digitizing logs' buffer.
     */
    Q_INVOKABLE void addCoordinate( const QgsPoint &point );

    /**
     * Removes the last point entered into the digitizing logs' buffer.
     */
    Q_INVOKABLE void removeLastCoordinate();

    /**
     * Writes the points buffer to the digitizing logs layer.
     */
    Q_INVOKABLE void writeCoordinates();

    /**
     * Clear the points buffer from the digitizing logs.
     */
    Q_INVOKABLE void clearCoordinates();

  signals:

    void typeChanged();
    void projectChanged();
    void mapSettingsChanged();
    void digitizingLayerChanged();
    void positionInformationChanged();
    void positionLockedChanged();
    void topSnappingResultChanged();
    void currentCoordinateChanged();
    void cloudUserInformationChanged();

  private:
    //! Finds and link to the logs layer in present in the project
    void findLogsLayer();

    QString mType;

    QgsProject *mProject = nullptr;
    QgsQuickMapSettings *mMapSettings = nullptr;
    QgsVectorLayer *mLogsLayer = nullptr;
    QgsVectorLayer *mDigitizingLayer = nullptr;

    GnssPositionInformation mPositionInformation;
    bool mPositionLocked = false;
    SnappingResult mTopSnappingResult;
    CloudUserInformation mCloudUserInformation;

    QList<QgsFeature> mPointFeatures;
};

#endif // DIGITIZINGLOGGER_H
