import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

ListModel {
    property url url
    property string json
    property string name
    property string file: StandardPaths.data + "/" + name + ".json"
    property bool saveFile: false
    property bool busy: false
    property int status: 200
    property date lastUpdate: new Date(0)

    onJsonChanged: refresh()

    function flush() {
        json = ""
        var filePut = new XMLHttpRequest
        filePut.open("PUT", file)
        filePut.send(json)
        clear()
        lastUpdate = new Date(0)
        status = 200
    }

    function refresh() {
        search("")
    }

    function search(query) {
        clear()
        var elements = parseJson()
        for (var element in elements) {
            elements[element].section = ""
            var match = false
            for (var child in elements[element]) {
                if (elements[element][child]) {
                    match = (elements[element][child].toString().toLowerCase().indexOf(query) >= 0) || match
                }
            }
            if (query === "" || match)
                append(elements[element])
        }
    }

    function parseJson() {
        var elements = JSON.parse(json)
        if (elements === null) {
            console.log("Error parsing " + name + "-JSON")
            elements = ""
            json = ""
            return null
        }
        else {
            clear()
            return elements
        }
    }

    function update() {
        busy = true
        var apiReq = new XMLHttpRequest
        apiReq.open("GET", url, true)//, appSettings.username, appSettings.password)
        apiReq.setRequestHeader('User-Agent', 'SailfishOS/harbour-lidderbuch')
        apiReq.setRequestHeader('OCS-APIRequest', 'true')
        apiReq.setRequestHeader("Authorization", "Basic " + Qt.btoa(appSettings.username + ":" + appSettings.password))
        apiReq.onreadystatechange = function() {
            if (apiReq.readyState === XMLHttpRequest.DONE) {
                if (apiReq.status === 200) {
                    console.log("Successfully loaded " + url)
                    json = apiReq.responseText
                    //console.log(json)
                    if (saveFile) {
                        var filePut = new XMLHttpRequest
                        filePut.open("PUT", file)
                        filePut.send(json)
                    }
                    lastUpdate = new Date()
                }
                else {
                    console.log("Error loading " + url + " - " + apiReq.status)
                   //lastUpdate = new Date(0)
                }
                status = apiReq.status
                busy = false
            }
        }
        apiReq.send()
    }

    Component.onCompleted: {
        if (saveFile) {
            if (name === "") {
                saveFile = false
            }
            else {
                busy = true
                var fileReq = new XMLHttpRequest
                fileReq.open("GET", file)
                fileReq.onreadystatechange = function() {
                    if (fileReq.readyState === XMLHttpRequest.DONE) {
                        if (fileReq.responseText === "") {
                            update()
                        }
                        else {
                            console.log("Loaded " + name + " from local JSON file")
                            json = fileReq.responseText
                            busy = false
                        }
                    }
                }
                fileReq.send()
            }
        }
    }
}
