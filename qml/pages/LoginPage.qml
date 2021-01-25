import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import NextcloudNotes 1.0

Dialog {
    id: loginDialog

    canAccept: false

    property string account
    property string peviousAccount: appSettings.currentAccount

    property bool legacyLoginPossible: false
    property bool flowLoginV2Possible: false

    property bool doNotVerifySsl: false
    property bool allowUnecrypted: false

    Component.onCompleted: {
        appSettings.currentAccount = null
    }

    onRejected: {
        notesApi.abortFlowV2Login()
        appSettings.currentAccount = peviousAccount
    }
    onAccepted: {
        appSettings.createAccount(notesApi.username, notesApi.password, notesApi.server, notesApi.statusProductName)
    }

    Timer {
        id: verifyServerTimer
        onTriggered: notesApi.getNcStatus()
    }

    Connections {
        target: notesApi
        onStatusInstalledChanged: {
            if (notesApi.statusInstalled)
                serverField.focus = false
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
        onStatusProductNameChanged: {
            if (notesApi.statusProductName) {
                productName = notesApi.statusProductName
                console.log(productName)
            }
            else {
                productName = null
            }
        }
        onLoginStatusChanged: {
            loginDialog.canAccept = false
            apiProgressBar.indeterminate = false
            switch(notesApi.loginStatus) {
            case NotesApi.LoginLegacyReady:
                console.log("LoginLegacyReady")
                apiProgressBar.label = qsTr("Enter your credentials")
                break;
            case NotesApi.LoginFlowV2Initiating:
                console.log("LoginFlowV2Initiating")
                apiProgressBar.indeterminate = true
                break;
            case NotesApi.LoginFlowV2Polling:
                console.log("LoginFlowV2Polling")
                apiProgressBar.label = qsTr("Follow the instructions in the browser")
                apiProgressBar.indeterminate = true
                break;
            case NotesApi.LoginFlowV2Success:
                console.log("LoginFlowV2Success")
                notesApi.verifyLogin()
                break;
            case NotesApi.LoginFlowV2Failed:
                console.log("LoginFlowV2Failed")
                apiProgressBar.label = qsTr("Login failed!")
                break
            case NotesApi.LoginSuccess:
                console.log("LoginSuccess")
                apiProgressBar.label = qsTr("Login successfull!")
                if (legacyLoginPossible || forceLegacyButton.checked)
                    notesApi.convertToAppPassword();
                loginDialog.canAccept = true
                break;
            case NotesApi.LoginFailed:
                console.log("LoginFailed")
                apiProgressBar.label = qsTr("Login failed!")
                break;
            default:
                console.log("None")
                apiProgressBar.label = ""
            }
        }
        onLoginUrlChanged: {
            if (notesApi.loginUrl) {
                Qt.openUrlExternally(notesApi.loginUrl)
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

            DialogHeader {
                id: dialogHeader
                title: qsTr("Nextcloud Login")
            }

            Image {
                id: nextcloudLogoImage
                anchors.horizontalCenter: parent.horizontalCenter
                height: Theme.itemSizeHuge
                fillMode: Image.PreserveAspectFit
                source: Theme.colorScheme === Theme.DarkOnLight ? "../img/nextcloud-logo-dark.png" : "../img/nextcloud-logo-light.png"
            }

            ProgressBar {
                id: apiProgressBar
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                label: verifyServerTimer.running ? qsTr("Verifying address") : " "
                indeterminate: notesApi.loginStatus === NotesApi.LoginFlowV2Initiating ||
                               notesApi.loginStatus === NotesApi.LoginFlowV2Polling ||
                               notesApi.ncStatusStatus === NotesApi.NextcloudBusy ||
                               verifyServerTimer.running
            }

            Row {
                width: parent.width
                TextField {
                    id: serverField
                    width: parent.width - statusIcon.width - Theme.horizontalPageMargin
                    text: notesApi.server
                    placeholderText: qsTr("Enter Nextcloud address")
                    label: notesApi.statusProductName ? notesApi.statusProductName : qsTr("Nextcloud address")
                    validator: RegExpValidator { regExp: allowUnecrypted ? /^https?:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/: /^https:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/ }
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    onClicked: if (text === "") text = allowUnecrypted ? "http://" : "https://"
                    onTextChanged: {
                        loginDialog.canAccept = false
                        if (acceptableInput) {
                            notesApi.server = text
                            verifyServerTimer.restart()
                        }
                    }
                    //EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: legacyLoginPossible ? "image://theme/icon-m-enter-next" : flowLoginV2Possible ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter-close"
                    EnterKey.onClicked: {
                        if (legacyLoginPossible)
                            usernameField.focus = true
                        else if (flowLoginV2Possible && notesApi.loginStatus !== notesApi.LoginFlowV2Polling)
                            notesApi.initiateFlowV2Login()
                        focus = false
                    }
                }
                Icon {
                    id: statusIcon
                    source: notesApi.statusInstalled ? "image://theme/icon-m-accept" : "image://theme/icon-m-cancel"
                    color: notesApi.statusInstalled ? "green" : Theme.errorColor
                }
            }

            TextSwitch {
                id: forceLegacyButton
                visible: debug || !notesApi.statusInstalled
                text: qsTr("Enforce legacy login")
                automaticCheck: true
                onCheckedChanged: {
                    if (!checked) {
                        notesApi.getNcStatus()
                    }
                }
            }

            Column {
                id: flowv2LoginColumn
                width: parent.width
                spacing: Theme.paddingLarge
                visible: opacity !== 0.0
                opacity: flowLoginV2Possible && !forceLegacyButton.checked ? 1.0 : 0.0
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
                opacity: legacyLoginPossible || forceLegacyButton.checked ? 1.0 : 0.0
                Behavior on opacity { FadeAnimator {} }
                TextField {
                    id: usernameField
                    width: parent.width
                    text: notesApi.username
                    placeholderText: qsTr("Enter Username")
                    label: qsTr("Username")
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    errorHighlight: text.length === 0// && focus === true
                    onTextChanged: {
                        loginDialog.canAccept = false
                        notesApi.username = text
                    }
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked: passwordField.focus = true
                }
                PasswordField {
                    id: passwordField
                    width: parent.width
                    text: notesApi.password
                    placeholderText: qsTr("Enter Password")
                    label: qsTr("Password")
                    errorHighlight: text.length === 0// && focus === true
                    onTextChanged: {
                        loginDialog.canAccept = false
                        notesApi.password = text
                    }
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: notesApi.verifyLogin()
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Test Login")
                    onClicked: notesApi.verifyLogin(passwordField.text, usernameField.text, serverField.text)
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
                checked: doNotVerifySsl
                text: qsTr("Do not check certificates")
                description: qsTr("Enable this option to allow selfsigned certificates")
                onCheckedChanged: {
                    notesApi.verifySsl = !checked
                }
            }
            TextSwitch {
                id: unencryptedConnectionTextSwitch
                checked: allowUnecrypted
                automaticCheck: false
                text: qsTr("Allow unencrypted connections")
                //description: qsTr("")
                onClicked: {
                    if (checked) {
                        allowUnecrypted = !checked
                    }
                    else {
                        var dialog = pageStack.push(Qt.resolvedUrl("UnencryptedDialog.qml"))
                        dialog.accepted.connect(function() {
                            allowUnecrypted = true
                        })
                        dialog.rejected.connect(function() {
                            allowUnecrypted = false
                        })
                    }
                }
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
