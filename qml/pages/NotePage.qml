import QtQuick 2.2
import Sailfish.Silica 1.0
import "../js/showdown/dist/showdown.js" as ShowDown

Dialog {
    id: noteDialog

    property int id
    property var note

    onIdChanged: note = notesModel.getNoteById(id)
    onNoteChanged: {
        if (note["content"].split('\n')[0].indexOf(note["title"]) > 0) {
            dialogHeader.title = ""
        }
        else {
            dialogHeader.title = note["title"]
        }
        favoriteButton.selected = note["favorite"]
        categoryField.text = note["category"]
        modifiedDetail.modified = note["modified"]
        parseContent()
    }

    Connections {
        target: notesModel
        onNoteUpdated: {
            if (id === noteDialog.id) {
                noteDialog.note = note
            }
        }
    }

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    onAcceptPendingChanged: {
        if (acceptPending)
            acceptDestinationInstance.id = id
    }
    onStatusChanged: {
        if (status === PageStatus.Activating) {
            notesModel.getNote(id)
        }
    }


    property var showdown: ShowDown.showdown
    property var converter: new showdown.Converter(
                                {   simplifiedAutoLink: true,
                                    excludeTrailingPunctuationFromURLs: true,
                                    parseImgDimensions: true,
                                    strikethrough: true,
                                    tables: true,
                                    tasklists: false, // this is handled by the function parseContent() because LinkedLabel HTML support is to basic
                                    smoothLivePreview: true,
                                    simpleLineBreaks: true,
                                    emoji: true } )

    function parseContent() {
        var convertedText = converter.makeHtml(note["content"])
        var occurence = -1
        convertedText = convertedText.replace(/^<li>(<p>)?\[ \] (.*)(<.*)$/gmi,
                                              function(match, p1, p2, p3, offset) {
                                                  occurence++
                                                  return '<li class="tasklist"><a class="checkbox" href="tasklist:checkbox_' + occurence + '">' + (p1 ? p1 : "") + '☐ ' + p2 + '</a>' + p3
                                              } )
        occurence = -1
        convertedText = convertedText.replace(/^<li>(<p>)?\[[xX]\] (.*)(<.*)$/gmi,
                                              function(match, p1, p2, p3, offset) {
                                                  occurence++
                                                  return '<li class="tasklist"><a class="checkbox" href="tasklist:uncheckbox_' + occurence + '">' + (p1 ? p1 : "") + '☑ ' + p2 + '</a>' + p3
                                              } )
        convertedText = convertedText.replace(/<table>/gmi, "<table border='1' cellpadding='" + Theme.paddingMedium + "'>")
        convertedText = convertedText.replace(/<del>(.*)<\/del>/gmi, function(match, p1) { return "<s>" + p1 + "</s>" })
        convertedText = convertedText.replace(/<hr \/>/gmi, "<p><img width=\"" + contentLabel.width + "\" height=\"1\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAIAAACQd1PeAAAABGdBTUEAANbY1E9YMgAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAAAPSURBVHjaYvr//z9AgAEABgYDACuJdK0AAAAASUVORK5CYII=\" /></p>")
        contentLabel.text = "<style>\n" +
                "p,ul,ol,table,img { margin: " + Theme.paddingLarge + "px 0px; }\n" +
                "a:link { color: " + Theme.primaryColor + "; }\n" +
                "a.checkbox { text-decoration: none; padding: " + Theme.paddingSmall + "px; display: inline-block; }\n" +
                "li.tasklist { font-size:large; margin: " + Theme.paddingMedium + "px 0px; }\n" +
                "table { border-color: " + Theme.secondaryColor + "; }\n" +
                "</style>\n" + convertedText
        //if (debug) console.log(contentLabel.text)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: mainColumn.height  + Theme.paddingLarge

        Column {
            id: mainColumn
            width: parent.width

            RemorsePopup {
                id: remorse
                onTriggered: pageStack.pop()
            }
            PullDownMenu {
                busy: notesApi.busy

                MenuItem {
                    text: qsTr("Delete")
                    onClicked: remorse.execute("Deleting", function() { notesApi.deleteNote(note["id"]) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: !notesApi.busy
                    onClicked: notesApi.getNote(note["id"])
                }
                /*MenuItem {
                    text: qsTr("Edit")
                    onClicked: pageStack.push(Qt.resolvedUrl("../pages/NotePage.qml"), { id: id } )
                }*/

                MenuLabel {
                    visible: appSettings.currentAccount.length >= 0
                    text: qsTr("Last update") + ": " + (
                              new Date(account.update).valueOf() !== 0 ?
                                  new Date(account.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                  qsTr("never"))
                }
            }

            DialogHeader {
                id: dialogHeader
                acceptText: qsTr("Edit")
                cancelText: qsTr("Notes")
            }
            BusyIndicator {
                parent: dialogHeader.extraContent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                size: BusyIndicatorSize.Medium
                running: notesApi.busy
            }

            Column {
                width: parent.width
                spacing: Theme.paddingLarge

                LinkedLabel {
                    id: contentLabel
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    textFormat: Text.RichText
                    linkColor: Theme.primaryColor
                    defaultLinkActions: false
                    onLinkActivated: {
                        //console.log(link)
                        var occurence = -1
                        var newContent = note["content"]
                        if (/^tasklist:checkbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[ \] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return (appSettings.useCapitalX ? '- [X] ' : '- [x] ') + p1 }
                                                                else { return match }
                                                            } )
                            note["content"] = newContent
                            parseContent()
                            notesApi.updateNote(id, { 'content': note["content"] } )
                        }
                        else if (/^tasklist:uncheckbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[[xX]\] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return '- [ ] ' + p1 }
                                                                else { return match }
                                                            } )
                            note["content"] = newContent
                            parseContent()
                            notesApi.updateNote(id, { 'content': note["content"] } )
                        }
                        else {
                            Qt.openUrlExternally(link)
                        }
                    }
                }

                Separator {
                    width: parent.width
                    color: Theme.primaryColor
                    horizontalAlignment: Qt.AlignHCenter
                }

                Flow {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    spacing: Theme.paddingMedium
                    visible: opacity > 0.0
                    opacity: categoryField.focus ? 1.0 : 0.0
                    Behavior on opacity { FadeAnimator { } }

                    Repeater {
                        id: categoryRepeater
                        model: notesApi.categories
                        BackgroundItem {
                            id: categoryBackground
                            width: categoryRectangle.width
                            height: categoryRectangle.height
                            Rectangle {
                                id: categoryRectangle
                                width: categoryLabel.width + Theme.paddingLarge
                                height: categoryLabel.height + Theme.paddingSmall
                                color: "transparent"
                                border.color: Theme.highlightColor
                                radius: height / 4
                                Label {
                                    id: categoryLabel
                                    anchors.centerIn: parent
                                    text: modelData
                                    color: categoryBackground.highlighted ? Theme.highlightColor : Theme.primaryColor
                                    font.pixelSize: Theme.fontSizeSmall
                                }
                            }
                            onClicked: categoryField.text = modelData
                        }
                    }
                }
            }

            Row {
                x: Theme.horizontalPageMargin
                width: parent.width - x
                IconButton {
                    id: favoriteButton
                    property bool selected: false
                    width: Theme.iconSizeMedium
                    icon.source: (selected ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: {
                        notesApi.updateNote(note["id"], {'favorite': selected, 'modified': new Date().valueOf() / 1000 })
                    }
                }
                TextField {
                    id: categoryField
                    width: parent.width - favoriteButton.width
                    placeholderText: qsTr("No category")
                    label: qsTr("Category")
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: {
                        categoryField.focus = false
                    }
                    onFocusChanged: {
                        if (focus === false && text !== note["category"]) {
                            notesApi.updateNote(id, {'content': note["content"], 'category': text, 'modified': new Date().valueOf() / 1000}) // This does not seem to work without adding the content
                        }
                    }
                }
            }

            DetailItem {
                id: modifiedDetail
                label: qsTr("Modified")
                property int modified
                onModifiedChanged: value = new Date(modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
