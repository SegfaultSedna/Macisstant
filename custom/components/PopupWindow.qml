import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: popupWindow
    Material.theme: Material.Dark
    property string popupText: "Popup text"
    property string popupTextColor: "#ecf0f1"
    property int popupTextSize: 22
    property int borderWidth: 2
    property int borderRadius: 4
    property bool hasButtons: true
    z: 100
    signal okButtonClicked
    signal cancelButtonClicked

    anchors.fill: parent
    visible: false

    Rectangle {
        anchors.fill: parent
        border.width: popupWindow.borderWidth
        radius: popupWindow.borderRadius
        color: "black"
        opacity: 0.95
    }

    Image {
        id: image
        property string iconSource
        source: "../images/confused.svg" // Use the dynamic image path
        sourceSize.width: 160
        sourceSize.height: 140
        anchors { centerIn: parent; verticalCenterOffset: -115 }
    }

    Label {
        id: popupWindowText
        text: popupWindow.popupText
        color: popupWindow.popupTextColor
        font.family: "Segoe UI"
        font.pixelSize: popupWindow.popupTextSize
        font.bold: true
        opacity: 1
        anchors { centerIn: parent; verticalCenterOffset: 0 }
    }

    Row {
        spacing: 16
        anchors { centerIn: parent; verticalCenterOffset: 90 }
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
