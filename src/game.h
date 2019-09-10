#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"

// data

enum { NO_SKILL = -1 };

#define DEATHMILLIS (300)

enum {
    FL_PLAYERMOD_NOSTRAFE = 1 << 0,
    FL_PLAYERMOD_CUSTOMSPEED = 1 << 1,
    FL_PLAYERMOD_NOJUMP = 1 << 2
};

// flags
enum {
    FL_PARRY = 1 << 0,
    FL_KNOCKBACK = 1 << 1,
    FL_BLOCKED = 1 << 2,
    FL_NO_DAMAGE = 1 << 3,
    FL_HALF_DAMAGE = 1 << 4,
    FL_STUN = 1 << 5,
    FL_CHAIN = 1 << 6,
    FL_HEADSHOT = 1 << 7
};

enum {
    FL_FX_MUZZLE = 1 << 0,
    FL_FX_POWDER = 1 << 1
};

enum {
    BNC_GIBS,
    BNC_DEBRIS,
    BNC_GRENADE
};

enum
{
    M_TEAM         = 1<<0,
    M_CTF          = 1<<1,
    M_OVERTIME     = 1<<2,
    M_EDIT         = 1<<3,
    M_DEMO         = 1<<4,
    M_LOCAL        = 1<<5,
    M_LOBBY        = 1<<6,
    M_DOMINATION   = 1<<7,
    M_SINGLEPLAYER = 1<<8,
    M_MAINMENU     = 1<<9
};

#include "data.h"

// console message types

enum
{
    CON_CHAT       = 1<<8,
    CON_TEAMCHAT   = 1<<9,
    CON_GAMEINFO   = 1<<10,
    CON_FRAG_SELF  = 1<<11,
    CON_FRAG_OTHER = 1<<12,
    CON_TEAMKILL   = 1<<13
};

// network quantization scale
#define DMF 16.0f                // for world locations
#define DNF 100.0f              // for normalized vectors
#define DVELF 1.0f              // for playerspeed based velocity vectors

struct gameentity : extentity
{
    vec muzzle;
    vec powder;
    int actiontime;
    bool uihud;

    gameentity();
};

#define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
#define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))

#define m_mainmenu     (m_check(gamemode, M_MAINMENU))
#define m_singleplayer (m_check(gamemode, M_SINGLEPLAYER))
#define m_domination   (m_check(gamemode, M_DOMINATION))
#define m_ctf          (m_check(gamemode, M_CTF))
#define m_teammode     (m_check(gamemode, M_TEAM))
#define m_overtime     (m_check(gamemode, M_OVERTIME))
#define isteam(a,b)    (m_teammode && a==b)

#define m_demo         (m_check(gamemode, M_DEMO))
#define m_edit         (m_check(gamemode, M_EDIT))
#define m_lobby        (m_check(gamemode, M_LOBBY))
#define m_timed        (m_checknot(gamemode, M_DEMO|M_EDIT|M_LOCAL))
#define m_botmode      (m_checknot(gamemode, M_DEMO))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH, MM_INVALID = MM_START - 1 };

static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// network messages codes, c2s, c2c, s2c

enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

#define TESSERACT_SERVER_PORT 42000
#define TESSERACT_LANINFO_PORT 41998
#define TESSERACT_MASTER_PORT 80
#define DEMO_MAGIC "VICTORIOUS_DEMO\0\0"

#define DEMO_VERSION 1                  // bump when demo format changes

#define PROTOCOL_VERSION 100

#define VERSION_MAJOR (PROTOCOL_VERSION / 100)
#define VERSION_MINOR ((PROTOCOL_VERSION / 10) % 10)
#define VERSION_REV (PROTOCOL_VERSION % 10)

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define MAXNAMELEN 15

enum
{
    HICON_RED_FLAG = 0,
    HICON_BLUE_FLAG,

    HICON_X       = 20,
    HICON_Y       = 1650,
    HICON_TEXTY   = 1644,
    HICON_STEP    = 490,
    HICON_SIZE    = 120,
    HICON_SPACE   = 40
};

#if 0
static struct itemstat { int add, max, sound; const char *name; int icon, info; } itemstats[] =
{
};
#endif

#define validitem(n) true

