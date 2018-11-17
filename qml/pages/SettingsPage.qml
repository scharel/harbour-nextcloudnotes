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
                visible: nextcloudAccounts.count <= 0
                text: qsTr("No Nextcloud account yet")
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryHighlightColor
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Repeater {
                model: nextcloudAccounts.count
                delegate: ListItem {
                    id: accountListItem
                    contentHeight: textSwitch.height
                    highlighted: textSwitch.down

                    TextSwitch {
                        id: textSwitch
                        automaticCheck: false
                        checked: index === appSettings.currentAccount
                        text: nextcloudAccounts.itemAt(index).name.length <= 0 ? qsTr("Unnamed account") : nextcloudAccounts.itemAt(index).name
                        description: nextcloudAccounts.itemAt(index).username + "@" + nextcloudAccounts.itemAt(index).server// : qsTr("Press and hold to configure")
                        onClicked: appSettings.currentAccount = index
                        onPressAndHold: openMenu()
                    }
                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Edit")
                            onClicked: {
                                var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { account: index })
                                login.accepted.connect(function() {
                                })
                                login.rejected.connect(function() {
                                })
                            }
                        }
                        MenuItem {
                            visible: index === nextcloudAccounts.count-1
                            text: qsTr("Delete")
                            onClicked:  {
                                accountListItem.remorseAction(qsTr("Deleting account"), function() {
                                    nextcloudAccounts.itemAt(index).clear()
                                    nextcloudAccounts.pop()
                                })
                            }
                        }
                    }
                }
            }

            Button {
                text: qsTr("Add account")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    nextcloudAccounts.add()
                    var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { account:nextcloudAccounts.count-1 })
                    login.accepted.connect(function() {
                        console.log("Adding account " + nextcloudAccounts.itemAt(login.account).name)
                        if (appSettings.currentAccount < 0)
                            appSettings.currentAccount = nextcloudUUIDs.value.length-1
                        appWindow.update()
                    })
                    login.rejected.connect(function() {
                        nextcloudAccounts.pop()
                    })
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
