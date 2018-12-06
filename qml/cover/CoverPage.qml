import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {

    CoverPlaceholder {
        icon.source: "../img/nextcloud-logo-transparent.png"
        icon.width: parent.width
        icon.fillMode: Image.PreserveAspectFit
        text: qsTr("Notes")
    }

    CoverActionList {
        id: coverAction
        enabled: appSettings.currentAccount >= 0

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: {
                nextcloudAccounts.itemAt(appSettings.currentAccount).createNote({'content': ""})
                appWindow.activate()
            }
        }
    }
}
