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
#include "configdialog.h"
#include "dataanalyzer.h"
#include "dockwindows.h"
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

    // Apply the settings after the gui is initialized
   // this->applySettings();

    // Create hard control instance
    this->hardControl = new HardControl(this->settings);

    // Connect all signals
    this->connectSignals();

    // Set up the oscilloscope
    this->dsoControl->connectDevice();
    this->initializeDevice();
    this->dsoControl->startSampling();

    this->hardControl->updateLEDs();


    //m_renderer = new ViewerRenderer(this->settings);
    //m_renderer->setGenerator(this->generator);
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(handleWindowChanged(QQuickWindow*)));
}

void Viewer::connectSignals() {
    // Connect general signals
    //connect(this, SIGNAL(settingsChanged()), this, SLOT(applySettings()));
    //connect(this->dsoWidget, SIGNAL(stopped()), this, SLOT(stopped()));
    connect(this->dsoControl, SIGNAL(statusMessage(QString, int)), this, SLOT(showMessage(QString, int)));
    connect(this->dsoControl, SIGNAL(samplesAvailable(const QList<double *> *, const QList<unsigned int> *, double, QMutex *)), this->dataAnalyzer, SLOT(analyze(const QList<double *> *, const QList<unsigned int> *, double, QMutex *)));

    // Connect signals to DSO controller and widget
//    connect(this->horizontalDock, SIGNAL(samplerateChanged(double)), this, SLOT(samplerateSelected()));
//    connect(this->horizontalDock, SIGNAL(timebaseChanged(double)), this, SLOT(timebaseSelected()));
//    connect(this->horizontalDock, SIGNAL(frequencybaseChanged(double)), this->dsoWidget, SLOT(updateFrequencybase(double)));
//    connect(this->horizontalDock, SIGNAL(recordLengthChanged(unsigned long)), this, SLOT(recordLengthSelected(unsigned long)));
    //connect(this->horizontalDock, SIGNAL(formatChanged(HorizontalFormat)), this->dsoWidget, SLOT(horizontalFormatChanged(HorizontalFormat)));

//    connect(this->triggerDock, SIGNAL(modeChanged(Dso::TriggerMode)), this->dsoControl, SLOT(setTriggerMode(Dso::TriggerMode)));
//    connect(this->triggerDock, SIGNAL(modeChanged(Dso::TriggerMode)), this->dsoWidget, SLOT(updateTriggerMode()));
//    connect(this->triggerDock, SIGNAL(modeChanged(Dso::TriggerMode)), this->hardControl, SLOT(updateLEDs()));
//    connect(this->triggerDock, SIGNAL(sourceChanged(bool, unsigned int)), this->dsoControl, SLOT(setTriggerSource(bool, unsigned int)));
//    connect(this->triggerDock, SIGNAL(sourceChanged(bool, unsigned int)), this->dsoWidget, SLOT(updateTriggerSource()));
//    connect(this->triggerDock, SIGNAL(sourceChanged(bool, unsigned int)), this->hardControl, SLOT(updateLEDs()));
//    connect(this->triggerDock, SIGNAL(slopeChanged(Dso::Slope)), this->dsoControl, SLOT(setTriggerSlope(Dso::Slope)));
//    connect(this->triggerDock, SIGNAL(slopeChanged(Dso::Slope)), this->dsoWidget, SLOT(updateTriggerSlope()));
//    connect(this->triggerDock, SIGNAL(slopeChanged(Dso::Slope)), this->hardControl, SLOT(updateLEDs()));
//    connect(this->dsoWidget, SIGNAL(triggerPositionChanged(double)), this->dsoControl, SLOT(setPretriggerPosition(double)));
//    connect(this->dsoWidget, SIGNAL(triggerLevelChanged(unsigned int, double)), this->dsoControl, SLOT(setTriggerLevel(unsigned int, double)));

//    connect(this->voltageDock, SIGNAL(usedChanged(unsigned int, bool)), this, SLOT(updateUsed(unsigned int)));
//    connect(this->voltageDock, SIGNAL(usedChanged(unsigned int, bool)), this->dsoWidget, SLOT(updateVoltageUsed(unsigned int, bool)));
//    connect(this->voltageDock, SIGNAL(usedChanged(unsigned int, bool)), this->hardControl, SLOT(updateLEDs()));
//    connect(this->voltageDock, SIGNAL(couplingChanged(unsigned int, Dso::Coupling)), this->dsoControl, SLOT(setCoupling(unsigned int, Dso::Coupling)));
//    connect(this->voltageDock, SIGNAL(couplingChanged(unsigned int, Dso::Coupling)), this->dsoWidget, SLOT(updateVoltageCoupling(unsigned int)));
//    connect(this->voltageDock, SIGNAL(couplingChanged(unsigned int, Dso::Coupling)), this->hardControl, SLOT(updateLEDs()));
//    connect(this->voltageDock, SIGNAL(modeChanged(Dso::MathMode)), this->dsoWidget, SLOT(updateMathMode()));
//    connect(this->voltageDock, SIGNAL(gainChanged(unsigned int, double)), this, SLOT(updateVoltageGain(unsigned int)));
//    connect(this->voltageDock, SIGNAL(gainChanged(unsigned int, double)), this->dsoWidget, SLOT(updateVoltageGain(unsigned int)));
//    connect(this->dsoWidget, SIGNAL(offsetChanged(unsigned int, double)), this, SLOT(updateOffset(unsigned int)));

//    connect(this->spectrumDock, SIGNAL(usedChanged(unsigned int, bool)), this, SLOT(updateUsed(unsigned int)));
//    connect(this->spectrumDock, SIGNAL(usedChanged(unsigned int, bool)), this->dsoWidget, SLOT(updateSpectrumUsed(unsigned int, bool)));
//    connect(this->spectrumDock, SIGNAL(magnitudeChanged(unsigned int, double)), this->dsoWidget, SLOT(updateSpectrumMagnitude(unsigned int)));

    // Started/stopped signals from oscilloscope
   // connect(this->dsoControl, SIGNAL(samplingStarted()), this, SLOT(started()));
    connect(this->dsoControl, SIGNAL(samplingStarted()), this->hardControl, SLOT(started()));
   // connect(this->dsoControl, SIGNAL(samplingStopped()), this, SLOT(stopped()));
    connect(this->dsoControl, SIGNAL(samplingStopped()), this->hardControl, SLOT(stopped()));

    //connect(this->dsoControl, SIGNAL(recordLengthChanged(unsigned long)), this, SLOT(recordLengthChanged()));
    //connect(this->dsoControl, SIGNAL(recordTimeChanged(double)), this, SLOT(recordTimeChanged(double)));
    //connect(this->dsoControl, SIGNAL(samplerateChanged(double)), this, SLOT(samplerateChanged(double)));

    //connect(this->dsoControl, SIGNAL(availableRecordLengthsChanged(QList<unsigned int>)), this->horizontalDock, SLOT(availableRecordLengthsChanged(QList<unsigned int>)));
   // connect(this->dsoControl, SIGNAL(samplerateLimitsChanged(double, double)), this->horizontalDock, SLOT(samplerateLimitsChanged(double, double)));

    // Hard Events
    connect(this->hardControl, SIGNAL(new_event(int, int)), this, SLOT(hard_event(int, int)));
    //connect(this->hardControl, SIGNAL(new_event(int, int)), this->dsoWidget, SLOT(hard_event(int, int)));
    //connect(this->hardControl, SIGNAL(new_event(int, int)), this->horizontalDock, SLOT(hard_event(int, int)));
    //connect(this->hardControl, SIGNAL(new_event(int, int)), this->triggerDock, SLOT(hard_event(int, int)));
    //connect(this->hardControl, SIGNAL(new_event(int, int)), this->voltageDock, SLOT(hard_event(int, int)));
}

