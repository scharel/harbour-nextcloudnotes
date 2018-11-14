import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {

    CoverPlaceholder {
        icon.source: "../img/nextcloud-logo-transparent.png"
        text: qsTr("Notes")
    }

    CoverActionList {
        id: coverAction
        enabled: appSettings.currentAccount >= 0

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: {
                console.log("Add note")
                appWindow.activate()
            }
        }

        /*CoverAction {
            iconSource: "image://theme/icon-cover-next"
        }*/
    }
}
