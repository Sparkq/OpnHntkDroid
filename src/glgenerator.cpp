////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  glgenerator.cpp
//
//  Copyright (C) 2010  Oliver Haag
//  oliver.haag@gmail.com
//
//  This program is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation, either version 3 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////


#include <QGLWidget>
#include <QMutex>
#include <QTimer>
#include <QDebug>


#include "glgenerator.h"

#include "dataanalyzer.h"
#include "settings.h"


////////////////////////////////////////////////////////////////////////////////
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
                //for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
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
                                //vaNewChannel[arrayPosition++] = position * horizontalFactor - DIVS_TIME / 2;
                                vaCha[channel] << (position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2);

                                //vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset;
                                vaCha[channel] << (this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset)/(DIVS_VOLTAGE/2);
                                //if(channel == 1) {
                               // qDebug("%f",(position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2));
                               // qDebug("%f",(this->dataAnalyzer->data(channel)->samples.voltage.sample[position] / this->settings->scope.voltage[channel].gain + this->settings->scope.voltage[channel].offset)/(DIVS_VOLTAGE/2));
                               // }
                                }
						}
						else {
							for(unsigned int position = 0; position < this->dataAnalyzer->data(channel)->samples.spectrum.count; ++position) {
                            //	vaNewChannel[arrayPosition++] = position * horizontalFactor - DIVS_TIME / 2;
                            vaCha[channel] << (position * horizontalFactor - DIVS_TIME / 2)/(DIVS_TIME/2);
                                //	vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(channel)->samples.spectrum.sample[position] / this->settings->scope.spectrum[channel].magnitude + this->settings->scope.spectrum[channel].offset;
                            vaCha[channel] << (this->dataAnalyzer->data(channel)->samples.spectrum.sample[position] / this->settings->scope.spectrum[channel].magnitude + this->settings->scope.spectrum[channel].offset)/(DIVS_VOLTAGE/2);
							}
						}
					}
                    //else {
						// Delete all vector arrays
                    //	for(int index = 0; index < this->digitalPhosphorDepth; ++index)
                    //		this->vaChannel[mode][channel][index]->setSize(0);
                    //}
				}
			}
			break;
			
//		case Dso::GRAPHFORMAT_XY:
//			for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
//				// For even channel numbers check if this channel is used and this and the following channel are available at the data analyzer
//				if(channel % 2 == 0 && channel + 1 < this->settings->scope.voltage.count() && this->settings->scope.voltage[channel].used && this->dataAnalyzer->data(channel) && this->dataAnalyzer->data(channel)->samples.voltage.sample && this->dataAnalyzer->data(channel + 1) && this->dataAnalyzer->data(channel + 1)->samples.voltage.sample) {
//					// Check if the sample count has changed
//					unsigned int neededSize = qMin(this->dataAnalyzer->data(channel)->samples.voltage.count, this->dataAnalyzer->data(channel + 1)->samples.voltage.count) * 2;
//					for(int index = 0; index < this->digitalPhosphorDepth; ++index) {
//						if(this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->getSize() != neededSize)
//							this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->setSize(0);
//					}
					
//					// Check if the array is allocated
//					if(!this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->data)
//						this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->setSize(neededSize);
					
//					GLfloat *vaNewChannel = this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel].first()->data;
					
//					// Fill vector array
//					unsigned int arrayPosition = 0;
//					unsigned int xChannel = channel;
//					unsigned int yChannel = channel + 1;
//					for(unsigned int position = 0; position < this->dataAnalyzer->data(channel)->samples.voltage.count; ++position) {
//						vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(xChannel)->samples.voltage.sample[position] / this->settings->scope.voltage[xChannel].gain + this->settings->scope.voltage[xChannel].offset;
//						vaNewChannel[arrayPosition++] = this->dataAnalyzer->data(yChannel)->samples.voltage.sample[position] / this->settings->scope.voltage[yChannel].gain + this->settings->scope.voltage[yChannel].offset;
//					}
//				}
//				else {
//					// Delete all vector arrays
//					for(int index = 0; index < this->digitalPhosphorDepth; ++index)
//						this->vaChannel[Dso::CHANNELMODE_VOLTAGE][channel][index]->setSize(0);
//				}
				
//				// Delete all spectrum graphs
//				for(int index = 0; index < this->digitalPhosphorDepth; ++index)
//					this->vaChannel[Dso::CHANNELMODE_SPECTRUM][channel][index]->setSize(0);
//			}
//			break;
		
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

                      double   horizontalFactor = this->settings->scope.horizontal.timebase;

                                vaEmu[channel].clear();
                                float x =-1.0;
                                for(int i=0; i<10240; i++)
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
