#define MAXHEALTH (100)
#define MAXSPEED (100)
#define COLOR_SPARK (0xFF9922)
#define COLOR_STEAM (0x897661)
#define COLOR_BLOOD (0x60FFFF)
#define COLOR_WATER (0xFF0000)
#define COLOR_EDIT (0xFFFFFF)
#define CHAIN_COOLDOWN (2000)
#define STUNDELAY (1458)
#define DODGE_MULIPLIER (5.0f)
#define DODGE_TIME (400)
#define DODGE_SPEED (40)
#define HEADSHOT_DIST (30)

enum {
MODEL_SP_SWORDMAN_BODY=0,
MODEL_SP_SWORDMAN_SWORD,
MODEL_JP_SWORDMAN_BODY,
MODEL_JP_SWORDMAN_SWORD,
MODEL_SP_MUSKET_BODY,
MODEL_SP_MUSKET_WEAPON,
MODEL_SP_MUSKET_STICK,
MODEL_JP_MUSKET_BODY,
MODEL_JP_MUSKET_WEAPON,
MODEL_JP_MUSKET_STICK,
MODEL_SP_FLAG,
MODEL_JP_FLAG,
MODEL_CANNONBALL,
MODEL_CANNON,
MODEL_CANNON_CARRY,
MODEL_TELEPORT,
MODEL_WHITE_FLAG,
MODEL_HANDPOINT,
NUMMODELS
};

enum {
S_JUMP=0,
S_LAND,
S_SPLASHIN,
S_SPLASHOUT,
S_BURN,
S_ITEMSPAWN,
S_TELEPORT,
S_JUMPPAD,
S_SWOOSH,
S_PARRY_SWORD,
S_PULSE2,
S_PULSEEXPLODE,
S_RAIL1,
S_RAIL2,
S_WEAPLOAD,
S_NOAMMO,
S_HIT,
S_ATTACK_START,
S_PAIN1,
S_PAIN2,
S_DIE1,
S_DIE2,
S_FLAGPICKUP,
S_FLAGDROP,
S_FLAGRETURN,
S_FLAGSCORE,
S_FLAGRESET,
S_FLAGFAIL,
S_MUSKET_FIRE,
S_CANNON,
S_JP_FIRE,
S_MUSKET_STOCK,
NUMSOUNDS
};

enum {
N_DUMMY=0,
N_CONNECT,
N_SERVINFO,
N_WELCOME,
N_INITCLIENT,
N_POS,
N_TEXT,
N_SOUND,
N_CDIS,
N_USE_SKILL,
N_COMBAT,
N_SUICIDE,
N_DIED,
N_HITPUSH,
N_TRYSPAWN,
N_SPAWNSTATE,
N_SPAWN,
N_FORCEDEATH,
N_TAUNT,
N_MAPCHANGE,
N_MAPVOTE,
N_TEAMINFO,
N_ITEMSPAWN,
N_ITEMPICKUP,
N_ITEMACC,
N_TELEPORT,
N_JUMPPAD,
N_PING,
N_PONG,
N_CLIENTPING,
N_TIMEUP,
N_FORCEINTERMISSION,
N_SERVMSG,
N_ITEMLIST,
N_RESUME,
N_EDITMODE,
N_EDITENT,
N_EDITF,
N_EDITT,
N_EDITM,
N_FLIP,
N_COPY,
N_PASTE,
N_ROTATE,
N_REPLACE,
N_DELCUBE,
N_CALCLIGHT,
N_REMIP,
N_EDITVSLOT,
N_UNDO,
N_REDO,
N_NEWMAP,
N_GETMAP,
N_SENDMAP,
N_CLIPBOARD,
N_EDITVAR,
N_MASTERMODE,
N_KICK,
N_CLEARBANS,
N_CURRENTMASTER,
N_SPECTATOR,
N_SETMASTER,
N_SETTEAM,
N_LISTDEMOS,
N_SENDDEMOLIST,
N_GETDEMO,
N_SENDDEMO,
N_DEMOPLAYBACK,
N_RECORDDEMO,
N_STOPDEMO,
N_CLEARDEMOS,
N_TAKEFLAG,
N_RETURNFLAG,
N_RESETFLAG,
N_TRYDROPFLAG,
N_DROPFLAG,
N_SCOREFLAG,
N_INITFLAGS,
N_SAYTEAM,
N_CLIENT,
N_AUTHTRY,
N_AUTHKICK,
N_AUTHCHAL,
N_AUTHANS,
N_REQAUTH,
N_PAUSEGAME,
N_GAMESPEED,
N_ADDBOT,
N_DELBOT,
N_INITAI,
N_FROMAI,
N_BOTLIMIT,
N_BOTBALANCE,
N_MAPCRC,
N_CHECKMAPS,
N_SWITCHNAME,
N_SWITCHCLASS,
N_SWITCHTEAM,
N_AMMO_ADD,
N_SWITCH_CLOAK_MODE,
N_CLOAK_MODE,
N_SERVCMD,
N_DEMOPACKET,
N_KNOCKBACK,
N_USE_ENTITY,
N_GRAB_ENTITY,
N_ITEMDROP,
N_INITDOMINATION,
N_STARTCAPTURE,
N_CANCELCAPTURE,
N_CAPTURED,
N_DOMINATIONSCORES,
N_CLIENTUPDATE,
NUMMSG
};

