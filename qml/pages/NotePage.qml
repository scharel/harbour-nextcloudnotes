import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties: { account: account; noteIndex: noteIndex }
    Component.onCompleted: acceptDestinationProperties = { account: account, noteIndex: noteIndex }//acceptDestinationInstance.note = note

    property var account
    property int noteIndex

    property var markdown: [
        { regex: new RegExp(/(^#\s)(.*)$/gm), replace: '<h1>$2</h1>' },
        { regex: new RegExp(/(^##\s)(.*)$/gm), replace: '<h2>$2</h2>' },
        { regex: new RegExp(/(^###\s)(.*)$/gm), replace: '<h3>$2</h3>' },
        { regex: new RegExp(/(^####\s)(.*)$/gm), replace: '<h4>$2</h4>' },
        { regex: new RegExp(/(^#####\s)(.*)$/gm), replace: '<h5>$2</h5>' },
        { regex: new RegExp(/(^######\s)(.*)$/gm), replace: '<h6>$2</h6>' },
        { regex: new RegExp(/(^-\s)(.*)$/gm), replace: '<ul><li>$2</li></ul>' },
        { regex: new RegExp(/(^\d{1,}.\s)(.*)$/gm), replace: '<ol><li>$2</li></ol>' }
    ]

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            DialogHeader {
                title: account.model.get(noteIndex).title
                acceptText: qsTr("Edit")
                cancelText: qsTr("Notes")
            }

            LinkedLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                textFormat: Text.StyledText
                Component.onCompleted: {
                    var lines = account.model.get(noteIndex).content.split('\n')
                    lines.splice(0,1);
                    text = lines.join('\n');
                    for (var i=0; i < markdown.length; i++) {
                        text = text.replace(markdown[i].regex, markdown[i].replace)
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
