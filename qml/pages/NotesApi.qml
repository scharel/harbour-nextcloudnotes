import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Item {
    property string uuid
    property string name
    property url server
    property string username
    property string password
    property date update
    property bool unsecureConnection
    property bool unencryptedConnection

    property var model: ListModel { }
    //property string file: StandardPaths.data + "/" + uuid + ".json"
    //property bool saveFile: false
    property bool busy: false

    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + uuid
    }

    Component.onCompleted: {
        name = account.value("name", "", String)
        server = account.value("server", "", String)
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

        var endpoint = server + "/index.php/apps/notes/api/v0.2/notes"
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
                    console.log("Successfull API request!")
                    //console.log(apiReq.responseText)

                    var json = JSON.parse(apiReq.responseText)
                    switch(method) {
                    case "GET":
                        if (Array.isArray(json)) {
                            console.log("Got all notes")
                            model.clear()
                            for (var element in json) {
                                model.append(json[element])
                            }
                            update = new Date()
                        }
                        else {
                            console.log("Got a single note")
                            for (var i = 0; i < model.count; i++) {
                                var listItem = model.get(i)
                                if (listItem.id === json.id){
                                    model.set(i, json)
                                }
                            }
                        }
                        break;
                    case "POST":
                        console.log("Created a note")
                        model.append(json)
                        model.move(model.count-1, 0, 1)
                        break;
                    case "PUT":
                        console.log("Updated a note")
                        for (var i = 0; i < model.count; i++) {
                            var listItem = model.get(i)
                            if (listItem.id === json.id){
                                model.set(i, json)
                            }
                        }
                        break;
                    case "DELETE":
                        console.log("Deleted a note")
                        for (var i = 0; i < model.count; i++) {
                            var listItem = model.get(i)
                            if (listItem.id === data.id){
                                model.remove(i)
                            }
                        }
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
                    console.log("Networking error: " + apiReq.statusText + " (" + apiReq.status + ")")
                }
                busy = false
            }
            else {
                //console.log("HTTP ready state: " + apiReq.readyState)
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
