# Nextcloud notes client for SailfishOS

A mobile App providing access to [Nextcloud Notes](https://github.com/nextcloud/notes) on [Sailfish OS](https://sailfishos.org).  
Communication with the nextcloud server happens over the [Notes API 0.2](https://github.com/nextcloud/notes/wiki/API-0.2).  
The [Sailfish OS SDK](https://sailfishos.org/wiki/Application_SDK) is used to write and build this project.  

## Current features

- List all notes on the server
- Show the content of the notes
- Toggle favorite property
- Group notes by category

## Planned features

(The ordering represents the priority for the implementation)
- Login screen for the nextcloud account (using the [Login Flow](https://docs.nextcloud.com/server/14/developer_manual/client_apis/LoginFlow/index.html))
- Create new notes
- Delete notes
- Edit notes
- Automatically push changes to the server while editing a note
- [Markdown](https://en.wikipedia.org/wiki/Markdown) rendering
- Search in notes
- Checklist support
- Offline support
- Overview of the supported markdown elements
- Usage of [Sailfish OS Secrets](https://sailfishos.org/wiki/Secrets_and_Crypto) to store the user credentials

## Features to be considered

- All features of the [Android App](https://github.com/stefan-niedermann/nextcloud-notes)
- Multiple nextcloud accounts
- Use user metadata and theming (via [OCS API](https://docs.nextcloud.com/server/14/developer_manual/client_apis/OCS/index.html))
- Share notes
- Import notes from textfiles
- Sync with the Sailfish OS build in notes app
- Versioning / trashbin
- Use [ETags](https://de.wikipedia.org/wiki/HTTP_ETag) to synchronize notes more efficient
