import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var note

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Markdown Cheatsheet")
                onClicked: pageStack.push(Qt.resolvedUrl("MarkDownPage.qml"))
            }
        }

        Column {
            id: column
            width: parent.width// - 2*x

            PageHeader {
                title: note.title
            }

            TextArea {
                width: parent.width
                text: note.content
            }
        }

        VerticalScrollDecorator {}
    }
}
