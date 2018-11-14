import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Dialog {
    id: loginDialog

    property var accountID
    ConfigurationGroup {
        id: account
        path: "/apps/harbour-nextcloudnotes/accounts/" + accountID
    }

    canAccept: (nameField.text.length > 0 && serverField.acceptableInput && usernameField.text.length > 0 && passwordField.text.length > 0)
    onAccepted: {
        account.setValue("name", nameField.text)
        account.setValue("server", serverField.text)
        account.setValue("username", usernameField.text)
        account.setValue("password", passwordField.text)
        //accounts.itemAt(iAccount).unsecureConnection = unsecureConnectionTextSwitch.checked
        //accounts.itemAt(iAccount).unencryptedConnection = unencryptedConnectionTextSwitch.checked
        account.setValue("valid", true)
        account.sync()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            DialogHeader {
                id: header
                //title: qsTr("Nextcloud Login")
                acceptText: qsTr("Login")
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                height: Theme.itemSizeHuge
                fillMode: Image.PreserveAspectFit
                source: "../img/nextcloud-logo-transparent.png"
            }

            TextField {
                id: nameField
                focus: true
                width: parent.width
                text: account.value("name", qsTr("My Nextcloud account"), String)
                placeholderText: qsTr("Account name")
                label: placeholderText
                errorHighlight: text.length === 0// && focus === true
                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: serverField.focus = true
            }

            TextField {
                id: serverField
                width: parent.width
                text: account.value("server", "https://", String)
                placeholderText: qsTr("Nextcloud server")
                label: placeholderText + " " + qsTr("(starting with \"https://\")")
                inputMethodHints: Qt.ImhUrlCharactersOnly
                // regExp from https://stackoverflow.com/a/3809435 (EDIT: removed ? after https to force SSL)
                validator: RegExpValidator { regExp: /https:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)/ }
                errorHighlight: !acceptableInput// && focus === true
                EnterKey.enabled: acceptableInput
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: usernameField.focus = true
            }

            TextField {
                id: usernameField
                width: parent.width
                text: account.value("username", "", String)
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
                text: account.value("password", "", String)
                label: placeholderText
                errorHighlight: text.length === 0// && focus === true
                EnterKey.enabled: text.length > 0
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: loginDialog.accept()
            }

            SectionHeader {
                text: qsTr("Security")
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: qsTr("Please consider creating a dedicated app password! Open your Nextcloud in a browser and go to <i>Settings</i> → <i>Security</i>.")
            }
            /*TextSwitch {
                id: unsecureConnectionTextSwitch
                checked: appSettings.unsecureConnection
                automaticCheck: true
                text: qsTr("Do not check certificates")
                description: qsTr("Enable this option to allow selfsigned certificates")
                onCheckedChanged: {
                    if (checked) {

                    }
                    else {
                        unencryptedConnection.checked = false
                    }
                }
            }
            TextSwitch {
                id: unencryptedConnectionTextSwitch
                enabled: unsecureConnectionTextSwitch.checked
                checked: appSettings.unencryptedConnection
                automaticCheck: false
                text: qsTr("Allow unencrypted connection")
                description: qsTr("")
                onClicked: {
                    if (!checked) {
                        var dialog = pageStack.push(Qt.resolvedUrl("UnencryptedDialog.qml"))
                        dialog.accepted.connect(function() {
                            checked = true
                        })
                        dialog.rejected.connect(function() {
                            checked = false
                        })
                    }
                    else
                        checked = false
                }
            }*/
        }
    }
}
