import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    Component {
        id: syntaxDrawer

        Drawer {
            id: drawer
            width: page.width
            height: rawTextLabel.height > convertedTextLabel.height ? rawTextLabel.height : convertedTextLabel.height
            backgroundSize: height
            foreground: BackgroundItem {
                width: parent.width
                height: parent.height
                onClicked: drawer.show()
                Label {
                    id: rawTextLabel
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    wrapMode: Text.Wrap
                    font.family: appSettings.useMonoFont ? "DejaVu Sans Mono" : Theme.fontFamily
                    text: rawText
                }
            }
            background: BackgroundItem {
                width: parent.width
                height: parent.height
                onClicked: drawer.hide()
                LinkedLabel {
                    id: convertedTextLabel
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2*x
                    textFormat: Text.RichText
                    linkColor: Theme.primaryColor
                    defaultLinkActions: false
                    onLinkActivated: drawer.hide()
                    text: "<style>\n" +
                          "ul,ol,table,img { margin: " + Theme.paddingLarge + "px 0px; }\n" +
                          "a:link { color: " + Theme.primaryColor + "; }\n" +
                          "a.checkbox { text-decoration: none; padding: " + Theme.paddingSmall + "px; display: inline-block; }\n" +
                          "li.tasklist { font-size:large; margin: " + Theme.paddingMedium + "px 0px; }\n" +
                          "del { text-decoration: line-through; }\n" +
                          "table { border-color: " + Theme.secondaryColor + "; }\n" +
                          "</style>\n" + convertedText
                }
            }
        }
    }

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
            Loader {
                sourceComponent: syntaxDrawer
                property string rawText: "
# Header 1
## Header 2
### Header 3
#### Header 4
##### Header 5
###### Header 6";
                property string convertedText: "
<h1>Header 1</h1>
<h2>Header 2</h2>
<h3>Header 3</h3>
<h4>Header 4</h4>
<h5>Header 5</h5>
<h6>Header 6</h6>"
            }

            SectionHeader {
                text: qsTr("Links")
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string rawText: "
https://jolla.com
info@jolla.com
[Homepage - Jolla](https://jolla.com)
[Link to Jolla][1]
[jolla][]

[1]: https://jolla.com
[jolla]: https://jolla.com"
                property string convertedText: "
<a href=\"https://jolla.com\">https://jolla.com</a><br>
<a href=\"mailto:info@jolla.com\">info@jolla.com</a><br>
<a href=\"https://jolla.com\">Homepage - Jolla</a><br>
<a href=\"https://jolla.com\">Homepage - Jolla</a><br>
<a href=\"https://jolla.com\">Homepage - Jolla</a>"
            }

            SectionHeader {
                text: qsTr("Tables")
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string rawText: "
|Header 1    |Header 2    |Header 3    |
|:-----------|:----------:|-----------:|
|Content 1.1 |Content 2.1 |Content 3.1 |
|Content 1.2 |Content 2.2 |Content 3.2 |
|Content 1.3 |Content 2.3 |Content 3.3 |"
                property string convertedText: "
<table>
<thead>
<tr><td>Header 1</td><td>Header 2</td><td>Header 3</td></tr>
</thead>
<tbody>
<tr><td>Content 1.1</td><td>Content 2.1</td><td>Content 3.1</td></tr>
<tr><td>Content 1.2</td><td>Content 2.2</td><td>Content 3.2</td></tr>
<tr><td>Content 1.3</td><td>Content 2.3</td><td>Content 3.3</td></tr>
</tbody>
</table>"
            }
        }

        VerticalScrollDecorator {}
    }

    allowedOrientations: defaultAllowedOrientations
}
