/*  StarProfileViewer
    Copyright (C) 2017 Robert Lancaster <rlancaste@gmail.com>

    Based on the QT Surface Example http://doc.qt.io/qt-5.9/qtdatavisualization-surface-example.html
    and the QT Bars Example https://doc-snapshots.qt.io/qt5-5.9/qtdatavisualization-bars-example.html

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#pragma once

#include "fitsdata.h"

#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtDataVisualization/q3dbars.h>
#include <QtDataVisualization/QCustom3DLabel>
#include <QtWidgets/QSlider>
#include <QDialog>

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>
#include <QtGui/QScreen>

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include <QMessageBox>

using namespace QtDataVisualization;

class StarProfileViewer : public QDialog
{
    Q_OBJECT
public:
    explicit StarProfileViewer(QWidget *parent = nullptr);
    ~StarProfileViewer();

    void setBlackToYellowGradient();
    void setGreenToRedGradient();

    void loadData(FITSData *imageData, QRect sub, QList<Edge *> starCenters);
    float getImageDataValue(int x, int y);
    void toggleSlice();
    void updateVerticalAxis();
    void updateHFRandPeakSelection();
    void updateDisplayData();
    void updateScale();
    void enableTrackingBox(bool enable);
    void changeSelection();
    void updateSelectorBars(QPoint position);
    void toggleCutoffEnabled(bool enable);

public slots:
    void changeSelectionType(int type);
    void zoomViewTo(int where);
    void updateSampleSize(const QString &text);
    void updateColor(int selection);
    void updateBarSpacing(int value);

signals:
    void sampleSizeUpdated(int size);
private:
    Q3DBars *m_graph { nullptr };
    QValue3DAxis *m_pixelValueAxis { nullptr };
    QCategory3DAxis *m_xPixelAxis { nullptr };
    QCategory3DAxis *m_yPixelAxis { nullptr };
    QBar3DSeries *m_3DPixelSeries { nullptr };

    QBarDataArray *dataSet { nullptr };

    template <typename T>
    float getImageDataValue(int x, int y);
    void getSubFrameMinMax(float *subFrameMin, float *subFrameMax, double *dataMin, double *dataMax);

    QPushButton *HFRReport { nullptr };
    QLabel *reportBox { nullptr };
    QPushButton *showPeakValues { nullptr };
    QPushButton *showCoordinates { nullptr };
    QCheckBox *autoScale { nullptr };
    QPushButton *showScaling { nullptr };
    QComboBox *sampleSize { nullptr };
    QComboBox *selectionType { nullptr };
    QComboBox *zoomView { nullptr };
    QComboBox *selectStar { nullptr };
    QPushButton *exploreMode { nullptr };
    QLabel *pixelReport { nullptr };
    QLabel *maxValue { nullptr };
    QLabel *minValue { nullptr };
    QLabel *cutoffValue { nullptr };
    QPushButton *sliceB { nullptr };
    FITSData * imageData { nullptr };
    QRect subFrame;

    QSlider *blackPointSlider { nullptr };
    QSlider *whitePointSlider { nullptr };
    QSlider *cutoffSlider { nullptr };
    QSlider *verticalSelector { nullptr };
    QSlider *horizontalSelector { nullptr };
    QList<Edge *> starCenters;

    bool cutOffEnabled { false };

    int convertToSliderValue(float value);
    float convertFromSliderValue(int value);
    void updatePixelReport();

};
