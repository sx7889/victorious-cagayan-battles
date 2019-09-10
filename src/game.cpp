#include "game.h"

VARFP( usedof, 0, 1, 1, { if( !usedof ) execident( "clearpostfx" ); else execident( "dof" ); });

VARP( lowmaps, 0, 0, 1 );
SVAR( gamenews, "" );
SVAR( gameversion, "" );

SVAR( centermsg, "" );

VAR( isspectator, 0, 0, 1 );

extern char *mastername;
extern int masterport;

void gameent::respawn() {
    dynent::reset();
    gamestate::respawn();
    respawned = suicided = -1;
    lastaction = 0;
    lastfiredatk = -1;
    nextskill = NO_SKILL;
    lastskill = NO_SKILL;
    lastactivate = 0;
    skilltrigger = false;
    lasttaunt = 0;
    lastpickup = -1;
    lastpickupmillis = 0;
    flagpickup = 0;
    lastnode = -1;
    lastpainfromlocal = 0;
    lasthalfdamagetime = 0;
    lastchaintime = 0;
    gunstarttime = 0;
    stuntime = 0;
    laststun = 0;
    aimstarttime = 0;
    walkspeed = classes[ classid ].walkspeed;
    normalspeed = classes[ classid ].maxspeed;
    speed = normalspeed;
    grabentity = -1;
    waitdrop = false;
    playermod = 0;
    customspeed = 0;
    xradius = 10.0f;
    yradius = 10.0f;
}

namespace game
{
    int particlecolor( int type ) {
        switch( type ) {
            case PART_RAIL_TRAIL:
            case PART_SPARK: return 0xff9922;
            case PART_BLOOD: return 0x60ffff;
            case PART_STEAM: return 0x897661;
        }
        return 0xffffff;
    }

    bool intermission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int lasthit = 0, lastspawnattempt = 0;
    float halfdamageoffsetx=0.0f, halfdamageoffsety=0.0f;

    gameent *player1 = NULL;         // our client
    vector<gameent *> players;       // other clients

    int following = -1;

    VAR(drawcenterguide, 0, 0, 1);

    VARFP(specmode, 0, 0, 2,
    {
        if(!specmode) stopfollowing();
        else if(following < 0) nextfollow();
    });

    gameent *followingplayer()
    {
        if(player1->state!=CS_SPECTATOR || following<0) return NULL;
        gameent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return NULL;
    }

    ICOMMAND(getfollow, "", (),
    {
        gameent *f = followingplayer();
        intret(f ? f->clientnum : -1);
    });

    void stopfollowing()
    {
        if(following<0) return;
        following = -1;
    }

    void follow(char *arg)
    {
        int cn = -1;
        if(arg[0])
        {
            if(player1->state != CS_SPECTATOR) return;
            cn = parseplayer(arg);
            if(cn == player1->clientnum) cn = -1;
        }
        if(cn < 0 && (following < 0 || specmode)) return;
        following = cn;
    }
    COMMAND(follow, "s");

    void nextfollow(int dir)
    {
        if(player1->state!=CS_SPECTATOR) return;
        int cur = following >= 0 ? following : (dir < 0 ? clients.length() - 1 : 0);
        loopv(clients)
        {
            cur = (cur + dir + clients.length()) % clients.length();
            if(clients[cur] && clients[cur]->state!=CS_SPECTATOR)
            {
                following = cur;
                return;
            }
        }
        stopfollowing();
    }
    ICOMMAND(nextfollow, "i", (int *dir), nextfollow(*dir < 0 ? -1 : 1));

    void checkfollow()
    {
        if(player1->state != CS_SPECTATOR)
        {
            if(following >= 0) stopfollowing();
        }
        else
        {
            if(following >= 0)
            {
                gameent *d = clients.inrange(following) ? clients[following] : NULL;
                if(!d || d->state == CS_SPECTATOR) stopfollowing();
            }
            if(following < 0 && specmode) nextfollow();
        }
    }

    const char *getclientmap() { return clientmap; }

    void resetgamestate()
    {
        clearprojectiles();
        clearbouncers();
    }

