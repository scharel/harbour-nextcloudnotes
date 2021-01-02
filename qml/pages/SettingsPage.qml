import QtQuick 2.2
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
                visible: accounts.value.length <= 0
                text: qsTr("No Nextcloud account yet")
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.secondaryHighlightColor
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                wrapMode: Text.Wrap
            }
            Repeater {
                id: accountRepeater
                model: accounts.value

                delegate: ListItem {
                    id: accountListItem
                    contentHeight: accountTextSwitch.height
                    highlighted: accountTextSwitch.down

                    ConfigurationGroup {
                        id: account
                        path: "/apps/harbour-nextcloudnotes/accounts/" + modelData
                        Component.onCompleted: {
                            accountTextSwitch.text =  account.value("username", qsTr("unknown"), String) + " @ " + value("name", qsTr("Unnamed account"), String)
                            accountTextSwitch.description =account.value("server", qsTr("unknown"), String)
                        }
                    }

                    TextSwitch {
                        id: accountTextSwitch
                        automaticCheck: false
                        checked: modelData === appSettings.currentAccount
                        onClicked: {
                            appSettings.currentAccount = modelData
                        }
                        onPressAndHold: openMenu()
                    }
                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Edit")
                            onClicked: {
                                var login = pageStack.push(Qt.resolvedUrl("LoginPage.qml"), { accountId: modelData })
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
                    var login = pageStack.push(Qt.resolvedUrl("LoginPage.qml"), { accountId: newAccountID, addingNew: true })
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
                        console.log(theAnswer.body)
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
                property var criteria: [
                    { role: "modifiedString", text: qsTr("Last edited") },
                    { role: "category", text: qsTr("Category") },
                    { role: "title", text: qsTr("Title alphabetically") },
                    { role: "none", text: qsTr("No sorting") }
                ]
                label: qsTr("Sort notes by")
                description: qsTr("This will also change how the notes are grouped")
                menu: ContextMenu {
                    Repeater {
                        id: sortByRepeater
                        model: sortByComboBox.criteria
                        MenuItem {
                            text: modelData.text
                            Component.onCompleted: {
                                if (modelData.role === appSettings.sortBy) {
                                    sortByComboBox.currentIndex = index
                                }
                            }
                            onClicked: appSettings.sortBy = modelData.role
                        }
                    }
                }
                /*onCurrentIndexChanged: {
                    appSettings.sortBy = sortByRepeater.model[currentIndex]
                }*/
            }
            TextSwitch {
                text: qsTr("Favorites on top")
                description: qsTr("Show notes marked as favorite above the others")
                checked: appSettings.favoritesOnTop
                onCheckedChanged: appSettings.favoritesOnTop = checked
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

            SectionHeader {
                text: qsTr("Reset")
            }
            Button {
                text: qsTr("Reset app settings")
                anchors.horizontalCenter: parent.horizontalCenter
                RemorseItem { id: resetRemorse }
                ConfigurationGroup {
                    id: appConfig
                    path: "/apps/harbour-nextcloudnotes"
                }
                onClicked: resetRemorse.execute(this, "Reset app", appConfig.clear())
            }
            LinkedLabel {
                text: qsTr("Resetting the app wipes all application data from the device! This includes offline synced notes, app settings and accounts.")
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
            }

        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
