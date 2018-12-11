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
                title: qsTr("Markdown Syntax")
            }

            SectionHeader {
                text: qsTr("Header")
            }
            Drawer {
                id: headerDrawer
                width: parent.width
                background: LinkedLabel {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    textFormat: Text.RichText
                    linkColor: Theme.primaryColor
                    text: "Formatted Text"
                }
                BackgroundItem {
                    anchors.fill: parent
                    onClicked: headerDrawer.open = !headerDrawer.open
                    Label {
                        text: "Raw Text"
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