    gameent *spawnstate(gameent *d)              // reset player state not persistent accross spawns
    {
        d->respawn();
        d->spawnstate(gamemode);
        return d;
    }

    void respawnself()
    {
        if(ispaused()) return;
        if(m_mp(gamemode))
        {
            int seq = (player1->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
            if(player1->respawned!=seq) { 
                addmsg(N_TRYSPAWN, "rc", player1); player1->respawned = seq;
            }
        }
        else
        {
            spawnplayer(player1);
            showscores(false);
            lasthit = 0;
            if(cmode) cmode->respawned(player1);
        }
    }

    gameent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    gameent *hudplayer()
    {
        if(thirdperson || specmode > 1) return player1;
        gameent *target = followingplayer();
        return target ? target : player1;
    }

    void setupcamera() {
        if( m_mainmenu ) {
            const vector<extentity *> &ents = entities::getents();
            int pick = findentity( MMSTART, 0 );
            if( pick >= 0 ) {
                camera1->o = ents[pick]->o;
                camera1->yaw = ents[pick]->attr1;
                camera1->pitch = ents[pick]->attr2;
                camera1->resetinterp();
            }
        }
        else {
            gameent *target = followingplayer();
            if(target)
            {
                player1->yaw = target->yaw;
                player1->pitch = target->state==CS_DEAD ? 0 : target->pitch;
                player1->o = target->o;
                player1->resetinterp();
            }
        }
    }

    bool detachcamera()
    {
        gameent *d = followingplayer();
        if(d) return specmode > 1 || d->state == CS_DEAD;
        return player1->state == CS_DEAD;
    }

    bool collidecamera()
    {
        switch(player1->state)
        {
            case CS_EDITING: return false;
            case CS_SPECTATOR: return followingplayer()!=NULL;
        }
        return true;
    }

    VARP(smoothmove, 0, 75, 100);
    VARP(smoothdist, 0, 32, 64);

    void predictplayer(gameent *d, bool move)
    {
        d->o = d->newpos;
        d->yaw = d->newyaw;
        d->pitch = d->newpitch;
        d->roll = d->newroll;
        if(move)
        {
            moveplayer(d, 1, false);
            d->newpos = d->o;
        }
        float k = 1.0f - float(lastmillis - d->smoothmillis)/smoothmove;
        if(k>0)
        {
            d->o.add(vec(d->deltapos).mul(k));
            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;
            d->roll += d->deltaroll*k;
        }
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || d->ai) continue;

            if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d);
            else if(!intermission)
            {
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
            }