enum {
NOTUSED=ET_EMPTY,
LIGHT=ET_LIGHT,
MAPMODEL=ET_MAPMODEL,
PLAYERSTART,
ENVMAP=ET_ENVMAP,
PARTICLES=ET_PARTICLES,
MAPSOUND=ET_SOUND,
SPOTLIGHT=ET_SPOTLIGHT,
DECAL=ET_DECAL,
TELEPORT,
TELEDEST,
JUMPPAD,
FLAG,
LADDER,
CANNON,
CAPTUREPOINT,
POINTOFINTEREST,
MMSTART,
MAXENTTYPES, I_FIRST = 0, I_LAST = -1
};

enum {
ANIM_DEAD=ANIM_GAMESPECIFIC,
ANIM_DYING,
ANIM_IDLE,
ANIM_RUN_N,
ANIM_RUN_NE,
ANIM_RUN_E,
ANIM_RUN_SE,
ANIM_RUN_S,
ANIM_RUN_SW,
ANIM_RUN_W,
ANIM_RUN_NW,
ANIM_JUMP,
ANIM_JUMP_N,
ANIM_JUMP_NE,
ANIM_JUMP_E,
ANIM_JUMP_SE,
ANIM_JUMP_S,
ANIM_JUMP_SW,
ANIM_JUMP_W,
ANIM_JUMP_NW,
ANIM_SINK,
ANIM_SWIM,
ANIM_CROUCH,
ANIM_CROUCH_N,
ANIM_CROUCH_NE,
ANIM_CROUCH_E,
ANIM_CROUCH_SE,
ANIM_CROUCH_S,
ANIM_CROUCH_SW,
ANIM_CROUCH_W,
ANIM_CROUCH_NW,
ANIM_CROUCH_JUMP,
ANIM_CROUCH_JUMP_N,
ANIM_CROUCH_JUMP_NE,
ANIM_CROUCH_JUMP_E,
ANIM_CROUCH_JUMP_SE,
ANIM_CROUCH_JUMP_S,
ANIM_CROUCH_JUMP_SW,
ANIM_CROUCH_JUMP_W,
ANIM_CROUCH_JUMP_NW,
ANIM_CROUCH_SINK,
ANIM_CROUCH_SWIM,
ANIM_SHOOT,
ANIM_MELEE,
ANIM_PAIN,
ANIM_EDIT,
ANIM_LAG,
ANIM_TAUNT,
ANIM_WIN,
ANIM_LOSE,
ANIM_GUN_IDLE,
ANIM_GUN_SHOOT,
ANIM_GUN_MELEE,
ANIM_VWEP_IDLE,
ANIM_VWEP_SHOOT,
ANIM_VWEP_MELEE,
ANIM_SP_SWORDMAN_THRUST,
ANIM_SP_SWORDMAN_STILL_THRUST,
ANIM_SP_SWORDMAN_FEINT,
ANIM_JP_SWORDMAN_SLASH,
ANIM_JP_SWORDMAN_PARRY,
ANIM_JP_SWORDMAN_STILL_SLASH,
ANIM_JP_SWORDMAN_STILL_PARRY,
ANIM_SP_MUSKET_SHOOT,
ANIM_SP_MUSKET_PUSH,
ANIM_SP_MUSKET_STILL_SHOOT,
ANIM_SP_MUSKET_STILL_PUSH,
ANIM_JP_MUSKET_SHOOT,
ANIM_JP_MUSKET_PUSH,
ANIM_JP_MUSKET_STILL_SHOOT,
ANIM_JP_MUSKET_STILL_PUSH,
ANIM_CANNON_IDLE,
ANIM_CANNON_SHOOT,
ANIM_STUN,
ANIM_SP_SWORDMAN_STILL_FEINT,
ANIM_PLAYER_DODGE_N,
ANIM_PLAYER_DODGE_S,
ANIM_PLAYER_DODGE_NE,
ANIM_PLAYER_DODGE_NW,
ANIM_PLAYER_GRAB_MOVE,
ANIM_PLAYER_GRAB_IDLE,
NUMANIMS
};

