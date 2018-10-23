import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property string name
    property var account
    property var model: ListModel { }

    property string json
    property string file: StandardPaths.data + "/" + name + ".json"
    property bool saveFile: false
    property bool busy: false
    //property date lastUpdate: new Date(0)

    function callApi(method, data) {
        busy = true

        var endpoint = account.server + "/index.php/apps/notes/api/v0.2/notes"
        if (data && (method === "GET" || method === "PUT" || method === "DELETE"))
            if (data.id)
                endpoint = endpoint + "/" + data.id

        var apiReq = new XMLHttpRequest
        apiReq.open(method, endpoint, true)
        apiReq.setRequestHeader('User-Agent', 'SailfishOS/harbour-nextcloudnotes')
        apiReq.setRequestHeader('OCS-APIRequest', 'true')
        apiReq.setRequestHeader("Content-Type", "application/json")
        apiReq.setRequestHeader("Authorization", "Basic " + Qt.btoa(account.username + ":" + account.password))
        apiReq.onreadystatechange = function() {
            if (apiReq.readyState === XMLHttpRequest.DONE) {
                if (apiReq.status === 200) {
                    console.log("Successfull request!")
                    console.log(apiReq.responseText)
                    // TODO handle non arrays
                    var elements = JSON.parse(apiReq.responseText)
                    for (var element in elements) {
                        model.append(elements[element])
                    }
                }/*
                else if (apiReq.status === 304) {
                    console.log("ETag does not differ!")
                }
                else if (apiReq.status === 401) {
                    console.log("Unauthorized!")
                }
                else if (apiReq.status === 404) {
                    console.log("Note does not exist!")
                }*/
                else {
                    console.log("Networking error: " + apiReq.statusText + " (" + apiReq.status + ")")
                }
                busy = false
            }
        }
        if (method === "GET") {
            apiReq.send()
        }
        else if (method === "POST" || method === "PUT" || method === "DELETE") {
            apiReq.send(JSON.stringify(data))
        }
        else {
            console.log("Unsupported method: " + method)
            apiReq.abort()
        }
    }

    function getNotes() {
        callApi("GET")
    }

    function getNote(id) {
        if (id)
            callApi("GET", { 'id': id } )
    }

    function createNote(data) {
        callApi("POST", data)
    }

    function updateNote(id, data) {
        if (id) {
            data.id = id
            callApi("PUT", data)
        }
    }

    function deleteNote(id) {
        if (id)
            callApi("DELETE", { 'id': id } )
    }

    onJsonChanged: refresh()

    function flush() {
        json = ""
        var filePut = new XMLHttpRequest
        filePut.open("PUT", file)
        filePut.send(json)
        model.clear()
        account.lastUpdate = new Date(0)
        status = 200
    }

    function refresh() {
        search("")
    }

    function search(query) {
        model.clear()
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
                model.append(elements[element])
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
            model.clear()
            return elements
        }
    }

    /*Component.onCompleted: {
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
    }*/
}
