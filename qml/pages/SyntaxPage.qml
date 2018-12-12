import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        id: mainFlickable
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("View more on the web")
                onClicked: Qt.openUrlExternally("https://github.com/showdownjs/showdown/wiki/Showdown's-Markdown-syntax")
            }

            MenuItem {
                id: resetMenuItem
                property bool allOpened: false
                text: allOpened ? qsTr("Close previews") :
                                  qsTr("Open previews")
                onClicked: allOpened = !allOpened
            }
        }

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: qsTr("Markdown Syntax")
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Paragraphs")
                property string description: qsTr("Paragraphs in Showdown are just one or more lines of consecutive text followed by one or more blank lines.")
                property string rawText:
"On July 2, an alien mothership entered Earth's orbit and deployed several dozen
saucer-shaped \"destroyer\" spacecraft, each 15 miles (24 km) wide.

On July 3, the Black Knights, a squadron of Marine Corps F/A-18 Hornets,
participated in an assault on a destroyer near the city of Los Angeles."
                property string convertedText:
"<p>On July 2, an alien mothership entered Earth's orbit and deployed several dozen<br />
saucer-shaped \"destroyer\" spacecraft, each 15 miles (24 km) wide.</p>
<p>On July 3, the Black Knights, a squadron of Marine Corps F/A-18 Hornets,<br />
participated in an assault on a destroyer near the city of Los Angeles.</p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Headings")
                property string description: qsTr("You can create a heading by adding one or more # symbols before your heading text. The number of # you use will determine the size of the heading.")
                property string rawText:
"# The largest heading (an <h1> tag)
## The second largest heading (an <h2> tag)
### The third largest heading (an <h3> tag)
#### The 4th largest heading (an <h4> tag)
##### The 5th largest heading (an <h5> tag)
###### The 6th largest heading (an <h6> tag)"
                property string convertedText:
"<h1 id=\"thelargestheadinganh1tag\">The largest heading (an \&lt;h1> tag)</h1>
<h2 id=\"thesecondlargestheadinganh2tag\">The second largest heading (an \&lt;h2> tag)</h2>
<h3 id=\"thethirdlargestheadinganh3tag\">The third largest heading (an \&lt;h3> tag)</h3>
<h4 id=\"the4thlargestheadinganh4tag\">The 4th largest heading (an \&lt;h4> tag)</h4>
<h5 id=\"the5thlargestheadinganh5tag\">The 5th largest heading (an \&lt;h5> tag)</h5>
<h6 id=\"the6thlargestheadinganh6tag\">The 6th largest heading (an \&lt;h6> tag)</h6>"
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string description: qsTr("If, for some reason, you need to keep a leading or trailing #, you can either add a space or escape it:")
                property string rawText:
"# # My header # #
# \\# My Header \\# #"
                property string convertedText:
"<h1 id=\"myheader\"># My header #</h1>
<h1 id=\"myheader-1\"># My Header #</h1>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Blockquotes")
                property string description: qsTr("You can indicate blockquotes with a >.")
                property string rawText:
"In the words of Abraham Lincoln:

> Pardon my french"
                property string convertedText:
"<p>In the words of Abraham Lincoln:</p>
<blockquote>
  <p>Pardon my french</p>
</blockquote>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Bold and Italic")
                property string description: qsTr("You can make text bold or italic.")
                property string rawText:
"*This text will be italic*
**This text will be bold**"
                property string convertedText:
"<p><em>This text will be italic</em><br />
<strong>This text will be bold</strong></p>"
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string description: qsTr("Both bold and italic can use either a * or an _ around the text for styling. This allows you to combine both bold and italic if needed.")
                property string rawText:
"**Everyone _must_ attend the meeting at 5 o'clock today.**"
                property string convertedText:
"<p><strong>Everyone <em>must</em> attend the meeting at 5 o'clock today.</strong></p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Strikethrough")
                property string description: qsTr("The syntax is the same as GFM, that is, by adding two tilde (~~) characters around a word or groups of words.")
                property string rawText:
