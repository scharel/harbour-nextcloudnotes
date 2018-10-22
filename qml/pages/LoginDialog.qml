import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: loginDialog

    property var account

    canAccept: (serverField.acceptableInput && usernameField.text.length > 0 && passwordField.text.length > 0)
    onAccepted: {
        account = {
            server: serverField.text,
            username: usernameField.text,
            password: passwordField.text,
            lastUpdate: new Date(0)
        }
    }

    Column {
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
            id: serverField
            focus: true
            width: parent.width
            text: (typeof(account) !== 'undefined' && account.server.toString().length > 0) ? account.server : "https://"
            placeholderText: qsTr("Nextcloud server")
            label: placeholderText + " " + qsTr("(starting with \"https://\")")
            inputMethodHints: Qt.ImhUrlCharactersOnly
            // regExp from https://stackoverflow.com/a/3809435 (EDIT: removed ? after https to force SSL)
            validator: RegExpValidator { regExp: /https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,6}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*)/ } // TODO disable unencrypted communication
            EnterKey.enabled: acceptableInput
            EnterKey.iconSource: "image://theme/icon-m-enter-next"
            EnterKey.onClicked: usernameField.focus = true
        }

        TextField {
            id: usernameField
            width: parent.width
            text: (typeof(account) !== 'undefined' && account.username.length > 0) ? account.username : ""
            placeholderText: qsTr("Username")
            label: placeholderText
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
            errorHighlight: text.length === 0 && focus === true
            EnterKey.enabled: text.length > 0
            EnterKey.iconSource: "image://theme/icon-m-enter-next"
            EnterKey.onClicked: passwordField.focus = true
        }

        PasswordField {
            id: passwordField
            width: parent.width
            text: (typeof(account) !== 'undefined' && account.password.length > 0) ? account.password : ""
            label: placeholderText
            errorHighlight: text.length === 0 && focus === true
            EnterKey.enabled: text.length > 0
            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            EnterKey.onClicked: loginDialog.accept()
        }
    }
}
