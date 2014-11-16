////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  dataanalyzer.cpp
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


#include <cmath>

#include <QColor>
#include <QMutex>

#include <fftw3.h>


#include "dataanalyzer.h"

//#include "glscope.h"
#include "helper.h"
#include "settings.h"


////////////////////////////////////////////////////////////////////////////////
// class HorizontalDock
/// \brief Initializes the buffers and other variables.
/// \param settings The settings that should be used.
/// \param parent The parent widget.
DataAnalyzer::DataAnalyzer(DsoSettings *settings, QObject *parent) : QThread(parent) {
	this->settings = settings;
	
	this->lastRecordLength = 0;
	this->lastWindow = (Dso::WindowFunction) -1;
	this->window = 0;
	
	this->analyzedDataMutex = new QMutex();
	
	this->maxSamples = 0;
	
	this->waitingDataSamplerate = 0.0;
	this->waitingDataMutex = 0;
}

/// \brief Deallocates the buffers.
DataAnalyzer::~DataAnalyzer() {
	for(int channel = 0; channel < this->analyzedData.count(); ++channel) {
		AnalyzedData *channelData = this->analyzedData[channel];
		
		if(channelData->samples.voltage.sample)
			delete[] channelData->samples.voltage.sample;
		if(channelData->samples.spectrum.sample)
			delete[] channelData->samples.spectrum.sample;
	}
}

/// \brief Returns the analyzed data.
/// \param channel Channel, whose data should be returned.
/// \return Analyzed data as AnalyzedData struct.
const AnalyzedData *DataAnalyzer::data(int channel) const {
	if(channel < 0 || channel >= this->analyzedData.count())
		return 0;
	
	return this->analyzedData[channel];
}

/// \brief Returns the sample count of the analyzed data.
/// \return The maximum sample count of the last analyzed data.
unsigned int DataAnalyzer::sampleCount() {
	return this->maxSamples;
}

/// \brief Returns the mutex for the data.
/// \return Mutex for the analyzed data.
QMutex *DataAnalyzer::mutex() const {
	return this->analyzedDataMutex;
}

