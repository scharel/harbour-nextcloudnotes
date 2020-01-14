import QtQuick 2.2
import Sailfish.Silica 1.0

Page {
    id: loginWebView
    property url loginUrl

    Component.onCompleted: {
        var req = new XMLHttpRequest()
        req.open("GET", endpoint, true)
        req.setRequestHeader('User-Agent', 'SailfishOS/harbour-nextcloudnotes')
        req.setRequestHeader("OCS-APIREQUEST", "true")
        req.onreadystatechange = function() {
            if (req.readyState === XMLHttpRequest.DONE) {
                if (req.status === 200) {
                    ncFlowWebView.data = req.responseText
                }
            }
        }
        req.send()
    }

    SilicaWebView {
        id: ncFlow2WebView
        anchors.fill: parent

        url: loginUrl

        header: PageHeader {
            title: ncFlow2WebView.title
            description: url
            BusyIndicator {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: ncFlow2WebView.loading
            }
        }
    }
}
