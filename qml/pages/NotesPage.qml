import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaListView {
        id: notesList
        anchors.fill: parent
        spacing: Theme.paddingLarge

        PullDownMenu {
            busy: notes.busy

            MenuLabel {
                visible: appSettings.accounts.length > 0
                text: appSettings.accounts.length > 0 ?
                          (qsTr("Last update") + ": " +
                           (appSettings.accounts[appSettings.currentAccount].lastUpdate.value === 0 ?
                                appSettings.accounts[appSettings.currentAccount].lastUpdate.toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                qsTr("never"))) : ""
            }
            MenuItem {
                text: qsTr("Reload")
                visible: appSettings.accounts.length > 0
                onClicked: notes.getNotes()
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: appSettings.accounts.length > 0
                onClicked: console.log("Add note")
            }
        }

        header: SearchField {
            width: parent.width
            placeholderText: qsTr("Nextcloud Notes")
            onTextChanged: notes.search(text.toLowerCase())

            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            enabled: false //notesList.count > 0 // TODO
        }

        currentIndex: -1
        Component.onCompleted: {
            if (appSettings.accounts.length > 0) {
                notes.getNotes()
            }
        }

        //Component.onCompleted: notes.getNotes()
        //Component.onCompleted: notes.getNote("1212725")
        //Component.onCompleted: notes.createNote("Hello World!", "Test")
        //Component.onCompleted: notes.updateNote(1212725, "# Hello World!\nIs this working?", "Test")
        //Component.onCompleted: notes.deleteNote(1212725)

        model: notes.model

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
                    notes.updateNote(id, {'favorite': favorite} )
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

            onClicked: pageStack.push(Qt.resolvedUrl("NotePage.qml"), { note: notesList.model.get(index) } )

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
            visible: notesList.count === 0 && notes.busy
            running: visible
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: (appSettings.accounts.length === 0)
            text: qsTr("No accounts yet")
        }

        ViewPlaceholder {
            enabled: notesList.count === 0 && !notes.busy && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        VerticalScrollDecorator { flickable: notesList }
    }

    TouchInteractionHint {
        id: addAccountHint
        Component.onCompleted: if (appSettings.accounts.length === 0) restart()
        interactionMode: TouchInteraction.Pull
        direction: TouchInteraction.Down
    }
    InteractionHintLabel {
        anchors.fill: parent
        text: qsTr("Open the settings to add a Nextcloud account")
        opacity: addAccountHint.running ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation {} }
        width: parent.width
    }

}
