# Burt's Fork of ST

Additional requirements:
  - Lua
  - Lua config saved to ~/.config/st/config.lua
  - Deja Vu Sans Mono 
    *[OPTIONAL: Edit Font field in sample_config.lua to an installed font to get st to work]*
  - Noto Font Family [REQUIRED: Hard-coded as secondary font for displaying Emojis and other UTF chars]

## Lua Config
Example:
```lua
CursorSape = 4
ColorScheme = {
    "#211E1C",
    "#A41623",
    "#439A86",
    "#FF1B1C",
    "#1F1FFF",
    "#F62DAE",
    "#0067A3",
    "#EAE0CC",
    "#3A405A",
    "#E63462",
    "#00E0B0",
    "#FCF300",
    "#004CFE",
    "#EC9DED",
    "#00A0FF",
    "#DFE0E2",
    "#110F0E",
    "#26ACF8",
    "#1F1FFF",
    "#E0E000"
}

Alpha = .8375
Font = "Deja Vu Sans Mono"
FontSize = 16
```

### Config Globals
- CursorShape: Sets cursor's shape
  - Possible Values:
    - 2 = Block
    - 4 = Underline
    - 6 = Bar
    - 7 = Snowman
- ColorScheme: Set the 16 terminal colors and the 
  foreground, background, cursor normal, cursor inverted colors.
- Alpha: Floating point value between 0.0 <= 1.0
- Font: Terminal Font
- FontSize: Size of font.

## Reset Terminal After Config Changes

CTRL+SHIFT+ESC

# ST's Original Readme contents

st - simple terminal
--------------------
st is a simple terminal emulator for X which sucks less.


Requirements
------------
In order to build st you need the Xlib header files.


Installation
------------
Edit config.mk to match your local setup (st is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install st (if
necessary as root):

    make clean install


Running st
----------
If you did not install st with make clean install, you must compile
the st terminfo entry with the following command:

    tic -sx st.info

See the man page for additional details.

Credits
-------
Based on Aurélien APTEL <aurelien dot aptel at gmail dot com> bt source code.

