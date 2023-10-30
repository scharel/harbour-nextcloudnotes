import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (appSettings.accountIDs.length <= 0) {
                addAccountHint.restart()
            }
            else {
                autoSyncTimer.restart()
            }
        }
    }

    SilicaListView {
        id: notesList
        anchors.fill: parent
        spacing: Theme.paddingLarge

        PullDownMenu {
            busy: api.busy

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: appSettings.currentAccount.length > 0
                onClicked: api.createNote({'content': ""})
            }
            MenuItem {
                text: enabled ? qsTr("Reload") : qsTr("Updating...")
                enabled: appSettings.currentAccount.length > 0 && !api.busy
                onClicked: api.getNotes()
            }
            MenuLabel {
                visible: appSettings.currentAccount.length > 0
                text: qsTr("Last update") + ": " + (
                          new Date(api.update).valueOf() !== 0 ?
                              new Date(api.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                              qsTr("never"))
            }
        }

        header: PageHeader {
            height: searchField.height + description.height
            SearchField {
                id: searchField
                width: parent.width
                placeholderText: api.name
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onTextChanged: api.search(text.toLowerCase())
            }
            Label {
                id: description
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.paddingMedium
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: api.username + "@" + api.server.toString().replace(/^https?:\/\//, '')
            }
            BusyIndicator {
                anchors.verticalCenter: searchField.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: api.busy && !busyIndicator.running
            }
        }

        currentIndex: -1

        model: api.model
        Connections {
            target: api
            onUuidChanged: notesList.model = api.model
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
                    api.updateNote(id, {'favorite': !favorite} )
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
                    color: note.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
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
                                      { note: api.model.get(index)} )
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
                            api.deleteNote(id)
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
            running: notesList.count === 0 && api.busy
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: appSettings.accountIDs.length <= 0
            text: qsTr("No account yet")
            hintText: qsTr("Got to the settings to add an account")
        }

        ViewPlaceholder {
            id: noNotesPlaceholder
            enabled: api.status === 204 && !busyIndicator.running && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noSearchPlaceholder
            enabled: notesList.count === 0 && api.searchActive
            text: qsTr("No result")
            hintText: qsTr("Try another query")
        }

        ViewPlaceholder {
            id: errorPlaceholder
            enabled: notesList.count === 0 && !busyIndicator.running && !noSearchPlaceholder.enabled && !noNotesPlaceholder.enabled && !noLoginPlaceholder.enabled
            text: qsTr("An error occurred")
            hintText: api.statusText
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