/// \brief Analyzes the data from the dso.
void DataAnalyzer::run() {
	this->analyzedDataMutex->lock();
	
	unsigned int maxSamples = 0;
	unsigned int channelCount = (unsigned int) this->settings->scope.voltage.count();
	
	// Adapt the number of channels for analyzed data
	for(unsigned int channel = this->analyzedData.count(); channel < channelCount; ++channel) {
		AnalyzedData *channelData = new AnalyzedData;
		channelData->samples.voltage.count = 0;
		channelData->samples.voltage.interval = 0;
		channelData->samples.voltage.sample = 0;
		channelData->samples.spectrum.count = 0;
		channelData->samples.spectrum.interval = 0;
		channelData->samples.spectrum.sample = 0;
		channelData->amplitude = 0;
		channelData->frequency = 0;
		this->analyzedData.append(channelData);
	}
	for(unsigned int channel = this->analyzedData.count(); channel > channelCount; --channel) {
		AnalyzedData *channelData = this->analyzedData.last();
		if(channelData->samples.voltage.sample)
			delete[] channelData->samples.voltage.sample;
		if(channelData->samples.spectrum.sample)
			delete[] channelData->samples.spectrum.sample;
		this->analyzedData.removeLast();
	}
	
	for(unsigned int channel = 0; channel < channelCount; ++channel) {
		AnalyzedData *channelData = this->analyzedData[channel];
		
		if( // Check...
			( // ...if we got data for this channel...
				channel < this->settings->scope.physicalChannels &&
				channel < (unsigned int) this->waitingData.count() &&
				this->waitingData[channel]) ||
			( // ...or if it's a math channel that can be calculated
				channel >= this->settings->scope.physicalChannels &&
				(this->settings->scope.voltage[channel].used || this->settings->scope.spectrum[channel].used) &&
				this->analyzedData.count() >= 2 &&
				this->analyzedData[0]->samples.voltage.sample &&
				this->analyzedData[1]->samples.voltage.sample
			)
		) {
			// Set sampling interval
			channelData->samples.voltage.interval = 1.0 / this->waitingDataSamplerate;
			
			unsigned int size;
			if(channel < this->settings->scope.physicalChannels) {
				size = this->waitingDataSize[channel];
				if(size > maxSamples)
					maxSamples = size;
			}
			else
				size = maxSamples;
			// Reallocate memory for samples if the sample count has changed
			if(channelData->samples.voltage.count != size) {
				channelData->samples.voltage.count = size;
				if(channelData->samples.voltage.sample)
					delete[] channelData->samples.voltage.sample;
                channelData->samples.voltage.sample = new float[size];
			}
			
			// Physical channels
			if(channel < this->settings->scope.physicalChannels) {
				// Copy the buffer of the oscilloscope into the sample buffer
				if(channel < (unsigned int) this->waitingData.count())
					for(unsigned int position = 0; position < this->waitingDataSize[channel]; ++position)
						channelData->samples.voltage.sample[position] = this->waitingData[channel][position];
			}
			// Math channel
			else {
				// Set sampling interval
				this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.interval = this->analyzedData[0]->samples.voltage.interval;
				
				// Reallocate memory for samples if the sample count has changed
				if(this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.count != this->analyzedData[0]->samples.voltage.count) {
					this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.count = this->analyzedData[0]->samples.voltage.count;
					if(this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample)
						delete[] this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample;
                    this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample = new float[this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.count];
				}
				
				// Calculate values and write them into the sample buffer
				for(unsigned int realPosition = 0; realPosition < this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.count; ++realPosition) {
					switch(this->settings->scope.voltage[this->settings->scope.physicalChannels].misc) {
						case Dso::MATHMODE_1ADD2:
							this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample[realPosition] = this->analyzedData[0]->samples.voltage.sample[realPosition] + this->analyzedData[1]->samples.voltage.sample[realPosition];
							break;
						case Dso::MATHMODE_1SUB2:
							this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample[realPosition] = this->analyzedData[0]->samples.voltage.sample[realPosition] - this->analyzedData[1]->samples.voltage.sample[realPosition];
							break;
						case Dso::MATHMODE_2SUB1:
							this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.sample[realPosition] = this->analyzedData[1]->samples.voltage.sample[realPosition] - this->analyzedData[0]->samples.voltage.sample[realPosition];
							break;
					}
				}
			}
		}
		else {
			// Clear unused channels
			channelData->samples.voltage.count = 0;
			this->analyzedData[this->settings->scope.physicalChannels]->samples.voltage.interval = 0;
			if(channelData->samples.voltage.sample) {
				delete[] channelData->samples.voltage.sample;
				channelData->samples.voltage.sample = 0;
			}
		}
	}
	
	this->waitingDataMutex->unlock();
	
	// Lower priority for spectrum calculation
	this->setPriority(QThread::LowPriority);
	
	
	// Calculate frequencies, peak-to-peak voltages and spectrums
	for(int channel = 0; channel < this->analyzedData.count(); ++channel) {
		AnalyzedData *channelData = this->analyzedData[channel];
		
		if(channelData->samples.voltage.sample) {
			// Calculate new window
			if(this->lastWindow != this->settings->scope.spectrumWindow || this->lastRecordLength != channelData->samples.voltage.count) {
				if(this->lastRecordLength != channelData->samples.voltage.count) {
					this->lastRecordLength = channelData->samples.voltage.count;
					
					if(this->window)
                        fftwf_free(this->window);
                    this->window = (float *) fftwf_malloc(sizeof(float) * this->lastRecordLength);
				}
				
				unsigned int windowEnd = this->lastRecordLength - 1;
				this->lastWindow = this->settings->scope.spectrumWindow;
				
				switch(this->settings->scope.spectrumWindow) {
					case Dso::WINDOW_HAMMING:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.54 - 0.46 * cos(2.0 * M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_HANN:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.5 * (1.0 - cos(2.0 * M_PI * windowPosition / windowEnd));
						break;
					case Dso::WINDOW_COSINE:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = sin(M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_LANCZOS:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition) {
                            float sincParameter = (2.0 * windowPosition / windowEnd - 1.0) * M_PI;
							if(sincParameter == 0)
								*(this->window + windowPosition) = 1;
							else
								*(this->window + windowPosition) = sin(sincParameter) / sincParameter;
						}
						break;
					case Dso::WINDOW_BARTLETT:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 2.0 / windowEnd * (windowEnd / 2 - abs(windowPosition - windowEnd / 2));
						break;
					case Dso::WINDOW_TRIANGULAR:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 2.0 / this->lastRecordLength * (this->lastRecordLength / 2 - abs(windowPosition - windowEnd / 2));
						break;
					case Dso::WINDOW_GAUSS:
						{
                            float sigma = 0.4;
							for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
								*(this->window + windowPosition) = exp(-0.5 * pow(((windowPosition - windowEnd / 2) / (sigma * windowEnd / 2)), 2));
						}
						break;
					case Dso::WINDOW_BARTLETTHANN:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.62 - 0.48 * abs(windowPosition / windowEnd - 0.5) - 0.38 * cos(2.0 * M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_BLACKMAN:
						{
                            float alpha = 0.16;
							for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
								*(this->window + windowPosition) = (1 - alpha) / 2 - 0.5 * cos(2.0 * M_PI * windowPosition / windowEnd) + alpha / 2 * cos(4.0 * M_PI * windowPosition / windowEnd);
						}
						break;
					//case WINDOW_KAISER:
						// TODO
                        //float alpha = 3.0;
						//for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							//*(this->window + windowPosition) = ;
						//break;
					case Dso::WINDOW_NUTTALL:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.355768 - 0.487396 * cos(2 * M_PI * windowPosition / windowEnd) + 0.144232 * cos(4 * M_PI * windowPosition / windowEnd) - 0.012604 * cos(6 * M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_BLACKMANHARRIS:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.35875 - 0.48829 * cos(2 * M_PI * windowPosition / windowEnd) + 0.14128 * cos(4 * M_PI * windowPosition / windowEnd) - 0.01168 * cos(6 * M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_BLACKMANNUTTALL:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 0.3635819 - 0.4891775 * cos(2 * M_PI * windowPosition / windowEnd) + 0.1365995 * cos(4 * M_PI * windowPosition / windowEnd) - 0.0106411 * cos(6 * M_PI * windowPosition / windowEnd);
						break;
					case Dso::WINDOW_FLATTOP:
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 1.0 - 1.93 * cos(2 * M_PI * windowPosition / windowEnd) + 1.29 * cos(4 * M_PI * windowPosition / windowEnd) - 0.388 * cos(6 * M_PI * windowPosition / windowEnd) + 0.032 * cos(8 * M_PI * windowPosition / windowEnd);
						break;
					default: // Dso::WINDOW_RECTANGULAR
						for(unsigned int windowPosition = 0; windowPosition < this->lastRecordLength; ++windowPosition)
							*(this->window + windowPosition) = 1.0;
				}
			}
			
			// Set sampling interval
			channelData->samples.spectrum.interval = 1.0 / channelData->samples.voltage.interval / channelData->samples.voltage.count;
			
			// Number of real/complex samples
			unsigned int dftLength = channelData->samples.voltage.count / 2;
			
			// Reallocate memory for samples if the sample count has changed
			if(channelData->samples.spectrum.count != dftLength) {
				channelData->samples.spectrum.count = dftLength;
				if(channelData->samples.spectrum.sample)
					delete[] channelData->samples.spectrum.sample;
                channelData->samples.spectrum.sample = new float[channelData->samples.voltage.count];
			}
			
			// Create sample buffer and apply window
            float *windowedValues = new float[channelData->samples.voltage.count];
			for(unsigned int position = 0; position < channelData->samples.voltage.count; ++position)
				windowedValues[position] = this->window[position] * channelData->samples.voltage.sample[position];
			
			// Do discrete real to half-complex transformation
			/// \todo Check if record length is multiple of 2
			/// \todo Reuse plan and use FFTW_MEASURE to get fastest algorithm
            fftwf_plan fftPlan = fftwf_plan_r2r_1d(channelData->samples.voltage.count, windowedValues, channelData->samples.spectrum.sample, FFTW_R2HC, FFTW_ESTIMATE);
            //FFTW_EXTERN fftwf_plan fftwf_plan_r2r_1d(int n, float *in, float *out, fftwf_r2r_kind kind, unsigned flags);
            fftwf_execute(fftPlan);
            fftwf_destroy_plan(fftPlan);
			
			// Do an autocorrelation to get the frequency of the signal
            float *conjugateComplex = windowedValues; // Reuse the windowedValues buffer
			
			// Real values
			unsigned int position;
            float correctionFactor = 1.0 / dftLength / dftLength;
			conjugateComplex[0] = (channelData->samples.spectrum.sample[0] * channelData->samples.spectrum.sample[0]) * correctionFactor;
			for(position = 1; position < dftLength; ++position)
				conjugateComplex[position] = (channelData->samples.spectrum.sample[position] * channelData->samples.spectrum.sample[position] + channelData->samples.spectrum.sample[channelData->samples.voltage.count - position] * channelData->samples.spectrum.sample[channelData->samples.voltage.count - position]) * correctionFactor;
			// Complex values, all zero for autocorrelation
			conjugateComplex[dftLength] = (channelData->samples.spectrum.sample[dftLength] * channelData->samples.spectrum.sample[dftLength]) * correctionFactor;
			for(++position; position < channelData->samples.voltage.count; ++position)
				conjugateComplex[position] = 0;
			
			// Do half-complex to real inverse transformation
            float *correlation = new float[channelData->samples.voltage.count];
            fftPlan = fftwf_plan_r2r_1d(channelData->samples.voltage.count, conjugateComplex, correlation, FFTW_HC2R, FFTW_ESTIMATE);
            fftwf_execute(fftPlan);
            fftwf_destroy_plan(fftPlan);
			delete[] conjugateComplex;
			
			// Calculate peak-to-peak voltage
            float minimalVoltage, maximalVoltage;
            minimalVoltage = maximalVoltage = channelData->samples.voltage.sample[0];
			
			for(unsigned int position = 1; position < channelData->samples.voltage.count; ++position) {
				if(channelData->samples.voltage.sample[position] < minimalVoltage)
					minimalVoltage = channelData->samples.voltage.sample[position];
				else if(channelData->samples.voltage.sample[position] > maximalVoltage)
					maximalVoltage = channelData->samples.voltage.sample[position];
			}
			
			channelData->amplitude = maximalVoltage - minimalVoltage;
			
			// Get the frequency from the correlation results
            float minimumCorrelation = correlation[0];
            float peakCorrelation = 0;
			unsigned int peakPosition = 0;
			
			for(unsigned int position = 1; position < channelData->samples.voltage.count / 2; ++position) {
				if(correlation[position] > peakCorrelation && correlation[position] > minimumCorrelation * 2) {
					peakCorrelation = correlation[position];
					peakPosition = position;
				}
				else if(correlation[position] < minimumCorrelation)
					minimumCorrelation = correlation[position];
			}
			delete[] correlation;
			
			// Calculate the frequency in Hz
			if(peakPosition)
				channelData->frequency = 1.0 / (channelData->samples.voltage.interval * peakPosition);
			else
				channelData->frequency = 0;
			
			// Finally calculate the real spectrum if we want it
			if(this->settings->scope.spectrum[channel].used) {
				// Convert values into dB (Relative to the reference level)
                float offset = 60 - this->settings->scope.spectrumReference - 20 * log10(dftLength);
                float offsetLimit = this->settings->scope.spectrumLimit - this->settings->scope.spectrumReference;
				for(unsigned int position = 0; position < channelData->samples.spectrum.count; ++position) {
					channelData->samples.spectrum.sample[position] = 20 * log10(fabs(channelData->samples.spectrum.sample[position])) + offset;
					
					// Check if this value has to be limited
					if(offsetLimit > channelData->samples.spectrum.sample[position])
						channelData->samples.spectrum.sample[position] = offsetLimit;
				}
			}
		}
		else if(channelData->samples.spectrum.sample) {
			// Clear unused channels
			channelData->samples.spectrum.count = 0;
			channelData->samples.spectrum.interval = 0;
			delete[] channelData->samples.spectrum.sample;
			channelData->samples.spectrum.sample = 0;
		}
	}
	
	this->maxSamples = maxSamples;
	emit(analyzed(maxSamples));
	
	this->analyzedDataMutex->unlock();
}

/// \brief Starts the analyzing of new input data.
/// \param data The data arrays with the input data.
/// \param size The sizes of the data arrays.
/// \param samplerate The samplerate for all input data.
/// \param mutex The mutex for all input data.
void DataAnalyzer::analyze(const QList<double *> *data, const QList<unsigned int> *size, double samplerate, QMutex *mutex) {
	// Previous analysis still running, drop the new data
	if(this->isRunning())
		return;
	
	// The thread will analyze it, just save the pointers
	mutex->lock();
	this->waitingData.clear();
	this->waitingData.append(*data);
	this->waitingDataSize.clear();
	this->waitingDataSize.append(*size);
	this->waitingDataMutex = mutex;
    this->waitingDataSamplerate = (float)(samplerate);
	this->start();
}
