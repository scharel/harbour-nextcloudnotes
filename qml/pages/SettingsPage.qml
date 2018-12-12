import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.Notifications 1.0


Page {
    id: page

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

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
                id: accountRepeater
                model: appSettings.accountIDs

                delegate: ListItem {
                    id: accountListItem
                    contentHeight: accountTextSwitch.height
                    highlighted: accountTextSwitch.down

                    ConfigurationGroup {
                        id: account
                        path: "/apps/harbour-nextcloudnotes/accounts/" + modelData
                        Component.onCompleted: {
                            accountTextSwitch.text = value("name", qsTr("Unnamed account"), String)
                            accountTextSwitch.description = account.value("username", qsTr("unknown"), String) + "@" + account.value("server", qsTr("unknown"), String)
                        }
                    }

                    TextSwitch {
                        id: accountTextSwitch
                        automaticCheck: false
                        checked: modelData === api.uuid
                        onClicked: {
                            api.uuid = modelData
                            api.getNotes()
                        }
                        onPressAndHold: openMenu()
                    }
                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Edit")
                            onClicked: {
                                var login = pageStack.replace(Qt.resolvedUrl("LoginDialog.qml"), { accountId: modelData })
                            }
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            onClicked:  {
                                accountListItem.remorseAction(qsTr("Deleting account"), function() {
                                    console.log("Deleting " + modelData)
                                    appSettings.removeAccount(modelData)
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
                    var newAccountID = appSettings.addAccount()
                    var login = pageStack.replace(Qt.resolvedUrl("LoginDialog.qml"), { accountId: newAccountID, addingNew: true })
                }
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
                        model: [0, 3, 5, 10, 20, 30, 42, 60, 120, 300, 600]
                        MenuItem {
                            text: modelData === 0 ?
                                      qsTr("Disabled") : (qsTr("every") + " " +
                                      (parseInt(modelData / 60) ?
                                           (parseInt(modelData / 60) + " " + qsTr("Minutes")) :
                                           (modelData + " " + qsTr("Seconds"))))
                            Component.onCompleted: {
                                if (modelData === appSettings.autoSyncInterval) {
                                    autoSyncComboBox.currentIndex = index
                                    theAnswer.enabled = true
                                }
                            }
                        }
                    }
                }
                onCurrentIndexChanged: {
                    appSettings.autoSyncInterval = autoSyncIntervalRepeater.model[currentIndex]
                    if (autoSyncIntervalRepeater.model[currentIndex] === 42 && theAnswer.enabled) {
                        theAnswer.publish()
                    }
                }
                Notification {
                    id: theAnswer
                    property bool enabled: false
                    icon: "image://theme/icon-lock-information"
                    summary: qsTr("The Answer is 42")
                    body: qsTr("Congratulation you found the Answer to the Ultimate Question of Life, The Universe, and Everything!")
                    previewSummary: summary
                    category: "Easter Egg"
                    urgency: Notification.Low
                }
            }

            SectionHeader {
                text: qsTr("Appearance")
            }
            ComboBox {
                id: sortByComboBox
                property var names: [qsTr("Last edited"), qsTr("Category"), qsTr("Title alphabetically")]
                label: qsTr("Sort notes by")
                description: qsTr("This will also change how the notes are grouped")
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
                label: qsTr("Number of lines in the preview")
                onSliderValueChanged: appSettings.previewLineCount = sliderValue
            }

            SectionHeader {
                text: qsTr("Editing")
            }
            TextSwitch {
                text: qsTr("Monospaced font")
                description: qsTr("Use a monospeced font to edit a note")
                checked: appSettings.useMonoFont
                onCheckedChanged: appSettings.useMonoFont = checked
            }
            TextSwitch {
                text: qsTr("Capital 'X' in checkboxes")
                description: qsTr("For interoperability with other apps such as Joplin")
                checked: appSettings.useCapitalX
                onCheckedChanged: appSettings.useCapitalX = checked
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
