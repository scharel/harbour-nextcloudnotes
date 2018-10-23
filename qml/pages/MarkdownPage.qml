import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Markdown Cheatsheet")
            }
        }

        VerticalScrollDecorator {}
    }

    ViewPlaceholder {
        enabled: true // TODO
        text: qsTr("Markdown Cheatsheet")
        hintText: qsTr("https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet")
    }
}
