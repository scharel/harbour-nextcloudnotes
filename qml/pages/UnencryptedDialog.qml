import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: unencryptedDialog

    canAccept: textSwitch.checked

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            DialogHeader {
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                linkColor: Theme.highlightColor
                text: qsTr("<strong>Your username and password will be transferred unencrypted over the network when you enable this option.<br>Do not accept unless you know exactly what you are doing!</strong   ><br><a href=\"https://github.com/nextcloud/notes/wiki/API-0.2\">More information...</a>")
            }

            TextSwitch {
                id: textSwitch
                text: qsTr("I do understand")
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