void Viewer::initializeDevice() {
#if 0
    qDebug("%s:%i\n", __func__, __LINE__);qDebug("%s:%i\n", __func__, __LINE__);
    for(unsigned int channel = 0; channel < this->settings->scope.physicalChannels; ++channel) {
        this->dsoControl->setCoupling(channel, (Dso::Coupling) this->settings->scope.voltage[channel].misc);
        this->updateVoltageGain(channel);
        this->updateOffset(channel);
        this->dsoControl->setTriggerLevel(channel, this->settings->scope.voltage[channel].trigger);
    }
#endif
    this->updateUsed(this->settings->scope.physicalChannels);
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

    // Apply the limits to the dock widgets
    //this->horizontalDock->availableRecordLengthsChanged(*this->dsoControl->getAvailableRecordLengths());
    //this->horizontalDock->samplerateLimitsChanged(this->dsoControl->getMinSamplerate(), this->dsoControl->getMaxSamplerate());
}

void Viewer::updateUsed(unsigned int channel) {
    if(channel >= (unsigned int) this->settings->scope.voltage.count())
        return;

    bool mathUsed = this->settings->scope.voltage[this->settings->scope.physicalChannels].used | this->settings->scope.spectrum[this->settings->scope.physicalChannels].used;

    // Normal channel, check if voltage/spectrum or math channel is used
    if(channel < this->settings->scope.physicalChannels)
        this->dsoControl->setChannelUsed(channel, mathUsed | this->settings->scope.voltage[channel].used | this->settings->scope.spectrum[channel].used);
    // Math channel, update all channels
    else if(channel == this->settings->scope.physicalChannels) {
        for(unsigned int channelCounter = 0; channelCounter < this->settings->scope.physicalChannels; ++channelCounter)
            this->dsoControl->setChannelUsed(channelCounter, mathUsed | this->settings->scope.voltage[channelCounter].used | this->settings->scope.spectrum[channelCounter].used);
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


//		if(channel < (int) this->settings->scope.physicalChannels)
//			this->adaptTriggerLevelSlider(channel);
    }
//	else if(channel < this->settings->scope.voltage.count() * 2)
//		this->settings->scope.spectrum[channel - this->settings->scope.voltage.count()].offset = value;

//	emit offsetChanged(channel, value);
    this->dsoControl->setOffset(channel, (this->settings->scope.voltage[channel].offset / DIVS_VOLTAGE) + 0.5);
}

