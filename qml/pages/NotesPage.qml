import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (nextcloudAccounts.count > 0) {
                nextcloudAccounts.itemAt(appSettings.currentAccount).getNotes()
            }
            else {
                addAccountHint.restart()
            }
        }
    }

    SilicaListView {
        id: notesList
        anchors.fill: parent
        spacing: Theme.paddingLarge

        PullDownMenu {
            busy: nextcloudAccounts.itemAt(appSettings.currentAccount) ? nextcloudAccounts.itemAt(appSettings.currentAccount).busy : false

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: nextcloudAccounts.itemAt(appSettings.currentAccount) ? !nextcloudAccounts.itemAt(appSettings.currentAccount).busy : false
                visible: appSettings.currentAccount >= 0
                onClicked: console.log("Add note")
            }
            MenuItem {
                text: qsTr("Reload")
                enabled: nextcloudAccounts.itemAt(appSettings.currentAccount) ? !nextcloudAccounts.itemAt(appSettings.currentAccount).busy : false
                visible: appSettings.currentAccount >= 0
                onClicked: nextcloudAccounts.itemAt(appSettings.currentAccount).getNotes()
            }
            MenuLabel {
                visible: appSettings.currentAccount >= 0
                text: nextcloudAccounts.itemAt(appSettings.currentAccount) ? (
                                                                                 qsTr("Last update") + ": " + (
                                                                                     new Date(nextcloudAccounts.itemAt(appSettings.currentAccount).update).valueOf() !== 0 ?
                                                                                         new Date(nextcloudAccounts.itemAt(appSettings.currentAccount).update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                                                                         qsTr("never"))) : ""
            }
        }

        header: PageHeader {
            title: qsTr("Nextclound Notes")
            description: nextcloudAccounts.itemAt(appSettings.currentAccount).username + "@" + nextcloudAccounts.itemAt(appSettings.currentAccount).server
            /*SearchField {
            width: parent.width
            placeholderText: qsTr("Nextcloud Notes")
            onTextChanged: notes.search(text.toLowerCase())

            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            enabled: notesList.count > 0*/
        }

        currentIndex: -1

        model: nextcloudAccounts.itemAt(appSettings.currentAccount)? nextcloudAccounts.itemAt(appSettings.currentAccount).model : 0
        Connections {
            target: appSettings
            onCurrentAccountChanged: notesList.model = nextcloudAccounts.itemAt(appSettings.currentAccount)? nextcloudAccounts.itemAt(appSettings.currentAccount).model : 0
        }

        delegate: ListItem {
            id: note
            contentHeight: titleLabel.height + previewLabel.height + 2*Theme.paddingSmall

            IconButton {
                id: isFavoriteIcon
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingSmall
                anchors.top: parent.top
                width: Theme.iconSizeMedium
                icon.source: (favorite ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                             (note.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                onClicked: {
                    console.log("Toggle favorite")
                    favorite = !favorite
                    nextcloudAccounts.itemAt(appSettings.currentAccount).updateNote(id, {'favorite': favorite} )
                }
            }

            Label {
                id: titleLabel
                anchors.left: isFavoriteIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.top: parent.top
                text: title
                truncationMode: TruncationMode.Fade
                color: note.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Label {
                id: previewLabel
                anchors.left: isFavoriteIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.top: titleLabel.bottom
                anchors.topMargin: Theme.paddingMedium
                height: Theme.itemSizeExtraLarge
                text: content
                font.pixelSize: Theme.fontSizeExtraSmall
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: note.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            }

            onClicked: pageStack.push(Qt.resolvedUrl("NotePage.qml"), { account: nextcloudAccounts.itemAt(appSettings.currentAccount), noteIndex: index } )

            menu: ContextMenu {
                Label {
                    id: modifiedLabel
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Modified") + ": " + new Date(modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.highlightColor
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: console.log("Delete note")
                }
            }
        }

        section.property: "category"
        section.criteria: ViewSection.FullString
        section.labelPositioning: ViewSection.InlineLabels
        section.delegate: SectionHeader {
            text: section
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            visible: nextcloudAccounts.itemAt(appSettings.currentAccount) ? nextcloudAccounts.itemAt(appSettings.currentAccount).busy && notesList.count === 0 : false
            running: visible
        }

        ViewPlaceholder {
            enabled: notesList.count === 0 && !busyIndicator.running && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: nextcloudUUIDs.value.length <= 0
            text: qsTr("No account yet")
            hintText: qsTr("Got to the settings to add an account")
        }

        TouchInteractionHint {
            id: addAccountHint
            interactionMode: TouchInteraction.Pull
            direction: TouchInteraction.Down
        }
        InteractionHintLabel {
            anchors.fill: parent
            text: qsTr("Open the settings to configure your Nextcloud accounts")
            opacity: addAccountHint.running ? 1.0 : 0.0
            Behavior on opacity { FadeAnimation {} }
            width: parent.width
        }

        VerticalScrollDecorator { flickable: notesList }
    }
}
