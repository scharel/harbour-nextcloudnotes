import QtQuick 2.0
import Sailfish.Silica 1.0
//import harbour.nextcloudnotes 0.1

Page {
    id: page

    SilicaListView {
        id: notesList
        anchors.fill: parent

        PullDownMenu {
            busy: notesModel.busy
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                onClicked: console.log("Add note")
            }
            MenuLabel {
                text: qsTr("Last update") + ": " + appSettings.lastUpdate
            }
        }

        header: SearchField {
            width: parent.width
            placeholderText: qsTr("Nextcloud Notes")
            onTextChanged: notesModel.search(text.toLowerCase())

            EnterKey.iconSource: "image://theme/icon-m-enter-close"
            EnterKey.onClicked: focus = false
            enabled: notesModel.json.length > 0
        }

        currentIndex: -1
        Component.onCompleted: notesModel.update()

        model: notesModel

        delegate: ListItem {
            id: note
            contentHeight: Theme.itemSizeMedium

            onClicked: {
                pageStack.push(Qt.resolvedUrl("NotePage.qml"),
                               {id: id,
                                etag: etag,
                                modified: modified,
                                title: title,
                                category: category,
                                favorite: favorite,
                                content: content,
                                error: error,
                                errorMessage: errorMessage})
            }

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
                text: new Date(modified * 1000).toLocaleDateString()
                font.pixelSize: Theme.fontSizeExtraSmall
                color: note.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

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
            text: section.empty ? qsTr("No category") : section
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            visible: notesList.count === 0
            running: visible
        }

        VerticalScrollDecorator { flickable: notesList }
    }
}
