////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \file glgenerator.h
/// \brief Declares the GlScope class.
//
//  Copyright (C) 2008, 2009  Oleg Khudyakov
//  prcoder@potrebitel.ru
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


#ifndef GLGENERATOR_H
#define GLGENERATOR_H


#include <QGLWidget>
#include <QList>
#include <QObject>


#include "dso.h"


#define DIVS_TIME                  10.0 ///< Number of horizontal screen divs
#define DIVS_VOLTAGE                8.0 ///< Number of vertical screen divs
#define DIVS_SUB                      5 ///< Number of sub-divisions per div


class DataAnalyzer;
class DsoSettings;
//class GlScope;


////////////////////////////////////////////////////////////////////////////////
/// \class GlArray                                                 glgenerator.h
/// \brief An array of GLfloat values and it's size.
class GlArray {
	public:
		GlArray();
		~GlArray();
		
		unsigned int getSize();
		void setSize(unsigned int size);
		
		GLfloat *data; ///< Pointer to the array
	
	protected:
		unsigned int size; ///< The array size (Number of GLfloat values)
};

////////////////////////////////////////////////////////////////////////////////
/// \class GlGenerator                                             glgenerator.h
/// \brief Generates the vertex arrays for the GlScope classes.
class GlGenerator : public QObject {
	Q_OBJECT
	
    //friend class GlScope;
	
	public:
        GlGenerator(DsoSettings *settings, QObject *parent = 0, bool emulate = 1);
		~GlGenerator();
		
		void setDataAnalyzer(DataAnalyzer *dataAnalyzer);
        int digitalPhosphorDepth =0;
        QList<QList<GlArray *> > vaChannel[Dso::CHANNELMODE_COUNT];
        GlArray vaGrid[3];
        GlArray vaEmulated[2];
        QList<float> vaEmu;
        QList<float> vaCha[2];
        bool emulate;
        void generateGrid();

	protected:

	
	private:
		DataAnalyzer *dataAnalyzer;
		DsoSettings *settings;

        //QList<QList<GlArray *> > vaChannel[Dso::CHANNELMODE_COUNT];
        //GlArray vaGrid[3];
		
        //int digitalPhosphorDepth;
	
	public slots:
		void generateGraphs();
        void emulateGraphs();
	
	signals:
		void graphsGenerated(); ///< The graphs are ready to be drawn
};


#endif
