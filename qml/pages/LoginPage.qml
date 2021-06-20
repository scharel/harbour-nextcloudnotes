import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import harbour.nextcloudapi 1.0

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
        appSettings.currentAccount = ""
    }

    onRejected: {
        Nextcloud.abortFlowV2Login()
        appSettings.currentAccount = peviousAccount
    }
    onAccepted: {
        appSettings.currentAccount = appSettings.createAccount(Nextcloud.username, Nextcloud.password, Nextcloud.server, Nextcloud.statusProductName)
    }

    Timer {
        id: verifyServerTimer
        onTriggered: Nextcloud.getStatus()
    }

    Connections {
        target: Nextcloud
        onStatusInstalledChanged: {
            if (Nextcloud.statusInstalled)
                serverField.focus = false
        }
        onStatusVersionChanged: {
            if (Nextcloud.statusVersion) {
                if (Nextcloud.statusVersion.split('.')[0] >= 16) {
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
        onLoginStatusChanged: {
            loginDialog.canAccept = false
            apiProgressBar.indeterminate = false
            switch(Nextcloud.loginStatus) {
            case Nextcloud.LoginLegacyReady:
                console.log("LoginLegacyReady")
                apiProgressBar.label = qsTr("Enter your credentials")
                break;
            case Nextcloud.LoginFlowV2Initiating:
                console.log("LoginFlowV2Initiating")
                apiProgressBar.indeterminate = true
                break;
            case Nextcloud.LoginFlowV2Polling:
                console.log("LoginFlowV2Polling")
                apiProgressBar.label = qsTr("Follow the instructions in the browser")
                apiProgressBar.indeterminate = true
                break;
            case Nextcloud.LoginFlowV2Success:
                console.log("LoginFlowV2Success")
                Nextcloud.verifyLogin()
                break;
            case Nextcloud.LoginFlowV2Failed:
                console.log("LoginFlowV2Failed")
                apiProgressBar.label = qsTr("Login failed!")
                break
            case Nextcloud.LoginSuccess:
                console.log("LoginSuccess")
                apiProgressBar.label = qsTr("Login successfull!")
                if (legacyLoginPossible || forceLegacyButton.checked)
                    Nextcloud.convertToAppPassword();
                loginDialog.canAccept = true
                break;
            case Nextcloud.LoginFailed:
                console.log("LoginFailed")
                apiProgressBar.label = qsTr("Login failed!")
                break;
            default:
                console.log("None")
                apiProgressBar.label = ""
            }
        }
        onLoginUrlChanged: {
            if (Nextcloud.loginUrl) {
                Qt.openUrlExternally(Nextcloud.loginUrl)
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
                indeterminate: Nextcloud.loginStatus === Nextcloud.LoginFlowV2Initiating ||
                               Nextcloud.loginStatus === Nextcloud.LoginFlowV2Polling ||
                               Nextcloud.statusStatus === Nextcloud.NextcloudBusy ||
                               verifyServerTimer.running
            }

            Row {
                width: parent.width
                TextField {
                    id: serverField
                    width: parent.width - statusIcon.width - Theme.horizontalPageMargin
                    text: Nextcloud.server
                    placeholderText: qsTr("Enter Nextcloud address")
                    label: Nextcloud.statusProductName ? Nextcloud.statusProductName : qsTr("Nextcloud address")
                    validator: RegExpValidator { regExp: allowUnecrypted ? /^https?:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/: /^https:\/\/([-a-zA-Z0-9@:%._\+~#=].*)/ }
                    inputMethodHints: Qt.ImhUrlCharactersOnly
                    onClicked: if (text === "") text = allowUnecrypted ? "http://" : "https://"
                    onTextChanged: {
                        loginDialog.canAccept = false
                        if (acceptableInput) {
                            Nextcloud.server = text
                            verifyServerTimer.restart()
                        }
                    }
                    //EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: legacyLoginPossible ? "image://theme/icon-m-enter-next" : flowLoginV2Possible ? "image://theme/icon-m-enter-accept" : "image://theme/icon-m-enter-close"
                    EnterKey.onClicked: {
                        if (legacyLoginPossible)
                            usernameField.focus = true
                        else if (flowLoginV2Possible && Nextcloud.loginStatus !== Nextcloud.LoginFlowV2Polling)
                            Nextcloud.initiateFlowV2Login()
                        focus = false
                    }
                }
                Icon {
                    id: statusIcon
                    source: Nextcloud.statusInstalled ? "image://theme/icon-m-accept" : "image://theme/icon-m-cancel"
                    color: Nextcloud.statusInstalled ? "green" : Theme.errorColor
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
                    text: Nextcloud.loginStatus === Nextcloud.LoginFlowV2Polling ? qsTr("Abort") : Nextcloud.loginStatus === Nextcloud.LoginSuccess ? qsTr("Re-Login") : qsTr("Login")
                    onClicked: Nextcloud.loginStatus === Nextcloud.LoginFlowV2Polling ? Nextcloud.abortFlowV2Login() : Nextcloud.initiateFlowV2Login()
                }
            }

            TextSwitch {
                id: forceLegacyButton
                visible: debug || !Nextcloud.statusInstalled
                text: qsTr("Enforce legacy login")
                automaticCheck: true
                onCheckedChanged: {
                    if (!checked) {
                        Nextcloud.getNcStatus()
                    }
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
                    text: Nextcloud.username
                    placeholderText: qsTr("Enter Username")
                    label: qsTr("Username")
                    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                    errorHighlight: text.length === 0// && focus === true
                    onTextChanged: {
                        loginDialog.canAccept = false
                        Nextcloud.username = text
                    }
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-next"
                    EnterKey.onClicked: passwordField.focus = true
                }
                PasswordField {
                    id: passwordField
                    width: parent.width
                    text: Nextcloud.password
                    placeholderText: qsTr("Enter Password")
                    label: qsTr("Password")
                    errorHighlight: text.length === 0// && focus === true
                    onTextChanged: {
                        loginDialog.canAccept = false
                        Nextcloud.password = text
                    }
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: Nextcloud.verifyLogin()
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Test Login")
                    onClicked: Nextcloud.verifyLogin(passwordField.text, usernameField.text, serverField.text)
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
                text: qsTr("The <a href=\"https://apps.Nextcloud.com/apps/notes\">Notes</a> app needs to be installed on the Nextcloud server for this app to work.")
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
                    Nextcloud.verifySsl = !checked
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

    allowedOrientations: appWindow.allowedOrientations
}
