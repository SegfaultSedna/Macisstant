pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects

Item {
    id: kbMacrosWindow
    objectName: "kbMacrosWindow"
    width: parent.width
    height: parent.height
    visible: true
    Material.theme: Material.Dark

    property int iconBaseSize: 32
    property string inputCodeCombined

    signal homeButtonClicked

    MultiEffect {
        id: macroWindowItemsShadow
        source: kbMacrosWindow
        anchors.fill: kbMacrosWindow
        shadowBlur: 1.1
        shadowEnabled: true
        shadowColor: "black"
        shadowVerticalOffset: 4
        shadowHorizontalOffset: 4
        opacity: 0
    }

    // Container for the icons
    Rectangle {
        id: topButtonContainer
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 20 }
        color: "transparent"
        width: parent.width - 20
        height: kbMacrosWindow.iconBaseSize + 18
        border.color: "#dfe6e9"
        border.width: 2
        radius: 4

        // Action item row
        IconButton {
            id: homeButton
            iconSource: "../images/home.svg"
            iconSize: kbMacrosWindow.iconBaseSize
            anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }

            onClicked: {
                kbMacrosWindow.homeButtonClicked();
            }
        }

        IconButton {
            id: saveButton
            iconSource: "../images/save.svg"
            iconSize: kbMacrosWindow.iconBaseSize
            anchors { left: homeButton.right; leftMargin: 4; verticalCenter: parent.verticalCenter }
        }


        IconButton {
            id: settingsButton
            iconSource: "../images/settings.svg"
            iconSize: kbMacrosWindow.iconBaseSize
            anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
        }
    }

    // Area where you can make a new macro
    Rectangle {
        id: macroMakeContainer
        anchors { bottom: parent.bottom; left: topButtonContainer.left; bottomMargin: 20 }
        color: "transparent"
        width: parent.width / 2.5
        height: parent.height - 60 - topButtonContainer.height
        border.color: "#ecf0f1"
        border.width: 2
        radius: 4

        ScrollView {
            id: newMacroScroll
            anchors.fill: parent  // Make the ScrollView fill the window
            clip: true
            contentWidth: keyInputColumn.width
            contentHeight: keyInputColumn.height + 120
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            ListModel {
                id: macroModel
                ListElement { name: "Macro 1" }
            }

            Column {
                id: keyInputColumn
                anchors { top: mainKeyInput.top; left: mainKeyInput.right; leftMargin: 8 }
                width: 130
                spacing: 4

                Repeater {
                    id: macroRepeater
                    model: macroModel // Where does it get its elements

                    delegate: KeyMacroInputField { // What does it spit out
                        required property int index
                        width: 130
                        height: 27
                        hasHoverText: index === 0 // Use index directly here
                    }

                    onItemAdded: (index, item) => {
                        //root.inputCodeCombined += itemAt(index-1).inputCode + ";";
                        //console.log(inputCodeCombined);
                    }
                }
            }


            Label {
                id: createMacroText
                anchors { top: parent.top; left: parent.left; topMargin: 12; leftMargin: 12 }
                text: "Create Macro"
                color: "#ecf0f1"
                font.family: "Segoe UI"
                font.pixelSize: 22
                font.bold: true
                opacity: 1
            }

            SimpleTextInput {
                id: macroNameInput
                anchors { top: createMacroText.bottom; left: createMacroText.left; right: createButton.right; topMargin: 8 }
                bgColor: "transparent"
                fontSize: 14
                width: 160
                height: 32
            }

            KeyTextInput {
                id: mainKeyInput
                width: 80
                height: 27
                anchors { left: createMacroText.left; top: macroNameInput.bottom; topMargin: 22 }
                bgColor: "#8e44ad"
                borderColor: "#ecf0f1"
                hoverCustomText: "On key"
                mainText: "add key"
            }

            IconButton {
                id: addKeyButton
                iconSource: "../images/arrow-down.svg"
                anchors { top: keyInputColumn.bottom; left: keyInputColumn.left; topMargin: 2; leftMargin: 67 }
                //anchors.horizontalCenter: keyAndDelayColumn.horizontalCenter
                iconSize: 28

                onClicked: {
                    // Add new item to the model
                    macroModel.append({})
                }

                SequentialAnimation {
                    loops: Animation.Infinite
                    running: true
                    NumberAnimation {
                        target: addKeyButton
                        property: "anchors.topMargin"
                        to: 10
                        duration: 1000 // Adjust duration as needed
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: addKeyButton
                        property: "anchors.topMargin"
                        to: 2
                        duration: 1000 // Adjust duration as needed
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            IconButton {
                id: createButton
                iconSource: "../images/square-check-big.svg"
                anchors.left: createMacroText.right
                anchors.top: createMacroText.top
                anchors.leftMargin: 6
                anchors.topMargin: 6
                iconSize: 22
                onClicked: {

                    var combined = ""
                    for (var i = 0; i < macroRepeater.count; i++) {
                        var item = macroRepeater.itemAt(i)
                        if (item) {
                            combined += item.inputCode;
                        }
                    }
                    inputCodeCombined = combined.trim()
                    console.log(inputCodeCombined);


                    macroItemModel.append({macroName: macroNameInput.textInputValue, macroCode: mainKeyInput.textInputValue + "->" + inputCodeCombined})

                    while (macroModel.count > 1) {
                        macroModel.remove(1)  // Always remove the second item until only the first remains
                    }
                    var firstItem = macroRepeater.itemAt(0)
                    if (firstItem) {
                        firstItem.clearKeyInputText()
                    }

                    mainKeyInput.clearInputField()
                    macroNameInput.clearInputField()
                }
            }
        }
    }

    // Area where your current macros show up
    Rectangle {
        id: macroListContainer
        anchors { bottom: parent.bottom; right: topButtonContainer.right; bottomMargin: 20 }
        color: "transparent"
        width: parent.width / 1.82
        height: parent.height - 60 - topButtonContainer.height
        border.color: "#ecf0f1"
        border.width: 2
        radius: 4

        signal deleteRequested(int index)


        onDeleteRequested:(index) => {
            if(macroItemModel.get(index).state === Qt.Checked) {
                macroItemModelCopy.remove(index, 1);
            }

            macroItemModel.remove(index, 1);
        }

        PopupWindow {
            id: deletePopup
            popupText: "Are you sure?"
            popupTextSize: 32
            popupTextColor: "#e74c3c"
            property int indexToDelete
            z: 2

            onOkButtonClicked: {
                visible = false
                macroListContainer.deleteRequested(deletePopup.indexToDelete)
            }
            onCancelButtonClicked: visible = false
        }

        ScrollView {
            id: listMacroScroll
            anchors.fill: parent  // Make the ScrollView fill the window
            clip: true
            contentWidth: macroItemColumn.width
            contentHeight: macroItemColumn.height + 120
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            Label {
                id: yourMacrosText
                text: "Your Macros"
                color: "#ecf0f1"
                font.family: "Segoe UI"
                font.pixelSize: 22
                font.bold: true
                opacity: 1
                anchors { left: parent.left; top: parent.top; leftMargin: 12; topMargin: 12 }
            }



            ListModel {
                id: macroItemModel
                objectName: "macroItemModel"
                //ListElement { macroName: "Unnamed Macro" }
            }


            ListModel {
                id: macroItemModelCopy
                objectName: "macroItemModelCopy"
                //ListElement { macroName: "Unnamed Macro" }
            }



            Column {
                id: macroItemColumn
                //anchors { left: yourMacrosText.left; top: yourMacrosText.bottom; leftMargin: 12; topMargin: 12 }
                width: 130
                spacing: 4
                anchors.top: parent.top
                anchors.topMargin: 54

                Repeater {
                    id: macrosRepeater
                    model: macroItemModel

                    delegate: MacroItem {
                        required property int index
                        required macroName
                        required macroCode

                        containerWidth: macroListContainer.width/1.5
                        // macroItemModel.get(index).macroCode


                        onDeleteButtonClicked: {
                            deletePopup.indexToDelete = index
                            deletePopup.visible = true
                        }

                        onCheckStateChanged: (state) => {
                            if(state === Qt.Checked) {
                                macroItemModelCopy.append({macroName: macroName, macroCode: macroCode});
                                console.log(macroItemModelCopy);
                            }
                            if(state === Qt.Unchecked) {
                                macroItemModelCopy.remove(index, 1);
                                console.log(macroItemModelCopy);
                            }
                        }
                    }
                }
            }

        }
    }

    function macroWindowFadeIn() {
        macroWindowFadeIn.start();
    }

    function macroWindowFadeOut() {
        kbMacrosWindow.visible = false;
        macroWindowFadeOut.start();
    }

    ParallelAnimation {
        id: macroWindowFadeIn
        running: false

        PropertyAnimation {
            target: kbMacrosWindow
            property: "opacity"
            from: 0
            to: 0.8
            duration: 800
            easing.type: Easing.InOutCirc
        }
        PropertyAnimation {
            target: macroWindowItemsShadow
            property: "opacity"
            from: 0
            to: 0.8
            duration: 800
            easing.type: Easing.InOutCirc
        }
    }

    ParallelAnimation {
        id: macroWindowFadeOut
        running: false
        PropertyAnimation {
            target: kbMacrosWindow
            property: "opacity"
            from: 0.8
            to: 0
            duration: 500
            easing.type: Easing.InOutCirc
        }
        PropertyAnimation {
            target: macroWindowItemsShadow
            property: "opacity"
            from: 0.8
            to: 0
            duration: 500
            easing.type: Easing.InOutCirc
        }
        onFinished: kbMacrosWindow.visible = false
    }
}
