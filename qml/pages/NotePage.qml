import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    property int id
    property string etag
    property date modified
    property string title
    property string category
    property bool favorite
    property string content
    property bool error
    property string errorMessage


    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            //x: Theme.horizontalPageMargin
            width: parent.width// - 2*x

            PageHeader {
                title: page.title
            }

            TextArea {
                id: note
                width: parent.width
                readOnly: true
                text: page.content
            }
        }
        VerticalScrollDecorator {}
    }
}
