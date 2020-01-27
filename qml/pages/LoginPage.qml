import QtQuick 2.2
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: loginPage

    property string accountId

    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + accountId
        Component.onCompleted: {
            //nameField.text = value("name", "", String)
            serverField.text = "https://" + value("server", "", String)
            usernameField.text = value("username", "", String)
            passwordField.text = value("password", "", String)
            unsecureConnectionTextSwitch.checked = value("unsecureConnection", false, Boolean)
            unencryptedConnectionTextSwitch.checked = value("unencryptedConnection", false, Boolean)
            serverField.text === "https://" ? serverField.focus = true : (usernameField.text === "" ? usernameField.focus = true : (passwordField.text === "" ? passwordField.focus = true : passwordField.focus = false))
        }
    }

    onStatusChanged: {
        if (status === PageStatus.Deactivating)
            notesApi.abortFlowV2Login()
    }

    Connections {
        target: notesApi
        onStatusStatusChanged: {

        }
        onLoginStatusChanged: {

        }
        onStatusInstalledChanged: {
            if (notesApi.statusInstalled) {
                console.log("Nextcloud instance found")
            }
        }
        onStatusVersionChanged: {
        }
        onLoginUrlChanged: {
            if (notesApi.loginUrl) {
                loginStatus = "flowV2Pending"
                Qt.openUrlExternally(notesApi.loginUrl)
            }
            else {
                loginStatus = "flowV2LoggedIn"
                console.log("Login successfull")
            }
        }
        onServerChanged: {
            console.log("Login server: " + notesApi.server)
            serverField.text = notesApi.server
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
                title: notesApi.statusProductName ? notesApi.statusProductName : qsTr("Nextcloud Login")
            }

            Image {
                id: nextcloudLogoImage
                anchors.horizontalCenter: parent.horizontalCenter
                height: Theme.itemSizeHuge
                fillMode: Image.PreserveAspectFit
                source: "../img/nextcloud-logo-transparent.png"
            }

            ProgressBar {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                indeterminate: notesApi.statusBusy
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
                        if (acceptableInput)
                            notesApi.server = text
                    }
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: legacyLoginColumn.visible ? "image://theme/icon-m-enter-next" : "icon-m-enter-accept"
                    EnterKey.onClicked: legacyLoginColumn.visible ? passwordField.focus = true : (notesApi.loginBusy ? notesApi.abortFlowV2Login() : notesApi.initiateFlowV2Login())
                }
                Icon {
                    id: statusIcon
                    highlighted: serverField.highlighted
                    source: notesApi.statusInstalled ? "image://theme/icon-s-accept" : "image://theme/icon-s-decline"
                }
            }

            Column {
                id: flowv2LoginColumn
                width: parent.width
                spacing: Theme.paddingLarge
                visible: notesApi.statusVersion.split('.')[0] >= 16
                Label {
                    text: "Flow Login v2"
                    x: Theme.horizontalPageMargin
                }
            }

            Column {
                id: legacyLoginColumn
                width: parent.width
                visible: notesApi.statusVersion.split('.')[0] < 16
                Label {
                    text: "Legacy Login"
                    x: Theme.horizontalPageMargin
                }
                TextField {
                    id: usernameField
                    width: parent.width
                    //text: account.value("name", "", String)
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
                    //text: account.value("password", "", String)
                    placeholderText: qsTr("Password")
                    label: placeholderText
                    errorHighlight: text.length === 0// && focus === true
                    EnterKey.enabled: text.length > 0
                    EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                    EnterKey.onClicked: loginDialog.accept()
                }
            }

            Button {
                id: loginButton
                anchors.horizontalCenter: parent.horizontalCenter
                property bool pushed: false
                text: notesApi.loginBusy ? qsTr("Abort") : qsTr("Login")
                onClicked: notesApi.loginBusy ? notesApi.abortFlowV2Login() : notesApi.initiateFlowV2Login()
            }

            /*
            TextField {
                id: nameField
                width: parent.width
                //text: account.value("name", "", String)
                text: notesApi.statusProductName
                enabled: false
                placeholderText: qsTr("Account name")
                label: placeholderText
                errorHighlight: text.length === 0// && focus === true
                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: serverField.focus = true
            }

            TextField {
                id: serverField
                // regExp combined from https://stackoverflow.com/a/3809435 (EDIT: removed ? after https to force SSL) and https://www.regextester.com/22
                //property var encryptedRegEx: /^https:\/\/(((www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b|((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))))([-a-zA-Z0-9@:%_\+.~#?&//=]*)$/
                //property var unencryptedRegEx : /^https?:\/\/(((www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b|((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))))([-a-zA-Z0-9@:%_\+.~#?&//=]*)$/
                width: parent.width
                //text: account.value("server", "https://", String)
                placeholderText: qsTr("Nextcloud server")
                label: placeholderText// + " " + qsTr("(starting with \"https://\")")
                inputMethodHints: Qt.ImhUrlCharactersOnly
                //validator: RegExpValidator { regExp: unencryptedConnectionTextSwitch.checked ? serverField.unencryptedRegEx : serverField.encryptedRegEx }
                errorHighlight: !acceptableInput// && focus === true
                EnterKey.enabled: acceptableInput
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: usernameField.focus = true
                onTextChanged: notesApi.host = text
            }
            */

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
                //checked: account.value("allowUnencryptedConnection", false, Boolean)
            }
            TextSwitch {
                id: unencryptedConnectionTextSwitch
                automaticCheck: false
                text: qsTr("Allow unencrypted connections")
                description: qsTr("")
                //checked: account.value("unencryptedConnection", false, Boolean)
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