#define MAXRAYS 1
#define EXP_SELFDAMDIV 2
#define EXP_SELFPUSH 2.5f
#define EXP_DISTSCALE 0.5f

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
    int health, maxhealth;
    int gunwait, reloadtime, gunfires, gunstarttime, aimstarttime;
    int stuntime, laststun;
    int aitype, skill;
    int grabentity;
    int dodgetime;

    gamestate() : maxhealth( MAXHEALTH ), aitype(AI_NONE), skill(0), grabentity(-1) {}

    bool canpickup(int type)
    {
        return validitem(type);
    }

    void pickup(int type)
    {
    }

    void respawn()
    {
        health = maxhealth;
        gunwait = 0;
        reloadtime = 0;
        gunfires = 0;
        gunstarttime = 0;
        aimstarttime = 0;
        stuntime = 0;
        laststun = 0;
        dodgetime = 0;
    }

    void spawnstate(int gamemode) {
        //conoutf( "spawn state" );
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        health -= damage;
        return damage;
    }

    int hasammo(int gun, int exclude = -1)
    {
        return true;
    }
};

#define MAXTEAMS NUMTEAMS-1
static inline int teamnumber(const char *name) { loopi(MAXTEAMS) if(!strcmp(teams[1+i].name, name)) return 1+i; return 0; }
#define validteam(n) ((n) >= 1 && (n) <= MAXTEAMS)
#define teamname(n) (teams[validteam(n) ? (n) : 0].name)

struct gameent : dynent, gamestate
{
    int classid;
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastfiredatk;
    int nextskill;
    int lastskill;
    int lastactivate;
    bool skilltrigger;
    int lasttaunt;
    int lastpickup, lastpickupmillis, flagpickup;
    int lastpainfromlocal, lasthalfdamagetime;
    int lastchaintime;
    int frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;
    int cloakmode;
    int walkspeed;
    int normalspeed;
    bool waitdrop;
    int playermod;
    int customspeed;

    string name, info;
    int team;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle;
    vec powder;
    vec taghead;

    gameent() : classid( 0 ), weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), lastpainfromlocal(0), lasthalfdamagetime(0), lastchaintime(0), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), cloakmode(0), team(0), ai(NULL), ownernum(-1), muzzle(-1, -1, -1), powder( -1, -1, -1 ), taghead( 0, 0, 0 )
    {
        name[0] = info[0] = 0;
        respawn();
    }
    virtual ~gameent()
    {
        freeeditinfo(edit);
        if(ai) delete ai;
    }

    void hitpush(int damage, const vec &dir, gameent *actor, int atk)
    {
        vec push(dir);
        push.mul((actor==this && skills[atk].exprad ? EXP_SELFPUSH : 1.0f)*skills[atk].hitpush*damage/weight);
        vel.add(push);
    }

    void respawn();

    void startgame()
    {
        frags = flags = deaths = 0;
        totaldamage = totalshots = 0;
        maxhealth = MAXHEALTH;
        lifesequence = -1;
        respawned = suicided = -2;
    }
};

struct teamscore
{
    int team, score;
    teamscore() {}
    teamscore(int team, int n) : team(team), score(n) {}

    static bool compare(const teamscore &x, const teamscore &y)
    {
        if(x.score > y.score) return true;
        if(x.score < y.score) return false;
        return x.team < y.team;
    }
};

static inline uint hthash(const teamscore &t) { return hthash(t.team); }
static inline bool htcmp(int team, const teamscore &t) { return team == t.team; }

struct teaminfo
{
    int frags;

    teaminfo() { reset(); }

    void reset() { frags = 0; }
};

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void checkitems(gameent *d);
    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void update( int, bool, float, float, float, int, int, int, int, int );
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, gameent *d);
    extern void pickupeffects(int n, gameent *d);
    extern void teleporteffects(gameent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(gameent *d, int jp, bool local = true);
    extern void effects( int n, gameent *d );
    extern void local_grab( int n, gameent *d );
    extern void drop( gameent* );
    extern void local_drop( gameent*, int );
}

namespace server {
    template <int N>
    struct projectilestate
    {
        int projs[N];
        int numprojs;

        projectilestate() : numprojs(0) {}

        void reset() { numprojs = 0; }

        void add(int val)
        {
            if(numprojs>=N) numprojs = 0;
            projs[numprojs++] = val;
        }

        bool remove(int val)
        {
            loopi(numprojs) if(projs[i]==val)
            {
                projs[i] = projs[--numprojs];
                return true;
            }
            return false;
        }
    };

