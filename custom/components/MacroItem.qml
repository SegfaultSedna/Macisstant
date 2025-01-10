import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Rectangle {
    id: macroItem
    required property real containerWidth
    required property string macroCode
    required property string macroName
    property alias state: checkBox.checkState
    //required property real containerHeight

    Material.theme: Material.Dark

    signal deleteButtonClicked()
    signal checkStateChanged(int state)

    border.color: "#ecf0f1"
    border.width: 2
    radius: 4
    width: containerWidth
    height: checkBox.height - 4
    anchors.left: parent.left
    anchors.leftMargin: 12
    color: "transparent"

    CheckBox {
        id: checkBox
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        activeFocusOnTab: false

        nextCheckState: function() {
            if (checkState === Qt.Checked) {
                macroItem.checkStateChanged(Qt.Unchecked);
                return Qt.Unchecked;
            }
            else {
                macroItem.checkStateChanged(Qt.Checked);
                return Qt.Checked;
            }
        }
    }

    Label {
        id: createMacroText
        anchors { verticalCenter: parent.verticalCenter; left: checkBox.right }
        text: macroItem.macroName
        color: "#ecf0f1"
        font.family: "Segoe UI"
        font.pixelSize: 14
        font.bold: true
        opacity: 1
    }

    IconButton {
        id: editMacroButton
        iconSize: 20
        source: "../images/edit-fill.svg"
        anchors { verticalCenter: parent.verticalCenter; right: deleteMacroButton.left; rightMargin: 6}
    }

    IconButton {
        id: deleteMacroButton
        iconSize: 26
        source: "../images/close-line.svg"
        anchors { verticalCenter: parent.verticalCenter; right: parent.right; rightMargin: 12}
        onClicked: {
            macroItem.deleteButtonClicked();
        }
    }

}
