#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <cmath>
#include <QColor>
#include <QDebug>
#include "dataanalyzer.h"
#include "glgenerator.h"
#include "settings.h"
#include "hardcontrol.h"
#include "dataanalyzer.h"
#include "dsocontrol.h"
#include "viewer.h"
#include "settings.h"
#include "hantek/control.h"
#include "requests.h"

Viewer::Viewer()

{
    m_message = "test";
    this->m_renderer = 0;
    // Create the controller for the oscilloscope, provides channel count for settings
    this->dsoControl = new Hantek::Control();

    // Application settings
    this->settings = new DsoSettings();
    this->settings->setChannelCount(this->dsoControl->getChannelCount());

    // The data analyzer
    this->dataAnalyzer = new DataAnalyzer(this->settings);


    // Create hard control instance
    this->hardControl = new HardControl(this->settings);

    // Connect all signals
    this->connectSignals();

    // Set up the oscilloscope
    this->dsoControl->connectDevice();
    this->initializeDevice();
    this->dsoControl->startSampling();

    this->hardControl->updateLEDs();

    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

void Viewer::connectSignals() {
    // Connect general signals
    connect(this->dsoControl, SIGNAL(statusMessage(QString, int)), this, SLOT(showMessage(QString, int)));
    connect(this->dsoControl, SIGNAL(samplesAvailable(const QList<double *> *, const QList<unsigned int> *, double, QMutex *)), this->dataAnalyzer, SLOT(analyze(const QList<double *> *, const QList<unsigned int> *, double, QMutex *)));

    // Started/stopped signals from oscilloscope
    connect(this->dsoControl, SIGNAL(samplingStarted()), this->hardControl, SLOT(started()));
    connect(this->dsoControl, SIGNAL(samplingStopped()), this->hardControl, SLOT(stopped()));

    // Hard Events
    connect(this->hardControl, SIGNAL(new_event(int, int)), this, SLOT(hard_event(int, int)));

}

void Viewer::initializeDevice() {

    this->updateUsed(this->settings->scope.physicalChannels, true);
    if(this->settings->scope.horizontal.samplerateSet)
        this->samplerateSelected();
    else
        this->timebaseSelected();
    if(this->dsoControl->getAvailableRecordLengths()->isEmpty())
        this->dsoControl->setRecordLength(this->settings->scope.horizontal.recordLength);
    else
        this->dsoControl->setRecordLength(this->dsoControl->getAvailableRecordLengths()->indexOf(this->settings->scope.horizontal.recordLength));
    this->dsoControl->setTriggerMode(this->settings->scope.trigger.mode);
    this->dsoControl->setPretriggerPosition(this->settings->scope.trigger.position * this->settings->scope.horizontal.timebase * DIVS_TIME);
    this->dsoControl->setTriggerSlope(this->settings->scope.trigger.slope);
    this->dsoControl->setTriggerSource(this->settings->scope.trigger.special, this->settings->scope.trigger.source);

    }

void Viewer::updateUsed(unsigned int channel, bool checked) {

    if(channel >= (unsigned int) this->settings->scope.voltage.count())
        return;
    else
        this->settings->scope.voltage[channel].used = checked;

    bool mathUsed = this->settings->scope.voltage[this->settings->scope.physicalChannels].used | this->settings->scope.spectrum[this->settings->scope.physicalChannels].used;

    // Normal channel, check if voltage/spectrum or math channel is used
    if(channel < this->settings->scope.physicalChannels)
        this->dsoControl->setChannelUsed(channel, mathUsed | this->settings->scope.voltage[channel].used | this->settings->scope.spectrum[channel].used);
    // Math channel, update all channels
    else if(channel == this->settings->scope.physicalChannels) {
        for(unsigned int channelCounter = 0; channelCounter < this->settings->scope.physicalChannels; ++channelCounter)
            this->dsoControl->setChannelUsed(channelCounter, mathUsed | this->settings->scope.voltage[channelCounter].used | this->settings->scope.spectrum[channelCounter].used);


        this->hardControl->updateLEDs();
    }
}
void Viewer::samplerateSelected(float samplerate ) {
    if(samplerate)
        this->settings->scope.horizontal.samplerate = samplerate;
    this->dsoControl->setSamplerate(this->settings->scope.horizontal.samplerate);
}

///// \brief Sets the record time of the oscilloscope.
void Viewer::timebaseSelected(int value, int power) {
    float timebase = (float) value * qPow(10,power);
    if(timebase)
        this->settings->scope.horizontal.timebase = timebase;
    this->dsoControl->setRecordTime(this->settings->scope.horizontal.timebase * DIVS_TIME);
}
void Viewer::offsetSelected(unsigned int channel,float value ) {
    if(channel < this->settings->scope.voltage.count()) {
        this->settings->scope.voltage[channel].offset = value * (DIVS_VOLTAGE/2);

    }

    this->dsoControl->setOffset(channel, (this->settings->scope.voltage[channel].offset / DIVS_VOLTAGE) + 0.5);
}


void Viewer::voltageGainSelected(unsigned int channel, int index) {
    if(channel >= this->settings->scope.physicalChannels)
        return;
   QList<double>  gainSteps ;
   gainSteps<< 1e-2 << 2e-2 << 5e-2 << 1e-1 << 2e-1 << 5e-1 <<  1e0 <<  2e0 <<  5e0;
    this->settings->scope.voltage[channel].gain = gainSteps.at(index);
    this->dsoControl->setGain(channel, this->settings->scope.voltage[channel].gain * DIVS_VOLTAGE);
}

void Viewer::emulateSelected(bool emulate) {
    this->emulate = emulate;
    if(m_renderer->generator)
        disconnect(m_renderer->generator, SIGNAL(graphsGenerated()), this, SLOT(update()));
    this->m_renderer->generator = 0;
    this->m_renderer = 0;
    this->sync();

}

void Viewer::showMessage(QString str, int) {
    m_message = str;
    emit settMessage();
}


void Viewer::hard_event(int type, int value) {
    switch (type)
    {
    case PANEL_SW_T_MANUAL:
        this->dsoControl->forceTrigger();
        break;
    }
}


void Viewer::update()
{
    for (int j = 0; j< 2; j++)
    {
    this->m_renderer->vaEmulated[j].setSize(this->generator->vaEmu[j].size());

    for (int i = 0; i< this->generator->vaEmu[j].size(); i++)
        this->m_renderer->vaEmulated[j].data[i] = this->generator->vaEmu[j].at(i);
    }
    for (int j = 0; j< 2; j++)
    {
    this->m_renderer->vaChannel[0][j][0].setSize(this->generator->vaCha[j].size());

    for (int i = 0; i< this->generator->vaCha[j].size(); i++)
        this->m_renderer->vaChannel[0][j][0].data[i] = this->generator->vaCha[j].at(i);
    }
    if (!emulate)
    {
    m_amplitude1 = Helper::valueToString(this->dataAnalyzer->data(0)->amplitude, Helper::UNIT_VOLTS,4);
    m_amplitude2 = Helper::valueToString(this->dataAnalyzer->data(1)->amplitude, Helper::UNIT_VOLTS,4);
    emit settAmpl();
    }
    if (window())
        window()->update();
}


void Viewer::handleWindowChanged(QQuickWindow *win)
{
    if (win) {
        connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(sync()), Qt::DirectConnection);
        connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(cleanup()), Qt::DirectConnection);

        // If we allow QML to do the clearing, they would clear what we paint
        // and nothing would show.
        win->setClearBeforeRendering(false);
    }
}