enum {
ATK_SP_SWORDMAN_THRUST=0,
ATK_SP_SWORDMAN_FEINT,
ATK_JP_SWORDMAN_SLASH,
ATK_JP_SWORDMAN_PARRY,
ATK_SP_MUSKET_SHOOT,
ATK_SP_MUSKET_PUSH,
ATK_JP_MUSKET_SHOOT,
ATK_JP_MUSKET_PUSH,
ATK_CANNON,
NUMSKILLS
};

enum {
CLASS_RAPIER=0,
CLASS_KATANA,
CLASS_MUSKET,
CLASS_JPMUSKET,
NUMCLASSES
};

enum {
TEAM_NEUTRAL=0,
TEAM_SPANISH,
TEAM_WOKOU,
NUMTEAMS
};

enum {
STARTGAMEMODE=-1,
GM_DEMO=0,
GM_EDIT,
GM_MAINMENU,
GM_SINGLEPLAYER,
GM_DM,
GM_TDM,
GM_CTF,
GM_DOMINATION,
NUMGAMEMODES
};

struct modelinfo {
const char* name;
bool ragdoll;
};



struct entityinfo {
const char* name;
int mdl;
int mdl2;
int mdl3;
int animidle;
int animaction;
int distance;
bool canbegrab;
const char* grabattachname;
int animplayergrabidle;
int animplayergrabwalk;
bool animate;
int grabplayermod;
int grabspeed;
};


struct skillinfo {
int anim;
int stillanim;
int secondarymodel;
int sound;
int hudsound;
int hitdelay;
int attackdelay;
int freezedelay;
int reloaddelay;
int magazine;
int damage;
bool haszoom;
int zoomfov;
bool chains;
int parry;
int parrysound;
int block;
int blocksound;
bool breaksblock;
int bleeding;
int knockback;
int spread;
int margin;
int projspeed;
int prekickamount;
int kickamount;
int range;
int fx;
int rays;
int hitpush;
int bulletsize;
int exprad;
int ttl;
int use;
bool owned;
bool canbeblocked;
bool aiholdtrigger;
};

struct classinfo {
const char* classname;
int team;
int model;
int weapon;
bool cangrab;
bool hascloak;
int cloakdelay;
int walkspeed;
int maxspeed;
int skill1;
int skill2;
int skill3;
int guardskill;
};

struct teamdatainfo {
const char* name;
const char* textcode;
int textcolor;
int scoreboardcolor;
const char* blipcolor;
const char* icon;
int playercolor;
int worldflag;
};

struct gamemodeinfo {
const char* name;
const char* prettyname;
int flags;
const char* info;
};

static const modelinfo models[] = {
{"player/sp_swordman",true},
{"player/sp_swordman/sword",false},
{"player/jp_swordman",true},
{"player/jp_swordman/sword",false},
{"player/sp_musket",true},
{"player/sp_musket/sword",false},
{"player/sp_musket/sword/stick",false},
{"player/jp_musket",true},
{"player/jp_musket/sword",false},
{"player/jp_musket/sword/stick",false},
{"game/flag/sp",false},
{"game/flag/jp",false},
{"cannonball",false},
{"cannon",false},
{"cannon/carrymodel",false},
{"game/teleport",false},
{"game/flag/non",false},
{"game/handpoint",false},
};