            const int lagtime = totalmillis-d->lastupdate;
            if(!lagtime || intermission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                crouchplayer(d, 10, false);
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true);
        }
    }

    void updateworld()        // main game update loop
    {
        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();
        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        processevents();
        if(connected)
        {
            if(player1->state == CS_DEAD)
            {
                if(player1->ragdoll) moveragdoll(player1);
                else if(lastmillis-player1->lastpain<2000)
                {
                    player1->move = player1->strafe = 0;
                    moveplayer(player1, 10, true);
                }
            }
            else if(!intermission)
            {
                if(player1->ragdoll) cleanragdoll(player1);
                crouchplayer(player1, 10, true);
                moveplayer(player1, 10, true);
                entities::checkitems(player1);
                if(cmode) cmode->checkitems(player1);
            }
        }
        if(player1->clientnum>=0) c2sinfo();   // do this last, to reduce the effective frame lag
    }

    void spawnplayer(gameent *d)   // place at random spawn
    {
        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, -1, m_teammode ? d->team : 0);
        spawnstate(d);
        if(d==player1)
        {
            if(editmode) d->state = CS_EDITING;
            else if(d->state != CS_SPECTATOR) d->state = CS_ALIVE;
        }
        else d->state = CS_ALIVE;
        checkfollow();
    }

    VARP(spawnwait, 0, 0, 1000);

    void respawn()
    {
        if(player1->state==CS_DEAD)
        {
            execident( "fxscopeoff" );
            player1->nextskill = NO_SKILL;
            int wait = cmode ? cmode->respawnwait(player1) : 0;
            if(wait>0)
            {
                lastspawnattempt = lastmillis;
                //conoutf(CON_GAMEINFO, "\f2you must wait %d second%s before respawn!", wait, wait!=1 ? "s" : "");
                return;
            }
            if(lastmillis < player1->lastpain + spawnwait) {
                return; }
            respawnself();
        }
    }

    // inputs

    void doaction(int act)
    {
        if(!connected
        || intermission
        || !allowedtoattack( player1 ) ) {
            return;
        }

        int skill1 = classes[ player1->classid ].skill1;
        int skill2 = classes[ player1->classid ].skill2;
        if( act == 1 ) act = skill1;
        else if( act == 2 ) act = skill2;

        if((player1->nextskill = act)) respawn();
    }

    ICOMMAND(primary, "D", (int *down), doaction(*down ? 1 : NO_SKILL));
    ICOMMAND(secondary, "D", (int *down), doaction(*down ? 2 : NO_SKILL));

    void doactivate() {
        player1->lastactivate = lastmillis;
    }
    ICOMMAND( activate, "D", (int *down), if(! *down ) doactivate() );

    bool canjump()
    {
        if(!connected || intermission) return false;
        respawn();
        return player1->state!=CS_DEAD;
    }

    bool cancrouch()
    {
        if(!connected || intermission) return false;
        return player1->state!=CS_DEAD;
    }

    bool allowmove(physent *d)
    {
        if(d->type!=ENT_PLAYER) return true;

        gameent *ge = (gameent *)d;
        int attacktime = lastmillis-ge->gunstarttime,
        skill = ge->lastskill;

        if( ge->stuntime > lastmillis ) {
            return false;
        }

        if( skill != NO_SKILL
        && attacktime < skills[ skill ].freezedelay ) {
            return false;
        }

        if( ge->dodgetime > lastmillis ) {
            return false;
        }

        return !ge->lasttaunt || lastmillis-ge->lasttaunt>=1000;
    }

    bool allowedtoattack( gameent *d ) {
        int attacktime = lastmillis-d->gunstarttime,
        skill = d->lastskill;

        if( d->stuntime > lastmillis ) {
            return false;
        }

        if( skill != NO_SKILL
        && attacktime < skills[ skill ].freezedelay ) {
            return false;
        }

        return true;
    }

    void preparetomove( physent *d ) {
        gameent *ge = (gameent *)d;

        if( ( ge->playermod & FL_PLAYERMOD_NOSTRAFE ) ) {
            d->strafe = 0;
        }

        if( ( ge->playermod & FL_PLAYERMOD_CUSTOMSPEED ) ) {
            d->speed = ge->customspeed;
        }

        if( ( ge->playermod & FL_PLAYERMOD_NOJUMP ) ) {
            d->jumping = false;
        }
    }

    void taunt()
    {
        if(player1->state!=CS_ALIVE || player1->physstate<PHYS_SLOPE) return;
        if(lastmillis-player1->lasttaunt<1000) return;
        player1->lasttaunt = lastmillis;
        addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

    VARP(deathscore, 0, 1, 1);

    void deathstate(gameent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        if(!restore)
        {
            gibeffect(max(-d->health, 0), d->vel, d);
            d->deaths++;
        }
        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            d->nextskill = NO_SKILL;
            //d->pitch = 0;
            d->roll = 0;
            playsound(S_DIE2);
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playsound(S_DIE1, &d->o);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    void killed(gameent *d, gameent *actor, int flags)
    {
        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            d->deaths++;
            if(d!=player1) d->resetinterp();
            return;
        }
        else if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        gameent *h = followingplayer();
        if(!h) h = player1;
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        const char *dname = "", *aname = "";
        if(m_teammode && teamcolorfrags)
        {
            dname = teamcolorname(d, "you");
            aname = teamcolorname(actor, "you");
        }
        else
        {
            dname = colorname(d, NULL, "you");
            aname = colorname(actor, NULL, "you");
        }
        if(d==actor)
            conoutf(contype, "\f2%s suicided%s", dname, d==player1 ? "!" : "");
        else if(isteam(d->team, actor->team))
        {
            contype |= CON_TEAMKILL;
            if(actor==player1) conoutf(contype, "\f6%s fragged a teammate (%s)", aname, dname);
            else if(d==player1) conoutf(contype, "\f6%s got fragged by a teammate (%s)", dname, aname);
            else conoutf(contype, "\f2%s fragged a teammate (%s)", aname, dname);
        }
        else
        {
            if( flags & FL_HEADSHOT ) {
                conoutf(contype, "\f2%s got fragged by a headshot from %s", dname, aname);
            }
            else {
                if(d==player1) conoutf(contype, "\f2%s got fragged by %s", dname, aname);
                else conoutf(contype, "\f2%s fragged %s", aname, dname);
            }
        }
        entities::drop( d );
        deathstate(d);
        ai::killed(d, actor);
    }

    void timeupdate(int secs)
    {
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->nextskill = NO_SKILL;
            if(cmode) cmode->gameover();
            conoutf(CON_GAMEINFO, "\f2intermission:");
            conoutf(CON_GAMEINFO, "\f2game has ended!");
            if(m_ctf) conoutf(CON_GAMEINFO, "\f2player frags: %d, flags: %d, deaths: %d", player1->frags, player1->flags, player1->deaths);
            else conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d", player1->frags, player1->deaths);
            int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            conoutf(CON_GAMEINFO, "\f2player total damage dealt: %d, damage wasted: %d, accuracy(%%): %d", player1->totaldamage, player1->totalshots-player1->totaldamage, accuracy);

            showscores(true);
            disablezoom();

            execident("intermission");
        }
    }

    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage));
    ICOMMAND(gettotalshots, "", (), intret(player1->totalshots));

    vector<gameent *> clients;

    gameent *newclient(int cn)   // ensure valid entity
    {
        if(cn < 0 || cn > max(0xFF, MAXCLIENTS + MAXBOTS))
        {
            neterr("clientnum", false);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= clients.length()) clients.add(NULL);
        if(!clients[cn])
        {
            gameent *d = new gameent;
            d->clientnum = cn;
            clients[cn] = d;
            players.add(d);
        }
        return clients[cn];
    }

    gameent *getclient(int cn)   // ensure valid entity
    {
        if(cn == player1->clientnum) return player1;
        return clients.inrange(cn) ? clients[cn] : NULL;
    }

    void clientdisconnected(int cn, bool notify)
    {
        if(!clients.inrange(cn)) return;
        unignore(cn);
        gameent *d = clients[cn];
        if(d)
        {
            if(notify && d->name[0]) conoutf("\f4leave:\f7 %s", colorname(d));
            removeweapons(d);
            removetrackedparticles(d);
            removetrackeddynlights(d);
            if(cmode) cmode->removeplayer(d);
            removegroupedplayer(d);
            players.removeobj(d);
            DELETEP(clients[cn]);
            cleardynentcache();
        }
        if(following == cn)
        {
            if(specmode) nextfollow();
            else stopfollowing();
        }
    }

    void clearclients(bool notify)
    {
        loopv(clients) if(clients[i]) clientdisconnected(i, notify);
    }

    void initclient()
    {
        player1 = spawnstate(new gameent);
        filtertext(player1->name, "unnamed", false, false, MAXNAMELEN);
        players.add(player1);
    }

    VARP(showmodeinfo, 0, 1, 1);

    void startgame()
    {
        clearprojectiles();
        clearbouncers();
        clearragdolls();

        clearteaminfo();

        // reset perma-state
        loopv(players) players[i]->startgame();

        setclientmode();

        intermission = false;
        maptime = maprealtime = 0;
        maplimit = -1;

        float tw, th;
        text_boundsf( "TOO QUICK!", tw, th );
        halfdamageoffsetx = -( tw/2 );
        halfdamageoffsety = -200;

        if(cmode)
        {
            cmode->preload();
            cmode->setup();
        }

        conoutf(CON_GAMEINFO, "\f2game mode is %s", server::modeprettyname(gamemode));

        const char *info = m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
        if(showmodeinfo && info) conoutf(CON_GAMEINFO, "\f0%s", info);

        showscores(false);
        disablezoom();
        lasthit = 0;

        execident("mapstart");
    }

    void startmap(const char *name)   // called just after a map load
    {
        if( usedof ) {
            execident( "dof" );
        }

        ai::savewaypoints();
        ai::clearwaypoints(true);

        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1, m_teammode ? player1->team : 0);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");

        sendmapinfo();
    }

    const char *getmapinfo()
    {
        return execidentstr( "gettooltip", false );
    }

    const char *getscreenshotinfo()
    {
        return server::modename(gamemode, NULL);
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if( m_mainmenu ) {
            return;
        }
        if     (waterlevel>0) { if(material!=MAT_LAVA) playsound(S_SPLASHOUT, d==player1 ? NULL : &d->o); }
        else if(waterlevel<0) playsound(material==MAT_LAVA ? S_BURN : S_SPLASHIN, d==player1 ? NULL : &d->o);
        if     (floorlevel>0) { if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(S_JUMP, d); }
        else if(floorlevel<0) { if(d==player1 || d->type!=ENT_PLAYER || ((gameent *)d)->ai) msgsound(S_LAND, d); }
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
    }

    void msgsound(int n, physent *d)
    {
        if(!d || d==player1)
        {
            addmsg(N_SOUND, "ci", d, n);
            playsound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((gameent *)d)->ai)
                addmsg(N_SOUND, "ci", d, n);
            playsound(n, &d->o);
        }
    }

    int numdynents() { return players.length(); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
        return NULL;
    }

    bool duplicatename(gameent *d, const char *name = NULL, const char *alt = NULL)
    {
        if(!name) name = d->name;
        if(alt && d != player1 && !strcmp(name, alt)) return true;
        loopv(players) if(d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    const char *colorname(gameent *d, const char *name, const char * alt, const char *color)
    {
        if(!name) name = alt && d == player1 ? alt : d->name;
        bool dup = !name[0] || duplicatename(d, name, alt) || d->aitype != AI_NONE;
        if(dup || color[0])
        {
            if(dup) return tempformatstring(d->aitype == AI_NONE ? "\fs%s%s \f5(%d)\fr" : "\fs%s%s \f5[%d]\fr", color, name, d->clientnum);
            return tempformatstring("\fs%s%s\fr", color, name);
        }
        return name;
    }

    VARP(teamcolortext, 0, 1, 1);

    const char *teamcolorname(gameent *d, const char *alt)
    {
        if(!teamcolortext || !m_teammode || !validteam(d->team)) return colorname(d, NULL, alt);
        return colorname(d, NULL, alt, teams[d->team].textcode);
    }

    const char *teamcolor(const char *prefix, const char *suffix, int team, const char *alt)
    {
        if(!teamcolortext || !m_teammode || !validteam(team)) return alt;
        return tempformatstring("\fs%s%s%s%s\fr", teams[team].textcode, prefix, teams[team].name, suffix);
    }

    void suicide(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((gameent *)d)->ai))
        {
            if(d->state!=CS_ALIVE) return;
            gameent *pl = (gameent *)d;
            if(!m_mp(gamemode)) killed(pl, pl,0);
            else
            {
                int seq = (pl->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
                if(pl->suicided!=seq) {
                    addmsg(N_SUICIDE, "rc", pl); 
                    pl->suicided = seq;
                }
            }
        }
    }
    ICOMMAND(suicide, "", (), suicide(player1));

    bool needminimap() { return m_ctf || m_domination; }

    void drawicon(int icon, float x, float y, float sz)
    {
        settexture("media/interface/hud/items.png");
        float tsz = 0.25f, tx = tsz*(icon%4), ty = tsz*(icon/4);
        gle::defvertex(2);
        gle::deftexcoord0();
        gle::begin(GL_TRIANGLE_STRIP);
        gle::attribf(x,    y);    gle::attribf(tx,     ty);
        gle::attribf(x+sz, y);    gle::attribf(tx+tsz, ty);
        gle::attribf(x,    y+sz); gle::attribf(tx,     ty+tsz);
        gle::attribf(x+sz, y+sz); gle::attribf(tx+tsz, ty+tsz);
        gle::end();
    }

    void drawplayerbar( int bar, float x, float y, float w, float h, float fill=1.0f )
    {
        settexture("media/interface/hud/playerbar.png");
        float tsz = 0.150390625f, tx = 0.0f, txsz = w/1024.0f, ty = tsz*bar;
        if( fill != 1.0f ) {
            float startx = 65.0f, texstartx = startx/1024.0f;
            txsz = texstartx + ((txsz-texstartx)*fill);
            w = startx + ((w-startx)*fill);
        }
        gle::defvertex(2);
        gle::deftexcoord0();
        gle::begin(GL_TRIANGLE_STRIP);
        gle::attribf(x,    y);    gle::attribf(tx,     ty);
        gle::attribf(x+w, y);    gle::attribf(tx+txsz, ty);
        gle::attribf(x,    y+h); gle::attribf(tx,     ty+tsz);
        gle::attribf(x+w, y+h); gle::attribf(tx+txsz, ty+tsz);
        gle::end();
    }

    float abovegameplayhud(int w, int h)
    {
        switch(hudplayer()->state)
        {
            case CS_EDITING:
            case CS_SPECTATOR:
                return 1;
            default:
                return 1650.0f/1800.0f;
        }
    }

    static bool firstTime = true;

    void mainmenuhud( int w, int h ) {
        if( firstTime ) {
            sprintf( gameversion, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_REV );

            vector<char> data;
            if( httpreq::start( mastername, masterport, HTTP_GET, "news" ) ) {
                int status = httpreq::getresponse( data );
                if( status == 200 ) {
                    execute(data.getbuf());
                }
                else {
                    conoutf( "got %d while getting news", status );
                }
            }

            firstTime = false;
        }
    }

    void gameplayhud(int w, int h)
    {
        if(player1->state == CS_DEAD
        && cmode )
        {
            int wait = cmode->respawnwait( player1 );
            if(wait>=0)
            {
                static const float waitscale = 640.0f;
                static const float centerwaitx = waitscale*w/h*0.5f;
                pushhudscale(h/waitscale);
                bool flash = wait>0 && lastspawnattempt>=player1->lastpain && lastmillis < lastspawnattempt+100;
                float pw, ph;
                text_boundsf("0", pw, ph);
                draw_textf("%s%d", centerwaitx - pw*0.5f, 100, flash ? "\f3" : "", wait);
                resethudshader();
                pophudmatrix();
            }
        }

        if(player1->state==CS_SPECTATOR)
        {
            UI::showui("spectator");
            isspectator = 1;
            return;
        }
        isspectator = 0;

        pushhudscale(h/1800.0f);

        static const float centerx = 1800*w/h*0.5f;

        gameent *d = hudplayer();
        if(d->state!=CS_EDITING)
        {
            if(d->state==CS_ALIVE) {
                static const float x = centerx - ( 1024 / 2 );
                static const float y = 1800-154;
                drawplayerbar( 0, x, y, 1024, 154 );

                float healthratio = float(d->health) / float(d->maxhealth);
                drawplayerbar( 1, x, y, 959, 154, healthratio );

                static float lastratio = -0.1f;
                float chargeratio = 
                lastmillis - d->lastchaintime < CHAIN_COOLDOWN ? 1.0f :
                d->nextskill != NO_SKILL && d->skilltrigger ? min( float( lastmillis - d->aimstarttime ) / 900.0f, 1.0f ) :
                0.0f;

                drawplayerbar( 2, x, y, 959, 154, chargeratio );

                if( chargeratio != lastratio ) {
                    if( chargeratio == 1.0f ) {
                        if( m_singleplayer ) {
                            execident( "playercharge" );
                        }
                    }
                    lastratio = chargeratio;
                }

                if( lastmillis - d->lasthalfdamagetime < 1000 ) {
                    draw_text( "TOO QUICK!",  centerx + halfdamageoffsetx, 1800 + halfdamageoffsety );
                    resethudshader();
                }
            }
            if(cmode) cmode->drawhud(d, w, h);

            if( drawcenterguide ) {
                float x = 1800*w/h*0.5f-HICON_SIZE/2, y = 1800*0.5f-HICON_SIZE/2;
                drawicon( HICON_BLUE_FLAG, x, y);
            }

            if( m_singleplayer ) {
                if( centermsg[0] != '\0' ) {
                    static const float centerx2 = 1200*w/h*0.5f;
                    pushhudscale(1.5f);
                    float tw, th;
                    text_boundsf( centermsg, tw, th );
                    float posx = -( tw/2 );
                    float posy = -1000;

                    fillcolorquad( 0, 1200+posy, 2400, th, 0x202020 );

                    draw_text( centermsg,  centerx2 + posx, 1200 + posy );
                    resethudshader();
                    pophudmatrix();
                }
            }
        }

        pophudmatrix();
    }

    float clipconsole(float w, float h)
    {
        if(cmode) return cmode->clipconsole(w, h);
        return 0;
    }

    VARP(teamcrosshair, 0, 1, 1);
    VARP(hitcrosshair, 0, 425, 1000);

    const char *defaultcrosshair(int index)
    {
        switch(index)
        {
            case 2: return "media/interface/crosshair/default_hit.png";
            case 1: return "media/interface/crosshair/teammate.png";
            default: return "media/interface/crosshair/default.png";
        }
    }

    int selectcrosshair(vec &col)
    {
        return -1;
    }

    const char *mastermodecolor(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodecolors)/sizeof(mastermodecolors[0])) ? mastermodecolors[n-MM_START] : unknown;
    }

    const char *mastermodeicon(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodeicons)/sizeof(mastermodeicons[0])) ? mastermodeicons[n-MM_START] : unknown;
    }

    ICOMMAND(servinfomode, "i", (int *i), GETSERVINFOATTR(*i, 0, mode, intret(mode)));
    ICOMMAND(servinfomodename, "i", (int *i),
        GETSERVINFOATTR(*i, 0, mode,
        {
            const char *name = server::modeprettyname(mode, NULL);
            if(name) result(name);
        }));
    ICOMMAND(servinfomastermode, "i", (int *i), GETSERVINFOATTR(*i, 2, mm, intret(mm)));
    ICOMMAND(servinfomastermodename, "i", (int *i),
        GETSERVINFOATTR(*i, 2, mm,
        {
            const char *name = server::mastermodename(mm, NULL);
            if(name) stringret(newconcatstring(mastermodecolor(mm, ""), name));
        }));
    ICOMMAND(servinfotime, "ii", (int *i, int *raw),
        GETSERVINFOATTR(*i, 1, secs,
        {
            secs = clamp(secs, 0, 59*60+59);
            if(*raw) intret(secs);
            else
            {
                int mins = secs/60;
                secs %= 60;
                result(tempformatstring("%d:%02d", mins, secs));
            }
        }));
    ICOMMAND(servinfoicon, "i", (int *i),
        GETSERVINFO(*i, si,
        {
            int mm = si->attr.inrange(2) ? si->attr[2] : MM_INVALID;
            result(si->maxplayers > 0 && si->numplayers >= si->maxplayers ? "serverfull" : mastermodeicon(mm, "serverunk"));
        }));

    // any data written into this vector will get saved with the map data. Must take care to do own versioning, and endianess if applicable. Will not get called when loading maps from other games, so provide defaults.
    void writegamedata(vector<char> &extras) {}
    void readgamedata(vector<char> &extras) {}

    const char *gameconfig() { return "config/game.cfg"; }
    const char *savedconfig() { return "config/saved.cfg"; }
    const char *restoreconfig() { return "config/restore.cfg"; }
    const char *defaultconfig() { return "config/default.cfg"; }
    const char *autoexec() { return "config/autoexec.cfg"; }
    const char *savedservers() { return "config/servers.cfg"; }
    const char *editorconfig() { return "config/editor.cfg"; }

    void loadconfigs()
    {
        execfile("config/auth.cfg", false);
    }

    bool clientoption(const char *arg) { return false; }

    void soundconfig() {
        resetsound();
        int vol = 255;
        for( int i=0; i<NUMSOUNDS; ++i ) {
            registersound( const_cast< char* >( soundnames[ i ] ), &vol );
        }
    }
}

