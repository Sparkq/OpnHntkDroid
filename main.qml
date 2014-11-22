import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.1
import OpenGLUnderQML 1.0
import "qmls"

ApplicationWindow {
        visible: true
        color: "transparent"
        title: qsTr("Hello World")
        onClosing: Qt.quit();

        Item {
            width: parent.width
            height: parent.height

            TabView {
                anchors.fill: parent
                style: touchStyle
                Tab {
                    title: "Settings"
                    Setts{ visible: true }
                }
                Tab {
                    title: "Graph"
                    Rectangle {
                        width: parent.width
                        height: parent.height
                        color: "transparent"
                        visible: true
                    }
                }


            }

            Component {
                id: touchStyle
                TabViewStyle {
                    tabsAlignment: Qt.AlignVCenter
                    tabOverlap: 0
                    frame: Item { }
                    tab: Item {
                        implicitWidth: control.width/control.count
                        implicitHeight: 50
                        BorderImage {
                            anchors.fill: parent
                            border.bottom: 8
                            border.top: 8
                            source: styleData.selected ? "../images/tab_selected.png":"../images/tabs_standard.png"
                            Text {
                                anchors.centerIn: parent
                                color: "white"
                                text: styleData.title.toUpperCase()
                                font.pixelSize: 16
                            }
                            Rectangle {
                                visible: index > 0
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                anchors.margins: 10
                                width:1
                                color: "#3a3a3a"
                            }
                        }
                    }
                }
            }


        Viewer {
            id: oscill


            onSettMessage: mesg.text = message
            onSettAmpl: {
                ch1ampl.text = amplitude1
                ch2ampl.text = amplitude2
            }
        }
        Label   {
            id:mesg
            anchors.bottom: parent.bottom
            text: oscill.message
            color: "#95ffffff"



        }
        Row     {
            id:ch1a
            anchors.bottom: mesg.top
            anchors.left: parent.left
            Label{
                text: "CH1:"
                color: "yellow"
            }
            Label{
                id: ch1ampl
                text: ""
                color: "yellow"
            }
        }
        Row     {
            id: ch2a
            anchors.bottom: mesg.top
            anchors.right: parent.right
            Label{
                text: "CH2:"
                color: "cyan"
            }
            Label{
                id: ch2ampl
                text: ""
                color: "cyan"
            }
        }
}

}