static const char* const soundnames[] = {
"uphys/jump",
"uphys/land",
"uphys/splashin",
"uphys/splashout",
"",
"",
"",
"uphys/jumppad",
"soundbible/swoosh3",
"soundbible/swordscollide",
"",
"",
"",
"",
"uphys/weapon_switch",
"",
"soundbible/stab",
"soundbible/malegrunt",
"soundbible/pain",
"soundbible/pain",
"uphys/die1",
"uphys/die2",
"ctf/flagpickup",
"ctf/flagdrop",
"ctf/flagreturn",
"ctf/flagscore",
"ctf/flagreturn",
"ctf/flagfail",
"findsounds/Gun-Musket-SingleShot",
"victorious/cannonfire",
"victorious/jp_rangedshot",
"victorious/stockswing"
};

static const int msgsizes[] = {N_DUMMY,0,
N_CONNECT,0,
N_SERVINFO,0,
N_WELCOME,1,
N_INITCLIENT,0,
N_POS,0,
N_TEXT,0,
N_SOUND,2,
N_CDIS,2,
N_USE_SKILL,0,
N_COMBAT,0,
N_SUICIDE,1,
N_DIED,5,
N_HITPUSH,7,
N_TRYSPAWN,1,
N_SPAWNSTATE,8,
N_SPAWN,2,
N_FORCEDEATH,2,
N_TAUNT,1,
N_MAPCHANGE,0,
N_MAPVOTE,0,
N_TEAMINFO,0,
N_ITEMSPAWN,2,
N_ITEMPICKUP,2,
N_ITEMACC,3,
N_TELEPORT,0,
N_JUMPPAD,0,
N_PING,2,
N_PONG,2,
N_CLIENTPING,2,
N_TIMEUP,2,
N_FORCEINTERMISSION,1,
N_SERVMSG,0,
N_ITEMLIST,0,
N_RESUME,0,
N_EDITMODE,2,
N_EDITENT,11,
N_EDITF,16,
N_EDITT,16,
N_EDITM,16,
N_FLIP,14,
N_COPY,14,
N_PASTE,14,
N_ROTATE,15,
N_REPLACE,17,
N_DELCUBE,14,
N_CALCLIGHT,1,
N_REMIP,1,
N_EDITVSLOT,16,
N_UNDO,0,
N_REDO,0,
N_NEWMAP,2,
N_GETMAP,1,
N_SENDMAP,0,
N_CLIPBOARD,0,
N_EDITVAR,0,
N_MASTERMODE,2,
N_KICK,0,
N_CLEARBANS,1,
N_CURRENTMASTER,0,
N_SPECTATOR,3,
N_SETMASTER,0,
N_SETTEAM,0,
N_LISTDEMOS,1,
N_SENDDEMOLIST,0,
N_GETDEMO,2,
N_SENDDEMO,0,
N_DEMOPLAYBACK,3,
N_RECORDDEMO,2,
N_STOPDEMO,1,
N_CLEARDEMOS,2,
N_TAKEFLAG,3,
N_RETURNFLAG,4,
N_RESETFLAG,3,
N_TRYDROPFLAG,1,
N_DROPFLAG,7,
N_SCOREFLAG,9,
N_INITFLAGS,0,
N_SAYTEAM,0,
N_CLIENT,0,
N_AUTHTRY,0,
N_AUTHKICK,0,
N_AUTHCHAL,0,
N_AUTHANS,0,
N_REQAUTH,0,
N_PAUSEGAME,0,
N_GAMESPEED,0,
N_ADDBOT,0,
N_DELBOT,1,
N_INITAI,0,
N_FROMAI,2,
N_BOTLIMIT,2,
N_BOTBALANCE,2,
N_MAPCRC,0,
N_CHECKMAPS,1,
N_SWITCHNAME,0,
N_SWITCHCLASS,2,
N_SWITCHTEAM,2,
N_AMMO_ADD,3,
N_SWITCH_CLOAK_MODE,2,
N_CLOAK_MODE,4,
N_SERVCMD,0,
N_DEMOPACKET,0,
N_KNOCKBACK,6,
N_USE_ENTITY,0,
N_GRAB_ENTITY,0,
N_ITEMDROP,0,
N_INITDOMINATION,0,
N_STARTCAPTURE,0,
N_CANCELCAPTURE,0,
N_CAPTURED,0,
N_DOMINATIONSCORES,0,
N_CLIENTUPDATE,0,
-1};

