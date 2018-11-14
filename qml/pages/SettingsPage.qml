import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

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
                visible: appSettings.accountIDs.length <= 0
                text: qsTr("No Nextcloud account yet")
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryHighlightColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Repeater {
                model: appSettings.accountIDs.length
                delegate: ListItem {
                    id: listItem
                    ConfigurationGroup {
                        id: account
                        path: "/apps/harbour-nextcloudnotes/accounts/" + appSettings.accountIDs[index]
                    }

                    TextSwitch {
                        anchors.verticalCenter: parent.verticalCenter
                        automaticCheck: false
                        checked: index === appSettings.currentAccount
                        text: account.value("name", qsTr("Account") + " " + (index+1), String)
                        //enabled: account.value("valid", false, Boolean)
                        description: account.value("valid", false, Boolean) ? account.value("username", qsTr("user"), String) + "@" + account.value("server", qsTr("server"), String) : qsTr("Press and hold to configure")
                        onClicked: if (account.value("valid", false, Boolean)) appSettings.currentAccount = index
                        onPressAndHold: listItem.openMenu()
                    }
                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Configure")
                            onClicked: {
                                var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { accountID: appSettings.accountIDs[index] })
                                login.accepted.connect(function() {
                                    update()
                                })
                                login.rejected.connect(function() {

                                })
                            }
                        }
                        /*MenuItem {
                            text: qsTr("Delete")
                            onClicked:  {
                                accounts.itemAt(index).clear()
                                // TODO reorder items
                            }
                        }*/
                    }
                }
            }
            Button {
                text: qsTr("Add account")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { accountID: accounts.add() })
                    login.accepted.connect(function() {
                        var tmpIDs = appSettings.accountIDs
                        tmpIDs.push(login.accountID)
                        appSettings.accountIDs = tmpIDs
                    })
                    login.rejected.connect(function() {
                    })
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
