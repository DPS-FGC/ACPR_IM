#include"Overlay/FrameMeter/MoveData.h"
#include "Core/interfaces.h"

std::vector<CommandGrabData> MoveData::_activeByMarkCommandGrabs;
std::unordered_map<CharIdActIdKey, std::vector<MoveEntry>> MoveData::actIdToMoveIds;

void MoveData::Initialize()
{
    actIdToMoveIds[{ 1, 100 }].push_back({ (0, 0) }); //S Volcanic Viper
    actIdToMoveIds[{ 1, 101 }].push_back({ (1, 0) }); //S Volcanic Viper
    actIdToMoveIds[{ 1, 102 }].push_back({ (2, 0) }); //S Volcanic Viper
    actIdToMoveIds[{ 1, 102 }].push_back({ (2, 1) }); //Air S Volcanic Viper
    actIdToMoveIds[{ 1, 102 }].push_back({ (2, 2) }); //H Volcanic Viper
    actIdToMoveIds[{ 1, 102 }].push_back({ (2, 3) }); //Air H Volcanic Viper
    actIdToMoveIds[{ 1, 102 }].push_back({ (2, 16) }); //DI H Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (3, 0) }); //S Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (3, 1) }); //Air S Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (3, 2) }); //H Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (3, 3) }); //Air H Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (2, 14) }); //DI S Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (2, 15) }); //Air DI S Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (3, 16) }); //DI H Volcanic Viper
    actIdToMoveIds[{ 1, 103 }].push_back({ (2, 17) }); //Air DI H Volcanic Viper
    actIdToMoveIds[{ 1, 110 }].push_back({ (0, 1) }); //Air S Volcanic Viper
    actIdToMoveIds[{ 1, 111 }].push_back({ (1, 1) }); //Air S Volcanic Viper
    actIdToMoveIds[{ 1, 104 }].push_back({ (0, 2) }); //H Volcanic Viper
    actIdToMoveIds[{ 1, 105 }].push_back({ (1, 2) }); //H Volcanic Viper
    actIdToMoveIds[{ 1, 112 }].push_back({ (0, 3) }); //Air H Volcanic Viper
    actIdToMoveIds[{ 1, 113 }].push_back({ (1, 3) }); //Air H Volcanic Viper
    actIdToMoveIds[{ 1, 106 }].push_back({ (0, 4) }); //Ground Viper
    actIdToMoveIds[{ 1, 295 }].push_back({ (1, 4) }); //Ground Viper
    actIdToMoveIds[{ 1, 219 }].push_back({ (0, 5) }); //Bandit Revolver
    actIdToMoveIds[{ 1, 219 }].push_back({ (0, 7) }); //Bandit Bringer
    actIdToMoveIds[{ 1, 220 }].push_back({ (1, 5) }); //Bandit Revolver
    actIdToMoveIds[{ 1, 221 }].push_back({ (2, 5) }); //Bandit Revolver
    actIdToMoveIds[{ 1, 132 }].push_back({ (0, 6) }); //Air Bandit Revolver
    actIdToMoveIds[{ 1, 133 }].push_back({ (1, 6) }); //Air Bandit Revolver
    actIdToMoveIds[{ 1, 134 }].push_back({ (2, 6) }); //Air Bandit Revolver
    actIdToMoveIds[{ 1, 187 }].push_back({ (1, 7) }); //Bandit Bringer
    actIdToMoveIds[{ 1, 188 }].push_back({ (2, 7) }); //Bandit Bringer
    actIdToMoveIds[{ 1, 222 }].push_back({ (0, 8) }); //Riot Stamp
    actIdToMoveIds[{ 1, 211 }].push_back({ (1, 8) }); //Riot Stamp
    actIdToMoveIds[{ 1, 212 }].push_back({ (2, 8) }); //Riot Stamp
    actIdToMoveIds[{ 1, 130 }].push_back({ (3, 8) }); //Riot Stamp
    actIdToMoveIds[{ 1, 131 }].push_back({ (4, 8) }); //Riot Stamp
    actIdToMoveIds[{ 1, 136 }].push_back({ (0, 9) }); //Wild Throw
    actIdToMoveIds[{ 1, 135 }].push_back({ (1, 9) }); //Wild Throw
    actIdToMoveIds[{ 1, 272 }].push_back({ (0, 10) }); //Sidewinder
    actIdToMoveIds[{ 1, 273 }].push_back({ (1, 10) }); //Sidewinder
    actIdToMoveIds[{ 1, 273 }].push_back({ (1, 11) }); //FB Sidewinder
    actIdToMoveIds[{ 1, 340 }].push_back({ (0, 11) }); //FB Sidewinder
    actIdToMoveIds[{ 1, 343 }].push_back({ (0, 12) }); //Slam
    actIdToMoveIds[{ 1, 342 }].push_back({ (1, 12) }); //Slam
    actIdToMoveIds[{ 1, 126 }].push_back({ (0, 13) }); //Napalm Death
    actIdToMoveIds[{ 1, 127 }].push_back({ (1, 13) }); //Napalm Death
    actIdToMoveIds[{ 1, 124 }].push_back({ (0, 14) }); //DI S Volcanic Viper
    actIdToMoveIds[{ 1, 125 }].push_back({ (1, 14) }); //DI S Volcanic Viper
    actIdToMoveIds[{ 1, 245 }].push_back({ (0, 15) }); //Air DI S Volcanic Viper
    actIdToMoveIds[{ 1, 245 }].push_back({ (0, 17) }); //Air DI H Volcanic Viper
    actIdToMoveIds[{ 1, 246 }].push_back({ (1, 15) }); //Air DI S Volcanic Viper
    actIdToMoveIds[{ 1, 246 }].push_back({ (1, 17) }); //Air DI H Volcanic Viper
    actIdToMoveIds[{ 1, 128 }].push_back({ (0, 16) }); //DI H Volcanic Viper
    actIdToMoveIds[{ 1, 129 }].push_back({ (1, 16) }); //DI H Volcanic Viper
    actIdToMoveIds[{ 1, 257 }].push_back({ (0, 18) }); //DI Bandit Revolver
    actIdToMoveIds[{ 1, 258 }].push_back({ (1, 18) }); //DI Bandit Revolver
    actIdToMoveIds[{ 1, 259 }].push_back({ (2, 18) }); //DI Bandit Revolver
    actIdToMoveIds[{ 2, 181 }].push_back({ (0, 19) }); //Air S Stun Edge
    actIdToMoveIds[{ 2, 142 }].push_back({ (1, 19) }); //Air S Stun Edge
    actIdToMoveIds[{ 2, 142 }].push_back({ (1, 20) }); //Air H Stun Edge
    actIdToMoveIds[{ 2, 142 }].push_back({ (1, 28) }); //Air Charged Stun Edge
    actIdToMoveIds[{ 2, 190 }].push_back({ (0, 20) }); //Air H Stun Edge
    actIdToMoveIds[{ 2, 98 }].push_back({ (0, 21) }); //S Vapor Thrust
    actIdToMoveIds[{ 2, 99 }].push_back({ (1, 21) }); //S Vapor Thrust
    actIdToMoveIds[{ 2, 100 }].push_back({ (2, 21) }); //S Vapor Thrust
    actIdToMoveIds[{ 2, 100 }].push_back({ (2, 23) }); //Air S Vapor Thrust
    actIdToMoveIds[{ 2, 100 }].push_back({ (2, 24) }); //H Vapor Thrust
    actIdToMoveIds[{ 2, 100 }].push_back({ (2, 25) }); //Air H Vapor Thrust
    actIdToMoveIds[{ 2, 101 }].push_back({ (3, 21) }); //S Vapor Thrust
    actIdToMoveIds[{ 2, 101 }].push_back({ (3, 23) }); //Air S Vapor Thrust
    actIdToMoveIds[{ 2, 101 }].push_back({ (3, 24) }); //H Vapor Thrust
    actIdToMoveIds[{ 2, 101 }].push_back({ (3, 25) }); //Air H Vapor Thrust
    actIdToMoveIds[{ 2, 250 }].push_back({ (0, 22) }); //Lightning Javelin
    actIdToMoveIds[{ 2, 254 }].push_back({ (1, 22) }); //Lightning Javelin
    actIdToMoveIds[{ 2, 137 }].push_back({ (0, 23) }); //Air S Vapor Thrust
    actIdToMoveIds[{ 2, 137 }].push_back({ (0, 25) }); //Air H Vapor Thrust
    actIdToMoveIds[{ 2, 138 }].push_back({ (1, 23) }); //Air S Vapor Thrust
    actIdToMoveIds[{ 2, 138 }].push_back({ (1, 25) }); //Air H Vapor Thrust
    actIdToMoveIds[{ 2, 135 }].push_back({ (0, 24) }); //H Vapor Thrust
    actIdToMoveIds[{ 2, 136 }].push_back({ (1, 24) }); //H Vapor Thrust
    actIdToMoveIds[{ 2, 187 }].push_back({ (0, 26) }); //Stun Dipper
    actIdToMoveIds[{ 2, 188 }].push_back({ (1, 26) }); //Stun Dipper
    actIdToMoveIds[{ 2, 103 }].push_back({ (0, 27) }); //Greed Sever
    actIdToMoveIds[{ 2, 104 }].push_back({ (1, 27) }); //Greed Sever
    actIdToMoveIds[{ 2, 105 }].push_back({ (2, 27) }); //Greed Sever
    actIdToMoveIds[{ 2, 229 }].push_back({ (0, 28) }); //Air Charged Stun Edge
    actIdToMoveIds[{ 2, 111 }].push_back({ (0, 29) }); //Lightning Sphere
    actIdToMoveIds[{ 2, 251 }].push_back({ (1, 29) }); //Lightning Sphere
    actIdToMoveIds[{ 2, 307 }].push_back({ (0, 30) }); //FB Greed Sever
    actIdToMoveIds[{ 2, 308 }].push_back({ (1, 30) }); //FB Greed Sever
    actIdToMoveIds[{ 2, 309 }].push_back({ (2, 30) }); //FB Greed Sever
    actIdToMoveIds[{ 2, 170 }].push_back({ (0, 31) }); //Air Ride the Lightning
    actIdToMoveIds[{ 2, 171 }].push_back({ (1, 31) }); //Air Ride the Lightning
    actIdToMoveIds[{ 3, 65 }].push_back({ (0, 32) }); //Charged 6H
    actIdToMoveIds[{ 3, 238 }].push_back({ (1, 32) }); //Charged 6H
    actIdToMoveIds[{ 3, 239 }].push_back({ (2, 32) }); //Charged 6H
    actIdToMoveIds[{ 3, 63 }].push_back({ (0, 33) }); //Just Kidding~
    actIdToMoveIds[{ 3, 257 }].push_back({ (1, 33) }); //Just Kidding~
    actIdToMoveIds[{ 3, 131 }].push_back({ (0, 34) }); //S Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 132 }].push_back({ (1, 34) }); //S Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 128 }].push_back({ (2, 34) }); //S Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 128 }].push_back({ (2, 35) }); //H Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 115 }].push_back({ (3, 34) }); //S Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 115 }].push_back({ (3, 35) }); //H Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 133 }].push_back({ (0, 35) }); //H Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 134 }].push_back({ (1, 35) }); //H Mr. Dolphin Horizontal
    actIdToMoveIds[{ 3, 126 }].push_back({ (0, 36) }); //S Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 127 }].push_back({ (1, 36) }); //S Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 327 }].push_back({ (2, 36) }); //S Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 327 }].push_back({ (2, 37) }); //H Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 328 }].push_back({ (3, 36) }); //S Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 328 }].push_back({ (3, 37) }); //H Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 129 }].push_back({ (0, 37) }); //H Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 130 }].push_back({ (1, 37) }); //H Mr. Dolphin Vertical
    actIdToMoveIds[{ 3, 103 }].push_back({ (0, 38) }); //S Restive Rolling
    actIdToMoveIds[{ 3, 104 }].push_back({ (1, 38) }); //S Restive Rolling
    actIdToMoveIds[{ 3, 105 }].push_back({ (2, 38) }); //S Restive Rolling
    actIdToMoveIds[{ 3, 105 }].push_back({ (2, 39) }); //H Restive Rolling
    actIdToMoveIds[{ 3, 105 }].push_back({ (2, 40) }); //Air S Restive Rolling
    actIdToMoveIds[{ 3, 105 }].push_back({ (2, 41) }); //Air H Restive Rolling
    actIdToMoveIds[{ 3, 117 }].push_back({ (3, 38) }); //S Restive Rolling
    actIdToMoveIds[{ 3, 117 }].push_back({ (3, 39) }); //H Restive Rolling
    actIdToMoveIds[{ 3, 117 }].push_back({ (3, 40) }); //Air S Restive Rolling
    actIdToMoveIds[{ 3, 117 }].push_back({ (3, 41) }); //Air H Restive Rolling
    actIdToMoveIds[{ 3, 236 }].push_back({ (0, 39) }); //H Restive Rolling
    actIdToMoveIds[{ 3, 237 }].push_back({ (1, 39) }); //H Restive Rolling
    actIdToMoveIds[{ 3, 153 }].push_back({ (0, 40) }); //Air S Restive Rolling
    actIdToMoveIds[{ 3, 233 }].push_back({ (1, 40) }); //Air S Restive Rolling
    actIdToMoveIds[{ 3, 234 }].push_back({ (0, 41) }); //Air H Restive Rolling
    actIdToMoveIds[{ 3, 235 }].push_back({ (1, 41) }); //Air H Restive Rolling
    actIdToMoveIds[{ 3, 107 }].push_back({ (0, 42) }); //Overhead Kiss
    actIdToMoveIds[{ 3, 123 }].push_back({ (1, 42) }); //Overhead Kiss
    actIdToMoveIds[{ 3, 124 }].push_back({ (2, 42) }); //Overhead Kiss
    actIdToMoveIds[{ 3, 258 }].push_back({ (0, 43) }); //Jackhound
    actIdToMoveIds[{ 3, 259 }].push_back({ (1, 43) }); //Jackhound
    actIdToMoveIds[{ 3, 111 }].push_back({ (0, 44) }); //Ultimate Whiner
    actIdToMoveIds[{ 3, 112 }].push_back({ (1, 44) }); //Ultimate Whiner
    actIdToMoveIds[{ 3, 162 }].push_back({ (0, 45) }); //May and the Jolly Crew
    actIdToMoveIds[{ 3, 163 }].push_back({ (1, 45) }); //May and the Jolly Crew
    actIdToMoveIds[{ 3, 164 }].push_back({ (2, 45) }); //May and the Jolly Crew
    actIdToMoveIds[{ 4, 186 }].push_back({ (0, 46) }); //Iron Savior
    actIdToMoveIds[{ 4, 117 }].push_back({ (1, 46) }); //Iron Savior
    actIdToMoveIds[{ 4, 118 }].push_back({ (2, 46) }); //Iron Savior
    actIdToMoveIds[{ 4, 119 }].push_back({ (3, 46) }); //Iron Savior
    actIdToMoveIds[{ 4, 183 }].push_back({ (0, 47) }); //Bad Moon
    actIdToMoveIds[{ 4, 184 }].push_back({ (1, 47) }); //Bad Moon
    actIdToMoveIds[{ 4, 111 }].push_back({ (2, 47) }); //Bad Moon
    actIdToMoveIds[{ 4, 191 }].push_back({ (0, 48) }); //Lush Shaker
    actIdToMoveIds[{ 4, 98 }].push_back({ (1, 48) }); //Lush Shaker
    actIdToMoveIds[{ 4, 282 }].push_back({ (0, 49) }); //FB Secret Garden
    actIdToMoveIds[{ 4, 283 }].push_back({ (1, 49) }); //FB Secret Garden
    actIdToMoveIds[{ 4, 284 }].push_back({ (2, 49) }); //FB Secret Garden
    actIdToMoveIds[{ 4, 100 }].push_back({ (0, 50) }); //Winger
    actIdToMoveIds[{ 4, 101 }].push_back({ (1, 50) }); //Winger
    actIdToMoveIds[{ 4, 102 }].push_back({ (2, 50) }); //Winger
    actIdToMoveIds[{ 4, 103 }].push_back({ (3, 50) }); //Winger
    actIdToMoveIds[{ 4, 285 }].push_back({ (0, 51) }); //Air Winger
    actIdToMoveIds[{ 4, 286 }].push_back({ (1, 51) }); //Air Winger
    actIdToMoveIds[{ 5, 65 }].push_back({ (0, 52) }); //6H
    actIdToMoveIds[{ 5, 196 }].push_back({ (1, 52) }); //6H
    actIdToMoveIds[{ 5, 197 }].push_back({ (2, 52) }); //6H
    actIdToMoveIds[{ 5, 140 }].push_back({ (0, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 151 }].push_back({ (1, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 152 }].push_back({ (2, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 153 }].push_back({ (3, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 172 }].push_back({ (4, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 173 }].push_back({ (5, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 174 }].push_back({ (6, 53) }); //Rashou Sen
    actIdToMoveIds[{ 5, 208 }].push_back({ (0, 54) }); //Hachi Subako
    actIdToMoveIds[{ 5, 208 }].push_back({ (0, 55) }); //Shiranami no Homura
    actIdToMoveIds[{ 5, 209 }].push_back({ (1, 54) }); //Hachi Subako
    actIdToMoveIds[{ 5, 210 }].push_back({ (1, 55) }); //Shiranami no Homura
    actIdToMoveIds[{ 5, 145 }].push_back({ (0, 56) }); //S Raiei Sageki
    actIdToMoveIds[{ 5, 146 }].push_back({ (1, 56) }); //S Raiei Sageki
    actIdToMoveIds[{ 5, 147 }].push_back({ (2, 56) }); //S Raiei Sageki
    actIdToMoveIds[{ 5, 148 }].push_back({ (0, 57) }); //H Raiei Sageki
    actIdToMoveIds[{ 5, 149 }].push_back({ (1, 57) }); //H Raiei Sageki
    actIdToMoveIds[{ 5, 150 }].push_back({ (2, 57) }); //H Raiei Sageki
    actIdToMoveIds[{ 5, 200 }].push_back({ (3, 57) }); //H Raiei Sageki
    actIdToMoveIds[{ 5, 201 }].push_back({ (4, 57) }); //H Raiei Sageki
    actIdToMoveIds[{ 6, 182 }].push_back({ (0, 58) }); //Mega Fist (forward)
    actIdToMoveIds[{ 6, 183 }].push_back({ (1, 58) }); //Mega Fist (forward)
    actIdToMoveIds[{ 6, 184 }].push_back({ (2, 58) }); //Mega Fist (forward)
    actIdToMoveIds[{ 6, 185 }].push_back({ (0, 59) }); //Mega Fist (backward)
    actIdToMoveIds[{ 6, 186 }].push_back({ (1, 59) }); //Mega Fist (backward)
    actIdToMoveIds[{ 6, 187 }].push_back({ (2, 59) }); //Mega Fist (backward)
    actIdToMoveIds[{ 6, 120 }].push_back({ (0, 60) }); //Hammerfall
    actIdToMoveIds[{ 6, 120 }].push_back({ (0, 61) }); //Hammerfall Break
    actIdToMoveIds[{ 6, 121 }].push_back({ (1, 60) }); //Hammerfall
    actIdToMoveIds[{ 6, 174 }].push_back({ (1, 61) }); //Hammerfall Break
    actIdToMoveIds[{ 6, 122 }].push_back({ (0, 62) }); //Potemkin Buster
    actIdToMoveIds[{ 6, 123 }].push_back({ (1, 62) }); //Potemkin Buster
    actIdToMoveIds[{ 6, 124 }].push_back({ (2, 62) }); //Potemkin Buster
    actIdToMoveIds[{ 6, 125 }].push_back({ (3, 62) }); //Potemkin Buster
    actIdToMoveIds[{ 6, 114 }].push_back({ (0, 63) }); //Heat Knuckle
    actIdToMoveIds[{ 6, 114 }].push_back({ (0, 64) }); //Heat Extend
    actIdToMoveIds[{ 6, 115 }].push_back({ (1, 63) }); //Heat Knuckle
    actIdToMoveIds[{ 6, 116 }].push_back({ (1, 63) }); //Heat Knuckle
    actIdToMoveIds[{ 6, 116 }].push_back({ (1, 64) }); //Heat Extend
    actIdToMoveIds[{ 6, 117 }].push_back({ (2, 63) }); //Heat Knuckle
    actIdToMoveIds[{ 6, 118 }].push_back({ (2, 64) }); //Heat Extend
    actIdToMoveIds[{ 6, 119 }].push_back({ (3, 64) }); //Heat Extend
    actIdToMoveIds[{ 6, 130 }].push_back({ (0, 65) }); //Heavenly Potemkin Buster
    actIdToMoveIds[{ 6, 131 }].push_back({ (1, 65) }); //Heavenly Potemkin Buster
    actIdToMoveIds[{ 6, 109 }].push_back({ (0, 66) }); //Gigantic Bullet
    actIdToMoveIds[{ 6, 110 }].push_back({ (1, 66) }); //Gigantic Bullet
    actIdToMoveIds[{ 6, 111 }].push_back({ (2, 66) }); //Gigantic Bullet
    actIdToMoveIds[{ 6, 150 }].push_back({ (0, 67) }); //Magnum Opera
    actIdToMoveIds[{ 6, 151 }].push_back({ (1, 67) }); //Magnum Opera
    actIdToMoveIds[{ 7, 142 }].push_back({ (0, 68) }); //Genrou Zan You
    actIdToMoveIds[{ 7, 280 }].push_back({ (1, 68) }); //Genrou Zan You
    actIdToMoveIds[{ 7, 179 }].push_back({ (0, 69) }); //Senshuu
    actIdToMoveIds[{ 7, 180 }].push_back({ (1, 69) }); //Senshuu
    actIdToMoveIds[{ 7, 181 }].push_back({ (2, 69) }); //Senshuu
    actIdToMoveIds[{ 7, 223 }].push_back({ (0, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 96 }].push_back({ (1, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 96 }].push_back({ (3, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 97 }].push_back({ (2, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 97 }].push_back({ (4, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 98 }].push_back({ (5, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 103 }].push_back({ (6, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 99 }].push_back({ (7, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 100 }].push_back({ (8, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 102 }].push_back({ (9, 70) }); //Zansei Rouga
    actIdToMoveIds[{ 7, 138 }].push_back({ (0, 71) }); //Banki Messai
    actIdToMoveIds[{ 7, 139 }].push_back({ (1, 71) }); //Banki Messai
    actIdToMoveIds[{ 7, 140 }].push_back({ (2, 71) }); //Banki Messai
    actIdToMoveIds[{ 7, 215 }].push_back({ (3, 71) }); //Banki Messai
    actIdToMoveIds[{ 7, 151 }].push_back({ (0, 72) }); //Delta End
    actIdToMoveIds[{ 7, 152 }].push_back({ (1, 72) }); //Delta End
    actIdToMoveIds[{ 7, 153 }].push_back({ (2, 72) }); //Delta End
    actIdToMoveIds[{ 7, 155 }].push_back({ (2, 72) }); //Delta End
    actIdToMoveIds[{ 7, 156 }].push_back({ (3, 72) }); //Delta End
    actIdToMoveIds[{ 8, 227 }].push_back({ (0, 73) }); //Shadow Gallery
    actIdToMoveIds[{ 8, 228 }].push_back({ (1, 73) }); //Shadow Gallery
    actIdToMoveIds[{ 8, 229 }].push_back({ (2, 73) }); //Shadow Gallery
    actIdToMoveIds[{ 8, 163 }].push_back({ (0, 74) }); //Air Shadow Gallery
    actIdToMoveIds[{ 8, 164 }].push_back({ (1, 74) }); //Air Shadow Gallery
    actIdToMoveIds[{ 8, 165 }].push_back({ (2, 74) }); //Air Shadow Gallery
    actIdToMoveIds[{ 8, 166 }].push_back({ (3, 74) }); //Air Shadow Gallery
    actIdToMoveIds[{ 8, 184 }].push_back({ (0, 75) }); //Executor-X
    actIdToMoveIds[{ 8, 230 }].push_back({ (1, 75) }); //Executor-X
    actIdToMoveIds[{ 8, 231 }].push_back({ (2, 75) }); //Executor-X
    actIdToMoveIds[{ 8, 214 }].push_back({ (3, 75) }); //Executor-X
    actIdToMoveIds[{ 9, 249 }].push_back({ (0, 76) }); //Ouren
    actIdToMoveIds[{ 9, 250 }].push_back({ (1, 76) }); //Ouren
    actIdToMoveIds[{ 9, 251 }].push_back({ (2, 76) }); //Ouren
    actIdToMoveIds[{ 9, 226 }].push_back({ (0, 77) }); //Baku Followup K
    actIdToMoveIds[{ 9, 227 }].push_back({ (1, 77) }); //Baku Followup K
    actIdToMoveIds[{ 9, 228 }].push_back({ (2, 77) }); //Baku Followup K
    actIdToMoveIds[{ 10, 217 }].push_back({ (0, 78) }); //Going My Way
    actIdToMoveIds[{ 10, 218 }].push_back({ (1, 78) }); //Going My Way
    actIdToMoveIds[{ 10, 219 }].push_back({ (2, 78) }); //Going My Way
    actIdToMoveIds[{ 10, 123 }].push_back({ (0, 79) }); //From the Front
    actIdToMoveIds[{ 10, 124 }].push_back({ (1, 79) }); //From the Front
    actIdToMoveIds[{ 10, 124 }].push_back({ (1, 80) }); //From Behind
    actIdToMoveIds[{ 10, 124 }].push_back({ (2, 82) }); //FB From Behind
    actIdToMoveIds[{ 10, 168 }].push_back({ (0, 80) }); //From Behind
    actIdToMoveIds[{ 10, 168 }].push_back({ (0, 82) }); //FB From Behind
    actIdToMoveIds[{ 10, 125 }].push_back({ (0, 81) }); //From Above
    actIdToMoveIds[{ 10, 126 }].push_back({ (1, 81) }); //From Above
    actIdToMoveIds[{ 10, 127 }].push_back({ (2, 81) }); //From Above
    actIdToMoveIds[{ 10, 199 }].push_back({ (3, 81) }); //From Above
    actIdToMoveIds[{ 10, 312 }].push_back({ (1, 82) }); //FB From Behind
    actIdToMoveIds[{ 10, 102 }].push_back({ (0, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 10, 103 }].push_back({ (1, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 10, 104 }].push_back({ (1, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 10, 100 }].push_back({ (2, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 10, 101 }].push_back({ (3, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 10, 161 }].push_back({ (4, 83) }); //This Week s Yamaban
    actIdToMoveIds[{ 11, 142 }].push_back({ (0, 84) }); //Warrant
    actIdToMoveIds[{ 11, 143 }].push_back({ (1, 84) }); //Warrant
    actIdToMoveIds[{ 11, 144 }].push_back({ (2, 84) }); //Warrant
    actIdToMoveIds[{ 11, 167 }].push_back({ (0, 85) }); //Grave Digger
    actIdToMoveIds[{ 11, 172 }].push_back({ (1, 85) }); //Grave Digger
    actIdToMoveIds[{ 11, 173 }].push_back({ (2, 85) }); //Grave Digger
    actIdToMoveIds[{ 11, 174 }].push_back({ (0, 86) }); //Seventh Sign
    actIdToMoveIds[{ 11, 150 }].push_back({ (1, 86) }); //Seventh Sign
    actIdToMoveIds[{ 12, 330 }].push_back({ (0, 87) }); //3H
    actIdToMoveIds[{ 12, 331 }].push_back({ (1, 87) }); //3H
    actIdToMoveIds[{ 12, 332 }].push_back({ (2, 87) }); //3H
    actIdToMoveIds[{ 12, 103 }].push_back({ (0, 88) }); //Ryujin
    actIdToMoveIds[{ 12, 105 }].push_back({ (1, 88) }); //Ryujin
    actIdToMoveIds[{ 12, 106 }].push_back({ (2, 88) }); //Ryujin
    actIdToMoveIds[{ 12, 182 }].push_back({ (0, 89) }); //Gekirin
    actIdToMoveIds[{ 12, 184 }].push_back({ (1, 89) }); //Gekirin
    actIdToMoveIds[{ 12, 185 }].push_back({ (2, 89) }); //Gekirin
    actIdToMoveIds[{ 12, 189 }].push_back({ (0, 90) }); //Kenroukaku
    actIdToMoveIds[{ 12, 191 }].push_back({ (1, 90) }); //Kenroukaku
    actIdToMoveIds[{ 12, 192 }].push_back({ (2, 90) }); //Kenroukaku
    actIdToMoveIds[{ 12, 122 }].push_back({ (0, 91) }); //Senri Shinshou
    actIdToMoveIds[{ 12, 123 }].push_back({ (1, 91) }); //Senri Shinshou
    actIdToMoveIds[{ 12, 179 }].push_back({ (0, 92) }); //Houeikyaku
    actIdToMoveIds[{ 12, 180 }].push_back({ (1, 92) }); //Houeikyaku
    actIdToMoveIds[{ 12, 181 }].push_back({ (2, 92) }); //Houeikyaku
    actIdToMoveIds[{ 12, 133 }].push_back({ (0, 93) }); //Choukyaku Hououshou
    actIdToMoveIds[{ 12, 135 }].push_back({ (1, 93) }); //Choukyaku Hououshou
    actIdToMoveIds[{ 12, 136 }].push_back({ (2, 93) }); //Choukyaku Hououshou
    actIdToMoveIds[{ 12, 169 }].push_back({ (3, 93) }); //Choukyaku Hououshou
    actIdToMoveIds[{ 12, 290 }].push_back({ (0, 94) }); //Tousai Hyakuretsuken
    actIdToMoveIds[{ 12, 291 }].push_back({ (1, 94) }); //Tousai Hyakuretsuken
    actIdToMoveIds[{ 12, 292 }].push_back({ (1, 94) }); //Tousai Hyakuretsuken
    actIdToMoveIds[{ 12, 101 }].push_back({ (0, 95) }); //Gasenkotsu
    actIdToMoveIds[{ 12, 115 }].push_back({ (1, 95) }); //Gasenkotsu
    actIdToMoveIds[{ 12, 102 }].push_back({ (1, 95) }); //Gasenkotsu
    actIdToMoveIds[{ 12, 137 }].push_back({ (2, 95) }); //Gasenkotsu
    actIdToMoveIds[{ 13, 170 }].push_back({ (0, 96) }); //On
    actIdToMoveIds[{ 13, 157 }].push_back({ (1, 96) }); //On
    actIdToMoveIds[{ 13, 159 }].push_back({ (2, 96) }); //On
    actIdToMoveIds[{ 13, 159 }].push_back({ (2, 99) }); //FB On
    actIdToMoveIds[{ 13, 172 }].push_back({ (0, 97) }); //P Kai
    actIdToMoveIds[{ 13, 173 }].push_back({ (1, 97) }); //P Kai
    actIdToMoveIds[{ 13, 174 }].push_back({ (2, 97) }); //P Kai
    actIdToMoveIds[{ 13, 102 }].push_back({ (0, 98) }); //K Kai
    actIdToMoveIds[{ 13, 96 }].push_back({ (1, 98) }); //K Kai
    actIdToMoveIds[{ 13, 143 }].push_back({ (2, 98) }); //K Kai
    actIdToMoveIds[{ 13, 300 }].push_back({ (0, 99) }); //FB On
    actIdToMoveIds[{ 13, 301 }].push_back({ (1, 99) }); //FB On
    actIdToMoveIds[{ 13, 221 }].push_back({ (0, 100) }); //Tenjinkyaku
    actIdToMoveIds[{ 13, 222 }].push_back({ (1, 100) }); //Tenjinkyaku
    actIdToMoveIds[{ 13, 223 }].push_back({ (2, 100) }); //Tenjinkyaku
    actIdToMoveIds[{ 13, 224 }].push_back({ (2, 100) }); //Tenjinkyaku
    actIdToMoveIds[{ 13, 101 }].push_back({ (0, 101) }); //Zetsu
    actIdToMoveIds[{ 13, 99 }].push_back({ (1, 101) }); //Zetsu
    actIdToMoveIds[{ 13, 100 }].push_back({ (2, 101) }); //Zetsu
    actIdToMoveIds[{ 13, 110 }].push_back({ (3, 101) }); //Zetsu
    actIdToMoveIds[{ 14, 120 }].push_back({ (0, 102) }); // That s My Name 
    actIdToMoveIds[{ 14, 122 }].push_back({ (1, 102) }); // That s My Name 
    actIdToMoveIds[{ 14, 176 }].push_back({ (2, 102) }); // That s My Name 
    actIdToMoveIds[{ 14, 305 }].push_back({ (0, 103) }); //Uncho s Iai
    actIdToMoveIds[{ 14, 306 }].push_back({ (1, 103) }); //Uncho s Iai
    actIdToMoveIds[{ 14, 307 }].push_back({ (1, 103) }); //Uncho s Iai
    actIdToMoveIds[{ 14, 308 }].push_back({ (2, 103) }); //Uncho s Iai
    actIdToMoveIds[{ 14, 124 }].push_back({ (0, 104) }); //Joker Trick
    actIdToMoveIds[{ 14, 127 }].push_back({ (1, 104) }); //Joker Trick
    actIdToMoveIds[{ 14, 126 }].push_back({ (2, 104) }); //Joker Trick
    actIdToMoveIds[{ 15, 98 }].push_back({ (0, 105) }); //Double Head Morbid
    actIdToMoveIds[{ 15, 99 }].push_back({ (1, 105) }); //Double Head Morbid
    actIdToMoveIds[{ 15, 114 }].push_back({ (0, 106) }); //H Mad Struggle
    actIdToMoveIds[{ 15, 115 }].push_back({ (1, 106) }); //H Mad Struggle
    actIdToMoveIds[{ 15, 108 }].push_back({ (0, 107) }); //Dimmu Borgir
    actIdToMoveIds[{ 15, 161 }].push_back({ (1, 107) }); //Dimmu Borgir
    actIdToMoveIds[{ 15, 107 }].push_back({ (2, 107) }); //Dimmu Borgir
    actIdToMoveIds[{ 15, 165 }].push_back({ (3, 107) }); //Dimmu Borgir
    actIdToMoveIds[{ 16, 97 }].push_back({ (0, 108) }); //Necro Unleashed
    actIdToMoveIds[{ 16, 98 }].push_back({ (1, 108) }); //Necro Unleashed
    actIdToMoveIds[{ 17, 71 }].push_back({ (0, 109) }); //6K
    actIdToMoveIds[{ 17, 71 }].push_back({ (0, 110) }); //6[K]
    actIdToMoveIds[{ 17, 128 }].push_back({ (1, 109) }); //6K
    actIdToMoveIds[{ 17, 128 }].push_back({ (1, 110) }); //6[K]
    actIdToMoveIds[{ 17, 129 }].push_back({ (2, 109) }); //6K
    actIdToMoveIds[{ 17, 129 }].push_back({ (3, 110) }); //6[K]
    actIdToMoveIds[{ 17, 168 }].push_back({ (2, 110) }); //6[K]
    actIdToMoveIds[{ 17, 63 }].push_back({ (0, 111) }); //Dust Feint
    actIdToMoveIds[{ 17, 263 }].push_back({ (1, 111) }); //Dust Feint
    actIdToMoveIds[{ 17, 100 }].push_back({ (0, 112) }); //Crosswise Heel
    actIdToMoveIds[{ 17, 101 }].push_back({ (1, 112) }); //Crosswise Heel
    actIdToMoveIds[{ 17, 112 }].push_back({ (0, 113) }); //Bloodsucking Universe
    actIdToMoveIds[{ 17, 113 }].push_back({ (1, 113) }); //Bloodsucking Universe
    actIdToMoveIds[{ 17, 299 }].push_back({ (2, 113) }); //Bloodsucking Universe
    actIdToMoveIds[{ 17, 165 }].push_back({ (0, 114) }); //Dead on Time
    actIdToMoveIds[{ 17, 166 }].push_back({ (1, 114) }); //Dead on Time
    actIdToMoveIds[{ 17, 167 }].push_back({ (2, 114) }); //Dead on Time
    actIdToMoveIds[{ 17, 104 }].push_back({ (0, 115) }); //Eternal Wings
    actIdToMoveIds[{ 17, 105 }].push_back({ (1, 115) }); //Eternal Wings
    actIdToMoveIds[{ 17, 106 }].push_back({ (2, 115) }); //Eternal Wings
    actIdToMoveIds[{ 17, 163 }].push_back({ (0, 116) }); //Up and Close Dandy
    actIdToMoveIds[{ 17, 164 }].push_back({ (1, 116) }); //Up and Close Dandy
    actIdToMoveIds[{ 17, 126 }].push_back({ (0, 117) }); //All Dead
    actIdToMoveIds[{ 17, 127 }].push_back({ (1, 117) }); //All Dead
    actIdToMoveIds[{ 18, 101 }].push_back({ (0, 118) }); //P Sultry Performance
    actIdToMoveIds[{ 18, 102 }].push_back({ (1, 118) }); //P Sultry Performance
    actIdToMoveIds[{ 18, 103 }].push_back({ (2, 118) }); //P Sultry Performance
    actIdToMoveIds[{ 18, 110 }].push_back({ (0, 119) }); //K Sultry Performance
    actIdToMoveIds[{ 18, 111 }].push_back({ (1, 119) }); //K Sultry Performance
    actIdToMoveIds[{ 18, 112 }].push_back({ (2, 119) }); //K Sultry Performance
    actIdToMoveIds[{ 18, 107 }].push_back({ (0, 120) }); //S Sultry Performance
    actIdToMoveIds[{ 18, 108 }].push_back({ (1, 120) }); //S Sultry Performance
    actIdToMoveIds[{ 18, 109 }].push_back({ (2, 120) }); //S Sultry Performance
    actIdToMoveIds[{ 18, 104 }].push_back({ (0, 121) }); //H Sultry Performance
    actIdToMoveIds[{ 18, 105 }].push_back({ (1, 121) }); //H Sultry Performance
    actIdToMoveIds[{ 18, 106 }].push_back({ (2, 121) }); //H Sultry Performance
    actIdToMoveIds[{ 18, 263 }].push_back({ (0, 122) }); //FB Sultry Performance
    actIdToMoveIds[{ 18, 264 }].push_back({ (1, 122) }); //FB Sultry Performance
    actIdToMoveIds[{ 18, 134 }].push_back({ (0, 123) }); //Chemical Love (Horizontal)
    actIdToMoveIds[{ 18, 135 }].push_back({ (1, 123) }); //Chemical Love (Horizontal)
    actIdToMoveIds[{ 18, 241 }].push_back({ (0, 124) }); //Chemical Love (Vertical)
    actIdToMoveIds[{ 18, 242 }].push_back({ (1, 124) }); //Chemical Love (Vertical)
    actIdToMoveIds[{ 18, 117 }].push_back({ (0, 125) }); //P Antidepressant Scale
    actIdToMoveIds[{ 18, 117 }].push_back({ (0, 129) }); //FB Antidepressant Scale
    actIdToMoveIds[{ 18, 118 }].push_back({ (1, 125) }); //P Antidepressant Scale
    actIdToMoveIds[{ 18, 118 }].push_back({ (1, 126) }); //H Antidepressant Scale
    actIdToMoveIds[{ 18, 118 }].push_back({ (1, 129) }); //FB Antidepressant Scale
    actIdToMoveIds[{ 18, 269 }].push_back({ (0, 126) }); //H Antidepressant Scale
    actIdToMoveIds[{ 18, 124 }].push_back({ (0, 127) }); //Air P Antidepressant Scale
    actIdToMoveIds[{ 18, 124 }].push_back({ (0, 130) }); //Air FB Antidepressant Scale
    actIdToMoveIds[{ 18, 125 }].push_back({ (1, 127) }); //Air P Antidepressant Scale
    actIdToMoveIds[{ 18, 125 }].push_back({ (1, 128) }); //Air H Antidepressant Scale
    actIdToMoveIds[{ 18, 125 }].push_back({ (1, 130) }); //Air FB Antidepressant Scale
    actIdToMoveIds[{ 18, 132 }].push_back({ (2, 127) }); //Air P Antidepressant Scale
    actIdToMoveIds[{ 18, 132 }].push_back({ (2, 128) }); //Air H Antidepressant Scale
    actIdToMoveIds[{ 18, 132 }].push_back({ (2, 130) }); //Air FB Antidepressant Scale
    actIdToMoveIds[{ 18, 270 }].push_back({ (0, 128) }); //Air H Antidepressant Scale
    actIdToMoveIds[{ 18, 126 }].push_back({ (0, 131) }); //Last Will and Testament
    actIdToMoveIds[{ 18, 127 }].push_back({ (1, 131) }); //Last Will and Testament
    actIdToMoveIds[{ 19, 65 }].push_back({ (0, 132) }); //6H
    actIdToMoveIds[{ 19, 117 }].push_back({ (1, 132) }); //6H
    actIdToMoveIds[{ 19, 118 }].push_back({ (2, 132) }); //6H
    actIdToMoveIds[{ 19, 126 }].push_back({ (0, 133) }); //I m Scared 
    actIdToMoveIds[{ 19, 127 }].push_back({ (1, 133) }); //I m Scared 
    actIdToMoveIds[{ 20, 133 }].push_back({ (0, 134) }); //Shoot
    actIdToMoveIds[{ 20, 134 }].push_back({ (1, 134) }); //Shoot
    actIdToMoveIds[{ 20, 156 }].push_back({ (0, 135) }); //Maintenance Disaster
    actIdToMoveIds[{ 20, 157 }].push_back({ (1, 135) }); //Maintenance Disaster
    actIdToMoveIds[{ 20, 158 }].push_back({ (2, 135) }); //Maintenance Disaster
    actIdToMoveIds[{ 21, 62 }].push_back({ (0, 136) }); //6P
    actIdToMoveIds[{ 21, 274 }].push_back({ (1, 136) }); //6P
    actIdToMoveIds[{ 21, 275 }].push_back({ (2, 136) }); //6P
    actIdToMoveIds[{ 21, 118 }].push_back({ (0, 137) }); //Throw
    actIdToMoveIds[{ 21, 119 }].push_back({ (1, 137) }); //Throw
    actIdToMoveIds[{ 21, 120 }].push_back({ (2, 137) }); //Throw
    actIdToMoveIds[{ 21, 229 }].push_back({ (0, 138) }); //Fun-KY Lv1
    actIdToMoveIds[{ 21, 230 }].push_back({ (1, 138) }); //Fun-KY Lv1
    actIdToMoveIds[{ 21, 220 }].push_back({ (2, 138) }); //Fun-KY Lv1
    actIdToMoveIds[{ 21, 231 }].push_back({ (3, 138) }); //Fun-KY Lv1
    actIdToMoveIds[{ 21, 232 }].push_back({ (0, 139) }); //Fun-KY Lv2
    actIdToMoveIds[{ 21, 233 }].push_back({ (1, 139) }); //Fun-KY Lv2
    actIdToMoveIds[{ 21, 221 }].push_back({ (2, 139) }); //Fun-KY Lv2
    actIdToMoveIds[{ 21, 234 }].push_back({ (3, 139) }); //Fun-KY Lv2
    actIdToMoveIds[{ 21, 235 }].push_back({ (0, 140) }); //Fun-KY Lv3
    actIdToMoveIds[{ 21, 236 }].push_back({ (1, 140) }); //Fun-KY Lv3
    actIdToMoveIds[{ 21, 222 }].push_back({ (2, 140) }); //Fun-KY Lv3
    actIdToMoveIds[{ 21, 237 }].push_back({ (3, 140) }); //Fun-KY Lv3
    actIdToMoveIds[{ 21, 97 }].push_back({ (0, 141) }); //Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 98 }].push_back({ (1, 141) }); //Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 101 }].push_back({ (2, 141) }); //Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 101 }].push_back({ (2, 142) }); //Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 101 }].push_back({ (3, 143) }); //Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 101 }].push_back({ (2, 144) }); //Air Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 101 }].push_back({ (2, 145) }); //Air Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 101 }].push_back({ (2, 146) }); //Air Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 99 }].push_back({ (0, 142) }); //Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 100 }].push_back({ (1, 142) }); //Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 135 }].push_back({ (0, 143) }); //Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 136 }].push_back({ (1, 143) }); //Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 108 }].push_back({ (1, 143) }); //Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 109 }].push_back({ (2, 143) }); //Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 349 }].push_back({ (0, 144) }); //Air Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 350 }].push_back({ (1, 144) }); //Air Hun-KY Homerun Lv1
    actIdToMoveIds[{ 21, 351 }].push_back({ (0, 145) }); //Air Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 352 }].push_back({ (1, 145) }); //Air Hun-KY Homerun Lv2
    actIdToMoveIds[{ 21, 353 }].push_back({ (0, 146) }); //Air Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 354 }].push_back({ (1, 146) }); //Air Hun-KY Homerun Lv3
    actIdToMoveIds[{ 21, 223 }].push_back({ (0, 147) }); //S-KY-Line
    actIdToMoveIds[{ 21, 224 }].push_back({ (1, 147) }); //S-KY-Line
    actIdToMoveIds[{ 21, 225 }].push_back({ (2, 147) }); //S-KY-Line
    actIdToMoveIds[{ 21, 170 }].push_back({ (0, 148) }); //What s Useless Will Always Be So
    actIdToMoveIds[{ 21, 251 }].push_back({ (1, 148) }); //What s Useless Will Always Be So
    actIdToMoveIds[{ 21, 252 }].push_back({ (0, 149) }); //Follow-up Attack
    actIdToMoveIds[{ 22, 309 }].push_back({ (0, 150) }); //Engorgement
    actIdToMoveIds[{ 22, 310 }].push_back({ (1, 150) }); //Engorgement
    actIdToMoveIds[{ 22, 311 }].push_back({ (2, 150) }); //Engorgement
    actIdToMoveIds[{ 22, 103 }].push_back({ (0, 151) }); //Darkness of Ignorance
    actIdToMoveIds[{ 22, 104 }].push_back({ (1, 151) }); //Darkness of Ignorance
    actIdToMoveIds[{ 22, 105 }].push_back({ (2, 151) }); //Darkness of Ignorance
    actIdToMoveIds[{ 22, 106 }].push_back({ (3, 151) }); //Darkness of Ignorance
    actIdToMoveIds[{ 23, 334 }].push_back({ (0, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 335 }].push_back({ (1, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 336 }].push_back({ (2, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 337 }].push_back({ (3, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 338 }].push_back({ (4, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 339 }].push_back({ (5, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 341 }].push_back({ (6, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 343 }].push_back({ (7, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 344 }].push_back({ (8, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 345 }].push_back({ (9, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 23, 346 }].push_back({ (10, 152) }); //Dragon Install: Sakkai
    actIdToMoveIds[{ 24, 63 }].push_back({ (0, 153) }); //5D
    actIdToMoveIds[{ 24, 107 }].push_back({ (1, 153) }); //5D
    actIdToMoveIds[{ 24, 254 }].push_back({ (2, 153) }); //5D
    actIdToMoveIds[{ 24, 62 }].push_back({ (0, 154) }); //6[P]
    actIdToMoveIds[{ 24, 255 }].push_back({ (1, 154) }); //6[P]
    actIdToMoveIds[{ 24, 256 }].push_back({ (2, 154) }); //6[P]
    actIdToMoveIds[{ 24, 65 }].push_back({ (0, 155) }); //6[H]
    actIdToMoveIds[{ 24, 249 }].push_back({ (1, 155) }); //6[H]
    actIdToMoveIds[{ 24, 250 }].push_back({ (2, 155) }); //6[H]
    actIdToMoveIds[{ 24, 32 }].push_back({ (0, 156) }); //2[H]
    actIdToMoveIds[{ 24, 251 }].push_back({ (1, 156) }); //2[H]
    actIdToMoveIds[{ 24, 252 }].push_back({ (2, 156) }); //2[H]
    actIdToMoveIds[{ 24, 114 }].push_back({ (0, 157) }); //Skull Crusher
    actIdToMoveIds[{ 24, 115 }].push_back({ (1, 157) }); //Skull Crusher
    actIdToMoveIds[{ 24, 111 }].push_back({ (0, 158) }); //Nape Saddle
    actIdToMoveIds[{ 24, 112 }].push_back({ (1, 158) }); //Nape Saddle
    actIdToMoveIds[{ 24, 113 }].push_back({ (2, 158) }); //Nape Saddle
    actIdToMoveIds[{ 24, 208 }].push_back({ (0, 159) }); //Limb Severer
    actIdToMoveIds[{ 24, 209 }].push_back({ (1, 159) }); //Limb Severer
    actIdToMoveIds[{ 24, 103 }].push_back({ (0, 160) }); //Pulverizing Dragon s Roar
    actIdToMoveIds[{ 24, 105 }].push_back({ (1, 160) }); //Pulverizing Dragon s Roar
    actIdToMoveIds[{ 24, 106 }].push_back({ (2, 160) }); //Pulverizing Dragon s Roar

    _activeByMarkCommandGrabs = {
        {1, 0x088, 1}, // Wild Throw
        {2, 0x109, 2}, // EX Ky Elegant Slash
        {3, 0x06B, 4}, // Overhead Kiss
        {3, 0x0A2, 15}, // IK
        {6, 0x07A, 3}, // Potbuster
        {7, 0x08F, 6}, // Leaf Grab
        {7, 0x130, 22}, // EX Chipp grab super
        {8, 0x07C, 5}, // Damned Fang
        {9, 0x109, 23}, // EX Baiken grab super
        {12, 0x0EE, 14}, // Unknown (Unused)
        {17, 0x070, 18}, // BSU
        {19, 0x07E, 20}, // IK
        {20, 0x11B, 24}, // EX Bridget grab super
        {21, 0x0DF, 21}, // S-KY-line
        {22, 0x112, 25}, // Close Key Grab
        {22, 0x113, 25}, // Moroha/ABA EX Close Key Grab
        {22, 0x11A, 26}, // Unknown (Unused Air keygrab?) actId 282, cmdGrabId 26
        {22, 0x11B, 26} // Unknown (Unused Air keygrab?) actId 283, cmdGrabId 26
    };
}

bool MoveData::IsPrevAnimSameMove(uint16_t charId, uint16_t actId1, uint16_t actId2)
{
    CharIdActIdKey key1 = { charId, actId1 };
    CharIdActIdKey key2 = { charId, actId2 };
    if (actIdToMoveIds.find(key1) != actIdToMoveIds.end() && actIdToMoveIds.find(key2) != actIdToMoveIds.end())
    {
        std::vector<MoveEntry> data1 = actIdToMoveIds[key1];
        std::vector<MoveEntry> data2 = actIdToMoveIds[key2];
        for (int i = 0; i < data1.size(); i++)
        {
            for (int j = 0; j < data2.size(); j++)
            {
                if (data2[j].moveId == data1[i].moveId && data2[j].sequenceIndex > data1[i].sequenceIndex)
                    return true;
            }
        }
    }
    return false;
}

bool MoveData::IsActiveByMark(uint16_t charId, uint16_t actId)
{
    for (int i = 0; i < _activeByMarkCommandGrabs.size(); i++)
    {
        if (_activeByMarkCommandGrabs[i].CharId != charId || _activeByMarkCommandGrabs[i].ActId != actId)
            continue;
        return true;
    }
    return false;
}

#define SPECIAL_CASE_COMMAND_THROW_ID 0x19
#define SPECIAL_CASE_COMMAND_THROW_RANGE 11000
int MoveData::GetCommandGrabRange(uint16_t charId, uint16_t actId)
{
    uint16_t cmdGrabId = 0;
    for (int i = 0; i < _activeByMarkCommandGrabs.size(); i++)
    {
        if (_activeByMarkCommandGrabs[i].CharId == charId && _activeByMarkCommandGrabs[i].ActId == actId)
        {
            cmdGrabId = _activeByMarkCommandGrabs[i].CmdGrabId;
            break;
        }
    }

    if (cmdGrabId == SPECIAL_CASE_COMMAND_THROW_ID) { return SPECIAL_CASE_COMMAND_THROW_RANGE; }

    return g_gameVals.pCommandGrabThrowRanges[cmdGrabId];
}