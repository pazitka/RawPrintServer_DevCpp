#### Hello small part of the world I'm:
                     _ _   _         
                    (_) | | |        
     _ __   __ _ _____| |_| | ____ _ 
    | '_ \ / _` |_  / | __| |/ / _` |
    | |_) | (_| |/ /| | |_|   < (_| |
    | .__/ \__,_/___|_|\__|_|\_\__,_|
    | |                              
    |_|                              
                               
About Pazitka person who made this fork:
========================================
I'm pazitka and that means chive in english. In Poland we have another word: szczypiorek.
In the surrounding cities of Greater Poland, people from my hometown are called onions. When the onion blooms, we get chives.
When I moved to the south of Poland, closer to the Czech Republic, I adopted the nickname pazitka, which means chives in Czech.

## About RawPrintServer
This project is only a fork. To see original one please go to [sourceforge][] or you may also visit [miso-limps][]. This fork came from Miso any way.

[sourceforge]: https://sourceforge.net/projects/rawprintserver/
[miso-limps]: https://github.com/miso-lims/rawprintserver

# Requirements
My fork of orignal project was made to be compiled with Dev-C++. So you will need to get one.

## BloodShed Dev-C++
I do not recommend to use original project unless you want to upgrade it with up to date G++ and GCC. You may want to visit their website any way [bloodshed][].

[bloodshed]: https://www.bloodshed.net/

## Embarcadero Dev-C++ Fork
If you are not using Windows XP in some virtual machine or old PC than you should get Embarcadero's Dev-C++ [embarcadero_devcpp][].

[embarcadero_devcpp]: https://www.embarcadero.com/free-tools/dev-cpp

## Important!
You will need to modify `MakefileCustom.win` in there there is fixed path to G++, linkers and libraries and that need to be changed.
After making changes please make sure that flag `-lws2_32` is set inside project. This way you will have less problems with compilation.
I have added `WinSpool.h` and `WinSpool.lib` to the project to make sure that compilation will work. Normaly G++ have some problems with working with printers.

# Original project author
     _____                                                                                  _____ 
    ( ___ )--------------------------------------------------------------------------------( ___ )
     |   |      RawPrintServer 1.00 created by Henk Jonas (www.metaviewsoft.de)             |   | 
     |   |                                                                                  |   | 
     |___|      This is 32bit app! Version compiled in Dev-C++                              |___| 
    (_____)--------------------------------------------------------------------------------(_____)
Plase make sure that you read readme.txt as well.

