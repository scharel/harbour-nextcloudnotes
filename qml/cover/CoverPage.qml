import QtQuick 2.2
import Sailfish.Silica 1.0

CoverBackground {

    CoverPlaceholder {
        icon.source: Theme.colorScheme === Theme.DarkOnLight ? "../img/nextcloud-logo-dark.png" : "../img/nextcloud-logo-light.png"
        icon.width: parent.width
        icon.fillMode: Image.PreserveAspectFit
        text: qsTr("Notes")
    }

    CoverActionList {
        id: coverAction
        enabled: appSettings.currentAccount.length > 0

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: {
                api.createNote({'content': ""})
                appWindow.activate()
            }
        }
    }
}
