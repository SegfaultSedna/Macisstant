import QtQuick

Image {
    id: root
    property string iconSource: ""
    required property int iconSize
    source: iconSource  // Use the dynamic image path
    sourceSize.width: iconSize
    sourceSize.height: iconSize
    opacity: mouseArea.containsMouse ? 1 : 0.7  // Set the initial opacity


    signal clicked

    /* Fade in animation
    PropertyAnimation {
        id: iconButtonFadeIn
        target: root
        property: "opacity"
        from: 0
        to: 0.7
        duration: 500
        easing.type: Easing.InOutCirc
        onStarted: root.visible = true  // Ensure visible is true when fading in
    }

    // Fade out animation
    PropertyAnimation {
        id: iconButtonFadeOut
        target: root
        property: "opacity"
        from: 0.7
        to: 0
        duration: 500
        easing.type: Easing.InOutCirc
        onFinished: root.visible = false  // Hide the image after fading out
    }*/

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            root.clicked()
        }
    }

    /*function fadeIn() {
        iconButtonFadeIn.start()
    }

    function fadeOut() {
        iconButtonFadeOut.start()
    }*/
}
