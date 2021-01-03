import QtQuick 2.2
import Sailfish.Silica 1.0

Dialog {
    id: unencryptedDialog

    canAccept: understandTextSwitch.checked && ownRiskTextSwitch.checked

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            DialogHeader {
            }

            LinkedLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                linkColor: Theme.highlightColor
                text: qsTr("<strong>" + qsTr("Your username and password will be transferred unencrypted over the network when you enable this option.") + "<br>" + qsTr("Do not accept unless you know exactly what you are doing!") + "</strong><br><a href=\"https://github.com/nextcloud/notes/wiki/API-0.2#authentication--basics\">" + qsTr("More information...") + "</a>")
            }

            TextSwitch {
                id: understandTextSwitch
                text: qsTr("I do understand")
                onCheckedChanged: if (!checked) ownRiskTextSwitch.checked = false
            }
            TextSwitch {
                id: ownRiskTextSwitch
                opacity: understandTextSwitch.checked ? 1.0 : 0.0
                Behavior on opacity { FadeAnimator {} }
                text: qsTr("I use this option at my own risk")
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
