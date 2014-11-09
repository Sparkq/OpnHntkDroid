/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/





import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.1

Rectangle {
    width: parent.width
    height: parent.height
    color: "#903a3a3a"

    property real progress: 0
    SequentialAnimation on progress {
        loops: Animation.Infinite
        running: true
        NumberAnimation {
            from: 0
            to: 1
            duration: 3000
        }
        NumberAnimation {
            from: 1
            to: 0
            duration: 3000
        }
    }

    Row {
        spacing: 40
        anchors.centerIn: parent


    Column {
        spacing: 40

//        Row {
//        Label   {
//            text: "Samplerate: "
//            color: "white"
//        }
//        Label   {
//            id: value1
//            text: "1"
//            color: "white"
//        }
//        ComboBox {
//            id : srate
//            currentIndex: 1
//            model: ["S", "kS", "MS", "GS"]
//            onActivated: oscill.samplerateSelected(slider1.value*(10^(3*(currentIndex))))
//        }
//        }
//        Slider  {
//            id: slider1
//            minimumValue: 1
//            maximumValue: 999
//            stepSize: 1
//            value: 10
//            anchors.margins: 20
//            style: sliderStyle
//            onValueChanged: {
//                value1.text = value
//                oscill.samplerateSelected(value*(10^(3*(srate.currentIndex))))
//            }
//        }


        Row {
        Label   {
            text: "Timebase: "
            color: "white"
        }
        Label   {
            id: valuebase
            text: "1"
            color: "white"
        }
        Label   {
            text: "*"
            color: "white"
        }
        ComboBox {
            id : tens
            currentIndex: 0
            model: ["1", "10", "100", "1000"]
            onCurrentIndexChanged: oscill.timebaseSelected(sliderbase.value, currentIndex+3*(base.currentIndex-3))
            onActivated: oscill.timebaseSelected(sliderbase.value, currentIndex+3*(base.currentIndex-3))
        }

        ComboBox {
            id : base
            currentIndex: 1
            model: ["ns", "us", "ms", "s"]
            onCurrentIndexChanged: oscill.timebaseSelected(sliderbase.value, tens.currentIndex+3*(currentIndex-3))
            onActivated: oscill.timebaseSelected(sliderbase.value, tens.currentIndex+3*(currentIndex-3))
        }

        }
        Slider  {
            id: sliderbase
            minimumValue: 1
            maximumValue: 10
            stepSize: 1
            value: 1
            anchors.margins: 20
            style: sliderStyle
            onValueChanged: {
                valuebase.text = value
                oscill.timebaseSelected(value, tens.currentIndex+3*(base.currentIndex-3))
            }
        }
        Row {

        Switch {
                style: switchStyle
                checked: true
                onCheckedChanged: oscill.updateUsed(0, checked)

        }

        Label   {
            text: "CH1 Offset: "
            color: "yellow"
        }
        Label   {
            id: valueoffset
            text: "1"
            color: "white"
        }

        }
        Slider  {
            id: slideroffset
            minimumValue: -1
            maximumValue: 1
            stepSize: 0.2
            value: 0
            anchors.margins: 20
            style: sliderStyle
            onValueChanged: {
                valueoffset.text = value
                oscill.offsetSelected(0, value)
            }
        }

        Row {
        Label   {
            text: "CH1 Voltage: "
            color: "yellow"
        }
        ComboBox {
            id : volt
            currentIndex: 8
            model: ["10 mV","20 mV","50 mV","100 mV","200 mV","500 mV", "1V", "2V", "5V"]
            onCurrentIndexChanged: oscill.voltageGainSelected(0, currentIndex)
            onActivated: oscill.voltageGainSelected(0, currentIndex)
        }


    }
        Row {
        Switch {
                    style: switchStyle
                    checked: false
                    onCheckedChanged: oscill.updateUsed(1, checked)

        }
        Label   {
            text: "CH2 Offset: "
            color: "cyan"
        }
        Label   {
            id: valueoffset2
            text: "1"
            color: "white"
        }

        }
        Slider  {
            id: slideroffset2
            minimumValue: -1
            maximumValue: 1
            stepSize: 0.2
            value: 0
            anchors.margins: 20
            style: sliderStyle
            onValueChanged: {
                valueoffset2.text = value
                oscill.offsetSelected(1, value)
            }
        }

        Row {
        Label   {
            text: "CH2 Voltage: "
            color: "cyan"
        }
        ComboBox {
            id : volt2
            currentIndex: 8
            model: ["10 mV","20 mV","50 mV","100 mV","200 mV","500 mV", "1V", "2V", "5V"]
            onCurrentIndexChanged: oscill.voltageGainSelected(1, currentIndex)
            onActivated: oscill.voltageGainSelected(1, currentIndex)
        }


    }
        Label   {
            text: "Emulate"
            color: "white"
        }
        Switch {
            style: switchStyle
            checked: false
            onCheckedChanged: oscill.emulateSelected(checked)

        }


//        Row {
//        Label   {
//            text: "Frequencybase: "
//            color: "white"
//        }
//        Label   {
//            id: value3
//            text: "1"
//            color: "white"
//        }
//        ComboBox {
//            id : fbase
//            model: ["Hz", "kHz", "MHz", "GHz"]
//            onActivated: oscill.samplerateSelected(slider1.value*(3*(currentIndex-1)))
//        }
//        }
//        Slider  {
//            id: slider3
//            minimumValue: 1
//            maximumValue: 999
//            stepSize: 1
//            value: 1
//            anchors.margins: 20
//            style: sliderStyle
//            onValueChanged: {
//                value3.text = value
//            }
//        }

//        Row {
//        Label   {
//            text: "Format: "
//            color: "white"
//        }
//        ComboBox    {
//            model: ["T - Y", "X - Y"]

//        }
//        }



    }
}








    Component {
        id: touchStyle

        ButtonStyle {
            panel: Item {
                implicitHeight: 50
                implicitWidth: 320
                BorderImage {
                    anchors.fill: parent
                    antialiasing: true
                    border.bottom: 8
                    border.top: 8
                    border.left: 8
                    border.right: 8
                    anchors.margins: control.pressed ? -4 : 0
                    source: control.pressed ? "../images/button_pressed.png" : "../images/button_default.png"
                    Text {
                        text: control.text
                        anchors.centerIn: parent
                        color: "white"
                        font.pixelSize: 23
                        renderType: Text.NativeRendering
                    }
                }
            }
        }
    }

    Component {
        id: sliderStyle
    SliderStyle {
        handle: Rectangle {
            width: 30
            height: 30
            radius: height
            antialiasing: true
            color: Qt.lighter("#468bb7", 1.2)
        }

        groove: Item {
            implicitHeight: 50
            implicitWidth: 400
            Rectangle {
                height: 8
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                color: "#444"
                opacity: 0.8
                Rectangle {
                    antialiasing: true
                    radius: 1
                    color: "#468bb7"
                    height: parent.height
                    width: parent.width * control.value / control.maximumValue
                }
            }
        }
}
    }
    Component {
        id: switchStyle
        SwitchStyle {

            groove: Rectangle {
                implicitHeight: 50
                implicitWidth: 152
                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    width: parent.width/2 - 2
                    height: 20
                    anchors.margins: 2
                    color: control.checked ? "#468bb7" : "#222"
                    Behavior on color {ColorAnimation {}}
                    Text {
                        font.pixelSize: 23
                        color: "white"
                        anchors.centerIn: parent
                        text: "ON"
                    }
                }
                Item {
                    width: parent.width/2
                    height: parent.height
                    anchors.right: parent.right
                    Text {
                        font.pixelSize: 23
                        color: "white"
                        anchors.centerIn: parent
                        text: "OFF"
                    }
                }
                color: "#222"
                border.color: "#444"
                border.width: 2
            }
            handle: Rectangle {
                width: parent.parent.width/2
                height: control.height
                color: "#444"
                border.color: "#555"
                border.width: 2
            }
        }
    }
}
