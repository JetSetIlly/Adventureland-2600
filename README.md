[Adventureland](https://en.wikipedia.org/wiki/Adventureland_(video_game)) is a game created by [Scott Adams](https://en.wikipedia.org/wiki/Scott_Adams_(game_designer)) and released commercially in 1977 for the TRS-80. The game has the distinction of being the first ever game to be released commercially and was the first in a serious of similar games by Adams.

This project a port of `Adventureland` to the `Atari2600`. Specifically, it is a port of the MS-DOS version, written in C (file `advland2.zip` downloadable from the [IF Archive](https://www.ifarchive.org/if-archive/scott-adams/games/))

<img src=".screenshots/first_screen.jpg"/>

The `advland.bin` file must be run on either a `Harmony` or `Melody` cartridge or in an emulator capable of emulating the ARM chip found in those cartridge types. [Stella](https://github.com/stella-emu/stella) is a good emulator for this and [Gopher2600](https://github.com/JetSetIlly/Gopher2600) is an alternative.

### Playing the Game

The game is played with the joystick plugged into the left-player port. Use the joystick to select a letter on the on-screen keyboard and the fire button to `type` the letter. The currently typed word appears just above the on-screen keyboard.

<img src=".screenshots/special_chars.png"/>

The above image shows the `special` characters on the keyboard. From left to right, the characters are `space`, `backspace` and `return`. These mimic the corresponding keys on a normal PC keyboard.

### Technical Details

As far as possible, the original C code has not been changed. Where it has been changed it is only for reasons of accomodating the limitations of the `Atari2600`. Changes made are documented in the `CHANGES.TXT` file in the `arm/advland` directory.

It is likely that some situations will cause the television image to roll briefly. This will depend on the television or emulator settings. The roll is caused by the advland program taking too long for the base 2600 hardware. Without changing more of the C code to accomodate these limitations there is very little that can be done to prevent this.

The `Atari2600` was in no way designed to display text so getting it to display `adventureland` is challenging. For this first version, I have written a very simple text renderer capable of drawing 15 columns of text. Each column can fit two letter `glyphs`, giving a 30 character display. The 15 columns are drawn in groups of five over three frames causing considerable flicker, so a television or emulator setting with high phosphor fade is recommended.

I believe it should be possible to have a 30 character display over two frames (rather than three) but I have not yet tackled that problem.

### Acknowledgements

Scott Adams for the original Adventureland

Morten Lohre for the C port

### Licence

All files with a licencing header are distributed under the BSD 2-Clause licence. All other files are (as far as I know) in the public domain.
