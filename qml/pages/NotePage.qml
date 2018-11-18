import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: noteDialog

    acceptDestination: Qt.resolvedUrl("EditPage.qml")
    acceptDestinationProperties: { account: account; noteIndex: noteIndex }
    Component.onCompleted: acceptDestinationProperties = { account: account, noteIndex: noteIndex }
    onStatusChanged: {
        if (status === PageStatus.Active) {
            dialogHeader.title = account.model.get(noteIndex).title
            contentLabel.plainText = account.model.get(noteIndex).content
            contentLabel.parse()
        }
    }
    Connections {
        target: account.model.get(noteIndex)
        onTitleChanged: dialogHeader.title = account.model.get(noteIndex).title
        onContentChanged: {
            contentLabel.plainText = account.model.get(noteIndex).content
            contentLabel.parse()
        }
    }

    property var account
    property int noteIndex

    property var markdown: [
        { regex: new RegExp(/^#\s(.*)$/gm), replace: '<h1>$1</h1>' },
        { regex: new RegExp(/^##\s(.*)$/gm), replace: '<h2>$1</h2>' },
        { regex: new RegExp(/^###\s(.*)$/gm), replace: '<h3>$1</h3>' },
        { regex: new RegExp(/^####\s(.*)$/gm), replace: '<h4>$1</h4>' },
        { regex: new RegExp(/^#####\s(.*)$/gm), replace: '<h5>$1</h5>' },
        { regex: new RegExp(/^######\s(.*)$/gm), replace: '<h6>$1</h6>' },
        { regex: new RegExp(/^-\s(.*)$/gm), replace: '<ul><li>$1</li></ul>' },
        { regex: new RegExp(/^\d{1,}.\s(.*)$/gm), replace: '<ol><li>$1</li></ol>' },
        //{ regex: new RegExp(/(<li class="ul">[\s\S]*<\/li>)/igm), replace: '<ul>$1</ul>' },
        //{ regex: new RegExp(/(<li class="ol">[\s\S]*<\/li>)/igm), replace: '<ol>$1</ol>' }
    ]

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            DialogHeader {
                id: dialogHeader
                acceptText: qsTr("Edit")
                cancelText: qsTr("Notes")
            }

            LinkedLabel {
                id: contentLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                textFormat: Text.StyledText

                function parse() {
                    var lines = plainText.split('\n')
                    lines.splice(0,1);
                    var tmpText = lines.join('\n');
                    for (var i=0; i < markdown.length; i++) {
                        tmpText = tmpText.replace(markdown[i].regex, markdown[i].replace)
                    }
                    text = tmpText
                    console.log(text)
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
