import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: page

    property string searchText: ""

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (appSettings.accountIDs.length <= 0) {
                addAccountHint.restart()
            }
            else {
                autoSyncTimer.restart()
            }
        }
    }

    SilicaListView {
        id: notesList
        anchors.fill: parent
        spacing: Theme.paddingLarge

        PullDownMenu {
            busy: api.busy

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Add note")
                enabled: appSettings.currentAccount.length > 0
                onClicked: api.createNote( { 'content': "" } )
            }
            MenuItem {
                text: enabled ? qsTr("Reload") : qsTr("Updating...")
                enabled: appSettings.currentAccount.length > 0 && !api.busy
                onClicked: api.getNotesFromApi()
            }
            MenuLabel {
                visible: appSettings.currentAccount.length > 0
                text: qsTr("Last update") + ": " + (
                          new Date(account.update).valueOf() !== 0 ?
                              new Date(account.update).toLocaleString(Qt.locale(), Locale.ShortFormat) :
                              qsTr("never"))
            }
        }

        header: PageHeader {
            height: searchField.height + description.height
            SearchField {
                id: searchField
                width: parent.width
                enabled: appSettings.accountIDs.length > 0
                placeholderText: account.name.length > 0 ? account.name : qsTr("Nextcloud Notes")
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onTextChanged: noteListModel.searchText = text
            }
            Label {
                id: description
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                visible: text.length > 1
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.paddingMedium
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: account.username + "@" + account.server
            }
            BusyIndicator {
                anchors.verticalCenter: searchField.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                size: BusyIndicatorSize.Medium
                running: api.busy && !busyIndicator.running
            }
        }

        currentIndex: -1

        model: noteListModel

        section.property: appSettings.sortBy
        section.criteria: appSettings.sortBy === "title" ? ViewSection.FirstCharacter : ViewSection.FullString
        section.labelPositioning: appSettings.sortBy === "title" ? ViewSection.CurrentLabelAtStart | ViewSection.NextLabelAtEnd : ViewSection.InlineLabels
        section.delegate: SectionHeader {
            text: section
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: notesList.count === 0 && api.busy
        }

        ViewPlaceholder {
            id: noLoginPlaceholder
            enabled: appSettings.accountIDs.length <= 0
            text: qsTr("No account yet")
            hintText: qsTr("Got to the settings to add an account")
        }

        ViewPlaceholder {
            id: noNotesPlaceholder
            enabled: api.status === 204 && !busyIndicator.running && !noLoginPlaceholder.enabled
            text: qsTr("No notes yet")
            hintText: qsTr("Pull down to add a note")
        }

        ViewPlaceholder {
            id: noSearchPlaceholder
            enabled: notesList.count === 0 && noteListModel.searchText !== ""
            text: qsTr("No result")
            hintText: qsTr("Try another query")
        }

        ViewPlaceholder {
            id: errorPlaceholder
            enabled: notesList.count === 0 && !busyIndicator.running && !noSearchPlaceholder.enabled && !noNotesPlaceholder.enabled && !noLoginPlaceholder.enabled
            text: qsTr("An error occurred")
            hintText: api.statusText
        }

        TouchInteractionHint {
            id: addAccountHint
            interactionMode: TouchInteraction.Pull
            direction: TouchInteraction.Down
        }
        InteractionHintLabel {
            anchors.fill: parent
            text: qsTr("Open the settings to configure your Nextcloud accounts")
            opacity: addAccountHint.running ? 1.0 : 0.0
            Behavior on opacity { FadeAnimation {} }
            width: parent.width
        }

        VerticalScrollDecorator { flickable: notesList }
    }

    allowedOrientations: defaultAllowedOrientations
}
