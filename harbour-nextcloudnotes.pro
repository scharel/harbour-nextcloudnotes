# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-nextcloudnotes

CONFIG += sailfishapp

SOURCES += src/harbour-nextcloudnotes.cpp \
    src/harbour-nextcloudnotes.cpp

DISTFILES += qml/harbour-nextcloudnotes.qml \
    qml/cover/CoverPage.qml \
    rpm/harbour-nextcloudnotes.changes.in \
    rpm/harbour-nextcloudnotes.changes.run.in \
    rpm/harbour-nextcloudnotes.spec \
    rpm/harbour-nextcloudnotes.yaml \
    translations/*.ts \
    harbour-nextcloudnotes.desktop \
    qml/pages/NotePage.qml \
    qml/pages/NotesPage.qml \
    qml/pages/LoginDialog.qml \
    qml/pages/EditPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/MarkdownPage.qml \
    qml/pages/UnencryptedDialog.qml \
    qml/pages/NotesApi.qml \
    js/showdown-1.9.0/bin/showdown.js \
    js/showdown-1.9.0/dist/showdown.js \
    js/showdown-1.9.0/dist/showdown.min.js \
    js/showdown-1.9.0/src/cli/cli.js \
    js/showdown-1.9.0/src/cli/makehtml.cmd.js \
    js/showdown-1.9.0/src/cli/messenger.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/blockquote.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/codeBlock.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/codeSpan.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/emphasis.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/header.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/hr.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/image.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/links.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/list.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/listItem.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/node.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/paragraph.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/pre.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/strikethrough.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/strong.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/table.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/tableCell.js \
    js/showdown-1.9.0/src/subParsers/makeMarkdown/txt.js \
    js/showdown-1.9.0/src/subParsers/anchors.js \
    js/showdown-1.9.0/src/subParsers/autoLinks.js \
    js/showdown-1.9.0/src/subParsers/blockGamut.js \
    js/showdown-1.9.0/src/subParsers/blockQuotes.js \
    js/showdown-1.9.0/src/subParsers/codeBlocks.js \
    js/showdown-1.9.0/src/subParsers/codeSpans.js \
    js/showdown-1.9.0/src/subParsers/completeHTMLDocument.js \
    js/showdown-1.9.0/src/subParsers/detab.js \
    js/showdown-1.9.0/src/subParsers/ellipsis.js \
    js/showdown-1.9.0/src/subParsers/emoji.js \
    js/showdown-1.9.0/src/subParsers/encodeAmpsAndAngles.js \
    js/showdown-1.9.0/src/subParsers/encodeBackslashEscapes.js \
    js/showdown-1.9.0/src/subParsers/encodeCode.js \
    js/showdown-1.9.0/src/subParsers/escapeSpecialCharsWithinTagAttributes.js \
    js/showdown-1.9.0/src/subParsers/githubCodeBlocks.js \
    js/showdown-1.9.0/src/subParsers/hashBlock.js \
    js/showdown-1.9.0/src/subParsers/hashCodeTags.js \
    js/showdown-1.9.0/src/subParsers/hashElement.js \
    js/showdown-1.9.0/src/subParsers/hashHTMLBlocks.js \
    js/showdown-1.9.0/src/subParsers/hashHTMLSpans.js \
    js/showdown-1.9.0/src/subParsers/hashPreCodeTags.js \
    js/showdown-1.9.0/src/subParsers/headers.js \
    js/showdown-1.9.0/src/subParsers/horizontalRule.js \
    js/showdown-1.9.0/src/subParsers/images.js \
    js/showdown-1.9.0/src/subParsers/italicsAndBold.js \
    js/showdown-1.9.0/src/subParsers/lists.js \
    js/showdown-1.9.0/src/subParsers/metadata.js \
    js/showdown-1.9.0/src/subParsers/outdent.js \
    js/showdown-1.9.0/src/subParsers/paragraphs.js \
    js/showdown-1.9.0/src/subParsers/runExtension.js \
    js/showdown-1.9.0/src/subParsers/spanGamut.js \
    js/showdown-1.9.0/src/subParsers/strikethrough.js \
    js/showdown-1.9.0/src/subParsers/stripLinkDefinitions.js \
    js/showdown-1.9.0/src/subParsers/tables.js \
    js/showdown-1.9.0/src/subParsers/underline.js \
    js/showdown-1.9.0/src/subParsers/unescapeSpecialChars.js \
    js/showdown-1.9.0/src/converter.js \
    js/showdown-1.9.0/src/helpers.js \
    js/showdown-1.9.0/src/loader.js \
    js/showdown-1.9.0/src/options.js \
    js/showdown-1.9.0/src/showdown.js \
    js/showdown-1.9.0/bower.json \
    js/showdown-1.9.0/package-lock.json \
    js/showdown-1.9.0/package.json \
    js/showdown-1.9.0/dist/showdown.js.map \
    js/showdown-1.9.0/dist/showdown.min.js.map \
    js/showdown-1.9.0/license.txt \
    js/showdown-1.9.0/CHANGELOG.md \
    js/showdown-1.9.0/CONTRIBUTING.md \
    js/showdown-1.9.0/CREDITS.md \
    js/showdown-1.9.0/DONATIONS.md \
    js/showdown-1.9.0/README.md

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-nextcloudnotes-de.ts

HEADERS +=