void Viewer::cleanup()
{
    if (m_renderer) {
        delete m_renderer;
        m_renderer = 0;
    }
}

ViewerRenderer::ViewerRenderer (DsoSettings *settings,bool emulate) {
    this->emulate = emulate;
    this->settings = settings;

    this->generator = 0;

    m_program = 0;
}

ViewerRenderer::~ViewerRenderer()
{
    delete m_program;
    this->generator = 0;
}



void Viewer::sync()
{

    if (!m_renderer) {
        this->generator = new GlGenerator(this->settings, this, this->emulate);
        this->generator->setDataAnalyzer(this->dataAnalyzer);
        m_renderer = new ViewerRenderer( this->settings, this->emulate);

        m_renderer->generator = generator;
        connect(m_renderer->generator, SIGNAL(graphsGenerated()), this, SLOT(update()), Qt::DirectConnection);

        connect(window(), SIGNAL(beforeRendering()), m_renderer, SLOT(paint()), Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());

}


void ViewerRenderer::paint()
{
    if (!m_program) {
        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();   
        m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                           "attribute highp vec4 vertices;"
                                           "varying highp vec2 coords;"
                                           "void main(void) {"
                                           "    gl_Position = vertices;"
                                           "    gl_PointSize = 5.0;"
                                           "    coords = vertices.xy;"
                                           "}");
        m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                           "uniform mediump vec4 color;"
                                           "varying highp vec2 coords;"
                                           "void main(void) {"
                                           "    gl_FragColor = color;"
                                           "}");

        m_program->bindAttributeLocation("vertices", 0);
        m_program->bindAttributeLocation("color", 1);
        m_program->link();

    }
    m_program->bind();

    m_program->enableAttributeArray(0);

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

