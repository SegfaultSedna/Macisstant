import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: popupWindow
    Material.theme: Material.Dark
    property string popupText: "Popup text"
    property string popupTextColor: "#ecf0f1"
    property int popupTextSize: 22
    property bool hasButtons: true

    signal okButtonClicked
    signal cancelButtonClicked

    anchors.fill: parent
    visible: false

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.85
        radius: 4
    }

    Label {
        id: popupWindowText
        text: popupWindow.popupText
        color: popupWindow.popupTextColor
        font.family: "Segoe UI"
        font.pixelSize: popupWindow.popupTextSize
        font.bold: true
        opacity: 1
        anchors { centerIn: parent; verticalCenterOffset: -110 }
    }

    Row {
        spacing: 16
        anchors { centerIn: parent }
        MainButton {
            id: okButton
            hasAppearAnimation: false
            buttonText: "OK"
            buttonColor: "#ecf0f1"
            buttonWidth: 100
            buttonHeight: 30

            onClicked: popupWindow.okButtonClicked()
        }

        MainButton {
            id: cancelButton
            hasAppearAnimation: false
            buttonText: "Cancel"
            buttonColor: "#ecf0f1"
            buttonWidth: 100
            buttonHeight: 30

            onClicked: popupWindow.cancelButtonClicked()
        }
    }
}
