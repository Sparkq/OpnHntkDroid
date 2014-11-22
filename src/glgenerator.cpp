#include <QGLWidget>
#include <QMutex>
#include <QTimer>
#include <QDebug>
#include "glgenerator.h"
#include "dataanalyzer.h"
#include "settings.h"

// class GlArray
/// \brief Initializes the array.
GlArray::GlArray() {
	this->data = 0;
	this->size = 0;
}

/// \brief Deletes the array.
GlArray::~GlArray() {
	if(this->data)
		delete this->data;
}

/// \brief Get the size of the array.
/// \return Number of array elements.
unsigned int GlArray::getSize() {
	return this->size;
}

/// \brief Set the size of the array.
/// Previous array contents are lost.
/// \param size New number of array elements.
void GlArray::setSize(unsigned int size) {
	if(this->size == size)
		return;
	
	if(this->data)
		delete[] this->data;
	if(size)
		this->data = new GLfloat[size];
	else
		this->data = 0;
	
	this->size = size;
}


////////////////////////////////////////////////////////////////////////////////
// class GlGenerator
/// \brief Initializes the scope widget.
/// \param settings The target settings object.
/// \param parent The parent widget.
GlGenerator::GlGenerator(DsoSettings *settings, QObject *parent, bool emulate) : QObject(parent) {
	this->settings = settings;
	
	this->dataAnalyzer = 0;
	this->digitalPhosphorDepth = 0;
	
	this->generateGrid();
    this->emulate = emulate;
    if(emulate) {
        QTimer *timerr = new QTimer(this);
        connect(timerr, SIGNAL(timeout()), this, SLOT(emulateGraphs()));
        timerr->start(10);
    }
}

/// \brief Deletes OpenGL objects.
GlGenerator::~GlGenerator() {
	/// \todo Clean up vaChannel
}

/// \brief Set the data analyzer whose data will be drawn.
/// \param dataAnalyzer Pointer to the DataAnalyzer class.
void GlGenerator::setDataAnalyzer(DataAnalyzer *dataAnalyzer) {
    if(this->dataAnalyzer)
		disconnect(this->dataAnalyzer, SIGNAL(finished()), this, SLOT(generateGraphs()));
	this->dataAnalyzer = dataAnalyzer;
	connect(this->dataAnalyzer, SIGNAL(finished()), this, SLOT(generateGraphs()));
}

/// \brief Prepare arrays for drawing the data we get from the data analyzer.
void GlGenerator::generateGraphs() {
    if( (!this->dataAnalyzer)||this->emulate )
		return;
	
	// Set digital phosphor depth to one if we don't use it
	if(this->settings->view.digitalPhosphor)
		this->digitalPhosphorDepth = this->settings->view.digitalPhosphorDepth;
	else
		this->digitalPhosphorDepth = 1;
	
	
	this->dataAnalyzer->mutex()->lock();

	
	switch(this->settings->scope.horizontal.format) {
		case Dso::GRAPHFORMAT_TY:
			// Add graphs for channels
			for(int mode = Dso::CHANNELMODE_VOLTAGE; mode < Dso::CHANNELMODE_COUNT; ++mode) {
                for(int channel = 0; channel < 2; ++channel) {
					// Check if this channel is used and available at the data analyzer
					if(((mode == Dso::CHANNELMODE_VOLTAGE) ? this->settings->scope.voltage[channel].used : this->settings->scope.spectrum[channel].used) && this->dataAnalyzer->data(channel) && this->dataAnalyzer->data(channel)->samples.voltage.sample) {
						
						
						// What's the horizontal distance between sampling points?
						double horizontalFactor;
						if(mode == Dso::CHANNELMODE_VOLTAGE)
							horizontalFactor = this->dataAnalyzer->data(channel)->samples.voltage.interval / this->settings->scope.horizontal.timebase;
						else
							horizontalFactor = this->dataAnalyzer->data(channel)->samples.spectrum.interval / this->settings->scope.horizontal.frequencybase;
						
						// Fill vector array
                        vaCha[channel].clear();

                        //unsigned int arrayPosition = 0;
						if(mode == Dso::CHANNELMODE_VOLTAGE) {
							for(unsigned int position = 0; position < this->dataAnalyzer->data(channel)->samples.voltage.count; ++position) {
                                vaCha[channel] << (position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2);
                                vaCha[channel] << (this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset)/(DIVS_VOLTAGE/2);
                                //if(channel == 1) {
                               // qDebug("%f",(position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2));
                               // qDebug("%f",(this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset)/(DIVS_VOLTAGE/2));
                               // }
                                }
						}
						else {
							for(unsigned int position = 0; position < this->dataAnalyzer->data(channel)->samples.spectrum.count; ++position) {
                            vaCha[channel] << (position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2);
                            vaCha[channel] << (this->dataAnalyzer->data(channel)->samples.spectrum.sample[position] / this->settings->scope.spectrum[channel].magnitude + this->settings->scope.spectrum[channel].offset)/(DIVS_VOLTAGE/2);
							}
						}
					}

				}
			}
			break;
			
		
		default:
			break;
	}
	
	this->dataAnalyzer->mutex()->unlock();
	
	emit graphsGenerated();
}

