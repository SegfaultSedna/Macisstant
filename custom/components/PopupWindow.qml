import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Item {
    id: popupWindow
    Material.theme: Material.Dark
    property string popupText: "Popup text"
    property string popupTextColor: "#e1ddf4"
    property int popupTextSize: 22
    property int borderWidth: 2
    property int borderRadius: 4
    property int imageOffset //115
    property int textOffset // 0
    property int buttonOffset // 90
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
        anchors { top: parent.top; topMargin: popupWindow.imageOffset; horizontalCenter: parent.horizontalCenter }
    }

    Label {
        id: popupWindowText
        text: popupWindow.popupText
        color: popupWindow.popupTextColor
        font { family: "Segoe UI"; pixelSize: popupWindow.popupTextSize; bold: true }
        opacity: 1
        anchors { top: image.bottom; topMargin: popupWindow.textOffset; horizontalCenter: parent.horizontalCenter }
    }

    Row {
        spacing: 16
        anchors { bottom: parent.bottom; bottomMargin: popupWindow.buttonOffset; horizontalCenter: parent.horizontalCenter }
        MainButton {
            id: okButton
            hasAppearAnimation: false
            buttonText: "OK"
            buttonColor: "#e1ddf4"
            buttonWidth: 100
            buttonHeight: 30

            onClicked: popupWindow.okButtonClicked()
        }

        MainButton {
            id: cancelButton
            hasAppearAnimation: false
            buttonText: "Cancel"
            buttonColor: "#e1ddf4"
            buttonWidth: 100
            buttonHeight: 30

            onClicked: popupWindow.cancelButtonClicked()
        }
    }
}