//    glDisable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 2);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);


    // Draw the graphs
    if(this->generator && this->generator->digitalPhosphorDepth > 0) {

        //if(this->settings->view.antialiasing) {
//			glEnable(GL_POINT_SMOOTH);
//			glEnable(GL_LINE_SMOOTH);
//			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        //}


        if(!this->emulate) {
    switch(this->settings->scope.horizontal.format) {
        case Dso::GRAPHFORMAT_TY:
            // Real and virtual channels
            for(int mode = Dso::CHANNELMODE_VOLTAGE; mode < Dso::CHANNELMODE_COUNT; ++mode) {
                for(int channel = 0; channel < 2; ++channel) {
                //for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
                    if((mode == Dso::CHANNELMODE_VOLTAGE) ? this->settings->scope.voltage[channel].used : this->settings->scope.spectrum[channel].used) {
                        // Draw graph for all available depths

                             for(int index = this->generator->digitalPhosphorDepth - 1; index >= 0; index--) {

                           if(this->vaChannel[mode][channel][index].data) {

                                if (channel == 0)
                                    m_program->setUniformValue(1, QColor(255,255,0,255));
                                else
                                    m_program->setUniformValue(1, QColor(0,255,255,255));



                                //for(int i=0; i< 40; i++)
                                //     qDebug("%f",this->vaChannel[mode][channel][index].data[i]);
                                m_program->setAttributeArray(0, GL_FLOAT, this->vaChannel[mode][channel][index].data, 2);

                                glDrawArrays((this->settings->view.interpolation == Dso::INTERPOLATION_OFF) ? GL_POINTS : GL_LINE_STRIP, 0, this->vaChannel[mode][channel][index].getSize() / 2);
                            }




                        }
                        }
                    }
                }

            break;

        default:
            break;
    }
        }
        else
        {
            for(int channel = 0; channel < 2; ++channel) {
            if (channel == 0)
                m_program->setUniformValue(1, QColor(255,255,0,255));
            else
                m_program->setUniformValue(1, QColor(0,255,255,255));
            m_program->setAttributeArray(0, GL_FLOAT, this->vaEmulated[channel].data, 2);

            glDrawArrays((this->settings->view.interpolation == Dso::INTERPOLATION_OFF) ? GL_POINTS : GL_LINE_STRIP, 0, this->vaEmulated[channel].getSize() / 2);
        }

        }

    //	glDisable(GL_POINT_SMOOTH);
    //	glDisable(GL_LINE_SMOOTH);

        // Grid
        m_program->setUniformValue(1, QColor(255,255,255,255));

        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[0].data, 2);

        glDrawArrays(GL_POINTS, 0, this->generator->vaGrid[0].getSize() / 2);
        // Axes
        m_program->setUniformValue(1, QColor(255,255,255,255));

        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[1].data, 2);

        glDrawArrays(GL_LINES, 0, this->generator->vaGrid[1].getSize() / 2);
        // Border
        m_program->setUniformValue(1, QColor(255,255,255,255));

        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[2].data, 2);

        glDrawArrays(GL_LINE_LOOP, 0, this->generator->vaGrid[2].getSize() / 2);


    m_program->disableAttributeArray(0);
    m_program->release();

    }
}
