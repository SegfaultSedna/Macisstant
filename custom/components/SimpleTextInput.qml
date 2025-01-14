import QtQuick 2.15
import QtQuick.Controls 2.15

FocusScope {
    id: simpleTextInput
    implicitWidth: 200
    implicitHeight: 100
    property string bgColor: "transparent"
    property string borderColor: "#e1ddf4"
    property string fontColor: "#e1ddf4"
    property string placeholderText: "Macro name"
    property string errorText: "Input error"
    property bool onlyUpperCaseText: false
    property int fontSize: 12
    property int borderRadius: 4
    property int borderWidth: 2
    property bool hasHoverText: true
    property int maximumLength: 12
    property string textInputValue: ""

    function clearInputField() {
        textInput.text = "";
        textInputValue = "";
    }

    Rectangle {
        id: rect
        width: parent.width
        height: parent.height
        border.color: simpleTextInput.borderColor
        border.width: 2
        radius: 4
        opacity: textInput.activeFocus ? 0.95 : (textInput.text.length === 0) ? 0.65 : 0.95
        color: simpleTextInput.bgColor
        z: -1

        TextInput {
            id: textInput
            color: simpleTextInput.fontColor
            text: simpleTextInput.textInputValue
            z: 1
            font.family: "Segoe UI"
            font.pixelSize: simpleTextInput.fontSize
            font.bold: true
            anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 12}
            selectByMouse: true
            activeFocusOnTab: true
            maximumLength: simpleTextInput.maximumLength

            onTextEdited: {
                if(simpleTextInput.onlyUpperCaseText) {
                    simpleTextInput.textInputValue = textInput.text.toUpperCase();
                } else {
                    simpleTextInput.textInputValue = textInput.text;
                }
            }
        }

        Component.onCompleted: textInput.forceActiveFocus();

        Label {
            id: placeholder
            anchors.centerIn: parent
            text: simpleTextInput.placeholderText
            color: "#e1ddf4"
            visible: textInput.text.length === 0
        }

        MouseArea {
            id: rectMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                textInput.forceActiveFocus()
            }
        }

        SequentialAnimation {
            id: errorAnimation
            running: false
            loops: 2
            ColorAnimation {
                target: rect
                property: "color"
                from: simpleTextInput.bgColor
                to: "#B71C1C"
                duration: 500
            }

            ColorAnimation {
                target: rect
                property: "color"
                from: "#B71C1C"
                to: simpleTextInput.bgColor
                duration: 500
            }
        }

        Label {
            id: errorText
            text: simpleTextInput.errorText
            color: "#F44336"
            opacity: 0
            font { family: "Segoe UI"; pixelSize: 14; bold: true }
            anchors { horizontalCenter: rect.horizontalCenter; top: rect.bottom }
        }

        ParallelAnimation {
            id: errorTextAnimation
            running: false
            PropertyAnimation {
                target: errorText
                property: "anchors.topMargin"
                from: -4
                to: 0
                duration: 1000
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                target: errorText
                property: "opacity"
                from: 0
                to: 1
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }


        // Timer to trigger fade-out animation after 4 seconds
        Timer {
            id: fadeOutTimer
            interval: 4000
            repeat: false
            onTriggered: {
                errorFadeOutAnimation.start();
            }
        }

        // Fade-out animation for error text
        PropertyAnimation {
            id: errorFadeOutAnimation
            target: errorText
            property: "opacity"
            from: 1
            to: 0
            duration: 2000
            easing.type: Easing.InOutQuad
        }
    }


    function triggerErrorAnimation() {
        errorAnimation.start();
        errorTextAnimation.start();
        fadeOutTimer.start();
    }
}