    struct servstate : gamestate
    {
        vec o;
        int state, editstate;
        int lastdeath, deadflush, lastspawn, lifesequence;
        int lastshot, nextskill, lastskill;
        int lastannounce;
        int lastchain, chainlevel;
        projectilestate<8> projs;
        int frags, flags, deaths, teamkills, shotdamage, damage;
        int lasttimeplayed, timeplayed;
        float effectiveness;

        servstate() : state(CS_DEAD), editstate(CS_DEAD), lifesequence(0) {}

        bool isalive(int gamemillis)
        {
            return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
        }

        bool waitexpired(int gamemillis)
        {
            return gamemillis - lastshot >= gunwait;
        }

        void reset()
        {
            if(state!=CS_SPECTATOR) state = editstate = CS_DEAD;
            maxhealth = MAXHEALTH;
            projs.reset();

            timeplayed = 0;
            effectiveness = 0;
            frags = flags = deaths = teamkills = shotdamage = damage = 0;

            lastdeath = 0;

            respawn();
        }

        void respawn()
        {
            gamestate::respawn();
            o = vec(-1e10f, -1e10f, -1e10f);
            deadflush = 0;
            lastspawn = -1;
            lastshot = 0;
            nextskill = NO_SKILL;
            lastskill = NO_SKILL;
            lastannounce = 0;
            lastchain = 0;
            chainlevel = 0;
        }

        void reassign()
        {
            respawn();
            projs.reset();
        }
    };

    struct clientinfo;
    struct gameevent
    {
        virtual ~gameevent() {}

        virtual bool flush(clientinfo *ci, int fmillis);
        virtual void process(clientinfo *ci) {}

        virtual bool keepable() const { return false; }
    };

    struct clientinfo
    {
        int clientnum, ownernum, connectmillis, sessionid, overflow;
        string name, mapvote;
        int team, classid;
        int modevote;
        int privilege;
        bool connected, local, timesync;
        int gameoffset, lastevent, pushed;
        servstate state;
        vector<gameevent *> events;
        vector<uchar> position, messages;
        uchar *wsdata;
        int wslen;
        vector<clientinfo *> bots;
        int ping, aireinit;
        string clientmap;
        int mapcrc;
        bool warned, gameclip;
        ENetPacket *getdemo, *getmap, *clipboard;
        int lastclipboard, needclipboard;
        int connectauth;
        uint authreq;
        string authname, authdesc;
        void *authchallenge;
        int authkickvictim;
        char *authkickreason;
        int cloakmode;

        clientinfo();
        ~clientinfo();

        void addevent(gameevent *e);

        enum
        {
            PUSHMILLIS = 3000
        };

        int calcpushrange();
        bool checkpushed(int millis, int range);
        void mapchange();
        void reassign();
        void cleanclipboard(bool fullclean = true);
        void cleanauthkick();
        void cleanauth(bool full = true);
        void reset();
        int geteventmillis(int servmillis, int clientmillis);
    };

    struct servmode
    {
        virtual ~servmode() {}

        virtual void entergame(clientinfo *ci) {}
        virtual void leavegame(clientinfo *ci, bool disconnecting = false) {}

        virtual void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip) {}
        virtual bool canspawn(clientinfo *ci, bool connecting = false) { return true; }
        virtual void spawned(clientinfo *ci) {}
        virtual int fragvalue(clientinfo *victim, clientinfo *actor);
        virtual void died(clientinfo *victim, clientinfo *actor) {}
        virtual bool canchangeteam(clientinfo *ci, int oldteam, int newteam) { return true; }
        virtual void changeteam(clientinfo *ci, int oldteam, int newteam) {}
        virtual void initclient(clientinfo *ci, packetbuf &p, bool connecting) {}
        virtual void update() {}
        virtual void cleanup() {}
        virtual void setup() {}
        virtual void newmap() {}
        virtual void intermission() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(int team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual bool extinfoteam(int team, ucharbuf &p) { return false; }
        virtual bool parseservermessage( int, packetbuf&, server::clientinfo*, server::clientinfo*, server::servmode* ) { return false; }
    };
}

namespace game
{
    extern int gamemode;

    struct clientmode;
    struct servmode;
    struct clientmode
    {
        virtual ~clientmode() {}

