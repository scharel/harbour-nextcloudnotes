import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Notifications 1.0

Dialog {
    id: page

    property var account
    property var note

    onAccepted: {
        account.updateNote(note.id, { 'category': categoryField.text, 'content': contentArea.text, 'favorite': favoriteButton.selected } )
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            note = account.getNote(note.id, false)
        }
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Reset")
                onClicked: {
                    note = account.getNote(note.id, false)
                    favoriteButton.selected = note.favorite
                    //categoryField.text = note.category
                    //contentArea.text = note.content
                    //favoriteButton.selected = note.favorite
                }
            }
            MenuItem {
                text: qsTr("Markdown syntax")
                onClicked: Qt.openUrlExternally("https://github.com/showdownjs/showdown/wiki/Showdown's-Markdown-syntax")
            }
        }

        Column {
            id: column
            width: parent.width// - 2*x

            DialogHeader {
                //title: account.model.get(noteIndex).title
            }

            TextArea {
                id: contentArea
                width: parent.width
                focus: true
                text: note.content
                onTextChanged: {
                    // TODO Autocomplete list symbols
                    /*var preText = text.substring(0, cursorPosition)
                    preText = preText.substring(preText.lastIndexOf('\n'))
                    console.log(preText)
                    console.log(text.substring(cursorPosition))*/
                }
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - x
                IconButton {
                    id: favoriteButton
                    property bool selected: note.favorite
                    width: Theme.iconSizeMedium
                    icon.source: (selected ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: selected = !selected
                }
                TextField {
                    id: categoryField
                    width: parent.width - favoriteButton.width
                    text: note.category
                    placeholderText: qsTr("No category")
                    label: qsTr("Category")
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
