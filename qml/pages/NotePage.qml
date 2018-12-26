import "../js/showdown-1.9.0/dist/showdown.js" as ShowDown
import QtQuick 2.4
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog

    property var note
    property int noteID

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


    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties:  { note: note }
    onAccepted: {
        acceptDestinationInstance.note = note
        acceptDestinationInstance.reloadContent()
    }
    onStatusChanged: {
        if (status === DialogStatus.Opened) {
            api.getNoteFromApi(noteID)
        }
    }
    Component.onCompleted: {
        noteID = note.id
        parseContent()
    }
    Connections {
        target: api
        onNoteChanged: {
            console.log("Some note changed")
            if (id === noteID) {
                console.log("This note changed")
                reloadContent()
            }
            else console.log("Other note changed")
        }
    }

    function reloadContent() {
        note = api.getNote(note.id)
        dialogHeader.title = note.title
        favoriteButton.selected = note.favorite
        categoryField.text = note.category
        modifiedDetail.modified = note.modified
        parseContent()
    }

    function parseContent() {
        //note = api.getNoteFromApi(note.id, false)
        var convertedText = converter.makeHtml(note.content)
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
                "ul,ol,table,img { margin: " + Theme.paddingLarge + "px 0px; }\n" +
                "a:link { color: " + Theme.primaryColor + "; }\n" +
                "a.checkbox { text-decoration: none; padding: " + Theme.paddingSmall + "px; display: inline-block; }\n" +
                "li.tasklist { font-size:large; margin: " + Theme.paddingMedium + "px 0px; }\n" +
                "table { border-color: " + Theme.secondaryColor + "; }\n" +
                "</style>\n" + convertedText
        if (debug) console.log(contentLabel.text)
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
                busy: api.busy

                MenuItem {
                    text: qsTr("Delete")
                    onClicked: remorse.execute("Deleting", function() { api.deleteNote(note.id) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: !api.busy
                    onClicked: api.getNoteFromApi(noteID)
                }
                MenuLabel {
                    visible: appSettings.currentAccount.length >= 0
                    text: qsTr("Last update") + ": " + (
                              new Date(api.update).valueOf() !== 0 ?
                                  new Date(api.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                  qsTr("never"))
                }
            }

            DialogHeader {
                id: dialogHeader
                title: note.title
                acceptText: qsTr("Edit")
                cancelText: qsTr("Notes")
            }
            BusyIndicator {
                parent: dialogHeader.extraContent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                size: BusyIndicatorSize.Medium
                running: api.busy
            }

            Column {
                width: parent.width
                spacing: Theme.paddingLarge

                Separator {
                    width: parent.width
                    color: Theme.primaryColor
                    horizontalAlignment: Qt.AlignHCenter
                }

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
                        var newContent = note.content
                        if (/^tasklist:checkbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[ \] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return (appSettings.useCapitalX ? '- [X] ' : '- [x] ') + p1 }
                                                                else { return match }
                                                            } )
                            note.content = newContent
                            parseContent()
                            api.updateNote(note.id, { 'content': note.content } )
                        }
                        else if (/^tasklist:uncheckbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[[xX]\] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return '- [ ] ' + p1 }
                                                                else { return match }
                                                            } )
                            note.content = newContent
                            parseContent()
                            api.updateNote(note.id, { 'content': note.content } )
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
                        model: api.categories
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
                    property bool selected: note.favorite
                    width: Theme.iconSizeMedium
                    icon.source: (selected ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: {
                        api.updateNote(note.id, {'favorite': !note.favorite})
                    }
                }
                TextField {
                    id: categoryField
                    width: parent.width - favoriteButton.width
                    text: note.category
                    placeholderText: qsTr("No category")
                    label: qsTr("Category")
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: {
                        categoryField.focus = false
                    }
                    onFocusChanged: {
                        if (focus === false && text !== note.category) {
                            api.updateNote(note.id, {'content': note.content, 'category': text}) // This does not seem to work without adding the content
                        }
                    }
                }
            }

            DetailItem {
                id: modifiedDetail
                label: qsTr("Modified")
                property int modified: note.modified
                value: new Date(modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
