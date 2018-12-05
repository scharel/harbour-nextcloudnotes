import "../js/showdown-1.9.0/dist/showdown.js" as ShowDown
import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog

    property var account
    property var note

    property var showdown: ShowDown.showdown
    property var converter: new showdown.Converter(
                                {   simplifiedAutoLink: true,
                                    excludeTrailingPunctuationFromURLs: true,
                                    strikethrough: true,
                                    tables: true,
                                    tasklists: false, // this is handled by the function parseContent() because LinkedLabel HTML support is to basic
                                    parseImgDimensions: true,
                                    simpleLineBreaks: true,
                                    emoji: true } )

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties: { account: account; note: note }
    Component.onCompleted: {
        acceptDestinationProperties = { account: account, note: note }
        //showdown.setFlavor('original')
        parseContent()
    }
    Connections {
        target: account
        onBusyChanged: {
            if (account.busy === false) {
                note = account.getNote(note.id, false)
                acceptDestinationProperties = { account: account, note: note }
                parseContent()
            }
        }
    }

    function parseContent() {
        note = account.getNote(note.id, false)
        var convertedText = converter.makeHtml(note.content)
        var occurence = -1
        convertedText = convertedText.replace(/^<li>(<p>)?\[ \] (.*)(<.*)$/gmi,
                                              function(match, p1, p2, p3, offset) {
                                                  occurence++
                                                  return '<li class="tasklist"><a href="tasklist:checkbox_' + occurence + '">' + (p1 ? p1 : "") + '☐ ' + p2 + '</a>' + p3
                                              } )
        occurence = -1
        convertedText = convertedText.replace(/^<li>(<p>)?\[[xX]\] (.*)(<.*)$/gmi,
                                              function(match, p1, p2, p3, offset) {
                                                  occurence++
                                                  return '<li class="tasklist"><a href="tasklist:uncheckbox_' + occurence + '">' + (p1 ? p1 : "") + '☑ ' + p2 + '</a>' + p3
                                              } )
        convertedText = convertedText.replace("<table>", "<table border='1' cellpadding='" + Theme.paddingMedium + "'>")
        contentLabel.text = "<style>ul,ol,table,img { margin-bottom: " + Theme.paddingLarge + "px; margin-top: " + Theme.paddingLarge + "px; }\n" +
                "a:link { color: " + Theme.primaryColor + "; }\n" +
                "li.tasklist { font-size:large; margin-bottom: " + Theme.paddingMedium + "px; margin-top: " + Theme.paddingMedium + "px; }\n" +
                "table { border-color: " + Theme.secondaryColor + "; }</style>" + convertedText
        console.log(contentLabel.text)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: mainColumn.height

        Column {
            id: mainColumn
            width: parent.width

            RemorsePopup {
                id: remorse
                onTriggered: pageStack.pop()
            }
            PullDownMenu {
                busy: account ? account.busy : false

                MenuItem {
                    text: qsTr("Delete")
                    enabled: account ? true : false
                    onClicked: remorse.execute("Deleting", function() { account.deleteNote(notey.id) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: account ? !account.busy : false
                    onClicked: account.getNote(note.id)
                }
                MenuLabel {
                    visible: appSettings.currentAccount >= 0
                    text: account ? (
                                        qsTr("Last update") + ": " + (
                                            new Date(account.update).valueOf() !== 0 ?
                                                new Date(account.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                                                qsTr("never"))) : ""
                }
            }

            DialogHeader {
                id: dialogHeader
                dialog: noteDialog
                acceptText: qsTr("Edit")
                cancelText: qsTr("Notes")
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
                        console.log(link)
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
                            account.updateNote(note.id, { 'content': note.content } )
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
                            account.updateNote(note.id, { 'content': note.content } )
                        }
                        else {
                            Qt.openUrlExternally(link)
                        }
                    }
                }

                Separator {
                    id: separator
                    width: parent.width
                    color: Theme.primaryColor
                    horizontalAlignment: Qt.AlignHCenter
                }

                Column {
                    width: parent.width

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
                                account.updateNote(note.id, {'favorite': !note.favorite})
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
                                if (focus === false) {
                                    account.updateNote(note.id, {'content': note.content, 'category': text}) // This does not seem to work without adding the content
                                }
                            }
                        }
                    }

                    DetailItem {
                        id: modifiedDetail
                        label: qsTr("Modified")
                        value: new Date(note.modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
