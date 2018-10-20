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
            MenuItem {
                text: qsTr("Settings")
                onClicked: {
                    var login = pageStack.push(Qt.resolvedUrl("LoginDialog.qml"), { server: appSettings.server, username: appSettings.username, password: appSettings.password } )
                    login.accepted.connect(function() {
                        console.log(login.username + ":" + login.password + "@" + login.server)
                        appSettings.server = login.server
                        appSettings.username = login.username
                        appSettings.password = login.password
                        notes.getNotes()
                    })
                }
            }
            MenuItem {
                text: qsTr("Add note")
                onClicked: console.log("Add note")
            }
            MenuItem {
                text: qsTr("Update")
                onClicked: notes.getNotes()
            }
            MenuLabel {
                text: qsTr("Last update") + ": " + new Date(appSettings.lastUpdate).toLocaleString(Qt.locale(), Locale.ShortFormat)
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
            if (appSettings.server.toString().length > 0 && appSettings.username.length > 0 && appSettings.password.length > 0)
                notes.getNotes()
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
            enabled: notesList.count === 0 && !notes.busy && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: (appSettings.server.length === 0 || appSettings.username.length === 0 || appSettings.password.length === 0)
            text: qsTr("No Nextcloud account")
            hintText: qsTr("Pull down to go to the settings")
        }

        VerticalScrollDecorator { flickable: notesList }
    }
}
