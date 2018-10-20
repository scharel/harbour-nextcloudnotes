import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property var note

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

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
