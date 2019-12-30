import QtQuick 2.5
import Sailfish.Silica 1.0
import "../js/showdown/dist/showdown.js" as ShowDown

Dialog {
    id: noteDialog

    property int id
    property int modified
    property string title
    property string category
    property string content
    property bool favorite
    property string etag
    property bool error
    property string errorMessage

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
    acceptDestinationProperties: (
                                     {   id: id,
                                         modified: modified,
                                         title: title,
                                         category: category,
                                         content: content,
                                         favorite: favorite,
                                         etag: etag,
                                         error: error,
                                         errorMessage: errorMessage
                                     } )
    onAccepted: {
        //acceptDestinationInstance.note = note
        acceptDestinationInstance.reloadContent()
    }
    onStatusChanged: {
        if (status === DialogStatus.Opened) {
            //notesApi.getNoteFromApi(id)
        }
    }
    Component.onCompleted: {
        console.log(title)
        parseContent()
    }

    function reloadContent() {
        //notesApi.getNoteFromApi(id)
        /*note = notesApi.getNote(id)
        dialogHeader.title = title
        favoriteButton.selected = favorite
        categoryField.text = category
        modifiedDetail.modified = modified
        parseContent()*/
    }

    function parseContent() {
        //note = notesApi.getNoteFromApi(id, false)
        var convertedText = converter.makeHtml(content)
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
                    onClicked: remorse.execute("Deleting", function() { notesApi.deleteNote(id) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: !notesApi.busy
                    onClicked: notesApi.getNoteFromApi(noteID)
                }
                MenuLabel {
                    visible: appSettings.currentAccount.length >= 0
                    text: qsTr("Last update") + ": " + (
                              new Date(notesApi.update).valueOf() !== 0 ?
                                  new Date(notesApi.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                  qsTr("never"))
                }
            }

            DialogHeader {
                id: dialogHeader
                title: noteDialog.title
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
                        var newContent = content
                        if (/^tasklist:checkbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[ \] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return (appSettings.useCapitalX ? '- [X] ' : '- [x] ') + p1 }
                                                                else { return match }
                                                            } )
                            content = newContent
                            parseContent()
                            notesApi.updateNote(id, { 'content': content } )
                        }
                        else if (/^tasklist:uncheckbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/- \[[xX]\] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return '- [ ] ' + p1 }
                                                                else { return match }
                                                            } )
                            content = newContent
                            parseContent()
                            notesApi.updateNote(id, { 'content': content } )
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
                    property bool selected: favorite
                    width: Theme.iconSizeMedium
                    icon.source: (selected ? "image://theme/icon-m-favorite-selected?" : "image://theme/icon-m-favorite?") +
                                 (favoriteButton.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor)
                    onClicked: {
                        notesApi.updateNote(id, {'favorite': !favorite})
                    }
                }
                TextField {
                    id: categoryField
                    width: parent.width - favoriteButton.width
                    text: category
                    placeholderText: qsTr("No category")
                    label: qsTr("Category")
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: {
                        categoryField.focus = false
                    }
                    onFocusChanged: {
                        if (focus === false && text !== category) {
                            notesApi.updateNote(id, {'content': content, 'category': text}) // This does not seem to work without adding the content
                        }
                    }
                }
            }

            DetailItem {
                id: modifiedDetail
                label: qsTr("Modified")
                value: new Date(noteDialog.modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
