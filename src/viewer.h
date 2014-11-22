#ifndef VIEWER_H
#define VIEWER_H

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


class ViewerRenderer : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    ViewerRenderer(DsoSettings *settings, bool emulate);

    ~ViewerRenderer();
    GlGenerator *generator;

    bool emulate;
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    GlArray vaEmulated[2];
    GlArray vaChannel[1][2][1];

public slots:
    void paint();

private:
    QSize m_viewportSize;
    QOpenGLShaderProgram *m_program;

    DsoSettings *settings;

    GlArray vaMarker[2];
};



class Viewer : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message)
    Q_PROPERTY(QString amplitude1 READ amplitude1)
    Q_PROPERTY(QString amplitude2 READ amplitude2)

public:
    Viewer();
    bool emulate = false;
    QString message() {return m_message;}
    QString amplitude1() {return m_amplitude1;}
    QString amplitude2() {return m_amplitude2;}

    GlGenerator *generator; ///< The generator for the OpenGL vertex arrays

    ViewerRenderer *mainScope; ///< The main scope screen
    DsoSettings *settings; ///< The settings provided by the main window

    DataAnalyzer *dataAnalyzer; ///< The data source provided by the main window
    HardControl *hardControl;
    DsoControl *dsoControl;
    QString m_message;
    QString m_amplitude1;
    QString m_amplitude2;
    void connectSignals();
    void initializeDevice();


public slots:
    void sync();
    void cleanup();
    void update();
    // Hard Events
    void hard_event(int type, int value);

    // Settings management
    void handleWindowChanged(QQuickWindow *win);
    void samplerateSelected(float samplerate = 0 );
    void timebaseSelected(int value = 10, int power = -3);
    void offsetSelected(unsigned int channel,float value );
    void voltageGainSelected(unsigned int channel, int index);
    void updateUsed(unsigned int channel, bool checked);
    void emulateSelected(bool emulate);
    void showMessage(QString str, int);


signals:
   void settMessage();
   void settAmpl();



private:
    ViewerRenderer *m_renderer;

};


#endif
