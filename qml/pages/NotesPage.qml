import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (nextcloudAccounts.count > 0) {
                autoSyncTimer.restart()
            }
            else {
                addAccountHint.restart()
            }
        }
    }

    Timer {
        id: autoSyncTimer
        interval: appSettings.autoSyncInterval * 1000
        repeat: true
        running: interval > 0 && appWindow.visible
        triggeredOnStart: true
        onTriggered: nextcloudAccounts.itemAt(appSettings.currentAccount).getNotes()
        onIntervalChanged: console.log("Auto-Sync every " + interval / 1000 + " seconds")
    }

    SilicaListView {
        id: notesList
        anchors.fill: parent

        PullDownMenu {
            busy: nextcloudAccounts.itemAt(appSettings.currentAccount) ? nextcloudAccounts.itemAt(appSettings.currentAccount).busy : false

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: nextcloudAccounts.itemAt(appSettings.currentAccount) ? true : false
                visible: appSettings.currentAccount >= 0
                onClicked: nextcloudAccounts.itemAt(appSettings.currentAccount).createNote({'content': ""})
            }
            MenuItem {
                text: enabled ? qsTr("Reload") : qsTr("Updating...")
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
            height: searchField.height + description.height
            SearchField {
                id: searchField
                width: parent.width
                placeholderText: nextcloudAccounts.itemAt(appSettings.currentAccount).name
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onTextChanged: nextcloudAccounts.itemAt(appSettings.currentAccount).search(text.toLowerCase())
            }
            Label {
                id: description
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.paddingMedium
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: nextcloudAccounts.itemAt(appSettings.currentAccount).username + "@" + nextcloudAccounts.itemAt(appSettings.currentAccount).server
            }
            BusyIndicator {
                anchors.verticalCenter: searchField.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: nextcloudAccounts.itemAt(appSettings.currentAccount) ? nextcloudAccounts.itemAt(appSettings.currentAccount).busy && !busyIndicator.running : false
            }
        }

        currentIndex: -1

        model: nextcloudAccounts.itemAt(appSettings.currentAccount)? nextcloudAccounts.itemAt(appSettings.currentAccount).model : 0
        Connections {
            target: appSettings
            onCurrentAccountChanged: notesList.model = nextcloudAccounts.itemAt(appSettings.currentAccount)? nextcloudAccounts.itemAt(appSettings.currentAccount).model : 0
        }

        delegate: BackgroundItem {
            id: note
            contentHeight: titleLabel.height + previewLabel.height + 2*Theme.paddingSmall
            height: contentHeight + menu.height
            width: parent.width
            highlighted: down || menu.active
            /*ListView.onAdd: AddAnimation {
                target: note
            }
            ListView.onRemove: RemoveAnimation {
                target: note
            }*/
            RemorseItem {
                id: remorse
            }

            Separator {
                width: parent.width
                color: Theme.primaryColor
                anchors.top: titleLabel.top
                visible: appSettings.showSeparator && index !== 0
            }

            IconButton {
                id: isFavoriteIcon
                anchors.left: parent.left
                anchors.top: parent.top
                icon.source: (favorite ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                             (note.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                onClicked: {
                    nextcloudAccounts.itemAt(appSettings.currentAccount).updateNote(id, {'favorite': !favorite} )
                }
            }

            Label {
                id: titleLabel
                anchors.left: isFavoriteIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: categoryRectangle.visible ? categoryRectangle.left : parent.right
                anchors.top: parent.top
                text: title
                truncationMode: TruncationMode.Fade
                color: note.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Rectangle {
                id: categoryRectangle
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.top: parent.top
                anchors.topMargin: Theme.paddingSmall
                width: categoryLabel.width + Theme.paddingLarge
                height: categoryLabel.height + Theme.paddingSmall
                color: "transparent"
                border.color: Theme.highlightColor
                radius: height / 4
                visible: appSettings.sortBy !== "category" && categoryLabel.text.length > 0
                Label {
                    id: categoryLabel
                    anchors.centerIn: parent
                    text: category
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }
            }

            Label {
                id: previewLabel
                anchors.left: isFavoriteIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.top: titleLabel.bottom
                text: parseText(content)
                font.pixelSize: Theme.fontSizeExtraSmall
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                maximumLineCount: appSettings.previewLineCount > 0 ? appSettings.previewLineCount : 1
                visible: appSettings.previewLineCount > 0
                color: note.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                function parseText (preText) {
                    var lines = preText.split('\n')
                    lines.splice(0,1);
                    var newText = lines.join('\n');
                    return newText.replace(/^\s*$(?:\r\n?|\n)/gm, "")
                }
            }

            onClicked: pageStack.push(Qt.resolvedUrl("NotePage.qml"),
                                      { account: nextcloudAccounts.itemAt(appSettings.currentAccount),
                                        note: nextcloudAccounts.itemAt(appSettings.currentAccount).model.get(index)} )
            onPressAndHold: menu.open(note)

            ContextMenu {
                id: menu
                MenuLabel {
                    id: modifiedLabel
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Modified") + ": " + new Date(modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        remorse.execute(note, qsTr("Deleting note"), function() {
                            nextcloudAccounts.itemAt(appSettings.currentAccount).deleteNote(id)
                        })
                    }
                }
            }
        }

        section.property: appSettings.sortBy
        section.criteria: appSettings.sortBy === "title" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.labelPositioning: appSettings.sortBy === "title" ? ViewSection.FirstCharacter : ViewSection.InlineLabels
        section.delegate: SectionHeader {
            text: section
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: nextcloudAccounts.itemAt(appSettings.currentAccount) ? (notesList.count === 0 && nextcloudAccounts.itemAt(appSettings.currentAccount).busy) : false
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: nextcloudUUIDs.value.length <= 0
            text: qsTr("No account yet")
            hintText: qsTr("Got to the settings to add an account")
        }

        ViewPlaceholder {
            id: noNotesPlaceholder
            enabled: nextcloudAccounts.itemAt(appSettings.currentAccount) ? (nextcloudAccounts.itemAt(appSettings.currentAccount).status === 204 && !busyIndicator.running && !noLoginPlaceholder.enabled) : false
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noSearchPlaceholder
            enabled: nextcloudAccounts.itemAt(appSettings.currentAccount) ? (notesList.count === 0 && nextcloudAccounts.itemAt(appSettings.currentAccount).modelData.length > 0) : false
            text: qsTr("No result")
            hintText: qsTr("Try another query")
        }

        ViewPlaceholder {
            id: errorPlaceholder
            enabled: notesList.count === 0 && !busyIndicator.running && !noSearchPlaceholder.enabled && !noNotesPlaceholder.enabled && !noLoginPlaceholder.enabled
            text: qsTr("An error occurred")
            hintText: nextcloudAccounts.itemAt(appSettings.currentAccount).statusText
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

    allowedOrientations: defaultAllowedOrientations
}
