import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Settings")
            }

            SectionHeader {
                text: qsTr("Accounts")
            }
            Label {
                id: noAccountsLabel
                visible: typeof(appSettings.accounts) !== 'undefined'
                text: qsTr("No Nextcloud account yet")
                font.pixelSize: Theme.fontSizeExtraLarge
                color: Theme.secondaryHighlightColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Repeater {
                model: appSettings.accounts
                delegate: BackgroundItem {
                    Label {
                        text: appSettings.accounts[index].username + "@" + appSettings.accounts[index].server
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2*x
                    }
                    onClicked: {
                        var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { account: appSettings.accounts[index] } )
                        login.accepted.connect(function() {
                            console.log(login.account.username + ":" + login.account.password + "@" + login.account.server.toString())
                            appSettings.accounts[index] = login.account
                        })
                    }
                }
            }

            Button {
                text: qsTr("Add account")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"))
                    login.accepted.connect(function() {
                        console.log(login.account.username + ":" + login.account.password + "@" + login.account.server.toString())
                        appSettings.currentAccount = appSettings.accounts.length
                        appSettings.accounts[appSettings.currentAccount] = login.account
                        notes.getNotes()
                    })
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
