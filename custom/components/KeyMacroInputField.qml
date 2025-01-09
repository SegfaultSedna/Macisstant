import QtQuick
import "qrc:/"

Item {
    id: keyMacroInputField
    property bool hasHoverText: true
    property string inputCode: useKeyInput.textInputValue + "(" + delayInput.textInputValue + ")"

    KeyTextInput {
        id: useKeyInput
        width: 80
        height: 27
        hoverCustomText: "Use key"
        mainText: "add key"
        bgColor: "#1abc9c"
        hasHoverText: keyMacroInputField.hasHoverText
    }


    NumberTextInput {
        id: delayInput
        width: 50
        height: 27
        anchors.top: useKeyInput.top
        anchors.left: useKeyInput.right
        anchors.leftMargin:4
        bgColor: "#1abc9c"
        hasHoverText: keyMacroInputField.hasHoverText
    }

    function clearKeyInputText() {
        useKeyInput.clearInputField()
        delayInput.clearInputField()
    }

}
