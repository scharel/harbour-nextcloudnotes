import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.nextcloudnotes.notesmodel 1.0

Page {
    id: page

    property NotesModel notesModel: notesApi.model()

    Connections {
        target: appSettings
        onSortByChanged: {
            if (appSettings.sortBy == "none")
                notesModel.invalidate()
            else
                notesModel.sortRole = notesModel.roleFromName(appSettings.sortBy)
        }
        onFavoritesOnTopChanged: {
            notesModel.favoritesOnTop = appSettings.favoritesOnTop
        }
    }
    Component.onCompleted: {
        notesModel.favoritesOnTop = appSettings.favoritesOnTop
        notesModel.sortRole = notesModel.roleFromName(appSettings.sortBy)
    }

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
            busy: notesApi.busy

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: appSettings.currentAccount.length > 0
                onClicked: notesApi.createNote( { 'content': "", 'modified': new Date().valueOf() / 1000 } )
            }
            MenuItem {
                text: enabled ? qsTr("Reload") : qsTr("Updating...")
                enabled: appSettings.currentAccount.length > 0 && notesApi.networkAccessible && !notesApi.busy
                onClicked: notesApi.getAllNotes()
            }
            MenuLabel {
                visible: appSettings.currentAccount.length > 0
                text: qsTr("Last update") + ": " + (
                          new Date(account.update).valueOf() !== 0 ?
                              new Date(account.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                              qsTr("never"))
            }
        }

        header: PageHeader {
            height: searchField.height + description.height
            SearchField {
                id: searchField
                width: parent.width
                enabled: !busyIndicator.running && !noLoginPlaceholder.enabled && !errorPlaceholder.enabled && !noNotesPlaceholder.enabled
                placeholderText: account.name.length > 0 ? account.name : qsTr("Nextcloud Notes")
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onTextChanged: notesModel.setFilterFixedString(text)
            }
            Label {
                id: description
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                visible: text.length > 1
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.paddingMedium
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: account.username + "@" + account.server
            }
            BusyIndicator {
                anchors.verticalCenter: searchField.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: notesApi.busy && !busyIndicator.running
            }
        }

        currentIndex: -1

        model: notesModel

        delegate: BackgroundItem {
            id: note

            contentHeight: titleLabel.height + previewLabel.height + 2*Theme.paddingSmall
            height: contentHeight + menu.height
            width: parent.width
            highlighted: down || menu.active
            ListView.onAdd: AddAnimation {
                target: note //searchText !== "" ? null : note
            }
            ListView.onRemove: RemoveAnimation {
                target: note //searchText !== "" ? null : note
            }
            RemorseItem {
                id: remorse
            }

            onClicked: pageStack.push(Qt.resolvedUrl("../pages/NotePage.qml"),
                                      {   //note: noteListModel.get(index),
                                          id: id,
                                          modified: modified,
                                          title: title,
                                          category: category,
                                          content: content,
                                          favorite: favorite,
                                          etag: etag,
                                          error: error,
                                          errorMessage: errorMessage
                                      })
            onPressAndHold: menu.open(note)

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
                    notesApi.updateNote(id, {'favorite': !favorite} )
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
                            notesApi.deleteNote(id)
                        })
                    }
                }
            }
        }

        section.property: appSettings.sortBy
        section.criteria: appSettings.sortBy === "title" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.labelPositioning: appSettings.sortBy === "title" ? ViewSection.CurrentLabelAtStart | ViewSection.NextLabelAtEnd : ViewSection.InlineLabels
        section.delegate: SectionHeader {
            text: section
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: notesList.count === 0 && notesApi.busy
        }
        Label {
            id: busyLabel
            anchors.top:  busyIndicator.bottom
            anchors.topMargin: Theme.paddingMedium
            visible: busyIndicator.running
            width: parent.width
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeExtraLarge
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("Loading notes...")
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: appSettings.accountIDs.length <= 0
            text: qsTr("No account yet")
            hintText: qsTr("Got to the settings to add an account")
        }

        ViewPlaceholder {
            id: noNotesPlaceholder
            enabled: notesApi.status === 204 && !busyIndicator.running && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noSearchPlaceholder
            enabled: notesList.count === 0 && searchField.text !== "" //notesModel.filterRegExp !== ""
            text: qsTr("No result")
            hintText: qsTr("Try another query")
        }

        ViewPlaceholder {
            id: errorPlaceholder
            enabled: notesList.count === 0 && !busyIndicator.running && !noSearchPlaceholder.enabled && !noNotesPlaceholder.enabled && !noLoginPlaceholder.enabled
            text: qsTr("An error occurred")
            //hintText: notesApi.statusText
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
