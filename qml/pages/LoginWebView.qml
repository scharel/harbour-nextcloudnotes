import QtQuick 2.2
import Sailfish.Silica 1.0

Page {
    id: loginWebView
    property string server
    property url ncurl: (account.allowUnecrypted ? "http://" : "https://") + server + "/index.php/login/flow"

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
        id: ncFlowWebView
        anchors.fill: parent

        //url: ncurl
        //experimental.userAgent: "SailfishBrowser 1 - Sailfish" //"Mozilla/5.0 (U; Linux; Maemo; Jolla; Sailfish; like Android 4.3) " + "AppleWebKit/" + wkversion + " (KHTML, like Gecko) WebPirate/" + version + " like Mobile Safari/" + wkversion + " (compatible)"
        onNavigationRequested: {
            console.log(url)
            if (url.toString().indexOf("nc://login") === 0) {
                var credentials = url.split("/", 1)
                console.log(credentials)
            }
        }

        header: PageHeader {
            title: ncFlowWebView.title
            description: loginWebView.ncurl
            BusyIndicator {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: ncFlowWebView.loading
            }
        }
    }
}
