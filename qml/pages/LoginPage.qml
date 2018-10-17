import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.Portrait

    SilicaFlickable {
        id: webView
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Reload")
                onClicked: webView.reload()
            }
        }

        PageHeader {
            title: qsTr("Login")
        }

        Column {
            TextField {
                focus: true
                width: parent.width
                placeholderText: qsTr("https://cloud.nextcloud.com")
                label: qsTr("Nextcloud server")
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }

            TextField {
                width: parent.width
                placeholderText: qsTr("Username")
                label: qsTr("Nextcloud user")
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }

            TextField {
                width: parent.width
                placeholderText: qsTr("Password")
                label: qsTr("Nextcloud App-Password")
                inputMethodHints: Qt.ImhUrlCharactersOnly
            }
        }
    }
}
