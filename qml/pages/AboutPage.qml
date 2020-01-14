import QtQuick 2.2
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: qsTr("About")
            }

            Label {
                id: debugModeLabel
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                visible: debug
                text: "<p><strong>" + qsTr("The app is running in DEBUG mode!") + "</strong></p>"
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
                text: "<p>" + qsTr("This app is released under the MIT license.") + "</p>" +
                                   //: Pre-Showdown Github link
                                   "<p>" + qsTr("The source code is available on") + " <a href=\"https://github.com/scharel/harbour-nextcloudnotes\">GitHub</a>" +
                      //: Post-Showdown Github link
                      qsTr(".") + "</p>"
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
                //: Pre-Showdown Github link
                text: "<p>" + qsTr("This app uses") + " <a href=\"https://github.com/showdownjs/showdown/tree/1.9.0\">ShowdownJS v1.9</a> " +
                      //: Post-Showdown Github link
                      qsTr("to convert Markdown to HTML.") + "</p>" +
                      "<p> " + qsTr("Showdown 1.x is released under GPL 2.0.") + "</p>"
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

    allowedOrientations: defaultAllowedOrientations
}
