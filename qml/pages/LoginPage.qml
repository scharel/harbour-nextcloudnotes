import QtQuick 2.5
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: loginPage

    property string accountId

    property bool legacyLoginPossible: false
    property bool flowLoginV2Possible: false

    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + accountId

        property string name: value("name", qsTr("Nextcloud Login"), String)
        property url server: value("server", "", String)
        property string version: value("version", "v0.2", String)
        property string username: value("username", "", String)
        property string password: account.value("password", "", String)
        property bool doNotVerifySsl: account.value("doNotVerifySsl", false, Boolean)
        property bool allowUnecrypted: account.value("allowUnecrypted", false, Boolean)

        Component.onCompleted: {
            pageHeader.title = name
            serverField.text = server ? server : allowUnecrypted ? "http://" : "https://"
            usernameField.text = username
            passwordField.text = password
            unsecureConnectionTextSwitch.checked = doNotVerifySsl
            unencryptedConnectionTextSwitch.checked = allowUnecrypted
            if (username !== "" && password !== "") {
                notesApi.server = server
                notesApi.username = username
                notesApi.password = password
                notesApi.verifySsl = !doNotVerifySsl
                notesApi.verifyLogin()
            }
        }
    }

    onStatusChanged: {
        if (status === PageStatus.Activating)
            notesApi.getNcStatus()
        if (status === PageStatus.Deactivating)
            notesApi.abortFlowV2Login()
    }

    Connections {
        target: notesApi
        onStatusInstalledChanged: {
            if (notesApi.statusInstalled)
                serverField.focus = false
            else {
                pageHeader.title
            }
        }
        onStatusVersionChanged: {
            if (notesApi.statusVersion) {
                if (notesApi.statusVersion.split('.')[0] >= 16) {
                    legacyLoginPossible = false
                    flowLoginV2Possible = true
                    console.log("Using Flow Login v2")
                }
                else {
                    legacyLoginPossible = true
                    flowLoginV2Possible = false
                    console.log("Using Legacy Login")
                }
            }
            else {
                legacyLoginPossible = false
                flowLoginV2Possible = false
            }
        }
        onStatusVersionStringChanged: {
            if (notesApi.statusVersionString)
                pageHeader.description = "Nextcloud " + notesApi.statusVersionString
        }
        onStatusProductNameChanged: {
            if (notesApi.statusProductName) {
                pageHeader.title = notesApi.statusProductName
                account.name = notesApi.statusProductName
            }
        }
        onLoginStatusChanged: {
            switch(notesApi.loginStatus) {
            case NotesApi.LoginLegacyReady:
                apiProgressBar.label = qsTr("Enter your credentials")
                break;
            //case NotesApi.LoginFlowV2Initiating:
            //    break;
            case NotesApi.LoginFlowV2Polling:
                apiProgressBar.label = qsTr("Follow the instructions in the browser")
                break;
            case NotesApi.LoginFlowV2Success:
                notesApi.verifyLogin()
                break;
            case NotesApi.LoginFlowV2Failed:
                apiProgressBar.label = qsTr("Login failed!")
                break
            case NotesApi.LoginSuccess:
                apiProgressBar.label = qsTr("Login successfull!")
                account.username = notesApi.username
                account.password = notesApi.password
                appSettings.currentAccount = accountId
                break;
            case NotesApi.LoginFailed:
                apiProgressBar.label = qsTr("Login failed!")
                break;
            default:
                apiProgressBar.label = ""
                break;
            }
        }
        onLoginUrlChanged: {
            if (notesApi.loginUrl) {
                Qt.openUrlExternally(notesApi.loginUrl)
            }
            else {
                console.log("Login successfull")
            }
        }
        onServerChanged: {
            if (notesApi.server) {
                console.log("Login server: " + notesApi.server)
                account.server = notesApi.server
                serverField.text = notesApi.server
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: pageHeader
            }

            Image {
                id: nextcloudLogoImage
                anchors.horizontalCenter: parent.horizontalCenter
                height: Theme.itemSizeHuge
                fillMode: Image.PreserveAspectFit
                source: "../img/nextcloud-logo-transparent.png"
            }

            ProgressBar {
                id: apiProgressBar
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                indeterminate: notesApi.loginStatus === NotesApi.LoginFlowV2Initiating ||
                               notesApi.loginStatus === NotesApi.LoginFlowV2Polling
            }

            Row {
                width: parent.width
                TextField {
                    id: serverField
                    width: parent.width - statusIcon.width - Theme.horizontalPageMargin
                    placeholderText: qsTr("Nextcloud server")
                    label: placeholderText
                    validator: RegExpValidator { regExp: unencryptedConnectionTextSwitch.checked ? /^https?:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/: /^https:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/ }
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    onClicked: if (text === "") text = "https://"
                    onTextChanged: {
                        statusBusyIndicatorTimer.restart()
                        if (acceptableInput)
                            notesApi.server = text
                    }
                    //EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: legacyLoginPossible ? "image://theme/icon-m-enter-next" : flowLoginV2Possible ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter-close"
                    EnterKey.onClicked: {
                        if (legacyLoginPossible)
                            usernameField.focus = true
                        else if (flowLoginV2Possible && notesApi.loginStatus !== NotesApi.LoginFlowV2Polling)
                            notesApi.initiateFlowV2Login()
                        focus = false
                    }
                }
                Icon {
                    id: statusIcon
                    highlighted: serverField.highlighted
                    source: notesApi.statusInstalled ? "image://theme/icon-m-acknowledge" : "image://theme/icon-m-question"
                    BusyIndicator {
                        anchors.centerIn: parent
                        size: BusyIndicatorSize.Medium
                        running: notesApi.ncStatusStatus === NotesApi.NextcloudBusy || (serverField.focus && statusBusyIndicatorTimer.running && !notesApi.statusInstalled)
                        Timer {
                            id: statusBusyIndicatorTimer
                            interval: 200
                        }
                    }
                }
            }

            Column {
                id: flowv2LoginColumn
                width: parent.width
                spacing: Theme.paddingLarge
                visible: opacity !== 0.0
                opacity: flowLoginV2Possible ? 1.0 : 0.0
                Behavior on opacity { FadeAnimator {} }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: notesApi.loginStatus === NotesApi.LoginFlowV2Polling ? qsTr("Abort") : notesApi.loginStatus === NotesApi.LoginSuccess ? qsTr("Re-Login") : qsTr("Login")
                    onClicked: notesApi.loginStatus === NotesApi.LoginFlowV2Polling ? notesApi.abortFlowV2Login() : notesApi.initiateFlowV2Login()
                }
            }

            Column {
                id: legacyLoginColumn
                width: parent.width
                visible: opacity !== 0.0
                opacity: legacyLoginPossible ? 1.0 : 0.0
                Behavior on opacity { FadeAnimator {} }
                TextField {
                    id: usernameField
                    width: parent.width
                    placeholderText: qsTr("Username")
                    label: placeholderText
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    errorHighlight: text.length === 0// && focus === true
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked: passwordField.focus = true
                }
                PasswordField {
                    id: passwordField
                    width: parent.width
                    placeholderText: qsTr("Password")
                    label: placeholderText
                    errorHighlight: text.length === 0// && focus === true
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: notesApi.verifyLogin(usernameField.text, passwordField.text)
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Test Login")
                    onClicked: notesApi.verifyLogin(usernameField.text, passwordField.text)
                }
            }

            SectionHeader {
                text: qsTr("Note")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                linkColor: Theme.secondaryHighlightColor
                text: qsTr("The <a href=\"https://apps.nextcloud.com/apps/notes\">Notes</a> app needs to be installed on the Nextcloud server for this app to work.")
            }

            SectionHeader {
                text: qsTr("Security")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: qsTr("<strong>CAUTION: Your password will be saved without any encryption on the device!</strong><br>Please consider creating a dedicated app password! Open your Nextcloud in a browser and go to <i>Settings</i> → <i>Security</i>.")
            }
            TextSwitch {
                id: unsecureConnectionTextSwitch
                text: qsTr("Do not check certificates")
                description: qsTr("Enable this option to allow selfsigned certificates")
                onCheckedChanged: {
                    account.doNotVerifySsl = checked
                    notesApi.verifySsl = !account.doNotVerifySsl
                }
            }
            TextSwitch {
                id: unencryptedConnectionTextSwitch
                automaticCheck: false
                text: qsTr("Allow unencrypted connections")
                description: qsTr("")
                onClicked: {
                    if (checked) {
                        checked = false
                    }
                    else {
                        var dialog = pageStack.push(Qt.resolvedUrl("UnencryptedDialog.qml"))
                        dialog.accepted.connect(function() {
                            checked = true
                        })
                        dialog.rejected.connect(function() {
                            checked = false
                        })
                    }
                }
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
