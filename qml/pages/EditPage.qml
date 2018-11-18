import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    onAccepted: {
        account.updateNote(account.model.get(noteIndex).id, { 'category': categoryField.text, 'content': contentArea.text, 'favorite': favoriteButton.down } )
    }

    property var account
    property int noteIndex

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            quickSelect: true
            MenuItem {
                text: qsTr("Markdown Cheatsheet")
                onClicked: pageStack.push(Qt.resolvedUrl("MarkdownPage.qml"))
            }
        }

        Column {
            id: column
            width: parent.width// - 2*x

            DialogHeader {
                title: account.model.get(noteIndex).title
            }

            TextArea {
                id: contentArea
                width: parent.width
                text: account.model.get(noteIndex).content
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - x
                IconButton {
                    id: favoriteButton
                    width: Theme.iconSizeMedium
                    icon.source: (account.model.get(noteIndex).favorite ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: account.model.get(noteIndex).favorite = !account.model.get(noteIndex).favorite
                }
                TextField {
                    id: categoryField
                    width: parent.width - favoriteButton.width
                    text: account.model.get(noteIndex).category
                    placeholderText: qsTr("Category")
                    label: placeholderText
                }

            }
        }

        VerticalScrollDecorator {}
    }
}
