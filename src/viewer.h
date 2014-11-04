/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef Viewer_H
#define Viewer_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>

#include <QtOpenGL>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include "glgenerator.h"
#include "dso.h"


class DataAnalyzer;
class DsoSettings;
class HardControl;
class DsoControl;

//! [1]
class ViewerRenderer : public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    ViewerRenderer(DsoSettings *settings, bool emulate);

    ~ViewerRenderer();
    GlGenerator *generator;

    bool emulate;
    void setT(qreal t) { m_t = t; }
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setGenerator(GlGenerator *generator);
    //GlArray vaGrid[3];
    GlArray vaEmulated[2];

public slots:
    void paint();

private:
    QSize m_viewportSize;
    qreal m_t;
    QOpenGLShaderProgram *m_program;

    DsoSettings *settings;

    GlArray vaMarker[2];
};
//! [1]

//! [2]
class Viewer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

public:
    Viewer();
    bool emulate = false;
    qreal t() const { return m_t; }

    GlGenerator *generator; ///< The generator for the OpenGL vertex arrays

    ViewerRenderer *mainScope; ///< The main scope screen
    DsoSettings *settings; ///< The settings provided by the main window

    DataAnalyzer *dataAnalyzer; ///< The data source provided by the main window
    HardControl *hardControl;
    DsoControl *dsoControl;
    void connectSignals();
    void initializeDevice();

signals:
    void tChanged();

public slots:
    void sync();
    void cleanup();
    void update();
    // File operations
   // int open();
   // int save();
   // int saveAs();
    // View
   // void digitalPhosphor(bool enabled);
   // void zoom(bool enabled);
    // Oscilloscope control
   // void started();
   // void stopped();
    // Hard Events
    //void hard_event(int type, int value);
    // Other
   // void config();
   // void about();

    // Settings management
   // void applySettings();
   // void updateSettings();
    void handleWindowChanged(QQuickWindow *win);
   // void recordTimeChanged(double duration);
    //void samplerateChanged(double samplerate);
   // void recordLengthSelected(unsigned long recordLength);
    void samplerateSelected(float samplerate = 0 );
    void timebaseSelected(float timebase = 0);
   // void recordLengthSelected(unsigned long recordLength);
    void voltageGainSelected(unsigned int channel, int index);
   // void updateOffset(unsigned int channel);
    void updateUsed(unsigned int channel);
   // void updateVoltageGain(unsigned int channel);
    void emulateSelected(bool emulate);

private:
    qreal m_t;
    ViewerRenderer *m_renderer;

};
//! [2]

#endif
