import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Item {
    property string response
    property var categories: [ ]
    property string file: StandardPaths.data + "/" + appSettings.currentAccount + ".json"
    property bool saveFile: false
    property bool busy: jobsRunning > 0
    property int jobsRunning: 0
    property int status: 0 //204
    property string statusText: "No Content"

    onStatusChanged: {
        //console.log("Network response: " + statusText + " (" + status + ")")
    }

    /*function getNote(id) {
        var dict
        if (id) {
            for (var i = 0; i < model.count; i++) {
                dict = model.get(i)
                if (dict.id === id) {
                    return dict
                }
            }
        }
    }*/

    function apiCall(method, data) {
        jobsRunning++

        var endpoint = account.server + "/index.php/apps/notes/api/" + account.version + "/notes"
        if (data) {
            if (method === "POST" || method === "PUT") {
                console.log("Adding note...")
            }
            else if (data.id && method === "DELETE") {
                console.log("Deleting note...")
            }
            if (method === "GET" || method === "PUT" || method === "DELETE") {
                if (data.id) {
                    endpoint = endpoint + "/" + data.id
                }
            }
        }

        console.log("Calling " + endpoint)
        var apiReq = new XMLHttpRequest
        apiReq.open(method, endpoint, true)
        apiReq.setRequestHeader('User-Agent', 'SailfishOS/harbour-nextcloudnotes')
        apiReq.setRequestHeader('OCS-APIRequest', 'true')
        apiReq.setRequestHeader("Content-Type", "application/json")
        apiReq.setRequestHeader("Authorization", "Basic " + Qt.btoa(account.username + ":" + account.password))
        apiReq.withCredentials = true
        //apiReq.timeout = 5000
        apiReq.onreadystatechange = function() {
            if (apiReq.readyState === XMLHttpRequest.DONE) {
                statusText = apiReq.statusText
                status = apiReq.status
                if (apiReq.status === 200) {
                    response = apiReq.responseText
                    //console.log(response)
                    console.log("Network response: " + statusText + " (" + status + ")")
                }
                else if(apiReq.status === 0) {
                    statusText = qsTr("Unable to connect")
                }
                /*
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
                jobsRunning--
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

    function getNotesFromApi() {
        apiCall("GET")
    }

    function getNoteFromApi(id) {
        if (id) {
            apiCall("GET", { 'id': id } )
        }
    }

    function createNote(data) {
        if (data)
            apiCall("POST", data)
    }

    function updateNote(id, data) {
        if (id && data) {
            data.id = id
            apiCall("PUT", data)
        }
    }

    function deleteNote(id) {
        if (id)
            apiCall("DELETE", { 'id': id } )
    }

    // source: https://stackoverflow.com/a/14339782
    /*function getPrettyDate(date) {
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
        } else if ((today.getTime() - compDate.getTime()) === (24 * 60 * 60 * 1000)) {
            return qsTr("Yesterday")
        } else if ((today.getTime() - compDate.getTime()) <= (7 * 24 * 60 * 60 * 1000)) {
            return compDate.toLocaleDateString(Qt.locale(), "dddd")
        } else if (today.getFullYear() === compDate.getFullYear()) {
            return compDate.toLocaleDateString(Qt.locale(), "MMMM")
        } else {
            return compDate.toLocaleDateString(Qt.locale(), "MMMM yyyy")
        }
    }*/

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
