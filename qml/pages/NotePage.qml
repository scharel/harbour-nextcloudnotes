import "../js/showdown-1.9.0/dist/showdown.js" as ShowDown
import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog
    property var showdown: ShowDown.showdown
    property var converter: new showdown.Converter( { noHeaderId: true, simplifiedAutoLink: true, tables: true, tasklists: false, simpleLineBreaks: true, emoji: true } )

    function reloadContent() {
        contentLabel.text = converter.makeHtml(account.model.get(noteIndex).content)
        //console.log(contentLabel.text)
    }

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties: { account: account; noteIndex: noteIndex }
    Component.onCompleted: acceptDestinationProperties = { account: account, noteIndex: noteIndex }
    onStatusChanged: {
        if (status === PageStatus.Active) {
            account.getNote(account.model.get(noteIndex).id)
            reloadContent()
        }
    }
    Connections {
        target: account/*.model.get(noteIndex)
        onTitleChanged: {
            console.log("Title changed")
            dialogHeader.title = account.model.get(noteIndex).title
        }
        onContentChanged: {
            console.log("Content changed")
            contentLabel.plainText = account.model.get(noteIndex).content
            contentLabel.parse()
        }*/
        onBusyChanged: {
            if (account.busy === false) {
                reloadContent()
            }
        }
    }

    property var account
    property int noteIndex

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
                    onClicked: remorse.execute("Deleting", function() { account.deleteNote(account.model.get(noteIndex).id) } )
                }
                MenuItem {
                    text: enabled ? qsTr("Reload") : qsTr("Updating...")
                    enabled: account ? !account.busy : false
                    //visible: appSettings.currentAccount >= 0
                    onClicked: account.getNote(account.model.get(noteIndex).id)
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
                    defaultLinkActions: true
                }

                Separator {
                    id: separator
                    width: parent.width
                    color: Theme.primaryColor
                    horizontalAlignment: Qt.AlignHCenter
                }

                Column {
                    width: parent.width

                    DetailItem {
                        label: qsTr("Modified")
                        value: new Date(account.model.get(noteIndex).modified * 1000).toLocaleString(Qt.locale(), Locale.ShortFormat)
                    }
                    DetailItem {
                        label: qsTr("Favorite")
                        value: account.model.get(noteIndex).favorite ? qsTr("yes") : qsTr("no")
                    }
                    DetailItem {
                        label: qsTr("Category")
                        value: account.model.get(noteIndex).category
                        visible: value.length > 0
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
