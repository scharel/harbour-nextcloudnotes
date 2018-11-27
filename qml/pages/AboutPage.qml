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

            /*DetailItem {
                label: qsTr("Name")
                value: Qt.application.name
            }*/
            SectionHeader {
                text: "Nextcloud " + qsTr("Notes") + " v" + Qt.application.version
            }

            LinkedLabel {
                id: nextcloudnotesLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                text: qsTr("<p>This app is released under the MIT license.</p><p>The source code is available on <a href=\"https://github.com/scharel/harbour-nextcloudnotes\">GitHub</a>.</p>")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("MIT License")
                onClicked: pageStack.push(Qt.resolvedUrl("MITLicense.qml"))
            }

            SectionHeader {
                text: "ShowdownJS"
            }

            LinkedLabel {
                id: showdownLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                text: qsTr("<p>This app uses <a href=\"https://github.com/showdownjs/showdown/tree/1.9.0\">ShowdownJS v1.9</a> to convert Markdown to HTML.</p><p>Showdown 1.x is released under GPL 2.0.</p>")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("GPL 2.0 License")
                onClicked: pageStack.push(Qt.resolvedUrl("GPLLicense.qml"))
            }
        }

        ViewPlaceholder {
            enabled: false
            text: qsTr("Nextcloud Notes")
            hintText: qsTr("for Sailfish OS")
        }

        VerticalScrollDecorator {}
    }
}
