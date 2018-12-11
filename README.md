# Nextcloud notes client for SailfishOS

A mobile App providing access to [Nextcloud Notes](https://github.com/nextcloud/notes) on [Sailfish OS](https://sailfishos.org).
Communication with the nextcloud server happens over the [Notes API 0.2](https://github.com/nextcloud/notes/wiki/API-0.2).
Mardown conversion is done by [Showdown](https://github.com/showdownjs/showdown).
The [Sailfish OS SDK](https://sailfishos.org/wiki/Application_SDK) is used to write and build this project.

Currently the app can be downloaded from [OpenRepos](https://openrepos.net/content/scharel/nextcloud-notes) or directly as RPM-Package [from here](https://cloud.scharel.name/s/harbour-nextcloudnotes). It is planned to publish the app in [Jolla Harbour](https://harbour.jolla.com) once it reaches a certain level of maturity.

You can preview some screenshots [here](https://cloud.scharel.name/apps/gallery/s/harbour-nextcloudnotes#Screenshots).

## Features

### Implemented

- Multiple Nextcloud accounts
- List all notes of an account
- Show the content of the notes
- Search for note titles or content
- [Markdown](https://en.wikipedia.org/wiki/Markdown) rendering powered by [ShowdownJS](https://github.com/showdownjs/showdown)
- Edit the notes content
- Create new notes
- Delete notes
- Edit favorite property
- Edit category property
- Checklists
- Link to external website with Markdown syntax

### Planned

(The ordering represents the priority for the implementation)
- Uusing the [Login Flow](https://docs.nextcloud.com/server/14/developer_manual/client_apis/LoginFlow/index.html) if possible
- Automatically push changes to the server while editing a note
- Offline support
- Usage of [Sailfish OS Secrets](https://sailfishos.org/wiki/Secrets_and_Crypto) to store the user credentials

### To be considered

- All features of the [Android App](https://github.com/stefan-niedermann/nextcloud-notes)
- Use user metadata and theming (via [OCS API](https://docs.nextcloud.com/server/14/developer_manual/client_apis/OCS/index.html))
- Share notes
- Import notes from textfiles
- Sync with the Sailfish OS build in notes app
- Versioning / trashbin
- Use [ETags](https://de.wikipedia.org/wiki/HTTP_ETag) to synchronize notes more efficient

## Translations

- English
- German
- Swedish

Contributions to the translations are welcome.
