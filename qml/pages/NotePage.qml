import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var note
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
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            quickSelect: true
            MenuItem {
                text: qsTr("Edit")
                onClicked: pageStack.push(Qt.resolvedUrl("EditPage.qml"), { note: note } )
            }
        }

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: note.title
            }

            LinkedLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                textFormat: Text.StyledText
                text: note.content
                Component.onCompleted: {
                    for (var i=0; i < markdown.length; i++) {
                        text = text.replace(markdown[i].regex, markdown[i].replace)
                    }
                    console.log(text)
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
