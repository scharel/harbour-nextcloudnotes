import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Item {
    property string uuid
    property string name
    property url server
    property url url
    property string version: "v1"
    property string username
    property string password
    property date update
    property bool unsecureConnection
    property bool unencryptedConnection

    property var modelData: [ ]
    property var model: ListModel { }
    property var categories: [ ]
    property string file: StandardPaths.data + "/" + uuid + ".json"
    property bool saveFile: false
    property bool busy: false
    property bool searchActive: false
    property int status: 204
    property string statusText: "No Content"

    Component.onCompleted: {
        refreshConfig()
    }

    onStatusChanged: {
        console.log("Network status: " + statusText + " (" + status + ")")
    }

    onUuidChanged: {
        account.setValue("uuid", uuid)
        onUuidChanged: console.log("Account : " + uuid)
        account.path = "/apps/harbour-nextcloudnotes/accounts/" + uuid
        refreshConfig()
        modelData = []
        model.clear()
        appSettings.currentAccount = uuid
        //getNotes()
    }
    onNameChanged: account.setValue("name", name)
    onServerChanged: account.setValue("server", server)
    onUsernameChanged: account.setValue("username", username)
    onPasswordChanged: account.setValue("password", password)
    onUpdateChanged: account.setValue("update", update)
    onUnsecureConnectionChanged: account.setValue("unsecureConnection", unsecureConnection)
    onUnencryptedConnectionChanged: account.setValue("unencryptedConnection", unencryptedConnection)

    Connections {
        target: appSettings
        onSortByChanged: _mapDataToModel()
    }

    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + uuid
        onValuesChanged: refreshConfig()
    }

    function refreshConfig() {
        account.sync()
        name = account.value("name", "", String)
        server = account.value("server", "", String)
        url = server + "/index.php/apps/notes/api/" + version + "/notes"
        username = account.value("username", "", String)
        password = account.value("password", "", String)
        update = account.value("update", "", Date)
        unsecureConnection = account.value("unsecureConnection", false, Boolean)
        unencryptedConnection = account.value("unencryptedConnection", false, Boolean)
    }

    function clear() {
        modelData = []
        model.clear()
        account.clear()
    }

    function _APIcall(method, data) {
        busy = true

        var endpoint = url
        if (data && (method === "GET" || method === "PUT" || method === "DELETE")) {
            if (data.id) {
                endpoint = endpoint + "/" + data.id
            }
        }

        console.log("Calling " + endpoint)
        var apiReq = new XMLHttpRequest
        apiReq.open(method, endpoint, true)
        apiReq.setRequestHeader('User-Agent', 'SailfishOS/harbour-nextcloudnotes')
        apiReq.setRequestHeader('OCS-APIRequest', 'true')
        apiReq.setRequestHeader("Content-Type", "application/json")
        apiReq.setRequestHeader("Authorization", "Basic " + Qt.btoa(username + ":" + password))
        apiReq.withCredentials = true
        apiReq.onreadystatechange = function() {
            if (apiReq.readyState === XMLHttpRequest.DONE) {
                if (apiReq.status === 200) {

                    var json = JSON.parse(apiReq.responseText)
                    switch(method) {
                    case "GET":
                        if (Array.isArray(json)) {
                            console.log("Received all notes via API: " + endpoint)
                            modelData = json
                            _mapDataToModel()
                            update = new Date()
                        }
                        else {
                            console.log("Received a single note via API: " + endpoint)
                            _addToModelData(json)
                        }
                        break;
                    case "POST":
                        console.log("Created a note via API: " + endpoint)
                        _addToModelData(json)
                        json['isnew'] = true
                        pageStack.push(Qt.resolvedUrl("NotePage.qml"), { note: json } )
                        pageStack.completeAnimation()
                        pageStack.navigateForward()
                        break;
                    case "PUT":
                        console.log("Updated a note via API: " + endpoint)
                        _addToModelData(json)
                        break;
                    case "DELETE":
                        console.log("Deleted a note via API: " + endpoint)
                        _removeFromModelData(data.id)
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

    function getNotes(refresh) {
        if (typeof(refresh) === 'undefined')
            refresh = true
        if (refresh)
            _APIcall("GET")
        return modelData
    }

    function getNote(id, refresh) {
        if (typeof(refresh) === 'undefined')
            refresh = true
        if (id) {
            if (refresh)
                _APIcall("GET", { 'id': id } )
            var note
            modelData.forEach(function(currentValue) {
                if (currentValue.id === id)
                    note = currentValue
            } )
            return note
        }
    }

    function createNote(data) {
        if (data)
            _APIcall("POST", data)
    }

    function updateNote(id, data) {
        if (id && data) {
            data.id = id
            _APIcall("PUT", data)
        }
    }

    function deleteNote(id) {
        if (id)
            _APIcall("DELETE", { 'id': id } )
    }

    function _addToModelData(data) {
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
        _mapDataToModel()
    }

    function _removeFromModelData(id) {
        modelData.forEach(function(currentValue, index, array) {
            if (currentValue.id === id) {
                modelData.splice(index, 1)
            }
        } )
        _mapDataToModel()
    }

    function _mapDataToModel() {
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
        if (!searchActive) {
            categories = [ ]
            for (var element in modelData) {
                if (categories.indexOf(modelData[element].category) === -1 && modelData[element].category !== "" && typeof(modelData[element].category) !== 'undefined') {
                    categories.push(modelData[element].category)
                }
                model.set(element, modelData[element])
            }
            element++
            while (model.count > element) {
                model.remove(element)
            }
        }
    }

    function search(query) {
        if (query !== "") {
            searchActive = true
            model.clear()
            for (var element in modelData) {
                if (modelData[element].title.toLowerCase().indexOf(query) >= 0 |
                    modelData[element].content.toLowerCase().indexOf(query) >= 0 |
                    modelData[element].category.toLowerCase().indexOf(query) >= 0) {
                    model.append(modelData[element])
                }
            }
        }
        else {
            searchActive = false
            _mapDataToModel()
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
