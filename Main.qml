pragma ComponentBehavior: Bound
import "custom/components/"
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
//import "global.js" as GlobalFunctions

Window {
    id: root
    width: 900
    height: 600
    visible: true
    title: qsTr("Macisstant v1.0")
    Material.theme: Material.Dark


    minimumWidth: 600
    minimumHeight: 400
    maximumWidth: 1200
    maximumHeight: 800

    signal kbMacrosWindowLoaded

    // Background image
    Image {
        id: backgroundImage
        source: "custom/images/bg.jpg"
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
    }

    // Make the background more dark
    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: 0.8
    }

    // Loader for the macro window
    Loader {
        id: kbMacrosWindowLoader
        objectName: "kbMacrosWindowLoader"
        anchors.fill: parent
        visible: false
        source: visible ? "custom/components/KbMacrosWindow.qml" : ""
        onLoaded: {
            item.macroWindowFadeIn();
            kbMacrosWindowLoaded();
            /* Connect to the homeButtonClicked signal
            item.homeButtonClicked.connect(function() {
                item.macroWindowFadeOut();
                mainWindowFadeIn.start()
                mainWindowContainer.visible = true
            });*/
        }
    }

    Connections {
        target: kbMacrosWindowLoader.item
        function onHomeButtonClicked() {
            kbMacrosWindowLoader.visible = false;
            mainWindowFadeIn.start();
            mainWindowContainer.visible = true;
        }
    }

    // Container for the welcome window items
    Item {
        id: mainWindowContainer
        width: parent.width
        height: parent.height
        signal mainWindowFadeOutFinished
        opacity: 1

        PropertyAnimation {
            id: mainWindowFadeIn
            target: mainWindowContainer
            property: "opacity"
            from: 0
            to: 1
            duration: 1500
            easing.type: Easing.InOutCirc
        }


        PropertyAnimation {
            id: mainWindowFadeOut
            target: mainWindowContainer
            property: "opacity"
            from: 1
            to: 0
            duration: 300
            easing.type: Easing.InOutCirc
            onFinished: {
                mainWindowContainer.visible = false
            }
        }


        MultiEffect {
            id: welcomeTextShadow
            source: welcomeText
            anchors.fill: welcomeText
            shadowBlur: 1.2
            shadowEnabled: true
            shadowColor: "black"
            shadowVerticalOffset: 9
            shadowHorizontalOffset: 9
            opacity: 1
        }

        // Welcome text
        Text {
            id: welcomeText
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -120
            text: ""
            font.family: "Segoe UI"
            font.pixelSize: 32
            font.bold: true
            color: "#ecf0f1"

            property int currentIndex: 0
            signal typingFinished

            onTypingFinished: {
                if (kbMacrosButton.hasAppearAnimation)
                    kbMacrosButton.buttonFadeInUp();
            }


            Component.onCompleted: {
                typingTimer.start();
            }

            Timer {
                id: typingTimer
                interval: 50
                repeat: true

                onTriggered: {
                    var welcomeTextStr = "What can I help you with?";
                    if (welcomeText.currentIndex < welcomeTextStr.length) {
                        welcomeText.text += welcomeTextStr[welcomeText.currentIndex]; // Add one character at a time
                        welcomeText.currentIndex++;
                    } else {
                        typingTimer.stop(); // Stop the timer when all characters are added
                        welcomeText.typingFinished();
                    }
                }
            }
        }

        MainButton {
            id: kbMacrosButton
            buttonText: "Keyboard macros"
            buttonColor: "#ecf0f1"
            anchors.centerIn: parent
            onClicked: {
                mainWindowFadeOut.start();
                kbMacrosWindowLoader.visible = true;
            }
        }
    }
}