static const entityinfo gameentities[] = {
{"none?",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"light",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"mapmodel",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"playerstart",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"envmap",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"particles",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"sound",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"spotlight",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"decal",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"teleport",MODEL_TELEPORT,-1,-1,-1,-1,16,false,"",-1,-1,false,0,0},
{"teledest",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"jumppad",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"flag",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"ladder",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"cannon",MODEL_CANNON,MODEL_CANNONBALL,MODEL_CANNON_CARRY,ANIM_CANNON_IDLE,ANIM_CANNON_SHOOT,14,true,"tag_cannon",ANIM_PLAYER_GRAB_IDLE,ANIM_PLAYER_GRAB_MOVE,false,FL_PLAYERMOD_NOSTRAFE | FL_PLAYERMOD_CUSTOMSPEED | FL_PLAYERMOD_NOJUMP,20},
{"capturepoint",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
{"pointofinterest",MODEL_HANDPOINT,-1,-1,-1,-1,22,false,"",-1,-1,false,0,0},
{"mmstart",-1,-1,-1,-1,-1,12,false,"",-1,-1,false,0,0},
};

static const char* const animnames[] = {
"mapmodel","dead",
"dying",
"idle",
"run N",
"run NE",
"run E",
"run SE",
"run S",
"run SW",
"run W",
"run NW",
"jump",
"jump N",
"jump NE",
"jump E",
"jump SE",
"jump S",
"jump SW",
"jump W",
"jump NW",
"sink",
"swim",
"crouch",
"crouch N",
"crouch NE",
"crouch E",
"crouch SE",
"crouch S",
"crouch SW",
"crouch W",
"crouch NW",
"crouch jump",
"crouch jump N",
"crouch jump NE",
"crouch jump E",
"crouch jump SE",
"crouch jump S",
"crouch jump SW",
"crouch jump W",
"crouch jump NW",
"crouch sink",
"crouch swim",
"shoot",
"melee",
"hurt",
"edit",
"lag",
"taunt",
"win",
"lose",
"gun idle",
"gun shoot",
"gun melee",
"vwep idle",
"vwep shoot",
"vwep melee",
"thrustattack",
"thrustattackstill",
"feint",
"attack",
"attack2",
"slashstill",
"parrystill",
"shoot",
"push",
"shootstill",
"pushstill",
"shoot",
"push",
"shootstill",
"pushstill",
"cannon_idle",
"cannon_shoot",
"stun",
"feint_still",
"dodge N",
"dodge S",
"dodge NE",
"dodge NW",
"cannonmove",
"cannonidle"
};

static const skillinfo skills[] = {
{ANIM_SP_SWORDMAN_THRUST,ANIM_SP_SWORDMAN_STILL_THRUST,-1,S_SWOOSH,S_SWOOSH,0,1125,1125,0,1,105,false,42,false,5,S_PARRY_SWORD,0,0,false,0,160,0,2,0,-70,0,21,0,1,0,0,0,0,0,true,true,false},
{ANIM_SP_SWORDMAN_FEINT,ANIM_SP_SWORDMAN_STILL_FEINT,-1,S_SWOOSH,S_SWOOSH,0,708,708,0,1,20,false,42,true,100,S_PARRY_SWORD,50,0,true,50,90,0,2,0,10,0,14,0,1,0,0,0,0,0,true,true,true},
{ANIM_JP_SWORDMAN_SLASH,ANIM_JP_SWORDMAN_STILL_SLASH,-1,S_SWOOSH,S_SWOOSH,0,666,666,0,1,55,false,42,false,5,S_PARRY_SWORD,0,0,false,50,90,0,2,0,-10,0,14,0,1,0,0,0,0,0,true,true,false},
{ANIM_JP_SWORDMAN_PARRY,ANIM_JP_SWORDMAN_STILL_PARRY,-1,S_SWOOSH,S_SWOOSH,0,666,666,0,1,25,false,42,true,100,S_PARRY_SWORD,100,S_PARRY_SWORD,true,5,160,0,2,0,0,0,14,0,1,0,0,0,0,0,true,true,true},
{ANIM_SP_MUSKET_SHOOT,ANIM_SP_MUSKET_STILL_SHOOT,MODEL_SP_MUSKET_STICK,S_MUSKET_FIRE,S_MUSKET_FIRE,0,2450,0,0,1,80,true,45,false,0,0,0,0,false,50,160,0,2,0,10,0,300,FL_FX_MUZZLE | FL_FX_POWDER,1,0,0,0,0,0,true,false,true},
{ANIM_SP_MUSKET_PUSH,ANIM_SP_MUSKET_STILL_PUSH,-1,S_MUSKET_STOCK,S_MUSKET_STOCK,0,458,458,0,1,50,false,42,true,100,S_PARRY_SWORD,100,S_PARRY_SWORD,true,0,90,0,2,0,0,0,14,0,1,0,0,0,0,0,true,true,false},
{ANIM_JP_MUSKET_SHOOT,ANIM_JP_MUSKET_STILL_SHOOT,MODEL_JP_MUSKET_STICK,S_JP_FIRE,S_JP_FIRE,0,1958,0,0,1,40,false,42,false,0,0,0,0,false,50,160,0,2,0,10,0,200,FL_FX_MUZZLE | FL_FX_POWDER,1,0,0,0,0,0,true,false,true},
{ANIM_JP_MUSKET_PUSH,ANIM_JP_MUSKET_STILL_PUSH,-1,S_SWOOSH,S_SWOOSH,0,375,250,0,1,25,false,42,true,100,S_PARRY_SWORD,100,S_PARRY_SWORD,true,0,90,0,2,0,0,0,14,0,1,0,0,0,0,0,true,true,false},
{ANIM_IDLE,ANIM_IDLE,-1,S_CANNON,S_CANNON,0,1000,0,0,1,105,false,42,false,0,S_PARRY_SWORD,0,S_PARRY_SWORD,false,0,0,0,0,0,0,0,0,0,1,0,0,10,0,0,false,false,false},
};

static const classinfo classes[] = {
{"Rodelero",TEAM_SPANISH,MODEL_SP_SWORDMAN_BODY,MODEL_SP_SWORDMAN_SWORD,false,false,0,30,60,ATK_SP_SWORDMAN_THRUST,ATK_SP_SWORDMAN_FEINT,-1,ATK_SP_SWORDMAN_FEINT},
{"Ronin",TEAM_WOKOU,MODEL_JP_SWORDMAN_BODY,MODEL_JP_SWORDMAN_SWORD,false,false,0,30,70,ATK_JP_SWORDMAN_SLASH,ATK_JP_SWORDMAN_PARRY,-1,ATK_JP_SWORDMAN_PARRY},
{"Arquebus",TEAM_SPANISH,MODEL_SP_MUSKET_BODY,MODEL_SP_MUSKET_WEAPON,true,false,0,10,50,ATK_SP_MUSKET_SHOOT,ATK_SP_MUSKET_PUSH,-1,ATK_SP_MUSKET_PUSH},
{"Musket",TEAM_WOKOU,MODEL_JP_MUSKET_BODY,MODEL_JP_MUSKET_WEAPON,true,false,0,15,55,ATK_JP_MUSKET_SHOOT,ATK_JP_MUSKET_PUSH,-1,ATK_JP_MUSKET_PUSH},
};

static const teamdatainfo teams[] = {
{"","\f0",0x1EC850,0,"_neutral","player",0xFFFFFF,-1},
{"Spanish Empire","\f3",0x6496FF,0x3030C0,"_spanish","player_rojo",0x27508A,MODEL_SP_FLAG},
{"Wokou","\f1",0xFF4B19,0xC03030,"_wokou","player_azul",0xAC2C2A,MODEL_JP_FLAG}
};

static const gamemodeinfo gamemodes[] = {
{"demo","demo",M_DEMO | M_LOCAL,0},
{"edit","Edit",M_EDIT,"Cooperative Editing Edit maps with multiple players simultaneously."},
{"mm","Main",M_MAINMENU | M_LOCAL,"Mainmenu"},
{"singleplayer","Singleplayer",M_SINGLEPLAYER|M_TEAM|M_LOCAL,"Tutorial: Learn the basics"},
{"dm","Deathmatch",M_LOBBY,"Deathmatch: Frag everyone to score points."},
{"tdm","Team Deathmatch",M_TEAM,"Team Deathmatch: Frag the enemy team to score points."},
{"ctf","Capture The Flag",M_CTF | M_TEAM,"Capture The Flag: Score points by capturing the enemy flag."},
{"domination","Domination",M_DOMINATION|M_TEAM,"Domination: Score points by capturing points."}
};


