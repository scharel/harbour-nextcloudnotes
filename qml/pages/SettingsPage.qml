import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

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
                visible: appSettings.accounts.length === 0
                text: qsTr("No Nextcloud account yet")
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryHighlightColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Repeater {
                model: appSettings.accounts
                delegate: ListItem {
                    id: listItem
                    TextSwitch {
                        anchors.verticalCenter: parent.verticalCenter
                        automaticCheck: false
                        checked: index === appSettings.currentAccount
                        text: appSettings.accounts[index].username + "@" + appSettings.accounts[index].server
                        description: checked ? qsTr("Press and hold to edit") : qsTr("Click to choose as active account")
                        onClicked: appSettings.currentAccount = index
                        onPressAndHold: listItem.openMenu()
                    }
                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Edit")
                            onClicked: {
                                var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { account: appSettings.accounts[index] } )
                                login.accepted.connect(function() {
                                    console.log(login.account.username + ":" + login.account.password + "@" + login.account.server.toString())
                                    appSettings.accounts[index] = login.account
                                })
                            }
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            visible: false // TODO
                        }
                    }
                }
            }
            Button {
                text: qsTr("Add account")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"))
                    login.accepted.connect(function() {
                        var list = appSettings.accounts
                        list.push(login.account)
                        appSettings.accounts = list
                        appSettings.sync()
                        appSettings.currentAccount = appSettings.accounts.length
                        appSettings.sync()
                        notes.account = appSettings.accounts[appSettings.currentAccount]
                        notes.getNotes()
                    })
                }
            }

            SectionHeader {
                text: qsTr("Security")
            }
        }

        VerticalScrollDecorator {}
    }
}
