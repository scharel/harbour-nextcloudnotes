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
                title: qsTr("About")
            }
        }

        VerticalScrollDecorator {}
    }

    ViewPlaceholder {
        enabled: true // TODO
        text: qsTr("Nextcloud Notes")
        hintText: qsTr("for Sailfish OS")
    }
}