        virtual void preload() {}
        virtual float clipconsole(float w, float h) { return 0; }
        virtual void drawhud(gameent *d, int w, int h) {}
        virtual void rendergame() {}
        virtual void respawned(gameent *d) {}
        virtual void setup() {}
        virtual void checkitems(gameent *d) {}
        virtual int respawnwait(gameent *d) { return 0; }
        virtual void pickspawn(gameent *d) { findplayerspawn(d, -1, m_teammode ? d->team : 0); }
        virtual void senditems(packetbuf &p) {}
        virtual void removeplayer(gameent *d) {}
        virtual void gameover() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(int team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests) {}
        virtual bool aicheck(gameent *d, ai::aistate &b) { return false; }
        virtual bool aidefend(gameent *d, ai::aistate &b) { return false; }
        virtual bool aipursue(gameent *d, ai::aistate &b) { return false; }
        virtual bool parseclientmessage( int, ucharbuf&, clientmode* ) { return false; } 
    };

    extern clientmode *cmode;
    extern void setclientmode();

    // game
    extern int nextmode;
    extern string clientmap;
    extern bool intermission;
    extern int maptime, maprealtime, maplimit;
    extern gameent *player1;
    extern vector<gameent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
    extern int following;
    extern int smoothmove, smoothdist;

    extern bool allowedtoattack( gameent * );
    extern bool clientoption(const char *arg);
    extern gameent *getclient(int cn);
    extern gameent *newclient(int cn);
    extern const char *colorname(gameent *d, const char *name = NULL, const char *alt = NULL, const char *color = "");
    extern const char *teamcolorname(gameent *d, const char *alt = "you");
    extern const char *teamcolor(const char *prefix, const char *suffix, int team, const char *alt);
    extern gameent *pointatplayer();
    extern gameent *hudplayer();
    extern gameent *followingplayer();
    extern void stopfollowing();
    extern void checkfollow();
    extern void nextfollow(int dir = 1);
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(gameent *);
    extern void deathstate(gameent *d, bool restore = false);
    extern void killed(gameent *d, gameent *actor, int flags);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);
    extern void onplayerdoubletap( physent* );

    // client
    extern bool connected, remote, demoplayback;
    extern string servdesc;
    extern vector<uchar> messages;

    extern void processevents();
    extern int parseplayer(const char *arg);
    extern void ignore(int cn);
    extern void unignore(int cn);
    extern bool isignored(int cn);
    extern bool addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name);
    extern void switchteam(const char *name);
    extern void switchplayermodel(int playermodel);
    extern void switchplayercolor(int playercolor);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void c2sinfo(bool force = false);
    extern void sendposition(gameent *d, bool reliable = false);

    // weapon
    extern int getweapon(const char *name);
    extern void reload( gameent* );
    extern bool canshoot( gameent* );
    extern void shoot( gameent *d, const vec &targ );
    extern void shoteffects( int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction, int );
    extern void blockeffect( gameent * );
    extern void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int dam, int atk);
    extern void explodeeffects(int atk, gameent *d, bool local, int id = 0);
    extern void damageeffect(int damage, gameent *d, bool thirdperson = true);
    extern void gibeffect(int damage, const vec &vel, gameent *d);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float margin = 0, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin = 0, float &dist = intersectdist);
    extern void newbouncer(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed, int atk);
    extern void newprojectile(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk, int color);
    extern void clearbouncers();
    extern void updatebouncers(int curtime);
    extern void removebouncers(gameent *owner);
    extern void renderbouncers();
    extern void clearprojectiles();
    extern void updateprojectiles(int curtime);
    extern void removeprojectiles(gameent *owner);
    extern void renderprojectiles();
    extern void preloadbouncers();
    extern void removeweapons(gameent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, gameent *d);
    extern void weaponswitch(gameent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);

    // scoreboard
    extern void showscores(bool on);
    extern void getbestplayers(vector<gameent *> &best);
    extern void getbestteams(vector<int> &best);
    extern void clearteaminfo();
    extern void setteaminfo(int team, int frags);
    extern void removegroupedplayer(gameent *d);

    extern void saveragdoll(gameent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern int particlecolor( int );
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *modeprettyname(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void stopdemo();
    extern void forcemap(const char *map, int mode);
    extern void forcepaused(bool paused);
    extern void forcegamespeed(int speed);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);
}

#endif

