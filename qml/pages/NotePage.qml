import "../js/showdown-1.9.0/dist/showdown.js" as ShowDown
import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog

    property var account
    property var note

    property var showdown: ShowDown.showdown
    property var converter: new showdown.Converter(
                                { noHeaderId: true,
                                    simplifiedAutoLink: true,
                                    tables: true,
                                    tasklists: false, // this is handled by the function parseContent() because LinkedLabel HTML support is to basic
                                    simpleLineBreaks: true,
                                    emoji: true } )

    function parseContent() {
        note = account.getNote(note.id, false)
        //modifiedDetail.value = new Date(note.modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
        //favoriteDetail.value = note.favorite ? qsTr("yes") : qsTr("no")
        //categoryDetail.value = note.category
        var convertedText = converter.makeHtml(note.content)
        var occurence = -1
        convertedText = convertedText.replace(/^<li>\[ \] (.*)<\/li>$/gm,
                                              function(match, p1, offset) {
                                                  occurence++
                                                  return '<li><font size="' + 4 + '"><a href="tasklist:checkbox_' + occurence + '">☐ ' + p1 + '</a></font></li>'
                                              } )
        occurence = -1
        convertedText = convertedText.replace(/^<li>\[[xX]\] (.*)<\/li>$/gm,
                                              function(match, p1, offset) {
                                                  occurence++
                                                  return '<li><font size="' + 4 + '"><a href="tasklist:uncheckbox_' + occurence + '">☑ ' + p1 + '</a></font></li>'
                                              } )
        contentLabel.text = convertedText
        //console.log(contentLabel.text)
    }

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties: { account: account; note: note }
    Component.onCompleted: {
        parseContent()
        acceptDestinationProperties = { account: account, note: note }
    }
    Connections {
        target: account
        onBusyChanged: {
            if (account.busy === false) {
                note = account.getNote(note.id, false)
                parseContent()
            }
        }
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
                    //visible: appSettings.currentAccount >= 0
                    onClicked: remorse.execute("Deleting", function() { account.deleteNote(notey.id) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: account ? !account.busy : false
                    //visible: appSettings.currentAccount >= 0
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
                    textFormat: Text.StyledText
                    defaultLinkActions: false
                    onLinkActivated: {
                        var occurence = -1
                        var newContent = note.content
                        if (/^tasklist:checkbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/^- \[ \] (.*)$/gm,
                                                            function(match, p1, offset, string) {
                                                                occurence++
                                                                if (occurence === parseInt(link.split('_')[1])) {
                                                                    return '- [x] ' + p1 }
                                                                else { return match }
                                                            } )
                            note.content = newContent
                            parseContent()
                            account.updateNote(note.id, { 'content': note.content } )
                        }
                        else if (/^tasklist:uncheckbox_(\d+)$/m.test(link)) {
                            newContent = newContent.replace(/^- \[[xX]\] (.*)$/gm,
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

                DetailItem {
                    id: modifiedDetail
                    label: qsTr("Modified")
                    value: new Date(note.modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                }

                Separator {
                    id: separator
                    width: parent.width
                    color: Theme.primaryColor
                    horizontalAlignment: Qt.AlignHCenter
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
                /*DetailItem {
                        id: favoriteDetail
                        label: qsTr("Favorite")
                    }
                    DetailItem {
                        id: categoryDetail
                        label: qsTr("Category")
                        visible: value.length > 0
                    }*/
            }
        }

        VerticalScrollDecorator {}
    }
}
