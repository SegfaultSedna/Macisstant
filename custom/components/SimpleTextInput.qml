import QtQuick
import QtQuick.Controls

FocusScope {
    id: simpleTextInput
    implicitWidth: 200
    implicitHeight: 100
    property string bgColor: "#1abc9c"
    property string borderColor: "#ecf0f1"
    property string fontColor: "#ecf0f1"
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
            z: 1
            font.family: "Segoe UI"
            font.pixelSize: simpleTextInput.fontSize
            font.bold: true
            //anchors.centerIn: parent
            anchors {verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 12}
            selectByMouse: true
            activeFocusOnTab: true
            maximumLength: simpleTextInput.maximumLength

            onTextEdited: {
                simpleTextInput.textInputValue = textInput.text;
                //console.log(root.textInputValue)
            }
        }

        Component.onCompleted: textInput.forceActiveFocus();

        Label {
            id: placeholder
            anchors.centerIn: parent
            text: qsTr("Macro name")
            color: "white"
            visible: textInput.text.length === 0
        }

        MouseArea {
            id: rectMouseArea
            anchors.fill: parent
            hoverEnabled: true  // Enable hover detection
            onClicked: {
                textInput.forceActiveFocus()
            }
            onEntered: {
            }
            onExited: {

            }
        }
    }
}
