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
    //GlArray vaGrid[3];
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

public:
    Viewer();
    bool emulate = false;
    QString message() {return m_message;}



    GlGenerator *generator; ///< The generator for the OpenGL vertex arrays

    ViewerRenderer *mainScope; ///< The main scope screen
    DsoSettings *settings; ///< The settings provided by the main window

    DataAnalyzer *dataAnalyzer; ///< The data source provided by the main window
    HardControl *hardControl;
    DsoControl *dsoControl;
    QString m_message;
    void connectSignals();
    void initializeDevice();


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
    void hard_event(int type, int value);
    // Other
   // void config();
   // void about();

    // Settings management
   // void applySettings();
   // void updateSettings();
    void handleWindowChanged(QQuickWindow *win);
   // void recordTimeChanged(double duration);
    void samplerateSelected(float samplerate = 0 );
    void timebaseSelected(int value = 10, int power = -3);
    void offsetSelected(unsigned int channel,float value );
   // void recordLengthSelected(unsigned long recordLength);
    void voltageGainSelected(unsigned int channel, int index);
    void updateUsed(unsigned int channel, bool checked);
    void emulateSelected(bool emulate);
    void showMessage(QString str, int);


signals:
   void settMessage();



private:
    ViewerRenderer *m_renderer;

};
//QML_DECLARE_TYPEINFO(Viewer, QML_HAS_ATTACHED_PROPERTIES)


#endif
