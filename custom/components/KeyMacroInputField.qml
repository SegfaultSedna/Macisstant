import QtQuick
import "qrc:/"

Item {
    id: keyMacroInputField
    property bool hasHoverText: true
    property string inputCode: useKeyInput.textInputValue + "(" + delayInput.textInputValue + ")"
    property string bgColor
    property string inputColor
    property string labelColor
    property string borderColor

    // Expose textInputValue from useKeyInput
    property alias useKeyInputValue: useKeyInput.textInputValue

    // Expose textInputValue from delayInput
    property alias delayInputValue: delayInput.textInputValue

    // Expose triggerErrorAnimation from useKeyInput
    function useKeyErrorAnimation() {
        useKeyInput.triggerErrorAnimation();
    }

    KeyTextInput {
        id: useKeyInput
        width: 80
        height: 27
        hoverCustomText: "Use key"
        mainText: "add key"
        bgColor: keyMacroInputField.bgColor
        hasHoverText: keyMacroInputField.hasHoverText
        inputColor: keyMacroInputField.inputColor
        labelColor: keyMacroInputField.labelColor
        borderColor: keyMacroInputField.borderColor
    }

    NumberTextInput {
        id: delayInput
        width: 50
        height: 27
        anchors.top: useKeyInput.top
        anchors.left: useKeyInput.right
        anchors.leftMargin: 4
        bgColor: keyMacroInputField.bgColor
        hasHoverText: keyMacroInputField.hasHoverText
        inputColor: keyMacroInputField.inputColor
        labelColor: keyMacroInputField.labelColor
        borderColor: keyMacroInputField.borderColor
    }

    function clearKeyInputText() {
        useKeyInput.clearInputField()
        delayInput.clearInputField()
    }
}