//void Viewer::horizontalFormatSelected(unsigned long recordLength) {
//    this->dsoControl->setRecordLength(recordLength);
//}

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
    int i;

    switch (type) {
    case PANEL_SW_R_RUN_STOP:
       // startStopAction->activate(QAction::Trigger);
        break;
    case PANEL_SW_H_TIMEBASE:
       // zoomAction->activate(QAction::Trigger);
        break;
    case PANEL_SW_R_DEFAULT:
       // horizontalDock->setSamplerate(1e6);
       // horizontalDock->setTimebase(1e-3);
       // horizontalDock->setFrequencybase(1e3);
#if 0
        horizontalDock->setRecordLength();
#endif
       // horizontalDock->setFormat(Dso::GRAPHFORMAT_TY);
       // triggerDock->setMode(Dso::TRIGGERMODE_AUTO);
       // triggerDock->setSource(false, 0);
       // triggerDock->setSlope(Dso::SLOPE_POSITIVE);

       // for (i = 0; i < HANTEK_CHANNELS; i++) {
       //     voltageDock->setCoupling(i, Dso::COUPLING_DC);
       //     voltageDock->setGain(i, 1);
       //     voltageDock->setUsed(i, 1);
       // }
       // voltageDock->setUsed(HANTEK_CHANNELS, 0);
       // voltageDock->setGain(HANTEK_CHANNELS, 1);
       // voltageDock->setMode(Dso::MATHMODE_1ADD2);

        break;
    case PANEL_SW_T_MANUAL:
        this->dsoControl->forceTrigger();
        break;
    }
}





///// \brief The oscilloscope started sampling.
//void Viewer::started() {
//    this->startStopAction->setText(tr("&Stop"));
//    this->startStopAction->setIcon(QIcon(":actions/stop.png"));
//    this->startStopAction->setStatusTip(tr("Stop the oscilloscope"));

//    disconnect(this->startStopAction, SIGNAL(triggered()), this->dsoControl, SLOT(startSampling()));
//    connect(this->startStopAction, SIGNAL(triggered()), this->dsoControl, SLOT(stopSampling()));
//}

/////// \brief The oscilloscope stopped sampling.
//void Viewer::stopped() {
//    this->startStopAction->setText(tr("&Start"));
//    this->startStopAction->setIcon(QIcon(":actions/start.png"));
//    this->startStopAction->setStatusTip(tr("Start the oscilloscope"));

//    disconnect(this->startStopAction, SIGNAL(triggered()), this->dsoControl, SLOT(stopSampling()));
//    connect(this->startStopAction, SIGNAL(triggered()), this->dsoControl, SLOT(startSampling()));
//}

