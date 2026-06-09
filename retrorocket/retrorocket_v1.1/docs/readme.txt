***************
* RetroRocket *
***************

http://www.retrorocketgame.org

1. About
2. Installation
3. TurboRaketti maps
4. Thrust maps
5. Strategy maps
6. Customization
7. Troubleshooting

1. About
========

RetroRocket is a high paced action game inspired by classic gravity
games like Thrust and TurboRaketti. It is a game designed for Nintendo
DS and requires a homebrew compatible flash card. RetroRocket allows
both single player or multiplayer DS-to-DS gaming. RetroRocket is free
and open source (GPL).

Thanks to Karstein Djupdal for composing the original RetroRocket
soundtrack, Cruising in Space. Thanks to Kyrre Glette for coding the
world generator.

For distribution terms, see the file COPYING.

2. Installation
===============

To install the game, extract the archive to your DS flash card. Make
sure the retrorocket_data directory is in the root directory of the
flash card. The executeable retrorocket.nds (or retrorocket.ds.gba)
can be placed anywhere on the flash card.

3. RetroRocket Game Modes
=============

Currently, RetroRocket support four different game mode:
- TurboRaketti Survival (Section 4)
- TurboRaketti Kills    (Section 4)
- Thrust                (Section 5)
- Strategy              (Section 6)

These games are described in more detail in the following sections.
Your choice of game mode and other settings will affect the maps you
can use for your game.

4. TurboRaketti
===============

All original TurboRaketti-II maps (TR) are included. The goal is
either to beat your opponent or alternatively try to achieve the best
lap times.

Two different game modes are supported:
- Survival: The player who still has lives left when the others are 
            dead wins the game.
- Kills:    The first player to reach a certain number of kills wins
            the game.

While playing the game, you can configure your ship by landing on your
platform and pressing the select button. Here, you can choose between
different weapons as well as modify the amount of fuel and ammunition
in your ship. It is also possible to save your favourite configuration
to reduce the configuration time as this can be the difference between 
success and failure in a heated battle.

5. Thrust maps
==============

All the levels from the original Thrust game are included. The goal is
to pick up the balls and carry them to outer space. In order to pick
up balls, the tractor beam needs to be activated while pulling away
from the ball.

6. Strategy maps
================

The goal of the real time strategy (RTS) maps is to conquer your
opponents by destroying their home platform. Your ship starts as a
downgraded version of the TurboRaketti ships. In order to progress in
the game, you must earn cash by placing gathering nodes on available
resources. Cash can be spent on buying weapons, upgrades and
ammunition.

6.1 Weapons
-----------

There are two categories of weapons, A and B. Weapons of type A are
high capacity fully automatic guns with inexpensive
ammunition. Weapons of type B are semi automatic guns with limited
capacity and high damage ammunition. The weapon B slot can
alternatively be loaded with building seeds (see 6.2).

In order to use a weapon, you must first buy the weapon and then
ammunition for that weapon. Finally, the ship must be loaded with the
correct weapon in the ship configuration menu.

Note: The nuclear bomb is delivered from the back of the ship. All
      other weapons are delivered from the front of the ship.

6.2 Buildings you can place
---------------------------

Placing buildings is done by buying a building seed (weapon B) and
shooting it at the location you want the building to be placed. If you
shoot the seed at a location where the building can not be placed, the
seed is lost.

6.2.1 Resource Nodes

Resource nodes are placed by shooting the seed at an available
resource. The node will gather cash as long as it is not destroyed.

Note: Resource Nodes are delivered from the back of the ship.

6.2.2 Sentry Guns

Sentry guns are placed by shooting the seed into a wall, at a location
not too close to existing buildings or resources. The sentry gun will
fire at opponents if they get within the guns tracking range. The
sentry gun will track targets within a sector defined by the angle at
which the building seed hits the ground.

7. Customization
================

RetroRocket uses normal text files defining all game types and
parameters. If you make changes to the configuration files, make sure
all players have the exact same configuration files. Failing to do so
will result in a CRC error.

8. Troubleshooting
==================

9.1 Network issues
------------------

The DS-to-DS networking library used by RetroRocket (liblobby) is work
in progress and has some bugs and limitations. Although RetroRocket
tries to work around these limitations, some situations may cause
problems. The problems are:

9.1.1 Random crashes in network games

The library does not fully tolerate alien network traffic on the same
wireless channel. If you experience random crashes, change your network
channels. Make sure all players use the same channel.

9.1.2 Can not find rooms

The library does not fully handle the use of rooms. If you experience
problems with finding each others rooms, all players should restart
their DS. Also, make sure all players use the same network channel.
Channel 11 is the default channel, and not all DS units support changing
the channel.

