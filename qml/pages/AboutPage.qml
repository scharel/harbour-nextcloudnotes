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

            PageHeader {
                title: qsTr("About")
            }

            DetailItem {
                label: qsTr("Name")
                value: Qt.application.name
            }
            DetailItem {
                label: qsTr("Version")
                value: Qt.application.version
            }
            DetailItem {
                label: qsTr("License")
                value: "MIT"
            }

            LinkedLabel {
                id: showdownLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                text: qsTr("<p>This app uses <a href=\"https://github.com/showdownjs/showdown/tree/1.9.0\">ShowdownJS v1.9</a> as Javascript Markdown to HTML converter.</p><p>ShowdownJS v 2.0 is release under the MIT version. Previous versions are release under GPL 2.0</p>")
            }
        }

        ViewPlaceholder {
            enabled: true // TODO
            text: qsTr("Nextcloud Notes")
            hintText: qsTr("for Sailfish OS")
        }

        VerticalScrollDecorator {}
    }
}