void GlGenerator::emulateGraphs() {

    // Set digital phosphor depth to one if we don't use it
    if(this->settings->view.digitalPhosphor)
        this->digitalPhosphorDepth = this->settings->view.digitalPhosphorDepth;
    else
        this->digitalPhosphorDepth = 1;

    switch(this->settings->scope.horizontal.format) {
        case Dso::GRAPHFORMAT_TY:
                for(int channel = 0; channel < 2 ; ++channel) {

                      double   horizontalFactor = 0.000000002/this->settings->scope.horizontal.timebase;

                                vaEmu[channel].clear();
                                float x =-1.0;
                                for(int i=0; i<10000; i++)
                                    {
                                    vaEmu[channel] << x;
                                    x += horizontalFactor;
                                    vaEmu[channel] << sin((float)i/10) / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset/(DIVS_VOLTAGE/2);
                                    }
            }

            break;


        default:
            break;
    }

    emit graphsGenerated();
}

/// \brief Create the needed OpenGL vertex arrays for the grid.
void GlGenerator::generateGrid() {
	// Grid
	this->vaGrid[0].setSize(((DIVS_TIME * DIVS_SUB - 2) * (DIVS_VOLTAGE - 2) + (DIVS_VOLTAGE * DIVS_SUB - 2) * (DIVS_TIME - 2) - ((DIVS_TIME - 2) * (DIVS_VOLTAGE - 2))) * 2);
	int pointIndex = 0;
	// Draw vertical lines
	for(int div = 1; div < DIVS_TIME / 2; ++div) {
		for(int dot = 1; dot < DIVS_VOLTAGE / 2 * DIVS_SUB; ++dot) {
			float dotPosition = (float) dot / DIVS_SUB;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
		}
	}
	// Draw horizontal lines
	for(int div = 1; div < DIVS_VOLTAGE / 2; ++div) {
		for(int dot = 1; dot < DIVS_TIME / 2 * DIVS_SUB; ++dot) {
			if(dot % DIVS_SUB == 0)
				continue;                   // Already done by vertical lines
			float dotPosition = (float) dot / DIVS_SUB;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = -div;
			this->vaGrid[0].data[pointIndex++] = -dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
			this->vaGrid[0].data[pointIndex++] = dotPosition;
			this->vaGrid[0].data[pointIndex++] = div;
		}
	}
	
	// Axes
	this->vaGrid[1].setSize((2 + (DIVS_TIME * DIVS_SUB - 2) + (DIVS_VOLTAGE * DIVS_SUB - 2)) * 4);
	pointIndex = 0;
	// Horizontal axis
	this->vaGrid[1].data[pointIndex++] = -DIVS_TIME / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = DIVS_TIME / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	// Vertical axis
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = -DIVS_VOLTAGE / 2;
	this->vaGrid[1].data[pointIndex++] = 0;
	this->vaGrid[1].data[pointIndex++] = DIVS_VOLTAGE / 2;
	// Subdiv lines on horizontal axis
	for(int line = 1; line < DIVS_TIME / 2 * DIVS_SUB; ++line) {
		float linePosition = (float) line / DIVS_SUB;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
	}
	// Subdiv lines on vertical axis
	for(int line = 1; line < DIVS_VOLTAGE / 2 * DIVS_SUB; ++line) {
		float linePosition = (float) line / DIVS_SUB;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = linePosition;
		this->vaGrid[1].data[pointIndex++] = -0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
		this->vaGrid[1].data[pointIndex++] = 0.05;
		this->vaGrid[1].data[pointIndex++] = -linePosition;
	}
	
	// Border
	this->vaGrid[2].setSize(4 * 2);
	this->vaGrid[2].data[0] = -DIVS_TIME / 2;
	this->vaGrid[2].data[1] = -DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[2] = DIVS_TIME / 2;
	this->vaGrid[2].data[3] = -DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[4] = DIVS_TIME / 2;
	this->vaGrid[2].data[5] = DIVS_VOLTAGE / 2;
	this->vaGrid[2].data[6] = -DIVS_TIME / 2;
	this->vaGrid[2].data[7] = DIVS_VOLTAGE / 2;
}
