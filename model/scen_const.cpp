/**
	AOK Trigger Studio (See aokts.cpp for legal conditions.)
	WINDOWS VERSION
	scen_const.cpp -- Defines various scenario constants in scen.h

	MODEL
**/

#include "scen_const.h"

size_t MapSizes[] =
{
	120,	//Tiny
	144,	//Small
	168,	//Medium
	200,	//Normal
	220,	//Large
	240,	//Giant
	480		//Ludakris
};

const char *message_names[] =
{
	"Instructions", "Hints", "Victory", "Loss", "History", "Scouts"
};

const char *cinem_names[] =
{
	"Pre-Game Cinematic", "Victory Cinematic", "Loss Cinematic", "Instruction Background"
};

const char *difficulties_18[] =
{
	"Easiest", "Easy", "Moderate", "Hard", "Hardest"
};

const char *difficulties_21[] =
{
	"Easy", "Standard", "Moderate", "Hard", "Hardest"
};

const char *ages[] =
{
	"Dark", "Feudal", "Castle", "Imperial", "Post-Imperial"
};

const struct PAIR groups[] =
{
	{ 0x00, "Archer" },
	{ 0x01, "Artifact" },
	{ 0x02, "Trade Boat" },
	{ 0x03, "Building" },
	{ 0x04, "Civilian" },
	{ 0x05, "Sea Fish" },
	{ 0x06, "Soldier" },
	{ 0x07, "Berry Bush" },
	{ 0x08, "Stone Mine" },
	{ 0x09, "Prey Animal" },
	{ 0x0A, "Predator Animal" },
	{ 0x0B, "Other" },
	{ 0x0C, "Cavalry" },
	{ 0x0D, "Siege Weapon" },
	{ 0x0E, "Map Decoration" },
	{ 0x0F, "Tree" },
	{ 0x12, "Priest" },
	{ 0x13, "Trade Cart" },
	{ 0x14, "Transport Boat" },
	{ 0x15, "Fishing Boat" },
	{ 0x16, "War Boat" },
	{ 0x17, "Conquistador" },
	{ 0x1B, "Walls" },
	{ 0x1C, "Phalanx" },
	{ 0x1E, "Flags" },
	{ 0x20, "Gold Mine" },
	{ 0x21, "Shore Fish" },
	{ 0x22, "Cliff" },
	{ 0x23, "Petard" },
	{ 0x24, "Cavalry Archer" },
	{ 0x25, "Dolphin" },
	{ 0x26, "Birds" },	//only FLDOG, lol
	{ 0x27, "Gates" },
	{ 0x28, "Piles" },
	{ 0x29, "Resource" },
	{ 0x2A, "Relic" },
	{ 0x2B, "Monk with Relic" },
	{ 0x2C, "Hand Cannoneer" },
	{ 0x2D, "Two-Handed Swordsman" },
	{ 0x2E, "Pikeman" },
	{ 0x2F, "Scout Cavalry" },
	{ 0x30, "Ore Mine" },
	{ 0x31, "Farm" },
	{ 0x32, "Spearman" },
	{ 0x33, "Packed Siege Units" },
	{ 0x34, "Tower" },
	{ 0x35, "Boarding Boat" },
	{ 0x36, "Unpacked Siege Units" },
	{ 0x37, "Scorpion" },
	{ 0x38, "Raider" },
	{ 0x39, "Cavalry Raider" },
	{ 0x3A, "Sheep" },
	{ 0x3B, "King" },	//doesn't include King?
	{ 0x3D, "Horse" }
};

const struct PAIR utypes[] =
{
	{ -1, "None" },
	{  1, "Other" },
	{  2, "Building" },
	{  3, "Civilian" },
	{  4, "Military" }
};