"a ~~strikethrough~~ element"
                property string convertedText:
"<p>a <del>strikethrough</del> element</p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Emojis")
                property string description: qsTr("Since version 1.8.0, showdown supports github's emojis. A complete list of available emojis can be found here: https://github.com/showdownjs/showdown/wiki/emojis.")
                property string rawText:
"this is a :smile: smile emoji"
                property string convertedText:
"<p>this is a &#128578; smile emoji</p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Code formatting")
                property string description: qsTr("Use single backticks (`) to format text in a special monospace format. Everything within the backticks appear as-is, with no other special formatting.")
                property string rawText:
"Here's an idea: why don't we take `SuperiorProject` and turn it into `**Reasonable**Project`."
                property string convertedText:
"<p>Here's an idea: why don't we take <code>SuperiorProject</code> and turn it into <code>**Reasonable**Project</code>.</p>"
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string description: qsTr("To create blocks of code you should indent it by four spaces.")
                property string rawText:
"    this is a piece
    of
    code"
                property string convertedText:
"<pre><code>this is a piece
of
code
</code></pre>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Lists")
                property string description: qsTr("You can make an unordered list by preceding list items with either a *, a - or a +. Markers are interchangeable too.")
                property string rawText:
"* Item
+ Item
- Item"
                property string convertedText:
"<ul>
<li>Item</li>
<li>Item</li>
<li>Item</li>
</ul>"
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string description: qsTr("You can make an ordered list by preceding list items with a number.")
                property string rawText:
"1. Item 1
2. Item 2
3. Item 3"
                property string convertedText:
