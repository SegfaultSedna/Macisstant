import QtQuick
import QtQuick.Controls

FocusScope {
    id: numberTextInput
    implicitWidth: 80
    implicitHeight: 32
    property alias bgColor: rect.color
    property alias borderColor: rect.border.color
    property alias inputColor: textInput.color
    property alias labelColor: placeholder.color
    property string textInputValue: textInput.text ? textInput.text : "0"
    property bool hasHoverText: true

    function clearInputField() {
        textInput.text = "";
    }
    Rectangle {
        id: rect
        width: parent.width
        height: parent.height
        border.color: numberTextInput.borderColor
        border.width: 2
        radius: 4
        opacity: textInput.activeFocus ? 0.95 : (textInput.text.length === 0) ? 0.65 : 0.95
        color: numberTextInput.bgColor
        z: -1

        Timer {
            id: revertTimer
            interval: 1200
            repeat: false
            onTriggered: {
                rect.state = ""
            }
        }

        states: [
            State {
                name: "hovered"
                when: rect.state === "hovered"
                PropertyChanges {
                    rect.y: 6
                    hoverText.opacity: 1.0
                }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "hovered"
                PropertyAnimation {
                    properties: "y"
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    target: hoverText
                    property: "opacity"
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            },
            Transition {
                from: "hovered"
                to: ""
                PropertyAnimation {
                    properties: "y"
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    target: hoverText
                    property: "opacity"
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        ]

        TextInput {
            id: textInput
            color: "black"
            z: 1
            validator: IntValidator {
                bottom: 0
                top: 999999  // Accept numbers from 0 to 999999
            }
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
            anchors.centerIn: parent
            activeFocusOnTab: true
            onTextChanged: {
                numberTextInput.textInputValue = textInput.text;
            }
        }

        Label {
            id: placeholder
            anchors.centerIn: parent
            text: qsTr("wait (ms)")
            color: "white"
            visible: textInput.text.length === 0
        }

        Label {
            id: hoverText
            text: "delay"
            color: "white"
            font.family: "Segoe UI"
            font.pixelSize: 14
            font.bold: true
            opacity: 0
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: rect.top
            anchors.bottomMargin: 1
        }

        MouseArea {
            id: rectMouseArea
            anchors.fill: parent
            hoverEnabled: true  // Enable hover detection
            onClicked: {
                textInput.forceActiveFocus()
            }
            onEntered: {
                if(numberTextInput.hasHoverText) {
                    rect.state = "hovered"
                    revertTimer.start()
                }
            }
            onExited: {
                if(numberTextInput.hasHoverText) {
                    rect.state = ""
                    revertTimer.stop()
                }
            }
        }
    }
}
