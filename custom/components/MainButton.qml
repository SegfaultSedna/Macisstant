import QtQuick

// Main button
Rectangle {
    id: mainButton
    property string buttonText: ""
    property string buttonColor: ""
    property bool isFadeInUpFinished: hasAppearAnimation ? false : true  // Flag to check if fadeInUp is finished
    property real buttonFontSize: 14
    property int buttonWidth: 140
    property int buttonHeight: 40
    property bool hasAppearAnimation: true
    //anchors.centerIn: parent
    width: buttonWidth
    height: buttonHeight
    border.color: buttonColor
    border.width: 2
    color: "transparent"
    radius: 1
    opacity: hasAppearAnimation ? 0 : 1

    signal clicked

    Rectangle {
        id: buttonFill
        anchors.fill: parent
        color: "white"
        opacity: 0
        z: 1

        PropertyAnimation {
            id: buttonFillIn
            target: buttonFill
            property: "opacity"
            from: 0
            to: 0.7
            duration: 200
            easing.type: Easing.InOutCirc
        }

        PropertyAnimation {
            id: buttonFillIOut
            target: buttonFill
            property: "opacity"
            from: 0.7
            to: 0
            duration: 200
            easing.type: Easing.InOutCirc
        }
    }

    Text {
        id: mainButtonText
        anchors.centerIn: parent  // Center the text inside the Rectangle
        text: mainButton.buttonText
        font.family: "Segoe UI"
        font.pixelSize: mainButton.buttonFontSize
        font.bold: true
        color: mainButton.buttonColor
        z: 2  // Ensure the text is above the buttonFill rectangle
    }

    ParallelAnimation {
        id: buttonFadeInUp
        running: false
        PropertyAnimation {
            target: mainButton
            property: "anchors.verticalCenterOffset"
            from: 100
            to: 20
            duration: 800
            easing.type: Easing.InOutCirc
        }
        PropertyAnimation {
            target: mainButton
            property: "opacity"
            from: 0
            to: 1
            duration: 800
            easing.type: Easing.InOutCirc
        }
        onFinished: {
            mainButton.isFadeInUpFinished = true  // Set the flag to true when animation finishes
        }
    }

    function buttonFadeInUp() {
        buttonFadeInUp.start();
    }


    MouseArea {
        id: mainButtonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        visible: true  // Make sure it's visible to detect mouse events

        onClicked: {
            mainButton.clicked()
        }

        onEntered: {
            if (mainButton.isFadeInUpFinished) {
                buttonFillIn.start()
                mainButtonTextColorChange.start()
            }
        }

        onExited: {
            buttonFillIOut.start()
            mainButtonTextColorChangeReverse.start()
        }
    }

    ColorAnimation {
        id: mainButtonTextColorChange
        target: mainButtonText
        property: "color"
        from: mainButton.buttonColor
        to: "#1e272e"
        duration: 200
    }

    ColorAnimation {
        id: mainButtonTextColorChangeReverse
        target: mainButtonText
        property: "color"
        from: "#1e272e"
        to: mainButton.buttonColor
        duration: 200
    }
}
