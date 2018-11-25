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
                    contentHeight: accountTextSwitch.height
                    highlighted: accountTextSwitch.down

                    TextSwitch {
                        id: accountTextSwitch
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

            SectionHeader {
                text: qsTr("Appearance")
            }
            ComboBox {
                id: sortByComboBox
                property var names: [qsTr("by Date"), qsTr("by Category"), qsTr("Alphabetically")]
                label: qsTr("Sort notes")
                menu: ContextMenu {
                    Repeater {
                        id: sortByRepeater
                        model: ["date", "category", "title"]
                        MenuItem {
                            text: sortByComboBox.names[index]
                            //enabled: modelData !== "title"
                            Component.onCompleted: {
                                if (modelData === appSettings.sortBy) {
                                    sortByComboBox.currentIndex = index
                                }
                            }
                        }
                    }
                }
                onCurrentIndexChanged: {
                    appSettings.sortBy = sortByRepeater.model[currentIndex]
                }
            }
            TextSwitch {
                text: qsTr("Show separator")
                description: qsTr("Show a separator line between the notes")
                checked: appSettings.showSeparator
                onCheckedChanged: appSettings.showSeparator = checked
            }
            Slider {
                width: parent.width
                minimumValue: 0
                maximumValue: 20
                stepSize: 1
                value: appSettings.previewLineCount
                valueText: sliderValue + " " + qsTr("lines")
                label: qsTr("Number of lines to preview in the list view")
                onSliderValueChanged: appSettings.previewLineCount = sliderValue
            }

            SectionHeader {
                text: qsTr("Synchronization")
            }
            ComboBox {
                id: autoSyncComboBox
                label: qsTr("Auto-Sync")
                description: qsTr("Periodically pull notes from the server")
                menu: ContextMenu {
                    Repeater {
                        id: autoSyncIntervalRepeater
                        model: [0, 3, 5, 10, 20, 30, 60, 120, 300, 600]
                        MenuItem {
                            text: modelData === 0 ?
                                      qsTr("Disabled") : (qsTr("every") + " " +
                                      (parseInt(modelData / 60) ?
                                           (parseInt(modelData / 60) + " " + qsTr("Minutes")) :
                                           (modelData + " " + qsTr("Seconds"))))
                            Component.onCompleted: {
                                if (modelData === appSettings.autoSyncInterval) {
                                    autoSyncComboBox.currentIndex = index
                                }
                            }
                        }
                    }
                }
                onCurrentIndexChanged: appSettings.autoSyncInterval = autoSyncIntervalRepeater.model[currentIndex]
            }
        }

        VerticalScrollDecorator {}
    }
}
