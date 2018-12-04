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
            favoriteButton.selected = note.favorite
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
                font.family: appSettings.useMonoFont ? "DejaVu Sans Mono" : Theme.fontFamily // "Courier"
                property int preTextLength: 0
                property var listPrefixes: [/^- /gm, /^\* /gm, /^\+ /gm, /^- \[ \] /gm, /^- \[[xX]\] /gm, /^\d+. /gm]
                onTextChanged: {
                    if (page.status === PageStatus.Active &&
                            text.length > preTextLength &&
                            text.charAt(cursorPosition-1) === "\n") {
                        var clipboard = ""
                        var preLine = text.substring(text.lastIndexOf("\n", cursorPosition-2), text.indexOf("\n", cursorPosition-1))
                        listPrefixes.forEach(function(currentValue, index) {
                            var prefix = preLine.match(currentValue)
                            if (prefix !== null) {
                                if (index === 5) {
                                    var newListNumber = parseInt(prefix[0].split(". ")[0]) + 1
                                    clipboard = newListNumber.toString() + ". "
                                }
                                else {
                                    clipboard = prefix[0]
                                }
                            }
                        })
                        if (clipboard !== "") {
                            var tmpClipboard = Clipboard.text
                            Clipboard.text = clipboard
                            contentArea.paste()
                            Clipboard.text = tmpClipboard
                        }
                    }
                    preTextLength = text.length
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