void Viewer::update()
{
    this->m_renderer->vaEmulated[0].setSize(this->generator->vaEmu.size());

    for (int i = 0; i< this->generator->vaEmu.size(); i++)
        this->m_renderer->vaEmulated[0].data[i] = this->generator->vaEmu.at(i);
    for (int j = 0; j< 2; j++)
    {
    this->m_renderer->vaChannel[0][j][0].setSize(this->generator->vaCha[j].size());

    for (int i = 0; i< this->generator->vaCha[j].size(); i++)
        this->m_renderer->vaChannel[0][j][0].data[i] = this->generator->vaCha[j].at(i);
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
    //this->zoomed = false;
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

        // Apply zoom settings via matrix transformation
        //if(this->zoomed) {
            //glPushMatrix();
            //glScalef(DIVS_TIME / fabs(this->settings->scope.horizontal.marker[1] - this->settings->scope.horizontal.marker[0]), 1.0, 1.0);
            //glTranslatef(-(this->settings->scope.horizontal.marker[0] + this->settings->scope.horizontal.marker[1]) / 2, 0.0, 0.0);
        //}

        // Values we need for the fading of the digital phosphor
//		double *fadingFactor = new double[this->generator->digitalPhosphorDepth];
//		fadingFactor[0] = 100;
//		double fadingRatio = pow(10.0, 2.0 / this->generator->digitalPhosphorDepth);
//		for(int index = 1; index < this->generator->digitalPhosphorDepth; ++index)
//			fadingFactor[index] = fadingFactor[index - 1] * fadingRatio;

        if(!this->emulate) {
    switch(this->settings->scope.horizontal.format) {
        case Dso::GRAPHFORMAT_TY:
            // Real and virtual channels
            for(int mode = Dso::CHANNELMODE_VOLTAGE; mode < Dso::CHANNELMODE_COUNT; ++mode) {
                for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
                    if((mode == Dso::CHANNELMODE_VOLTAGE) ? this->settings->scope.voltage[channel].used : this->settings->scope.spectrum[channel].used) {
                        // Draw graph for all available depths

                             for(int index = this->generator->digitalPhosphorDepth - 1; index >= 0; index--) {

                           if(this->vaChannel[mode][channel][index].data) {
                                if(mode == Dso::CHANNELMODE_VOLTAGE)
                                    m_program->setUniformValue(1, QColor(255,255,0,255));
                                    //this->qglColor(this->settings->view.color.screen.voltage[channel].darker(fadingFactor[index]));
                                else
                                    m_program->setUniformValue(1, QColor(0,255,255,255));
                                //this->qglColor(this->settings->view.color.screen.spectrum[channel].darker(fadingFactor[index]));



                                for(int i=0; i< 40; i++)
                                     qDebug("%f",this->vaChannel[mode][channel][index].data[i]);
                                m_program->setAttributeArray(0, GL_FLOAT, this->vaChannel[mode][channel][index].data, 2);
                                //glVertexPointer(2, GL_FLOAT, 0, this->generator->vaChannel[mode][channel][index]->data);
                                glDrawArrays((this->settings->view.interpolation == Dso::INTERPOLATION_OFF) ? GL_POINTS : GL_LINE_STRIP, 0, this->vaChannel[mode][channel][index].getSize() / 2);
                            }




                        }
                        }
                    }
                }

            break;

        case Dso::GRAPHFORMAT_XY:
            // Real and virtual channels
            for(int channel = 0; channel < this->settings->scope.voltage.count() - 1; channel += 2) {
                if(this->settings->scope.voltage[channel].used) {
                    // Draw graph for all available depths
                    for(int index =  this->generator->digitalPhosphorDepth - 1; index >= 0; index--) {
                       // if(this->generator->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->data) {
                            m_program->setUniformValue(1, QColor(0,255,0,255));
                            //this->qglColor(this->settings->view.color.screen.voltage[channel].darker(fadingFactor[index]));
                            m_program->setAttributeArray(0, GL_FLOAT, this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index].data, 2);
                            //glVertexPointer(2, GL_FLOAT, 0, this->generator->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->data);
                            glDrawArrays((this->settings->view.interpolation == Dso::INTERPOLATION_OFF) ? GL_POINTS : GL_LINE_STRIP, 0, this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index].getSize() / 2);
                      //  }
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


                m_program->setUniformValue(1, QColor(255,0,0,255));

           // for(int i=0; i< 40; i++)
            //    qDebug("%f",this->vaEmulated[0].data[i]);

            m_program->setAttributeArray(0, GL_FLOAT, this->vaEmulated[0].data, 2);

            glDrawArrays((this->settings->view.interpolation == Dso::INTERPOLATION_OFF) ? GL_POINTS : GL_LINE_STRIP, 0, this->vaEmulated[0].getSize() / 2);


        }

    //	glDisable(GL_POINT_SMOOTH);
    //	glDisable(GL_LINE_SMOOTH);

        // Grid
        m_program->setUniformValue(1, QColor(255,255,255,255));
        //this->qglColor(this->settings->view.color.screen.grid);
        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[0].data, 2);
        //glVertexPointer(2, GL_FLOAT, 0, this->generator->vaGrid[0].data);
        glDrawArrays(GL_POINTS, 0, this->generator->vaGrid[0].getSize() / 2);
        // Axes
        m_program->setUniformValue(1, QColor(255,255,255,255));
        //this->qglColor(this->settings->view.color.screen.axes);
        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[1].data, 2);
        //glVertexPointer(2, GL_FLOAT, 0, this->generator->vaGrid[1].data);
        glDrawArrays(GL_LINES, 0, this->generator->vaGrid[1].getSize() / 2);
        // Border
        m_program->setUniformValue(1, QColor(255,255,255,255));
        //this->qglColor(this->settings->view.color.screen.border);
        m_program->setAttributeArray(0, GL_FLOAT, this->generator->vaGrid[2].data, 2);
        //glVertexPointer(2, GL_FLOAT, 0, this->generator->vaGrid[2].data);
        glDrawArrays(GL_LINE_LOOP, 0, this->generator->vaGrid[2].getSize() / 2);


    m_program->disableAttributeArray(0);
    m_program->release();

    }
}
