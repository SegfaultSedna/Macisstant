// KeyComboTextInput.qml
import QtQuick
import QtQuick.Controls

FocusScope {
    id: keyTextInput
    implicitWidth: 80
    implicitHeight: 32
    property alias bgColor: rect.color
    property alias borderColor: rect.border.color
    property alias inputColor: textInput.color
    property alias labelColor: placeholder.color
    property string hoverCustomText: "hover text"
    property string mainText: "add key"
    property string textInputValue: ""
    property bool hasHoverText: true
    property var keyMap: ({
        [Qt.Key_Insert]: "Insert",
        [Qt.Key_Delete]: "Delete",
        [Qt.Key_Home]: "Home",
        [Qt.Key_End]: "End",
        [Qt.Key_PageUp]: "Page Up",
        [Qt.Key_PageDown]: "Page Down",
        [Qt.Key_F1]: "F1",
        [Qt.Key_F2]: "F2",
        [Qt.Key_F3]: "F3",
        [Qt.Key_F4]: "F4",
        [Qt.Key_F5]: "F5",
        [Qt.Key_F6]: "F6",
        [Qt.Key_F7]: "F7",
        [Qt.Key_F8]: "F8",
        [Qt.Key_F9]: "F9",
        [Qt.Key_F10]: "F10",
        [Qt.Key_F11]: "F11",
        [Qt.Key_F12]: "F12"
    })

    function clearInputField() {
        textInput.text = "";
    }

    function specialKeysHandler(key) {
        return keyMap[key] || null;
    }

    Timer {
        id: revertTimer
        interval: 1200  // 1.2 seconds
        repeat: false
        onTriggered: {
            keyTextInput.state = ""
        }
    }

    Rectangle {
        id: rect
        width: parent.width
        height: parent.height
        border.color: keyTextInput.borderColor
        border.width: 2
        radius: 4
        opacity: textInput.activeFocus ? 0.95 : (textInput.text.length === 0) ? 0.65 : 0.95
        color: keyTextInput.bgColor
        z: -1

        states: [
            State {
                name: "hovered"
                when: keyTextInput.state === "hovered"
                PropertyChanges {
                    rect.y: 6
                }
                PropertyChanges {
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
            color: "#322854"
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
            anchors.centerIn: parent
            activeFocusOnTab: true
            z: 1

            onTextChanged: {
                keyTextInput.textInputValue = textInput.text;
            }

            function keyToString(event) {
                var modifiers = ""
                if (event.modifiers & Qt.ControlModifier) {
                    modifiers += "Ctrl+"
                }
                if (event.modifiers & Qt.ShiftModifier) {
                    modifiers += "Shift+"
                }
                if (event.modifiers & Qt.AltModifier) {
                    modifiers += "Alt+"
                }
                if (event.modifiers & Qt.MetaModifier) {
                    modifiers += "Meta+"
                }
                return modifiers
            }

            function keyPressedHandler(event) {
                // Update the text of the input field with the key combination
                textInput.text = keyToString(event)
                event.accepted = true
            }


            property int keyCount: 0
            Keys.onPressed: (event)=> {
                if(event.key === Qt.Key_Tab) {event.accepted = false; return;}

                let modifier = keyToString(event);
                let specialString = keyTextInput.specialKeysHandler(event.key);
                if(event.key === Qt.Key_Backspace || event.key === Qt.Key_Backtab || event.key === Qt.Key_Tab || event.key === Qt.Key_Escape) { event.accepted = true; textInput.text=""; return;}

                if(modifier) {
                    textInput.text = modifier;
                    keyCount = 0;
                }
                if(keyCount > 0) {
                    if(specialString) {
                        textInput.text = specialString.toUpperCase();
                    } else {
                        textInput.text = (event.text).toUpperCase();
                    }
                }
                else if(!modifier) {
                    if(specialString) {
                        textInput.text += specialString.toUpperCase();
                    } else {
                        textInput.text += (event.text).toUpperCase();
                    }
                    keyCount++;
                }
                event.accepted = true;
            }

            Label {
                id: placeholder
                anchors.centerIn: parent
                text: keyTextInput.mainText
                color: keyTextInput.labelColor
                visible: textInput.text.length === 0
            }
        }

        Label {
            id: hoverText
            text: keyTextInput.hoverCustomText
            color: "#e1ddf4"
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
                //inputField.focus = true // Set focus when the MouseArea is clicked
                textInput.forceActiveFocus()
            }
            onEntered: {
                if(keyTextInput.hasHoverText) {
                    keyTextInput.state = "hovered"
                    revertTimer.start()
                }
            }
            onExited: {
                if(keyTextInput.hasHoverText) {
                    keyTextInput.state = ""
                    revertTimer.stop()
                }
            }
        }
    }
}
