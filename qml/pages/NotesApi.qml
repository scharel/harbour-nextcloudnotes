import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Item {
    property string uuid
    property string name
    property url server
    property url url
    property string version: "v0.2"
    property string username
    property string password
    property date update
    property bool unsecureConnection
    property bool unencryptedConnection

    property var modelData: [ ] // TODO use note id as key { note1.id: note1, note2.id: note2, ... }
    property var model: ListModel { }
    property string file: StandardPaths.data + "/" + uuid + ".json"
    property bool saveFile: false
    property bool busy: false
    property int status: 204
    property string statusText: "No Content"

    onStatusChanged: {
        console.log("Network status: " + statusText + " (" + status + ")")
    }

    Connections {
        target: appSettings
        onSortByChanged: mapDataToModel()
    }

    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + uuid
    }

    Component.onCompleted: {
        name = account.value("name", "", String)
        server = account.value("server", "", String)
        url = server + "/index.php/apps/notes/api/" + version + "/notes"
        username = account.value("username", "", String)
        password = account.value("password", "", String)
        update = account.value("update", "", Date)
        unsecureConnection = account.value("unsecureConnection", false, Boolean)
        unencryptedConnection = account.value("unencryptedConnection", false, Boolean)
    }

    onUuidChanged: account.setValue("uuid", uuid)
    onNameChanged: account.setValue("name", name)
    onServerChanged: account.setValue("server", server)
    onUsernameChanged: account.setValue("username", username)
    onPasswordChanged: account.setValue("password", password)
    onUpdateChanged: account.setValue("update", update)
    onUnsecureConnectionChanged: account.setValue("unsecureConnection", unsecureConnection)
    onUnencryptedConnectionChanged: account.setValue("unencryptedConnection", unencryptedConnection)

    function clear() {
        model.clear()
        account.clear()
    }

    function callApi(method, data) {
        busy = true

        var endpoint = url
        if (data && (method === "GET" || method === "PUT" || method === "DELETE")) {
            if (data.id) {
                endpoint = endpoint + "/" + data.id
            }
        }

        var apiReq = new XMLHttpRequest
        apiReq.open(method, endpoint, true)
        apiReq.setRequestHeader('User-Agent', 'SailfishOS/harbour-nextcloudnotes')
        apiReq.setRequestHeader('OCS-APIRequest', 'true')
        apiReq.setRequestHeader("Content-Type", "application/json")
        apiReq.setRequestHeader("Authorization", "Basic " + Qt.btoa(username + ":" + password))
        apiReq.onreadystatechange = function() {
            if (apiReq.readyState === XMLHttpRequest.DONE) {
                if (apiReq.status === 200) {

                    var json = JSON.parse(apiReq.responseText)
                    switch(method) {
                    case "GET":
                        if (Array.isArray(json)) {
                            console.log("Received all notes via API: " + endpoint)
                            modelData = json
                            mapDataToModel()
                            update = new Date()
                        }
                        else {
                            console.log("Received a single note via API: " + endpoint)
                            addToModelData(json)
                        }
                        break;
                    case "POST":
                        console.log("Created a note via API: " + endpoint)
                        addToModelData(json)
                        break;
                    case "PUT":
                        console.log("Updated a note via API: " + endpoint)
                        addToModelData(json)
                        break;
                    case "DELETE":
                        console.log("Deleted a note via API: " + endpoint)
                        removeFromModelData(data.id)
                        break;
                    default:
                        console.log("Unsupported method: " + method)
                        break;
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
                    //console.log("Network error: " + apiReq.statusText + " (" + apiReq.status + ")")
                }
                statusText = apiReq.statusText
                status = apiReq.status
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
        modelData.forEach(function(currentValue) {
            if (currentValue.id === id)
                return currentValue
        } )
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

    function addToModelData(data) {
        var dataUpdated = false
        modelData.forEach(function(currentValue, index, array) {
            if (currentValue.id === data.id) {
                array[index] = data
                dataUpdated = true
            }
        } )
        if (!dataUpdated) {
            modelData.push(data)
        }
        mapDataToModel()
    }

    function removeFromModelData(id) {
        modelData.forEach(function(currentValue, index, array) {
            if (currentValue.id === id) {
                modelData.splice(i, 1)
            }
        } )
        mapDataToModel()
    }

    function mapDataToModel() {
        modelData.forEach(function(value) { value.date = getPrettyDate(value.modified) } )
        switch(appSettings.sortBy) {
        case "date":
            modelData.sort(function(a, b) { return b.modified-a.modified } )
            break
        case "category":
            modelData.sort(function(a, b) { return b.modified-a.modified } )
            modelData.sort(function(a, b) { return ((a.category > b.category) ? 1 : ((b.category > a.category) ? -1 : 0)) } )
            break
        case "title":
            modelData.sort(function(a, b) { return b.modified-a.modified } )
            modelData.sort(function(a, b) { return ((a.title > b.title) ? 1 : ((b.title > a.title) ? -1 : 0)) } )
            break
        }
        for (var element in modelData) {
            model.set(element, modelData[element])
        }
        element++
        while (model.count > element) {
            model.remove(element)
        }
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

    // source: https://stackoverflow.com/a/14339782
    function getPrettyDate(date) {
        var today = new Date()
        today.setHours(0)
        today.setMinutes(0)
        today.setSeconds(0)
        today.setMilliseconds(0)
        var compDate = new Date(date*1000)
        compDate.setHours(0)
        compDate.setMinutes(0)
        compDate.setSeconds(0)
        compDate.setMilliseconds(0)
        if (compDate.getTime() === today.getTime()) {
            return qsTr("Today")
        } else if ((today.getTime() - compDate.getTime()) <= (24 * 60 * 60 *1000)) {
            return qsTr("Yesterday")
        } else {
            return compDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
        }
    }

    /*Component.onCompleted: {
        if (saveFile) {
            if (account.name === "") {
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
                            console.log("Loaded " + account.name + " from local JSON file")
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