"<ol>
<li>Item 1</li>
<li>Item 2</li>
<li>Item 3</li>
</ol>"
            }
            Loader {
                sourceComponent: syntaxDrawer
                property string description: qsTr("TaskLists")
                property string rawText:
"- [x] checked list item
- [ ] unchecked list item"
                property string convertedText:
"<ul>
<li class=\"tasklist\"><a class=\"checkbox\" href=\"tasklist:uncheckbox_0\">☐ checked list item</a></li>
<li class=\"tasklist\"><a class=\"checkbox\" href=\"tasklist:checkbox_0\">☑ unchecked list item</li>
</ul>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Links")
                property string description: qsTr("Showdown will automagically turn every valid URL it finds in the text body to links for you. In the case of email addreses, Showdown will also perform a bit of randomized decimal and hex entity-encoding to help obscure your address from address-harvesting spambots.")
                property string rawText:
"https://jolla.com
info@jolla.com
[Homepage - Jolla](https://jolla.com)
[Link to Jolla][1]
[jolla][]

[1]: https://jolla.com
[jolla]: https://jolla.com"
                property string convertedText:
"<p><a href=\"https://jolla.com\">https://jolla.com</a><br />
<a href=\"&#109;&#97;&#105;&#108;&#x74;&#x6f;&#58;&#105;&#110;&#x66;&#x6f;&#64;&#106;&#111;&#108;&#x6c;a&#46;c&#111;&#109;\">&#105;&#110;&#102;&#111;&#x40;&#x6a;&#x6f;&#x6c;&#x6c;&#x61;&#46;&#x63;&#111;&#x6d;</a><br />
<a href=\"https://jolla.com\">Homepage - Jolla</a><br />
<a href=\"https://jolla.com\">Link to Jolla</a><br />
<a href=\"https://jolla.com\">jolla</a></p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Images")
                property string description: qsTr("Markdown uses an image syntax that is intended to resemble the syntax for links, also allowing for two styles: inline and reference.")
                property string rawText:
"![Alt text](https://cloud.scharel.name/s/harbour-nextcloudnotes/download?path=/&files=nextcloud-logo-transparent.png \"Optional title\")
![Alt text][img]

[img]:
data:image/png;base64,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 \"Optional title\""
                property string convertedText:
"<p><img src=\"https://cloud.scharel.name/s/harbour-nextcloudnotes/download?path=/&files=nextcloud-logo-transparent.png\" alt=\"Alt text\" title=\"Optional title\" /><br />
<img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAFYAAABWCAYAAABVVmH3AAAACXBIWXMAAA9hAAAPYQGoP6dpAAAWfUlEQVR4nOWdeXQbx33Hv7OLxX2QBMELBCkeoiiRFHVaimVdvi07TmMnTZPYytH4JU1eXpvXlyZu+9o4TXM4sZs4R1/cxHlJ49px0tSX4luyI8mSLMkSJUoiKV4gQRAESRzEjQVm+gcACgSBBUiCZNz+/lrszO+YDwezs7+ZAQn+hIQxJvN4PBYA9RzH1TLGDIQQNSFEDUADQJapw3Fc1mtCSEE+c+lL1ctz3wngicK8L5N4vV5jLBbbTgjZBmAzADMhhE+VFwoqvSz9upj6UjYy73Mc99sVBzs5OVnDcdwBxtjNhJCmbIEtF5yVgstx3JvzvlrLId3d3fLKysoDAO7iOK4zMxDGWM7PlNLZxmXWS5f0suXSl7KReX9ZwQ4NDSm1Wu29AO4jhJQvJLD3OtxlAcsY4ycmJj7K8/xBxlhpymm+xi0HXOdMQPD4w/w6c3l4peACQNHHWLvdvpXjuK8QQhqArAP77HWhD4OFjJnRGCXvXLVpzw+Oa3vt0+pgNMp/++CtQ5UGrVio/8yyhY65hJDijbF9fX16g8HwZcbY7en3M/+yhfS8hfbcQCTKne63a0/12fTd1gltNBYnAMDzHPuHD++1Vug14kL8Z5YtpucWBazD4egA8E3GWFW+wDKDWwrc/vFp5aGzV0tP99sNsXh8npFP7NvkaLdUhHLpS/mXakMhcJcM1m63308p/TzHcfNsLQdcBuDoZav+tQtDpYMOlzpXXB1rqvy3bmryrNYDbdFgGWO8w+H4GoDbpYIrFlzGGI71jOh+dOh0nSsQFioMqkjqEaGSC3RnS6336JWRklg8TgQZzz5z0xZHNr8rBXdRYEdGRlTj4+MPM8Z2LnQashi4vWNTyl8c6ao+0283iHHKAUAgEuMbKkpDN29scN20sdF75OLQ7HBw19aWqapSrZhuY6XhLhisy+UyBIPBxxhjG6QMFxqYVOOmfUHZr97sqjx5ddwwNRNQxCkjAGDSK6N/ecvWsds3Nbs5QkAZw4tn+8oTZZrove9rm14snGLApZQuDOzIyIgqGAw+BmDDUifQ+eCeHXRo/uP1s2aXPyy4fEF5nILIZRzd27bG9eA9N4zyHGEp3XODDo0nEJYBwI0bG9wy/lrZYuAUA27BYBlj/NjY2MMANmRzUiy4lBHy9PHu8lfOD5RHxDg37QvKCSEo1Sqi9+5onfj4no5Jjpv7QDt2xWoAEtOrGzsavMWAs1T9gsHabLavEUJ2SjlZKtwpX0j2o5dOmwcn3Op4HGTaF5Sr5ELcoFHGPr67ffzOrWvdmTpxSsn5oQkdAKytNgZ1Snk8n/9C4SxFvyCwo6Oj9wG4vRBQi4XbO+5S/eDQKUsgLPIMDN5QSCjVKEWlXBb/yK4NEymomfrDE15FJBbjAGBjXYW/mHAWqw8AuTO7SRkZGelgjH0h3VguJymhlOYsy3b/sm1S9chzJ+sCYZEHADnPM4NaJSrlsvh1a83eO7c0u3Lp945PqVKfOxsqA9liyOU/syxXGxajLwm2r69PD+CbyBgyigm3xzatevSFU/WpXtdYVRqkFOA5wioM2uhf3bZ1nOO4nPqDE24lkBhfLUZDtJhwlqIvCVahUHwZQFU2w8WAO+0Lyn708ulaMU4JAGyqr/Rp5UI8zhKfP7a7zcGTa0/4bD7tLr8CACoM6mhqprDacCmlucFardZtjLHbpQwvBa5IKfm3Q+9YfOGoDAC2NVV7P7anY+LCiFMHAJ31lb4tDVWBfI0LRhLDR6VBGykkhpWCmxUsY4ynlP5dIYYXC/d3J3rKbdM+JQDUlOrCn715y/jxHqs+VeeOzY1zJvm5YghGozzAEInGuG/+/pjlO/9zrDb1IrGacLOCHRwc/CiAhkINLxSuddKrePX8QDkAKGQy+vnbt44JMo6dGXDoAaBcp462mstDmXYzY2AA3P6w4PSGFCf7bKWXRpzaC1an7rvPHjeLlBYVri8U4Q+d6S157Xy/IZ9+1jevoaEhJYCD6YYLmYYsZCr2n0e7q1JhffSGNkdtmS7q9ASEMVeiB29rrp6ReomIU0YOdw8bXj4/aHR6gwoAUMpls/PXiyNO3def+WPdVz+4a1SjEOhip1JijJJTV8e0R68MG7pHnNp4nJIyrUq8cWOjl08+UHPpzwNLCLmXJZdTlgNu37hLOeBwqwGgzWLy7d1g8QLAgNOjTNVfW1UayjZHZIzhaM+o/rfHL1d6QxEZAJjLtCGTXhO9eWOja1NDVeD7h07WOtx+xaDDpf6np4+s+fytW+1N1cZwoW3wBiP8mX679uzAmPbSiFMbicXnfKujMco5PD7BXGYQpdo8B2x3d7ccwH3zahUR7kvnBoyp63uua51MlTk8PnmqbrlOPS9oh8cn/PzwheresSlNql6ZVi3eu7PVubutfoZP1vv6X+wbfuT5k7W9Y1Mah9uv+OffvNWwrdnsvbWz0b2+tjyUijvVBk8gzHePONX94y5Vz9ikemTSq6IZX3OlINBNDVW+Pe0N3q1N1QGOkLw9fw5YhUJxgFJaXsibxmLgeoMRvms4MY6uNxv9jZWG2Sd5WKSzPcNkUM+m/BhjON0/rn3iSJc5FBU5IJF/vXPr2skDm5vcckE2O8UihEAtF+iD9+wa/fVbF02HLw4aKQNO948ZTvePGTRKIV5VootoFUI8FI3xrkBINjUTlCOL8BzBekuF/4bWOu/OdbV+tUJOM+tIMZgDluf5u6TALBXuBeuEhiY7w861Nd50/VQulSOASpDRlI2njl0yvXphqDxlo6PO5PvcbdvHdUohnisGGcexT+7vdO5vr/c+feyS6dKoU0cZEAiL/IDEqgMANFSUhq5vtXj3tDXMlGgUWfMOhTCYBWu1WmtisVhnrodGMeD22hON4gnHrltr9qXr65XyGABQBgSjMU6tEOjPDndVHe8ZLU0AJ+yena3Ou7Y0uzLjyhVDvakk8pUP7rJ5AhH+Z6+drT5vTSRrMqXCoI3uaDF797U3zJjLdNFi5BZmwUaj0QNcxpOu2HD7kw8tS7kuLKTlUymlKNWqYqnPAw6X8u2+McPJvrESACjTqsTP3bLZ1lJjDC8kWS7GKHnr8rD+la4B43jyDS0lerUitq2pZmbPhjXetdWlYULI8uRjCSE351NeKlxPMCoAQJ1RP2/ltK3WGEzVf/bMVdNg8o9g0muif//B662lWmUsX2ypGCiAw10Dhmff6a3wBiPX2gigc02l76aNTe7NjVWBlOay5WMvX75sZIw1MZZ9t0ox4EbEOIkmEy0lGkUsU79Eo4yZS7Xh4Umv+u2e0TKTXhMp0yjFr6ZBzdcwxhje7h3V/feJK6YJb0CRgibjOXbD+jr3XVvXuqpKEmthJIe9YsDleT4BluO47SmFXIaXCtcTDM32HG1aMjq93k0dDa7H3zinBoBAKCr78t07hsvS/ghSDbN7AvLHXz1bM+T0zKYRtSp5fH/bGted21rcuhw+F9KGfDGk68sAgBCyrRDDS4ErxuisoiCTzXsfZIzhhlbzzB/ODYTlPM/iDLPjrtTcGABeuzBc8syJy1VicgeMUpDRD1zX6rx9U6NHkPEFrX8V0oZ8NtKvOQAghGxODzRlOLMBmde5GppN36C+NnVx+4NZVy44Qtj9e9vHCQGTcWCPPH+qzu1P9PRsft3+sOy7z52wPHn0YnUK6qaGqplvffzGgTu3NLnTFxwLyWfka0MuycaHO3LkiAyAeSGGFwNXJfBxtSIx97S5/IpcNtrMxuD7t651AoA7EBK+98IpSyiaGJvTdXrtLtU//ubNxku2KS0AlGiU4hfv2D76pTuvGzPqrs0wCoFTbLiUUnDV1dUWALxUpWz3FwqX4zg0V5QEAaB3fForxijJZeMD25qnd7aYPQAw5vIpH37uRJ0/FOVTti+NTqq/98K1pZzrWy3ub31s3+C2pmp/tthWAy7H83x9vkrFgrtzXQJWICzyr14YLM2slxJCCD69b6NjvdnoB4Ahp0f1jd8fq5/yBWU2l0/+/T+ctojJ5Mg9O1onHrix06GSJ7JYuWJbabgcY6w2XyUpwwuBu3VNhd9cqg0DwCsXhsuzfcVTwnOE/fWd223tdSYfADg8AcW//O54wyMvnKpLQT24t8N+97a1rswHUK7YVhIuRyk1LPaBVKiTdLl7W/MkkFhSefZ0n1HKhsBx7G8ObLdd32pxA4AnEBac3oCCgWHP+jrX/rZ6bz6fqwWXI4kzVIt+2hfiJL2ss87kX5Mca9/otpafGRzXStkgAD69t91xx+amSULABJ6jPCFoM5cHpfyuNlyOMabOV6lQw4XoE0LwwL4Oe2qG8MSRi2a7yy/PrJcuhBB8aMe6qY/sWu/gCEGcMvz0jXdr3x10aKT8riZcDoCmGPPUheiX61Xip/a2jxEA0Vice+zlMxZvMMpn1suUWzsa3A/ctMmWgvv46+dr+x0upZTOasHlCCHzJuArAXdjnSlwx+ZGJwBM+ULyRw+9UxeIiFxmvUy5rrna98n9HWOEEIRFkfv+odN1Tk9AkNJZabiMMZEr9hvWQvTv3tI0vaO52g0k5quPvvhOXUiM5YW7q6V25sM7Wx0A4A9H+afevlyRT2cl4RJCApyU4eWGSwjBp/Z1ODrrTTMAYJ2aUT32hzO16XmFXDZu62xwb6yr8AHAuSGH/uq4S5lPZwXhBrkcBXmd5DFcsD4B8NmbOu3ra8r8ANBjd2kfPfRObSy56ULKxp9fv97JcxwDgGfSeq2UzkrAnQNWyvByw+U5jh3c1z7uD4uyCU9Aed7qLHn89fPVLKNeplSXaKK71pndAHDV4dZMegML3ry3HHB5nvfOGWOlDC833FK1MmZQKURBxtFYnJJ3hxyGp45dytsLd6ytmUldd9umNAvdQpqtzlLhRqPR0ayvlKsBF4yhwqCOlGlVUbmMowBw+NKI8XivTTe32lwbTZWl4dT11ExQyPQv5Xe54DLGrDmfwKsBVy7jKM8R1lxZGlArEluGfv3HS+Yhp2fOQmC6DYHnmFyWWC73hSKzc+HVggsg+sMf/tDOSVVaabgCzzHGGEAI+8z+jTaOJLZ7/vzIBXM8R++IUzq7lqYUZDSXfym/xYTLGLM99NBDlJOqJGV4OeDGkhuQ5TKetdWWB2/Z2DAJJDJbb/fY9Nm+ep7AtV6qSb4mryZcjuPeBXLvjy3IcLHhpjYha+SyOGMMBzobXFqFEAOAQ+cGTLE4m5ccdyfPdwGAXj33xEw2/1JxFwMuY+wMILFVfqXhMgDe5L4Dg1ohAoBC4OldW5omgcRr7zsDdl2m/tVx1+yqbHOVMVSIf6m4lwKXEELngC10YF5OuOPugFxM7t8yl13b9r671ewVZDwFgAsjk/NSjF3WxNZ6tUKIV5eoo4X6l4p7sXAZY31lZWVeIK3HrjbcwbT9sfXl+tkpFM9xbE25LgQAtukZZbp+MCJyAxMeNYDZ9bSF+JeKezFwGWOvp65zJmGknC8H3O5kbxRkHF1jMsw5qFGqVUUZY5gJJcbglP7hbmsJZYlX390bLO50e4X6L1QnHwNCCFUoFC+l7s0bClYDrhijpMfu0gHAuqqyQPp+AACIiLHEwbrkkAAAM8EI/0rXkAkAStRKcdOaa4fnVgMupfSsTqdzpj5nHQpWGu654QltKl3YWW/yZepPehObg8u1ytkDci++229M6RzY3DiJHPPcQvwvRCcXA0rpofT7OV8QVhLuW1cSe2BVcoFua6z0pdcJi3FuciakAACjTiUCQJfVqXnz8qgRAMxl2vDetjpvpt1C21AMuIwxh8lkeiX9HicVzErAHZ6cUQ44vRoA2NpQ4VEmd3On6hzrsRmiydlCh8Xkt3sC8ife7K6ljIEnHLtvV9s4pJ/UeduwVLiMsV8RQuZs3puX6JYyIOV8sXB/f7q/Akisxt7cUedK14lRRt68MlIGADqlPNZYYQj/5NVzltRZhA/tbHE0VhrCUv4LbcNi4RJCppxO53OZ9bImurMZyOck00Yh+ueGndqrE24NALyvpcZl0qrmHOp4/my/cXImJAeADZZy38Mvnq6fTB7G2LXO7Nq/weIpZCpXaBsWCfcX7e3t8+bPORPdWQzkdZJpQ0rfH47yvznZVw0AMp7Q929umErXvzruVr1+cdgEAApBRt8ddBg8gbAAANe31Ljvv2HDRDbbKwy378c//vHvspXPm26tBFzGGP7rRE/VTDI3cPfmpon0jcHWKa/i39/ossQYMBOMCP5wVCbGKUcIwc0d9VMH97Q7MverrjTc5Ovrtx966CE6rxBpZxCS48Ws4Vy/kpF+nek832GL1PUL54bKz1un9ACwrrrUv39DrSdVNuicUf708AWLJxAVPKGwXKsQYgLPUY1SiH9iT9vYRospkH0LcO42ZMZWSBukGCQ/v1hVVXUxRyjzf+BhueEe7R0zvHwh8RU3alXiJ3dvGE/p/LFnzPDUiT6zNxQWItE4r1UKMY1CiK0p1wcfuHHj7L7XXP6l2lBMuIQQG6X00awBJEXyZ/OKDfd4n13/21NXa4DEU/6Lt3ZaDWpFLBqn5CevddWeGhgvi4iUJwD0arlYolKI+9osU3+2fe0UV6B/qTYUCW40Ho8/aDabA/MU0yTrlvViw43H4zjUZS1/tXvEBAAqgadfuKVzRKsQ4i+dHyp76mRvrT8cSx40ATNqVNHdrebpu7c0TRnU8hgoBZbpNM8i4P7AbDb3ZHWaJrLMgboAwwuC6wuL/NMn+6ou2qb1QGJr5u515umXzg+WXx5z60RKSSR5jlaQ8XRXS9X0R3asm6g0zE0BSo3ZKwWXEPJCTU3NM1mdZcjsvq2lDObZ9BljODkwoX/u3cGqYDQ2u3wiUkpSPTclRp0yYi7Vhg/esN5uMerm/QTJcp7myVYvh/7R6urqb2R1kkWyzgoKCSxX4xiAM0NO3ZtXbEabO6CaZzBNGk364PvW1ri3N1X6BI5jS4GzzHC7zGbzVwkh2b/eWSTnrKCQwIBEys8xE5SPTvuVNpdfednu0nlCiSWWTOEAWIy64IaaUv+WhgpfdYk2CqBocJYJbpfP5/sSIWTe25WUyJJjh5ThOfejcUqOXbGXjHsDSrvbr5z0RRQxSucopOur5bJ4g0kfaDcb/ZvqTX6NQhbPZVvKf2bZCsE9muypC4IK5BhjpeBSysiLXcNV8wqTYtQooxajNtRcWRJsqSwJVhpUOY+pLxecYsCllD5vsVj+dSFf/3TJOcbmCk4h46hcxtFojHJKgad6pVysLdOGmisNwdbq0oAx7UDxUt/QVglulBDyA4vFUtDTP5fIFjOY/+1tmwZ1KiGuTC6VLOfr70rCZYzZ4vH4gw0NDXnnqflEcuNxts8AUK5ViAqem1VazXxuMfQJIZRS+rxcLr+vGFCBtKFgOZMWf8o9l1LaJwjCdywWy4WsFRYpBW08zvY52/33Us8lhEwxxr77y1/+8v5iQwWyzAr+H/RcB8dxv/L5fM9ly/wXS/LmY/8vwAVAAZxljB2yWq2v7N+/P+uvdhRTCsrHvhfhkkSGvw/A64yxl5qbm51ZFZZJZEj8j795gQHvObgigBEA5wghpymlZ1paWmawSiKTyWRPiKJIAcxmnBib+2tG6XDSRaosHUAuHSn9dEmbZ4qEkABjLEgICTLGvIyxUY7jrE8++aQ91/rTasj/Andxq7VOKfFFAAAAAElFTkSuQmCC\" alt=\"Alt text\" title=\"Optional title\" /></p>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("Tables")
                property string description: qsTr("Tables aren't part of the core Markdown spec, but they are part of GFM and Showdown supports them by turning on the option tables.

Colons can be used to align columns.

In the new version, the outer pipes (|) are optional, matching GFM spec.

You also don't need to make the raw Markdown line up prettily.

You can also use other markdown syntax inside them.")
                property string rawText:
"| Tables        | Are           | Cool  |
| ------------- |:-------------:| -----:|
| **col 3 is**  | right-aligned | $1600 |
| col 2 is      | *centered*    |   $12 |
| zebra stripes | ~~are neat~~  |    $1 |"
                property string convertedText:
"<table border='1' cellpadding='20'>
<thead>
<tr>
<th>Tables</th>
<th style=\"text-align:center;\">Are</th>
<th style=\"text-align:right;\">Cool</th>
</tr>
</thead>
<tbody>
<tr>
<td><strong>col 3 is</strong></td>
<td style=\"text-align:center;\">right-aligned</td>
<td style=\"text-align:right;\">$1600</td>
</tr>
<tr>
<td>col 2 is</td>
<td style=\"text-align:center;\"><em>centered</em></td>
<td style=\"text-align:right;\">$12</td>
</tr>
<tr>
<td>zebra stripes</td>
<td style=\"text-align:center;\"><del>are neat</del></td>
<td style=\"text-align:right;\">$1</td>
</tr>
</tbody>
</table>"
            }

            Loader {
                sourceComponent: syntaxDrawer
                property string title: qsTr("HTML")
                property string description: qsTr("In most cases, HTML tags are leaved untouched in the output document.")
                property string rawText:
"some markdown **here**
<div>this is *not* **parsed**</div>
Use <b>HTML</b> <i>Tags</i> to format your text."
                property string convertedText:
"<p>some markdown <strong>here</strong></p>
<div>this is *not* **parsed**</div>
<p>Use <b>HTML</b> <i>Tags</i> to format your text.</p>"
            }
        }

        VerticalScrollDecorator { flickable: mainFlickable }
    }

    Component {
        id: syntaxDrawer

        Column {
            width: page.width

            SectionHeader {
                text: title
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                bottomPadding: Theme.paddingLarge
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: description
            }
            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignLeft
            }
            Drawer {
                id: drawer
                width: parent.width
                height: rawTextLabel.height + 2*Theme.paddingLarge > 2*Theme.itemSizeHuge ? 2*Theme.itemSizeHuge : rawTextLabel.height + 2*Theme.paddingLarge
                backgroundSize: height
                hideOnMinimize: true
                dock: Dock.Bottom

                Connections {
                    target: resetMenuItem
                    onAllOpenedChanged: resetMenuItem.allOpened ? drawer.show() : drawer.hide()
                }

                foreground: SilicaFlickable {
                    id: rawTextFlickable
                    anchors.fill: parent
                    contentWidth: parent.width > rawTextLabel.contentWidth ? parent.width : rawTextLabel.contentWidth + 2*Theme.horizontalPageMargin
                    contentHeight: parent.height > rawTextLabel.height ? parent.height : rawTextLabel.height
                    BackgroundItem {
                        anchors.fill: parent
                        onClicked: drawer.show()
                        Label {
                            id: rawTextLabel
                            x: Theme.horizontalPageMargin
                            y: Theme.paddingLarge
                            color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                            font.family: appSettings.useMonoFont ? "DejaVu Sans Mono" : Theme.fontFamily
                            textFormat: Text.PlainText
                            text: rawText
                        }
                    }
                    ScrollDecorator { /*flickable: rawTextFlickable*/ }
                }

                background: SilicaFlickable {
                    id: convertedTextFlickable
                    anchors.fill: parent
                    contentWidth: parent.width > convertedTextLabel.contentWidth ? parent.width : convertedTextLabel.contentWidth + 2*Theme.horizontalPageMargin
                    contentHeight: parent.height > convertedTextLabel.height ? parent.height : convertedTextLabel.height + 2*Theme.paddingLarge
                    BackgroundItem {
                        anchors.fill: parent
                        onClicked: drawer.hide()
                        LinkedLabel {
                            id: convertedTextLabel
                            x: Theme.horizontalPageMargin
                            y: Theme.paddingLarge
                            textFormat: Text.RichText
                            linkColor: Theme.primaryColor
                            defaultLinkActions: false
                            onLinkActivated: drawer.hide()
                            text: "<style>\n" +
                                  "ul,ol,table,img { margin: " + Theme.paddingLarge + "px 0px; }\n" +
                                  "a:link { color: " + Theme.primaryColor + "; }\n" +
                                  "a.checkbox { text-decoration: none; padding: " + Theme.paddingSmall + "px; display: inline-block; }\n" +
                                  "li.tasklist { font-size:large; margin: " + Theme.paddingMedium + "px 0px; }\n" +
                                  "del { text-decoration: line-through; }\n" +
                                  "table { border-color: " + Theme.secondaryColor + "; }\n" +
                                  "</style>\n" + convertedText
                        }
                    }
                    ScrollDecorator { /*flickable: convertedTextFlickable*/ }
                }
            }
            Separator {
                width: parent.width
                color: Theme.primaryColor
                horizontalAlignment: Qt.AlignRight
            }
        }
    }

    allowedOrientations: defaultAllowedOrientations
}
