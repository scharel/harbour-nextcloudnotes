import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaListView {
        id: notesList
        anchors.fill: parent

        PullDownMenu {
            busy: notes.busy
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                onClicked: console.log("Add note")
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
            enabled: notes.json.length > 0
        }

        currentIndex: -1
        Component.onCompleted: notes.getNotes()
        //Component.onCompleted: notes.getNote("1212725")
        //Component.onCompleted: notes.createNote("Hello World!", "Test")
        //Component.onCompleted: notes.updateNote(1212725, "# Hello World!\nIs this working?", "Test")
        //Component.onCompleted: notes.deleteNote(1212725)

        model: notes.model

        delegate: ListItem {
            id: note
            contentHeight: Theme.itemSizeMedium

            IconButton {
                id: isFavoriteIcon
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
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
                anchors.bottom: parent.verticalCenter
                text: title
                truncationMode: TruncationMode.Fade
                color: note.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Label {
                id: modifiedLabel
                anchors.left: isFavoriteIcon.right
                anchors.leftMargin: Theme.paddingSmall
                anchors.right: parent.right
                anchors.top: parent.verticalCenter
                text: new Date(modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                font.pixelSize: Theme.fontSizeExtraSmall
                color: note.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            onClicked: pageStack.push(Qt.resolvedUrl("NotePage.qml"), { note: notesList.model.get(index) } )

            menu: ContextMenu {
                Text {
                    id: descriptionText
                    width: parent.width - 2*Theme.horizontalPageMargin
                    height: contentHeight + Theme.paddingMedium
                    x: Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeSmall
                    maximumLineCount: 5
                    text: content
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
            enabled: notesList.count === 0 && !notes.busy
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        VerticalScrollDecorator { flickable: notesList }
    }
}
