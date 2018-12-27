import QtQuick 2.5
import Sailfish.Silica 1.0
import QtQml.Models 2.2

DelegateModel {
    id: noteListModel
    property string searchText: ""
    property bool favoritesOnTop
    property string sortBy
    property bool showSeparator
    property int previewLineCount

    onSearchTextChanged: reload()
    onSortByChanged: reload()
    Connections {
        target: api
        onModelChanged: {
            console.log("API model changed!")
            reload()
        }
        onNoteCreated: {
            console.log("New note created: " + id)
        }
        onNoteRemoved: {
            console.log("Note removed: " + id)
        }
        onNoteChanged: {
            console.log("Note changed: " + id)
        }
    }

    function reload() {
        if (items.count > 0)
            items.setGroups(0, items.count, "unsorted")
        if (searchItems.count > 0)
            searchItems.setGroups(0, searchItems.count, "unsorted")
    }

    items.includeByDefault: false
    groups: [
        DelegateModelGroup {
            id: searchItems
            name: "search"
        },
        DelegateModelGroup {
            id: unsortedItems
            name: "unsorted"
            includeByDefault: true
            onChanged: {
                switch(sortBy) {
                case "date":
                    noteListModel.sort(function(left, right) {
                        if (favoritesOnTop) {
                            if (left.favorite === right.favorite)
                                return left.modified > right.modified
                            else
                                return left.favorite
                        }
                        else
                            return left.modified > right.modified
                    })
                    break
                case "category":
                    noteListModel.sort(function(left, right) {
                        if (favoritesOnTop) {
                            if (left.favorite === right.favorite)
                                return left.category < right.category
                            else
                                return left.favorite
                        }
                        else
                            return left.category < right.category
                    })
                    break
                case "title":
                    noteListModel.sort(function(left, right) {
                        if (favoritesOnTop) {
                            if (left.favorite === right.favorite)
                                return left.title < right.title
                            else
                                return left.favorite
                        }
                        else
                            return left.title < right.title
                    })
                    break
                default:
                    setGroups(0, unsortedItems.count, "items")
                    break
                }
            }
        }
    ]
    Connections {
        target: items
        onCountChanged: console.log(count)
    }

    function insertPosition(lessThan, item) {
        var lower = 0
        var upper = items.count
        while (lower < upper) {
            var middle = Math.floor(lower + (upper - lower) / 2)
            var result = lessThan(item.model, items.get(middle).model);
            if (result) {
                upper = middle
            } else {
                lower = middle + 1
            }
        }
        return lower
    }
    function sort(lessThan) {
        while (unsortedItems.count > 0) {
            var item = unsortedItems.get(0)
            var index = insertPosition(lessThan, item)

            if (searchText === "" ||
                    item.model.title.toLowerCase().indexOf(searchText.toLowerCase()) >= 0 ||
                    item.model.content.toLowerCase().indexOf(searchText.toLowerCase()) >= 0 ||
                    item.model.category.toLowerCase().indexOf(searchText.toLowerCase()) >= 0) {
                //console.log("Adding " + item.model.title + " to model")
                item.groups = "items"
                items.move(item.itemsIndex, index)
            }
            else if (searchText !== "") {
                item.groups = "search"
            }
        }
    }

    delegate: BackgroundItem {
        id: note

        contentHeight: titleLabel.height + previewLabel.height + 2*Theme.paddingSmall
        height: contentHeight + menu.height
        width: parent.width
        highlighted: down || menu.active
        /*ListView.onAdd: AddAnimation {
            target: note //searchText !== "" ? null : note
        }
        ListView.onRemove: RemoveAnimation {
            target: note //searchText !== "" ? null : note
        }*/
        RemorseItem {
            id: remorse
        }

        onClicked: pageStack.push(Qt.resolvedUrl("../pages/NotePage.qml"),
                                  { note: api.model.get(index) })
        onPressAndHold: menu.open(note)

        Separator {
            width: parent.width
            color: Theme.primaryColor
            anchors.top: titleLabel.top
            visible: showSeparator && index !== 0
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
            visible: sortBy !== "category" && categoryLabel.text.length > 0
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
            maximumLineCount: previewLineCount > 0 ? previewLineCount : 1
            visible: previewLineCount > 0
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
                        api.deleteNote(id)
                    })
                }
            }
        }
    }
}
