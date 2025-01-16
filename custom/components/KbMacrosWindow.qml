pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Dialogs

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
        shadowBlur: 0.4
        shadowEnabled: true
        shadowColor: "#322854"
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

            FileDialog {
                    id: fileDialog
                    title: "Export keyboard macros"
                    acceptLabel: "Save"
                    rejectLabel: "Cancel"
                    defaultSuffix: "json"
                    fileMode: FileDialog.SaveFile
                    nameFilters: ["JSON Files (*.json)", "All Files (*)"]

                    onAccepted: {
                        if (selectedFile !== "") {
                            saveToFile(selectedFile.toString())
                        }
                    }
                    onRejected: {
                        console.log("File save canceled")
                    }

                    function saveToFile(fileUrl) {
                        var data = []
                        for (var i = 0; i < macroItemModel.count; i++) {
                            data.push(macroItemModel.get(i))
                        }
                        var jsonString = JSON.stringify(data)
                        let filePathFixed = fileUrl.slice(8);
                        fileOperator.saveToFile(filePathFixed, jsonString)
                    }
                }

            onClicked: {
                fileDialog.open();
            }
        }

        IconButton {
            id: importButton
            iconSource: "../images/import-line.svg"
            iconSize: kbMacrosWindow.iconBaseSize
            anchors { left: saveButton.right; leftMargin: 4; verticalCenter: parent.verticalCenter }

            FileDialog {
                id: importFileDialog
                title: "Import keyboard macros"
                acceptLabel: "Open"
                rejectLabel: "Cancel"
                fileMode: FileDialog.OpenFile
                nameFilters: ["JSON Files (*.json)", "All Files (*)"]

                onAccepted: {
                    if (selectedFile !== "") {
                        importFromFile(selectedFile.toString())
                    } else {
                        console.log("No file selected")
                    }
                }
                onRejected: {
                    console.log("File import canceled")
                }

                function importFromFile(fileUrl) {
                    let filePathFixed = fileUrl.slice(8);
                    var importedData = fileOperator.importFromFile(filePathFixed)
                    if (importedData) {
                        for (var i = 0; i < importedData.length; i++) {
                            macroItemModel.append(importedData[i])
                        }
                        console.log("File imported successfully from " + filePathFixed)
                    } else {
                        console.log("Failed to import file from " + filePathFixed)
                    }
                }
            }

            onClicked: {
                importFileDialog.open()
            }
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
        border.color: "#e1ddf4"
        border.width: 2
        radius: 4

        ScrollView {
            id: newMacroScroll
            anchors.fill: parent  // Make the ScrollView fill the window
            clip: true
            contentWidth: keyInputColumn.width
            contentHeight: keyInputColumn.height + 200
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

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
                        inputColor: "#322854"
                        labelColor: "#322854"
                        bgColor: "#cac5ed"
                        borderColor: "#e1ddf4"
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
                color: "#e1ddf4"
                font { family: "Segoe UI"; pixelSize: 22; bold: true }
                opacity: 1
            }

            SimpleTextInput {
                id: macroNameInput
                anchors { top: createMacroText.bottom; left: createMacroText.left; right: createButton.right; topMargin: 8 }
                errorText: "Field cannot be empty"
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
                bgColor: "#514283"
                inputColor: "#e1ddf4"
                borderColor: "#e1ddf4"
                labelColor: "#e1ddf4"
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
                iconSource: "../images/check.svg"
                anchors.left: createMacroText.right
                anchors.top: createMacroText.top
                anchors.leftMargin: 6
                anchors.topMargin: 4
                iconSize: 26
                onClicked: {
                    var combined = "";
                    var hasEmptyInput = false;

                    // Iterate over each item in the macroRepeater
                    for (var i = 0; i < macroRepeater.count; i++) {
                        var item = macroRepeater.itemAt(i);
                        if (item) {

                            if (item.useKeyInputValue.length !== 0) {
                                combined += item.inputCode;
                            }

                            // Check if the KeyTextInput inside the KeyMacroInputField is empty
                            else if (item.useKeyInputValue.length === 0) {
                                item.useKeyErrorAnimation();
                            }
                        }
                    }

                    inputCodeCombined = combined.trim();
                    console.log(inputCodeCombined);

                    if (macroNameInput.textInputValue.length === 0) {
                        macroNameInput.triggerErrorAnimation();
                        hasEmptyInput = true;
                    }
                    if (mainKeyInput.textInputValue.length === 0) {
                        mainKeyInput.triggerErrorAnimation();
                        hasEmptyInput = true;
                    }

                    if (!hasEmptyInput) {
                        macroItemModel.append({macroName: macroNameInput.textInputValue, macroCode: mainKeyInput.textInputValue + "->" + inputCodeCombined});

                        while (macroModel.count > 1) {
                            macroModel.remove(1);  // Always remove the second item until only the first remains
                        }
                        var firstItem = macroRepeater.itemAt(0);
                        if (firstItem) {
                            firstItem.clearKeyInputText();
                        }

                        mainKeyInput.clearInputField();
                        macroNameInput.clearInputField();
                    }
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
        border.color: "#e1ddf4"
        border.width: 2
        radius: 4

        signal deleteRequested(int index)
        property bool isEmpty: macroItemModel.count


        PopupWindow {
            id: helpPopup
            popupText: "Remember the macro code syntax is:\non key->use key(delay)\n\n\nmodifiers+key->modifiers+key(delay)\nExample: F2->ALT+W(0)R(40)\n"
            popupTextSize: 18
            popupTextColor: "#f0c3e2"
            borderWidth: 0
            borderRadius: 0
            imageURL: "../images/smart.svg"
            hasButtons: true
            imageOffset: 42 // from top of the window
            textOffset: 6 // from the image
            buttonOffset: 72 // from the bottom of the window
            onOkButtonClicked: helpPopup.visible = false
            onCancelButtonClicked: helpPopup.visible = false
        }

        Image {
            id: image
            source: macroListContainer.isEmpty ? "../images/happy.svg" : "../images/sad.svg"
            sourceSize.width: 60
            sourceSize.height: 55
            anchors { right: parent.right; top: parent.top; topMargin: 8; rightMargin: 10 }
            z: 20

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                z: 2000
                onClicked: {
                    console.log(clicked)
                    helpPopup.visible = true;
                }
            }

        }


        onDeleteRequested: function(index) {
            console.log("Delete requested for index:", index);
            var item = macroItemModel.get(index);

            if (item) {
                // Access the MacroItem component via the Repeater's itemAt method
                var macroItem = macrosRepeater.itemAt(index);

                if (macroItem) {
                    console.log("MacroItem found:", macroItem);
                    if (macroItem.checked) {
                            macroItemModelCopy.remove(index);
                    }

                    console.log("Removing MacroItem from macroItemModel");
                    macroItemModel.remove(index);
                }
            }
        }

        PopupWindow {
            id: deletePopup
            popupText: "Are you sure?"
            popupTextSize: 32
            popupTextColor: "#f0c3e2"
            imageURL: "../images/scared.svg"
            borderWidth: 0
            borderRadius: 0
            imageOffset: 42 // from top of the window
            textOffset: 6 // from the image
            buttonOffset: 72 // from the bottom of the window

            property int indexToDelete

            onOkButtonClicked: {
                visible = false
                macroListContainer.deleteRequested(deletePopup.indexToDelete)
            }
            onCancelButtonClicked: visible = false
        }

        PopupWindow {
            id: editPopup
            popupText: "Edit macro"
            popupTextSize: 32
            popupTextColor: "#f0c3e2"
            imageURL: "../images/confused.svg"
            borderWidth: 0
            borderRadius: 0
            imageOffset: 22
            textOffset: 6
            buttonOffset: 46

            property int indexToEdit
            signal editRequested(int index, string name, string code)

            Label {
                id: nameText
                text: "Macro name"
                color: "#e1ddf4"
                font { family: "Segoe UI"; pixelSize: 15; bold: true }
                opacity: 1
                anchors { left: editNameInput.left; bottom: editNameInput.top; bottomMargin: 4 }
            }

            Label {
                id: codeText
                text: "Macro code"
                color: "#e1ddf4"
                font { family: "Segoe UI"; pixelSize: 15; bold: true }
                opacity: 1
                anchors { left: editMacroCodeInput.left; bottom: editMacroCodeInput.top; bottomMargin: 4 }
            }

            SimpleTextInput {
                id: editNameInput
                placeholderText: ""
                fontSize: 15
                hasHoverText: false
                anchors { top: parent.top; topMargin: 260; horizontalCenter: parent.horizontalCenter }
                width: 270
                height: 36
            }

            SimpleTextInput {
                id: editMacroCodeInput
                placeholderText: ""
                onlyUpperCaseText: true
                errorText: "Invalid macro code syntax"
                fontSize: 15
                maximumLength: 1024
                hasHoverText: false
                anchors { top: editNameInput.bottom; topMargin: 42; horizontalCenter: parent.horizontalCenter }
                width: 270
                height: 36
            }

            onCancelButtonClicked: {
                editPopup.visible = false
            }

            onOkButtonClicked: {
                editRequested(indexToEdit, editNameInput.textInputValue, editMacroCodeInput.textInputValue);
                console.log(editNameInput.textInputValue + ", " + editMacroCodeInput.textInputValue)
            }

            onEditRequested:(index, name, code) => {
                //let test = macroItemModel.get(index).macroCode.toString()
                const regex = new RegExp('^((CTRL\\+|SHIFT\\+|ALT\\+){0,3}[A-Z0-9]+)->((CTRL\\+|SHIFT\\+|ALT\\+){0,3}[A-Z0-9]+\\(\\d+\\))+$');
                if(regex.test(editMacroCodeInput.textInputValue)) {
                    macroItemModel.get(index).macroName = name;
                    editPopup.visible = false;
                    macroItemModel.get(index).macroCode = code;
                    console.log("regex matched");

                    if(macrosRepeater.itemAt(index).checked) {
                        macroItemModelCopy.remove(index);
                        macroItemModelCopy.append({macroName: name, macroCode: code});
                    }
                }
                else {
                    editMacroCodeInput.triggerErrorAnimation();
                }

            }

        }


        ScrollView {
            id: listMacroScroll
            anchors.fill: parent  // Make the ScrollView fill the window
            clip: true
            contentWidth: macroItemColumn.width
            contentHeight: macroItemColumn.height + 120
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Label {
                id: yourMacrosText
                text: "Your Macros"
                color: "#e1ddf4"
                font { family: "Segoe UI"; pixelSize: 22; bold: true }
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

                        onEditButtonClicked: {
                            editPopup.visible = true;
                            editNameInput.textInputValue = macroName;
                            editMacroCodeInput.textInputValue = macroCode;
                            editPopup.indexToEdit = index;
                        }

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
            to: 1
            duration: 800
            easing.type: Easing.InOutCirc
        }
        PropertyAnimation {
            target: macroWindowItemsShadow
            property: "opacity"
            from: 0
            to: 0.4
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
