import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Notifications 1.0

Dialog {
    id: page

    onAccepted: {
        account.updateNote(account.model.get(noteIndex).id, { 'category': categoryField.text, 'content': contentArea.text, 'favorite': favoriteButton.selected } )
    }

    property var account
    property int noteIndex

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Reset")
                onClicked: {
                    categoryField.text = account.model.get(noteIndex).category
                    contentArea.text = account.model.get(noteIndex).content
                    favoriteButton.selected = account.model.get(noteIndex).favorite
                }
            }
            MenuItem {
                text: qsTr("Markdown syntax")
                onClicked: Qt.openUrlExternally("https://github.com/showdownjs/showdown/wiki/Showdown's-Markdown-syntax")//pageStack.push(Qt.resolvedUrl("MarkdownPage.qml"))
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
                text: account.model.get(noteIndex).content
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - x
                IconButton {
                    id: favoriteButton
                    property bool selected: account.model.get(noteIndex).favorite
                    width: Theme.iconSizeMedium
                    icon.source: (selected ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: selected = !selected
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
