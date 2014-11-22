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

    Column {
        spacing: 40
        height: parent.height
        anchors.centerIn: parent

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
            onCurrentIndexChanged: oscill.timebaseSelected(sliderbase.value, currentIndex+3*(base.currentIndex-3)-3)
            onActivated: oscill.timebaseSelected(sliderbase.value, currentIndex+3*(base.currentIndex-3)-3)
        }

        ComboBox {
            id : base
            currentIndex: 2
            model: ["ns", "us", "ms", "s"]
            onCurrentIndexChanged: oscill.timebaseSelected(sliderbase.value, tens.currentIndex+3*(currentIndex-3)-3)
            onActivated: oscill.timebaseSelected(sliderbase.value, tens.currentIndex+3*(currentIndex-3)-3)
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
                oscill.timebaseSelected(value, tens.currentIndex+3*(base.currentIndex-3)-3)
            }
        }
        Row {

        Switch {
                id: switch1
                style: switchStyle
                checked: true
                onCheckedChanged: oscill.updateUsed(0, checked)

        }

        Label   {
            text: "CH1 Offset: "
            color: "yellow"
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
            model: ["10 mV/div","20 mV/div","50 mV/div","100 mV/div","200 mV/div","500 mV/div", "1V/div", "2V/div", "5V/div"]
            onCurrentIndexChanged: oscill.voltageGainSelected(0, currentIndex)
            onActivated: oscill.voltageGainSelected(0, currentIndex)
        }


    }
        Row {
        Switch {
                    id: switch2
                    style: switchStyle
                    checked: false
                    onCheckedChanged: oscill.updateUsed(1, checked)

        }
        Label   {
            text: "CH2 Offset: "
            color: "cyan"
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
            model: ["10 mV/div","20 mV/div","50 mV/div","100 mV/div","200 mV/div","500 mV/div", "1V/div", "2V/div", "5V/div"]
            onCurrentIndexChanged: oscill.voltageGainSelected(1, currentIndex)
            onActivated: oscill.voltageGainSelected(1, currentIndex)
        }


    }
        Label   {
            text: "Emulate 10V ~15kHz Sin"
            color: "white"
        }
        Switch {
            style: switchStyle
            checked: false
            onCheckedChanged: {
                oscill.emulateSelected(checked)
                switch1.checked = checked
                switch2.checked = checked
                ch1a.visible = !checked
                ch2a.visible = !checked
            }


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
